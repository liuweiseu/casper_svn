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

function b = xps_xaui(blk_obj)

if ~isa(blk_obj,'xps_block')
    error('XPS_XAUI class requires a xps_block class object');
end
if ~strcmp(get(blk_obj,'type'),'xps_xaui')
    error(['Wrong XPS block type: ',get(blk_obj,'type')]);
end

blk_name = get(blk_obj,'simulink_name');
xsg_obj = get(blk_obj,'xsg_obj');
toks = regexp(get_param(blk_name,'port'),'^(.*):(.*)$','tokens');

s.board = toks{1}{1};
s.port = toks{1}{2};
s.hw_sys = s.board;
s.preemph = get_param(blk_name, 'pre_emph');
s.swing = get_param(blk_name, 'swing');

b = class(s,'xps_xaui',blk_obj);

% ip name
b = set(b,'ip_name','XAUI_interface');

% misc ports
misc_ports.app_clk     = {1 'in' get(xsg_obj,'clk_src')};

switch s.board
	case 'CORR'
		misc_ports.mgt_clk     = {1 'in' 'bref_clk'};
	case 'iBOB'
		switch s.port
			case '0'
				misc_ports.mgt_clk     = {1 'in' 'bref_clk_top'};
			case '1'
				misc_ports.mgt_clk     = {1 'in' 'bref_clk_bottom'};
		end
	case 'BEE2_usr'
		switch s.port
			case '0'
				misc_ports.mgt_clk     = {1 'in' 'bref_clk_top'};
			case '1'
				misc_ports.mgt_clk     = {1 'in' 'bref_clk_top'};
			case '2'
				misc_ports.mgt_clk     = {1 'in' 'bref_clk_bottom'};
			case '3'
				misc_ports.mgt_clk     = {1 'in' 'bref_clk_bottom'};
		end
	case 'BEE2_ctrl'
		switch s.port
			case '0'
				misc_ports.mgt_clk     = {1 'in' 'bref_clk_top'};
			case '1'
				misc_ports.mgt_clk     = {1 'in' 'bref_clk_top'};
		end

end

b = set(b,'misc_ports',misc_ports);

%parameters
parameters.DEMUX = get_param(blk_name,'demux');
b = set(b,'parameters',parameters);
ext_ports.mgt_tx_l0_p    = {1 'out' ['XAUI',s.port,'_tx_l0_p'] [s.board, '.XAUI',s.port, '.tx_l0_p'] 'null' 'vector=false'};
ext_ports.mgt_tx_l0_n    = {1 'out' ['XAUI',s.port,'_tx_l0_n'] [s.board, '.XAUI',s.port, '.tx_l0_n'] 'null' 'vector=false'};
ext_ports.mgt_tx_l1_p    = {1 'out' ['XAUI',s.port,'_tx_l1_p'] [s.board, '.XAUI',s.port, '.tx_l1_p'] 'null' 'vector=false'};
ext_ports.mgt_tx_l1_n    = {1 'out' ['XAUI',s.port,'_tx_l1_n'] [s.board, '.XAUI',s.port, '.tx_l1_n'] 'null' 'vector=false'};
ext_ports.mgt_tx_l2_p    = {1 'out' ['XAUI',s.port,'_tx_l2_p'] [s.board, '.XAUI',s.port, '.tx_l2_p'] 'null' 'vector=false'};
ext_ports.mgt_tx_l2_n    = {1 'out' ['XAUI',s.port,'_tx_l2_n'] [s.board, '.XAUI',s.port, '.tx_l2_n'] 'null' 'vector=false'};
ext_ports.mgt_tx_l3_p    = {1 'out' ['XAUI',s.port,'_tx_l3_p'] [s.board, '.XAUI',s.port, '.tx_l3_p'] 'null' 'vector=false'};
ext_ports.mgt_tx_l3_n    = {1 'out' ['XAUI',s.port,'_tx_l3_n'] [s.board, '.XAUI',s.port, '.tx_l3_n'] 'null' 'vector=false'};
ext_ports.mgt_rx_l0_p    = {1 'in'  ['XAUI',s.port,'_rx_l0_p'] [s.board, '.XAUI',s.port, '.rx_l0_p'] 'null' 'vector=false'};
ext_ports.mgt_rx_l0_n    = {1 'in'  ['XAUI',s.port,'_rx_l0_n'] [s.board, '.XAUI',s.port, '.rx_l0_n'] 'null' 'vector=false'};
ext_ports.mgt_rx_l1_p    = {1 'in'  ['XAUI',s.port,'_rx_l1_p'] [s.board, '.XAUI',s.port, '.rx_l1_p'] 'null' 'vector=false'};
ext_ports.mgt_rx_l1_n    = {1 'in'  ['XAUI',s.port,'_rx_l1_n'] [s.board, '.XAUI',s.port, '.rx_l1_n'] 'null' 'vector=false'};
ext_ports.mgt_rx_l2_p    = {1 'in'  ['XAUI',s.port,'_rx_l2_p'] [s.board, '.XAUI',s.port, '.rx_l2_p'] 'null' 'vector=false'};
ext_ports.mgt_rx_l2_n    = {1 'in'  ['XAUI',s.port,'_rx_l2_n'] [s.board, '.XAUI',s.port, '.rx_l2_n'] 'null' 'vector=false'};
ext_ports.mgt_rx_l3_p    = {1 'in'  ['XAUI',s.port,'_rx_l3_p'] [s.board, '.XAUI',s.port, '.rx_l3_p'] 'null' 'vector=false'};
ext_ports.mgt_rx_l3_n    = {1 'in'  ['XAUI',s.port,'_rx_l3_n'] [s.board, '.XAUI',s.port, '.rx_l3_n'] 'null' 'vector=false'};

b = set(b,'ext_ports',ext_ports);
