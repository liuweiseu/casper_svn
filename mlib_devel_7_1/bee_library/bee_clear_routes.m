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

function bee_clear_routes(root,onoff)
%BEE_CLEAR_ROUTES(sys)  clears all Xilinx Gateway I/O pad location information
%       for the system (sys), by turning off the specify I/O localation flag


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

if nargin == 2 & onoff
    cur_status = 'off';
    new_status = 'on';
else
    cur_status = 'on';
    new_status = 'off';
end

gws = find_system(root,'FollowLinks','on','LookUnderMasks','all','FunctionName',gw_in_name,'locs_specified',cur_status);
for i = 1:length(gws)
    gw = gws(i);
    gw = gw{1,1};
    set_param(gw,'locs_specified',new_status);
end

gws = find_system(root,'FollowLinks','on','LookUnderMasks','all','FunctionName',gw_out_name,'locs_specified',cur_status);
for i = 1:length(gws)
    gw = gws(i);
    gw = gw{1,1};
    set_param(gw,'locs_specified',new_status);
end