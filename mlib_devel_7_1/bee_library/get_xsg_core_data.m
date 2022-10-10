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

function result=get_xsg_core_data(xsg_dir)
xsg = xlversion;
xsg = strtok(xsg{1},' ');
switch xsg
    case {'6.2.R14' '6.2'}
    otherwise
        error(['Unsupported Xilinx System Generator version: ',xsg]);
end
core_dir = [xsg_dir,'/temp/coregen_tmp'];
if ~exist(core_dir)
    error('Make sure you have run Xilinx System Generator, and the temp directory generated is not deleted.');
end


files = dir(core_dir);
for i=1:length(files)
    file = files(i);
    if regexp(file.name, '.xco$')
        core_name = file.name(1:length(file.name)-4);
        core_data = parse_xco_file([core_dir,'/',file.name]);
        eval(['result.',core_name,'=core_data;']);
    end
end