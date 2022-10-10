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

function b = xps_block(blk,xsg_obj)
if ~isempty(blk)
    tmp = regexp(get_param(blk,'Tag'),'^xps:(\w+)','tokens');
    if isempty(tmp)
        error('Simulink block is not tagged as a XPS block');
    end
    s.simulink_name = [get_param(blk,'parent'),'/',get_param(blk,'name')];
    s.parent = get_param(blk,'parent');
	s.xsg_obj = xsg_obj;
    s.type = ['xps_',tmp{1}{1}];
	s.ip_name = {};
	s.ip_version = {};
	s.opb_address_offset = 0;
	s.plb_address_offset = 0;
	s.opb_address_align = 0;
	s.plb_address_align = 0;
    s.ports = {};
    s.ext_ports = {};
	s.misc_ports = {};
	s.parameters = {};
	s.soft_driver = {};
	s.c_params = '';
	s.mode = 0;
	s.size = 0;
	s.buses = {};
    s.interfaces = {};

    b = class(s,'xps_block');

	% add ports to the object
	gateways_blk_in = find_system(blk, 'FollowLinks','on','LookUnderMasks','all','masktype','Xilinx Gateway In');
	gateways_blk_out = find_system(blk, 'FollowLinks','on','LookUnderMasks','all','masktype','Xilinx Gateway Out');

	ports = {};

	for i=[1:length(gateways_blk_in)]
	    parent = get_param(gateways_blk_in(i),'parent');
		if isempty(find_system(parent,'SearchDepth', 1, 'FollowLinks','on','LookUnderMasks','all','masktype','Xilinx Disregard Subsystem For Generation'))
			io_name = get_param(gateways_blk_in{i},'name');
			if strcmp(get_param(gateways_blk_in{i},'arith_type'),'Boolean')
				io_bitwidth = 1;
			else
				io_bitwidth = eval_param(gateways_blk_in{i},'n_bits');
			end
			% extract the interface IO name
			parent_name = clear_name(parent{1});
			if length(clear_name(io_name)) >= 63
			    error(['The Gateway block ',io_name,' of interface block ',parent_name,' has a name longer than 63 characters, which will be truncated due to Matlab limitations']);
			end
			if ~strcmp([parent_name,'_'],io_name([1:length(parent_name)+1]))
				error(['The Gateway block ',io_name,' does not respect the hierachical naming convention (it should start with ',parent_name,').']);
			end
			if_io_name = io_name([length(parent_name)+2:length(io_name)]);
			eval(['ports.',io_name,' = {',num2str(io_bitwidth),' ''in'',''',if_io_name,'''};']);
		end
	end

	for i=[1:length(gateways_blk_out)]
	    parent = get_param(gateways_blk_out(i),'parent');
		if isempty(find_system(parent,'SearchDepth', 1, 'FollowLinks','on','LookUnderMasks','all','masktype','Xilinx Disregard Subsystem For Generation'))
			io_name = get_param(gateways_blk_out{i},'name');

            parent_name = clear_name(parent{1});
			if length(clear_name(io_name)) >= 63
			    error(['The Gateway block ',io_name,' of interface block ',parent_name,' has a name longer than 63 characters, which will be truncated due to Matlab limitations']);
			end

			% find closest convert block
			portconnectivity = get_param(gateways_blk_out{i}, 'PortConnectivity');
			convert_block = find_system(parent,'FollowLinks','on','lookundermasks','all','Handle',portconnectivity(1).SrcBlock,'masktype','Xilinx Converter Block');
			if isempty(convert_block)
			    force_block = find_system(parent,'FollowLinks','on','lookundermasks','all','Handle',portconnectivity(1).SrcBlock,'masktype','Xilinx Type Reinterpreting Block');
			    if ~isempty(force_block)
			        portconnectivity = get_param(force_block{1}, 'PortConnectivity');
			    end
			    convert_block = find_system(parent,'FollowLinks','on','lookundermasks','all','Handle',portconnectivity(1).SrcBlock,'masktype','Xilinx Converter Block');
			    if isempty(convert_block)
                    error(['The gateway : ', gateways_blk_out{i}, ' is not preceded by a Xilinx converter block as it should be.']);
				end
			end
			if strcmp(get_param(convert_block{1},'arith_type'),'Boolean')
				io_bitwidth = 1;
			else
				io_bitwidth = eval_param(convert_block{1},'n_bits');
			end

			% extract the interface IO name
			parent_name = clear_name(parent{1});
			if ~strcmp([parent_name,'_'],io_name([1:length(parent_name)+1]))
				error(['The Gateway block ',io_name,' does not respect the hierachical naming convention (it should start with ',parent_name,').']);
			end
			if_io_name = io_name([length(parent_name)+2:length(io_name)]);
			eval(['ports.',io_name,' = {',num2str(io_bitwidth),' ''out'',''',if_io_name,'''};']);
		end
	end

	if ~isempty(ports)
		b = set(b,'ports',ports);
	end

else
    s.simulink_name = '';
    s.parent = '';
	s.xsg_obj = '';
    s.type = '';
	s.ip_name = {};
	s.ip_version = {};
	s.opb_address_offset = 0;
	s.plb_address_offset = 0;
	s.opb_address_align = 0;
	s.plb_address_align = 0;
    s.ports = {};
    s.ext_ports = {};
	s.misc_ports = {};
	s.parameters = {};
	s.soft_driver = {};
	s.c_params = '';
	s.mode = 0;
	s.size = 0;
	s.buses = {};
    s.interfaces = {};
    b = class(s,'xps_block');
end