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

function bee_package(indir,outdir)
if ~exist(indir,'dir')
    error(['Non-existing input directory: ',indir]);
else
    eval(['cd ',indir]);
end
if ~exist(outdir,'dir')
    slash_pos = findstr(outdir,'/');
    parent_dir = outdir(1:slash_pos(length(slash_pos))-1);
    new_dir = outdir(slash_pos(length(slash_pos))+1:length(outdir));
    mkdir(parent_dir,new_dir);
end
D = dir(indir);
for i=3:length(D)
    cur_file = D(i);
    if cur_file.isdir
        bee_package([indir,'/',cur_file.name],[outdir,'/',cur_file.name]);
        cd ..;
    else
        file_name = cur_file.name;
        if regexp(file_name,'slblocks.m$')
            dos(strrep(['copy ',file_name,' ',outdir],'/','\'));
            continue;
        end
        periods = strfind(file_name,'.');
        file_ext = file_name(periods(length(periods))+1:length(file_name));
        switch file_ext
            case 'm'
                eval(['mcc -B pcode -d ',outdir,' ',file_name,';']);
            case {'mdl','mat','fig','tcl','vhd','png','p'}
                dos(strrep(['copy ',file_name,' ',outdir],'/','\'));
            otherwise
                disp(['Unknow file type, skipped: ',indir,'/',file_name]);
        end
        
    end
end