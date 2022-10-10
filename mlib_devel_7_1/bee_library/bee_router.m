%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%   Center for Astronomy Signal Processing and Electronics Research           %
%   http://seti.ssl.berkeley.edu/casper/                                      %
%   Copyright (C) 2006 University of California, Berkeley                     %
%                                                                             %
%   This program is free software; you can redistribute it and/or modify      %
%   it under the terms of the GNU General Public License as published by      %
%   the Free Software Foundation; either version 2 of the License, or         %
%   (at your option) any later version.                                       %
%                                                                             %
%   This program is distributed in the hope that it will be useful,           %
%   but WITHOUT ANY WARRANTY; without even the implied warranty of            %
%   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             %
%   GNU General Public License for more details.                              %
%                                                                             %
%   You should have received a copy of the GNU General Public License along   %
%   with this program; if not, write to the Free Software Foundation, Inc.,   %
%   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.               %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function bee_router(root)
%BEE_ROUTER     BEE Board Level Router
%
%BEE_ROUTER(top_sys)    routes the top-level Simulink system, top_sys
%               The Simulink model need to have have top level subsystems
%               tagged for actual FPGA name, and Xilinx Gateways one level
%               below the top-level.


%open_system(root);
load bee_routes.mat;
load bee2_routes.mat;
root = bdroot(root);
xsg_ver = xlversion;
xsg_ver = strtok(xsg_ver{1},' ');
switch xsg_ver
    case '3.1sp1'
        gw_in_name = 'm2xfix';
        gw_out_name = 'xfix2m';
    case {'6.1.1' '6.2.R14' '6.2' '6.3' '6.3.p03'}
        gw_in_name = 'xlgatewayin';
        gw_out_name = 'xlgatewayout';
    otherwise
        error(['Current Xilinx System Generator version (',xsg_ver,') not supported by BEE_router']);
end
    

eval(sprintf('%s([],[],[],%ccompile%c);',root,char(39),char(39)));
chips =find_system(root, 'SearchDepth',1,'FollowLinks','on','LookUnderMasks','all','BlockType','SubSystem');
totalchips = length(chips);
gw_todo_list = [];
for i = 1:totalchips
    if strcmp(get_param(chips{i},'tag'),'simonly')
        continue;
    end
    disp(['enter subsystem: ',chips{i,1}]);
    [root_sys,root_dir,glb_ce,glb_clr,clk_period] = get_xsg_info(chips{i,1});
    if ~strcmp(root_sys,chips{i,1})
        continue;
    end
    external_dst= 0;
    src = '';
    dst = '';
    dst_gw = -1;
    disp(sprintf('\nRouting %s',chips{i,1}));
    src = get_param(chips{i,1},'Tag');
    gw_outs = find_system(chips{i,1},'FollowLinks','on','LookUnderMasks','all','FunctionName',gw_out_name,'locs_specified','off');
    for j = 1:length(gw_outs);
        gw_out = gw_outs(j);
        gw_out = gw_out{1};
        gw_str_width = length(gw_out);
        if strcmp(gw_out(gw_str_width-1:gw_str_width),'_n')
            continue;
        end
        gw_out_tag = get_param(gw_out,'Tag');
        dst = '';
        iostandard = '';
        if ~strcmp(gw_out_tag,'')
           [dst,external_dst,iostandard] = get_dst_from_tag(gw_out_tag);
           if strcmp(dst,'')
               disp(sprintf('Warning: invalid tage value "%s" at %s, routing on the gateway skipped.',gw_out_tag,gw_out));
               continue;
           end
        else
           [dst,external_dst,dst_gw] = get_dst_by_trace(gw_out, gw_in_name);
        end
        if strcmp(dst,'')
           disp(sprintf('Warning: cannot resolve destination of %s, routing on the gateway skipped.',gw_out));
           continue;
        else
           route_link(0,external_dst,iostandard,src,gw_out,dst,dst_gw,root_sys,xsg_ver);
        end
    end

    dst = get_param(chips{i,1},'Tag');
    gw_ins = find_system(chips{i,1},'FollowLinks','on','LookUnderMasks','all','FunctionName',gw_in_name,'locs_specified','off');
    for j = 1:length(gw_ins);
        src = '';
        iostandard = '';
        gw_in = gw_ins(j);
        gw_in = gw_in{1};
        if exist_todo_list(gw_todo_list, gw_in)
            continue;
        end
        gw_in_tag = get_param(gw_in,'Tag');
        if ~strcmp(gw_in_tag,'')
           [src,external_src,iostandard] = get_dst_from_tag(gw_in_tag);
           if strcmp(src,'')
               disp(sprintf('Warning: invalid tage value "%s" at %s, routing on the gateway skipped.',gw_in_tag,gw_in));
           end
        end
        if strcmp(src,'')
           disp(sprintf('Warning: cannot resolve destination of %s, routing on the gateway skipped.',gw_in));
        else
           route_link(external_src,0,iostandard,src,-1,dst,gw_in,root_sys,xsg_ver);
        end
    end
end
eval(sprintf('%s([],[],[],%cterm%c);',root,char(39),char(39)));




for i = 1:2:length(gw_todo_list)-1
    cur_gw = gw_todo_list{i};
    pin_list = gw_todo_list{i+1};
    set_param(cur_gw,'LOCs',pin_list,'locs_specified','on');
end

disp(sprintf('\nBEE routing complete.'));
return;

function [dst,external_dst,iostandard] = get_dst_from_tag(tag);
iostandard = '';
dst = '';
external_dst = 0;
if ~isempty(sscanf(tag,'fpga%d'))
    dst = tag;
elseif ~isempty(sscanf(tag,'xbar%d'))
    dst = tag;
elseif strcmp(tag,'ctrlfpga')
    dst = tag;
elseif length(sscanf(tag,'conn%d_bus%d')) == 2
    [dst,dummy] = strtok(tag,':');
    external_dst = 1;
    iostandard = dummy(2:length(dummy));
elseif strcmp(tag,'led') | strcmp(tag,'rst')
    dst = tag;
    external_dst = 1;
else
    disp(sprintf('Error: unknown tag value %s',tag));
end
    
function [dst,external_dst,dst_gw] = get_dst_by_trace(gw_out,gw_in_name);
dst_gw = -1;
dst = '';
external_dst = 0;
gw_out_port = get_blk_outports(gw_out);
out_port = get_dst_blk_handle(gw_out_port(1));
if out_port ~= -1 & strcmp(get_param(out_port,'BlockType'), 'Outport')
    parent_outport = get_parent_port_handle(out_port);
    dst_syses = get_dst_blk_handle(parent_outport);
    dst_prt_ports =  get_dst_port_handle(parent_outport);
    if dst_syses == -1
        disp(sprintf('Warning: cannot trace destination of %s, no connection found.',gw_out));
        dst = '';
        return;
    end
    found_sys = 0;
    for i = 1:length(dst_syses)
        sys_tag = get_param(dst_syses(i),'Tag');
        if ~strcmp(sys_tag,'')
            [dst,external_dst,iostandard] = get_dst_from_tag(sys_tag);
            if ~strcmp(dst,'')
                dst_prt_port = dst_prt_ports(i);
                found_sys = 1;
                break;
            end
        end
    end
    if ~found_sys
        dst = '';
        disp(sprintf('Warning: cannot trace destination of %s, no valid destination found.',gw_out));
        return;
    elseif ~external_dst
        dst_port = get_child_port_handle(dst_prt_port);
        in_port = get_blk_outports(dst_port);
        gw_in = get_dst_blk_handle(in_port(1));
        if gw_in == -1
            disp(sprintf('Error tracing output gateway %s to %s, due to destination subsystem child port is not connected.', gw_in, dst));
            return    
        end
        if ~strcmp(get_param(gw_in,'FunctionName'),gw_in_name)
            disp(sprintf('Error tracing output gateway %s to %s, due to destination subsystem child port is not connected to a Xilinx Gateway In.', gw_in, dst));
            return    
        end
        dst_gw = gw_in;
    end
else
    disp(sprintf('Warning: cannot trace destination of %s, output of the gateway is not connected.',gw_out));
    return;
end




function result = exist_todo_list(routed_gws,gw)
for i = 1:2:length(routed_gws)-1
    if strcmp(gw,routed_gws(i))
        result = 1;
        return;
    end
end
result = 0;
return;

function route_link(external_src,external_dst,iostandard,src,src_gw, dst,dst_gw,root_sys,xsg_ver)

if ~evalin('caller',['exist(',char(39),src,'_',dst,char(39),')']) | ~evalin('caller',['exist(',char(39),dst,'_',src,char(39),')'])
    disp(sprintf('Error routing gateway %s(%s) to %s(%s), no physical connection exist.', src_gw, src, dst_gw,dst));
    return
end
sb_src = evalin('caller',sprintf('%s_%s_sb(1);',src,dst));
sb_dst = evalin('caller',sprintf('%s_%s_sb(1);',dst,src));
route_width = evalin('caller',sprintf('%s_%s_sb(2);',src,dst));;

if external_src & external_dst
    disp(sprintf('Error routing gateway %s(%s) to %s(%s), both source and destination are external.', src_gw, src, dst_gw,dst));
    return
end

if strcmp(src,'') | strcmp(dst,'')
    disp(sprintf('Error routing gateway %s(%s) to %s(%s), source/destination is not specified.', src_gw, src, dst_gw,dst));
    return
end

if (~external_src & src_gw == -1) | (~external_dst & dst_gw == -1)
    disp(sprintf('Error routing gateway %s(%s) to %s(%s), internal source/destination gateway is not specified.', src_gw, src, dst_gw,dst));
    return
end

if src_gw ~= -1
    tmp = get_blk_inports(src_gw);
    inport = tmp(1);
    bit_width = get_xwidth(get_param(inport,'CompiledPortDataType'));
elseif dst_gw ~= -1
    tmp = get_blk_outports(dst_gw);
    outport = tmp(1);
    bit_width = get_xwidth(get_param(outport,'CompiledPortDataType'));
end

use_lvds = 0;
if isempty(iostandard)
elseif strcmp(iostandard,'lvds')
    bit_width = bit_width * 2;
    use_lvds = 1;
else
    disp(sprintf('Warning: unkown external I/O standard: %s',iostandard));
end

if xsg_ver == 1 & ~use_lvds
    bit_width = bit_width + 1;
end

if bit_width > route_width-sb_src+1 | bit_width > route_width-sb_dst+1
    disp(sprintf('Error routing output gateway %s(%s) to %s(%s), due to bit width %d exceeds available channels %d.', src_gw, src, dst_gw,dst, bit_width,route_width-sb_src+1));
    return    
end
if ~external_src & external_dst & ~use_lvds
    evalin('caller', ['gw_todo_list = [gw_todo_list, {',char(39),get_param(src_gw,'Parent'),'/',get_param(src_gw,'Name'),char(39),' ',char(39),sprintf('%s_%s(%d:%d)',src,dst,sb_src,sb_src+bit_width-1),char(39),'}];']);
    evalin('caller',sprintf('%s_%s_sb(1) = %d;',src,dst, sb_src+bit_width));
    evalin('caller',sprintf('%s_%s_sb(1) = %d;',dst,src, sb_dst+bit_width));
    disp(sprintf('Successfully routed link from %s(%s) to %s',[get_param(src_gw,'Parent'),'/',get_param(src_gw,'Name')],src,dst));
elseif ~external_src & ~external_dst & ~use_lvds
    evalin('caller', ['gw_todo_list = [gw_todo_list, {',char(39),get_param(src_gw,'Parent'),'/',get_param(src_gw,'Name'),char(39),' ',char(39),sprintf('%s_%s(%d:%d)',src,dst,sb_src,sb_src+bit_width-1),char(39),'}];']);
    evalin('caller', ['gw_todo_list = [gw_todo_list, {',char(39),get_param(dst_gw,'Parent'),'/',get_param(dst_gw,'Name'),char(39),' ',char(39),sprintf('%s_%s(%d:%d)',dst,src,sb_dst,sb_dst+bit_width-1),char(39),'}];']);
    evalin('caller',sprintf('%s_%s_sb(1) = %d;',src,dst, sb_src+bit_width));
    evalin('caller',sprintf('%s_%s_sb(1) = %d;',dst,src, sb_dst+bit_width));
    disp(sprintf('Successfully routed link from %s(%s) to %s(%s)',[get_param(src_gw,'Parent'),'/',get_param(src_gw,'Name')],src,[get_param(dst_gw,'Parent'),'/',get_param(dst_gw,'Name')],dst));
elseif external_src & ~external_dst & ~use_lvds
    evalin('caller', ['gw_todo_list = [gw_todo_list, {',char(39),get_param(dst_gw,'Parent'),'/',get_param(dst_gw,'Name'),char(39),' ',char(39),sprintf('%s_%s(%d:%d)',dst,src,sb_dst,sb_dst+bit_width-1),char(39),'}];']);
    evalin('caller',sprintf('%s_%s_sb(1) = %d;',src,dst, sb_src+bit_width));
    evalin('caller',sprintf('%s_%s_sb(1) = %d;',dst,src, sb_dst+bit_width));
    disp(sprintf('Successfully routed link from %s to %s(%s)',src,[get_param(dst_gw,'Parent'),'/',get_param(dst_gw,'Name')],dst));
elseif ~external_src & external_dst & use_lvds
    evalin('caller', ['gw_todo_list = [gw_todo_list, {',char(39),get_param(src_gw,'Parent'),'/',get_param(src_gw,'Name'),char(39),' ',char(39),sprintf('%s_%s(%d:2:%d)',src,dst,sb_src,sb_src+bit_width-2),char(39),'}];']);
    evalin('caller', ['gw_todo_list = [gw_todo_list, {',char(39),get_param(src_gw,'Parent'),'/',get_param(src_gw,'Name'),'_n',char(39),' ',char(39),sprintf('%s_%s(%d:2:%d)',src,dst,sb_src+1,sb_src+bit_width-1),char(39),'}];']);
    evalin('caller',sprintf('%s_%s_sb(1) = %d;',src,dst, sb_src+bit_width));
    evalin('caller',sprintf('%s_%s_sb(1) = %d;',dst,src, sb_dst+bit_width));
    net_name = clear_name(strrep([get_param(src_gw,'Parent'),'/',get_param(src_gw,'Name')],[root_sys,'/'],''));
    disp(sprintf('Successfully routed link from %s(%s) to %s',[get_param(src_gw,'Parent'),'/',get_param(src_gw,'Name')],src,dst));
elseif external_src & ~external_dst & use_lvds
    evalin('caller', ['gw_todo_list = [gw_todo_list, {',char(39),get_param(dst_gw,'Parent'),'/',get_param(dst_gw,'Name'),char(39),' ',char(39),sprintf('%s_%s(%d:2:%d)',dst,src,sb_dst,sb_dst+bit_width-2),char(39),'}];']);
    evalin('caller',sprintf('%s_%s_sb(1) = %d;',src,dst, sb_src+bit_width));
    evalin('caller',sprintf('%s_%s_sb(1) = %d;',dst,src, sb_dst+bit_width));
    net_name = clear_name(strrep([get_param(dst_gw,'Parent'),'/',get_param(dst_gw,'Name')],[root_sys,'/'],''));
    disp(sprintf('Successfully routed link from %s to %s(%s)',src,[get_param(dst_gw,'Parent'),'/',get_param(dst_gw,'Name')],dst));
end
return;





