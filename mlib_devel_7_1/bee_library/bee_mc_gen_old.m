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

function bee_mc_gen_old(sys,xsg_ver,gate_clk)
%BEE_MC_GEN_OLD generates the necessary SHELL script to run the MC scripts.
%  bee_mc_gen_old(sys_name, xsg_ver, gate_clk)    'sys_name' is the Simulink system name.
%  'xsg_ver' is an optional flag, the valid values are 1 or 2 indicating
%  Xilinx System Generator Version V2.1 or V2.2/V2.3. When not specified,
%  V2.2/V2.3 is assumed.
%  'gate_clk' is an optional flag, the valid values are 0 or 1 indicating
%  weather to use clock gating in MC. Default value is 0. When used,
%  'xsg_ver' flag is required.

if nargin == 1
    disp('Warning: Xilinx System Generator version not specified, assuming V2.2/V2.3');
    xsg_ver = 2;
else
    if xsg_ver == 2 | xsg_ver == 1
    else
        error(['Error: unknown version of Xilinx System Generator: ',xsg_ver]);
    end
end

if nargin < 3
    gate_clk = 0;
end

bee_df_path = strrep(getenv('BEE_DF_PATH'),'\','/');
if exist(bee_df_path,'dir')
    tcl_lib_path = [bee_df_path,'/lib/tcl'];
else
    error('System environment variable BEE_DF_PATH need to be properly defined to point to the root directory of the BEE Design Flow installation');
end

if exist([bee_df_path,'/conf/env.mat'])
    eval(['load ',bee_df_path,'/conf/env.mat;']);
else
    tcl_cmd = 'tclsh83';
end

[root,root_dir,glb_ce,glb_clr,clk_period] = get_xsg_info(sys);
[glb_root,R] = strtok(root,'/');

eval([glb_root,'([],[],[],',char(39),'compile',char(39),');']);

if exist([root_dir,'/MC']) ~= 7
    mkdir(root_dir,'MC');
end
vhdl_fid = fopen([root_dir,'/MC/core_vhdl.list'],'w');
syses = find_system(sys,'FollowLinks', 'on','LookUnderMasks','all','BlockType','SubSystem');
syses = [{root};syses];
for k = 1:length(syses)
    sys = syses{k};
    xblks = find_system(sys,'SearchDepth',1,'FollowLinks', 'on','LookUnderMasks','all','RegExp','on','FunctionName','^xl','gen_core','on');
    if length(find_system(sys,'SearchDepth',1,'FollowLinks', 'on','LookUnderMasks','all','FunctionName','xl_BlackBox')) ~= 0
        continue;
    end
    for i=1:length(xblks)
        xblk = xblks(i);
        blk = xblk{1,1};
        try
            if strcmp(get_param(blk,'use_core'),'off')
                continue;
            end
        catch
        end
        blk_name = get_param(blk,'Name');
        blk_type = get_param(blk,'FunctionName');
        switch blk_type
            case {'xlmux' 'xllogical' 'xlddsreg' 'xlddsscmodereg'}
                ports = get_param(blk,'inputs');
                mc_args = ['num_inputs=',ports,','];
            case 'xlfir'
                chs = get_param(blk,'num_channels');
                mc_args = ['num_channels=',chs,','];
                coef = get_blk_wsvar(blk,'coef');
                n_bits = get_blk_wsvar(blk,'coef_n_bits');
                bin_pt = get_blk_wsvar(blk,'coef_bin_pt');
                for i = 1:length(coef)                
                    mc_args = [mc_args, 'c',num2str(i-1),'=',fix2string(coef(i),n_bits,bin_pt,1,1),','];
                end
            case 'xlcounter'
                if xsg_ver == 1 & strcmp(get_param(blk,'reset_pin'),'on')
                    blk_type = 'xlcounter_rst';
                elseif xsg_ver == 2
                    switch get_param(blk,'cnt_type')
                        case 'Free Running'
                            blk_type = 'xlcounter_free';
                        case 'Count Limited'
                            blk_type = 'xlcounter_limit';
                    end
                end
                n_bits = get_param(blk,'n_bits');
                cnt_by_val = get_param(blk,'cnt_by_val');
                start_count = get_param(blk,'start_count');
                mc_args = ['width=',n_bits,',count_by=',cnt_by_val,',ainit_val=',start_count,',sinit_val=',start_count,','];
            case 'xlmult' 
                user_latency = get_blk_wsvar(blk,'latency');
                if strcmp(get_param(blk,'pipeline'),'on')
                    inports = get_blk_inports(blk);
                    min_width = min(get_port_bit_width(inports(1)),get_port_bit_width(inports(2)));
                    if min_width < 3
                        latency = 0;
                    elseif min_width < 5
                        latency = 1;
                    elseif min_width < 9
                        latency = 2;
                    elseif min_width < 17
                        latency = 3;
                    elseif min_width < 34
                        latency = 4;
                    elseif min_width < 65
                        latency = 5;
                    else
                        error(['Invalid bit_width: ',num2str(min_width)]);
                    end
                else
                    latency = '0';
                end
                mc_args = ['latency=',num2str(min(latency+2,user_latency)),','];
            otherwise
                mc_args = '';
        end
            

        tmp_index = findstr(blk,blk_name);
        tmp_index = tmp_index(length(tmp_index));
        parent_file = clear_name(blk(1:tmp_index-1));
        parent_file = [parent_file,'.vhd'];
        [fid,msg] = fopen([root_dir,'/',parent_file],'r');
        if fid == -1
            disp(sprintf('Warning: cannot open VHDL file %s',[root_dir,'/',parent_file]));
            continue;
        end
        core_name = '';
        blk_name = clear_name(blk_name);
        while 1
            line = fgetl(fid);
            if ~ischar(line)
                break;
            else
                [T,R] = strtok(line);
                if strcmpi(T,[blk_name,':'])
                    [core_name,R] = strtok(R);
                    break;
                end
            end
        end
        fclose(fid);
        if strcmp(core_name,'')
            disp(sprintf('Warning: core component "%s" not found in VHDL file "%s"',blk_name,parent_file));
            continue;
        end

        vhdl_file_name = [root_dir,'/',core_name,'.vhd'];
        fprintf(vhdl_fid,'%s\t%s\t%s\n',vhdl_file_name,blk_type,mc_args);    
    end
end
fclose(vhdl_fid);
eval([glb_root,'([],[],[],',char(39),'term',char(39),');']);
disp('Core VHDL file list generation complete.');
% dos(sprintf('tclsh83 c:/home/chenzh/simulink/lib/gen_mc_script.tcl %s %s %s',[root_dir,'/mc/core_vhdl.list'],[root_dir,'/mc'],num2str(gate_clk)));
dos([tcl_cmd,' ',tcl_lib_path,'/gen_mc_script.tcl ',root_dir,'/mc/core_vhdl.list',' ',root_dir,'/mc',' ',num2str(gate_clk)]);


