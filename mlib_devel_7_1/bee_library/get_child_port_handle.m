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

function child_port_hdl = get_child_port_handle (port_hdl);
port = num2str(get_param(port_hdl,'PortNumber'));
port_type = get_param(port_hdl, 'PortType');
parent = get_param(port_hdl, 'Parent');
switch port_type
case 'outport',
    child_port_hdl = find_system(parent,'FollowLinks','on','LookUnderMasks','all','SearchDepth',1,'BlockType','Outport','Port',port);
case 'inport',
    child_port_hdl = find_system(parent,'FollowLinks','on','LookUnderMasks','all','SearchDepth',1,'BlockType','Inport','Port',port);
case 'enable',
    child_port_hdl = find_system(parent,'FollowLinks','on','LookUnderMasks','all','SearchDepth',1,'BlockType','EnablePort');
otherwise,
    error ('Error getting child port handle: invalid port type')
end
child_port_hdl = child_port_hdl{1};
