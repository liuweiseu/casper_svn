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

function [dimension, width,dec_pt,sig_type] = trace_xwidth(port_hdl)
dimension = get_param(port_hdl,'CompiledPortWidth');
[port_width,port_dec_pt,port_sig_type] = get_xwidth(get_param(port_hdl,'CompiledPortDataType'));
if  port_width ~= -1
    width = port_width*ones(1,dimension);
    dec_pt = port_dec_pt*ones(1,dimension);
    sig_type = cell(1,dimension);
    for i=1:dimension
        sig_type{i} = port_sig_type;
    end
    return;
else
    width = [];
    dec_pt = [];
    sig_type = {};
end

src_blk = get_src_blk_handle(port_hdl);
if src_blk == -1
    error(['Unconnected port: ',get_param(port_hdl,'parent'),'/',get_param(port_hdl,'Name')]);
    return;
end
switch get_param(src_blk,'BlockType')
    case 'Mux'
        inports = get_blk_inports(src_blk);
        for i=1:length(inports)
            inport = inports(i);
            [port_dim, port_width, port_dec_pt, port_sig_type] = trace_xwidth(inport);
            width = [width,port_width];
            dec_pt = [dec_pt, port_dec_pt];
            sig_type = [sig_type, port_sig_type];
        end
    case 'Demux'
        inports = get_blk_inports(src_blk);
        inport = inports(1);
        [port_dim, port_width, port_dec_pt, port_sig_type] = trace_xwidth(inport);
        port_num = get_param(get_src_port_handle(port_hdl),'PortNumber');
        num_outputs = get_blk_wsvar(src_blk,'Outputs');
        if port_num == 1
            width = port_width(1:dimension);
            dec_pt = port_dec_pt(1:dimension);
            sig_type = port_sig_type(1:dimension);
        else
            cur_index = port_dim - num_outports + port_num;
            width = port_width(cur_index);
            dec_pt = port_dec_pt(cur_index);
            sig_type = port_sig_type(cur_index);
        end
    case 'Selector'
        if ~strcmp(get_param(src_blk,'InputType'),'Vector')
            error('Cannot resolve Selector block, unless the input type is a vector.');
        end
        if ~strcmp(get_param(src_blk,'ElementSrc'),'Internal')
            error('Cannot resolve Selector block, unless the element source is internal.');
        end
        num_inputs = get_blk_wsvar(src_blk,'InputPortWidth');
        elements = get_blk_wsvar(src_blk,'Elements');
        if elements == -1 %all elements
            elements = [1:num_inputs];
        end
        inports = get_blk_inports(src_blk);
        inport = inports(1);
        [port_dim, port_width, port_dec_pt, port_sig_type] = trace_xwidth(inport);
        width = port_width(elements);
        dec_pt = port_dec_pt(elements);
        sig_type = port_sig_type(elements);
    case {'Inport' 'Enport'}
        [dimension,width,dec_pt, sig_type] = trace_xwidth(get_parent_port_handle(src_blk));
    case 'SubSystem'
        [dimension,width,dec_pt, sig_type] = trace_xwidth(get_blk_inports(get_child_port_handle(get_src_port_handle(port_hdl))));
    otherwise
        error(['Unknown block type: ',get_param(src_blk,'BlockType')]);
end
        
        