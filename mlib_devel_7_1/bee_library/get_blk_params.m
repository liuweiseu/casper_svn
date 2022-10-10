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

function params = get_blk_params(blk_hdl)
params = struct([]);
obj_params = get_param(blk_hdl,'ObjectParameters');
param_names = fieldnames(obj_params);
for i=1:length(param_names)
    value = get_param(blk_hdl,param_names{i});
    if isempty(params)
        params = struct(param_names{i},value);
    else
        params = setfield(params,param_names{i},value);
    end
end
user_params = get_param(blk_hdl, 'UserData');
if isstruct(user_params)
    user_param_names = fieldnames(user_params);
    for j=1:length(user_param_names)
        params = setfield(params,user_param_names{j},getfield(user_params,user_param_names{j}));
    end
end