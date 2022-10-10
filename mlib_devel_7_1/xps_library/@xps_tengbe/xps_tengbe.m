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

function b = xps_tengbe(blk_obj)

if ~isa(blk_obj,'xps_block')
    error('XPS_TENGBE class requires a xps_block class object');
end
if ~strcmp(get(blk_obj,'type'),'xps_tengbe')
    error(['Wrong XPS block type: ',get(blk_obj,'type')]);
end

blk_name = get(blk_obj,'simulink_name');
xsg_obj = get(blk_obj,'xsg_obj');
toks = regexp(get_param(blk_name,'port'),'^(.*):(.*)$','tokens');

s.board = toks{1}{1};
s.port = toks{1}{2};
s.hw_sys = s.board;
s.mac_lite = get_param(blk_name, 'mac_lite');
s.preemph = get_param(blk_name, 'pre_emph');
s.swing = get_param(blk_name, 'swing');

b = class(s,'xps_tengbe',blk_obj);

% ip name & version
b = set(b,'ip_name','ten_gb_eth');

switch s.board
    case 'BEE2_ctrl'
        b = set(b,'ip_version','2.00.a');
    case 'BEE2_usr'
        b = set(b,'ip_version','2.00.a');
    case 'iBOB'
        b = set(b,'ip_version','2.00.b');
    otherwise
        error(['10GbE not supported for platform ', s.board]);
end

% bus offset
b = set(b,'plb_address_offset',16384);
b = set(b,'plb_address_align', hex2dec('4000'));

% misc ports
misc_ports.clk     = {1 'in' get(xsg_obj,'clk_src')};
b = set(b,'misc_ports',misc_ports);

% parameters
parameters.CONNECTOR = s.port;
parameters.SWING = s.swing;
parameters.PREEMPHASYS = s.preemph;

if strcmp(s.mac_lite, 'on')
    parameters.USE_XILINX_MAC = '0';
    parameters.USE_UCB_MAC = '1';
else
    parameters.USE_XILINX_MAC = '1';
    parameters.USE_UCB_MAC = '0';
end

b = set(b,'parameters',parameters);

% bus interfaces
interfaces.XAUI_SYS = ['xaui_sys',s.port];
b = set(b,'interfaces',interfaces);

% external ports
ext_ports.mgt_tx_l0_p    = {1 'out' ['XAUI',s.port,'_tx_l0_p'] 'null' 'null' 'vector=false'};
ext_ports.mgt_tx_l0_n    = {1 'out' ['XAUI',s.port,'_tx_l0_n'] 'null' 'null' 'vector=false'};
ext_ports.mgt_tx_l1_p    = {1 'out' ['XAUI',s.port,'_tx_l1_p'] 'null' 'null' 'vector=false'};
ext_ports.mgt_tx_l1_n    = {1 'out' ['XAUI',s.port,'_tx_l1_n'] 'null' 'null' 'vector=false'};
ext_ports.mgt_tx_l2_p    = {1 'out' ['XAUI',s.port,'_tx_l2_p'] 'null' 'null' 'vector=false'};
ext_ports.mgt_tx_l2_n    = {1 'out' ['XAUI',s.port,'_tx_l2_n'] 'null' 'null' 'vector=false'};
ext_ports.mgt_tx_l3_p    = {1 'out' ['XAUI',s.port,'_tx_l3_p'] 'null' 'null' 'vector=false'};
ext_ports.mgt_tx_l3_n    = {1 'out' ['XAUI',s.port,'_tx_l3_n'] 'null' 'null' 'vector=false'};
ext_ports.mgt_rx_l0_p    = {1 'in'  ['XAUI',s.port,'_rx_l0_p'] 'null' 'null' 'vector=false'};
ext_ports.mgt_rx_l0_n    = {1 'in'  ['XAUI',s.port,'_rx_l0_n'] 'null' 'null' 'vector=false'};
ext_ports.mgt_rx_l1_p    = {1 'in'  ['XAUI',s.port,'_rx_l1_p'] 'null' 'null' 'vector=false'};
ext_ports.mgt_rx_l1_n    = {1 'in'  ['XAUI',s.port,'_rx_l1_n'] 'null' 'null' 'vector=false'};
ext_ports.mgt_rx_l2_p    = {1 'in'  ['XAUI',s.port,'_rx_l2_p'] 'null' 'null' 'vector=false'};
ext_ports.mgt_rx_l2_n    = {1 'in'  ['XAUI',s.port,'_rx_l2_n'] 'null' 'null' 'vector=false'};
ext_ports.mgt_rx_l3_p    = {1 'in'  ['XAUI',s.port,'_rx_l3_p'] 'null' 'null' 'vector=false'};
ext_ports.mgt_rx_l3_n    = {1 'in'  ['XAUI',s.port,'_rx_l3_n'] 'null' 'null' 'vector=false'};
b = set(b,'ext_ports',ext_ports);

% borf parameters
b = set(b,'mode',7);
b = set(b,'size',1);

