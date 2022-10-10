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

function gateway_naming_mask()
prefix = clear_name(gcb);
gw_in_name = 'xlgatewayin';
gw_out_name = 'xlgatewayout';
gw_outs = find_system(gcb,'FollowLinks','on','LookUnderMasks','all','FunctionName',gw_out_name);
for j = 1:length(gw_outs);
    gw_out = gw_outs(j);
    gw_out = gw_out{1};
	dst = get_outport(gw_out);
    if ~isempty(dst);
        set_param(gw_out,'Name',[prefix,'_',get_param(dst,'Name')]);
    end
end

gw_ins = find_system(gcb,'FollowLinks','on','LookUnderMasks','all','FunctionName',gw_in_name);
for j = 1:length(gw_ins);
    gw_in = gw_ins(j);
    gw_in = gw_in{1};
    src = get_inport(gw_in);
    if ~isempty(src);
        set_param(gw_in,'Name',[prefix,'_',get_param(src,'Name')]);
    end
end


function dst = get_outport(gw_out)
dst = '';
gw_out_port = get_blk_outports(gw_out);
out_port = get_dst_blk_handle(gw_out_port(1));
if out_port ~= -1 & strcmp(get_param(out_port,'BlockType'), 'Outport')
    dst = out_port;
end

function src = get_inport(gw_in);
src = '';
gw_in_port = get_blk_inports(gw_in);
in_port = get_src_blk_handle(gw_in_port(1));
if in_port ~= -1 & strcmp(get_param(in_port,'BlockType'), 'Inport')
    src = in_port;
end