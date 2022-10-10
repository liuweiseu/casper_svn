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

function result = get_core_est(sys,temp_dir,core_data)
result=struct('primitives',0,'clbs',0,'slices',0,'lut_sites',0,'luts',0,'reg',0,'srl16s',0,'dist_ram',0,'bram',0,'mults',0,'hu_sets',0);

sub_systems = find_system(sys, 'SearchDepth',1,'FollowLinks','on','LookUnderMasks','all','BlockType', 'SubSystem');
for i=1:length(sub_systems)
    sub_sys = sub_systems{i};
    if ~strcmp(sub_sys,sys)
        sub_result = get_core_est(sub_sys,temp_dir,core_data);
        result = add_core_data(result,sub_result);
    else
        cur_cores = find_vhdl_core([temp_dir,'/',clear_name(sys),'.vhd']);
        for j=1:length(cur_cores)
            cur_core = cur_cores{j};
            result = add_core_data(result,getfield(core_data,cur_core));
        end
    end
end