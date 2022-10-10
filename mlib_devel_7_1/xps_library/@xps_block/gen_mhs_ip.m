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

function [str,opb_addr_end,plb_addr_end] = gen_mhs_ip(blk_obj,opb_addr_start,plb_addr_start,plb_name,opb_name)
str = '';
opb_addr_end = opb_addr_start;
plb_addr_end = plb_addr_start;

try
	ip_name = blk_obj.ip_name;
catch
	ip_name = '';
end

try
	ip_version = blk_obj.ip_version;
catch
	ip_version = '';
end

try
	ports = blk_obj.ports;
catch
	ports = {};
end

try
	interfaces = blk_obj.interfaces;
catch
	interfaces = {};
end

try
	ext_ports = blk_obj.ext_ports;
catch
	ext_ports = {};
end

try
	misc_ports = blk_obj.misc_ports;
catch
	misc_ports = {};
end

try
	parameters = blk_obj.parameters;
catch
	parameters = {};
end

try
	range_opb = blk_obj.opb_address_offset;
catch
	range_opb = 0;
end
try
	range_plb = blk_obj.plb_address_offset;
catch
	range_plb = 0;
end

try
    align_opb = blk_obj.opb_address_align;
catch
    align_opb = 0;
end

try
    align_plb = blk_obj.plb_address_align;
catch
    align_plb = 0;
end

if ~isempty(ip_name)
	str = [str, 'BEGIN ', get(blk_obj, 'ip_name'),'\n'];
	str = [str, ' PARAMETER INSTANCE = ',clear_name(get(blk_obj,'simulink_name')),'\n'];
    if isempty(ip_version)
    	str = [str, ' PARAMETER HW_VER = 1.00.a\n'];
    else
    	str = [str, ' PARAMETER HW_VER = ',ip_version,'\n'];
    end

	if ~isempty(parameters)
		prop_names = fieldnames(parameters);

		for j = 1:length(prop_names)
		    cur_prop = getfield(parameters,prop_names{j});
		    str = [str, ' PARAMETER ',prop_names{j},' = ',cur_prop,'\n'];
		end
	end

	if range_opb ~= 0
		if range_plb ~= 0
			error('The default gen_mhs_ip does not support multiple busses attachments. You should write your own gen_mhs_ip for this interface.');
		end

        if align_opb ~= 0
    		opb_addr_start = ceil(opb_addr_start/align_opb) * align_opb;
    	end

	    opb_addr_end = opb_addr_start + range_opb;

	    str = [str, ' PARAMETER C_BASEADDR = 0x',dec2hex(opb_addr_start),'\n'];
	    str = [str, ' PARAMETER C_HIGHADDR = 0x',dec2hex(opb_addr_end-1),'\n'];
	    str = [str, ' BUS_INTERFACE SOPB = ',opb_name,'\n'];
	    str = [str, ' PORT OPB_Clk = sys_clk\n'];
	end

	if range_plb ~= 0
		if range_opb ~= 0
			error('The default gen_mhs_ip does not support multiple busses attachments. You should write your own gen_mhs_ip for this interface.');
		end

		if align_plb ~= 0
    		plb_addr_start = ceil(plb_addr_start/align_plb) * align_plb;
    	end

	    plb_addr_end = plb_addr_start + range_plb;

	    str = [str, ' PARAMETER C_BASEADDR = 0x',dec2hex(plb_addr_start),'\n'];
	    str = [str, ' PARAMETER C_HIGHADDR = 0x',dec2hex(plb_addr_end-1),'\n'];
	    str = [str, ' BUS_INTERFACE SPLB = ',plb_name,'\n'];
	    str = [str, ' PORT PLB_Clk = sys_clk\n'];
    end

	if ~isempty(interfaces)
		interfaces_names = fieldnames(interfaces);

		for j = 1:length(interfaces_names)
		    cur_interface = getfield(interfaces,interfaces_names{j});
		    str = [str, ' BUS_INTERFACE ',interfaces_names{j},' = ',cur_interface,'\n'];
		end
    end

    ports = blk_obj.ports;
	if ~isempty(ports)
		port_names = fieldnames(ports);

		for j = 1:length(port_names)
		    cur_port = getfield(ports,port_names{j});
		    str = [str, ' PORT ',cur_port{3},' = ',port_names{j},'\n'];
		end
	end


	if ~isempty(ext_ports)
		ext_port_names = fieldnames(ext_ports);

		for j = 1:length(ext_port_names)
		    cur_ext_port = getfield(ext_ports,ext_port_names{j});
		    if cur_ext_port{1} == 1
		        str = [str, ' PORT ',ext_port_names{j},' = ',cur_ext_port{3},'\n'];
		    else
		        str = [str, ' PORT ',ext_port_names{j},' = ',cur_ext_port{3},'\n'];
		    end
		end
	end

	if ~isempty(misc_ports)
		misc_port_names = fieldnames(misc_ports);

		for j = 1:length(misc_port_names)
		    cur_misc_port = getfield(misc_ports,misc_port_names{j});
		    if cur_misc_port{1} == 1
		        str = [str, ' PORT ',misc_port_names{j},' = ',cur_misc_port{3},'\n'];
		    else
		        str = [str, ' PORT ',misc_port_names{j},' = ',cur_misc_port{3},'\n'];
		    end
		end
	end

	str = [str, 'END\n'];
end

if ~isempty(ext_ports)
	ext_port_names = fieldnames(ext_ports);

	for j = 1:length(ext_port_names)
	    cur_ext_port = getfield(ext_ports,ext_port_names{j});
	    if cur_ext_port{1} == 1
            try
                if strcmp(cur_ext_port{6}, 'vector=true')
                    str = [str, 'PORT ',cur_ext_port{3},' = ',cur_ext_port{3},', DIR = ',cur_ext_port{2},', VEC = [0:0]\n'];
                else
                    str = [str, 'PORT ',cur_ext_port{3},' = ',cur_ext_port{3},', DIR = ',cur_ext_port{2},'\n'];
                end
            catch
                str = [str, 'PORT ',cur_ext_port{3},' = ',cur_ext_port{3},', DIR = ',cur_ext_port{2},'\n'];
            end
	    else
	        str = [str, 'PORT ',cur_ext_port{3},' = ',cur_ext_port{3},', DIR = ',cur_ext_port{2},', VEC = [',num2str(cur_ext_port{1}-1),':0]\n'];
	    end
	end
end

