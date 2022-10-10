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

function result = parse_xco_file(file_name);
result=struct('primitives',0,'clbs',0,'slices',0,'lut_sites',0,'luts',0,'reg',0,'srl16s',0,'dist_ram',0,'bram',0,'mults',0,'hu_sets',0);
fid = fopen(file_name,'r');
while 1
    line = fgetl(fid);
    if ~ischar(line)
        break;
    end
    if isempty(strfind(line,':'))
        continue;
    end
    matches = regexp(line,'^\#\s+Number\s+of\s+(?<type>(\w+\s*)+)\s+used\s*(\w+\s*)*\:\s+(?<count>\d+)$','names');
    if ~isempty(matches)
        switch matches.type
            case 'Primitives'
                result.primitives = str2num(matches.count);
            case 'CLBs'
                result.clbs = str2num(matches.count);
            case 'Slices'
                result.slices = str2num(matches.count);
            case 'LUT sites'
                result.lut_sites = str2num(matches.count);
            case 'LUTs'
                result.luts = str2num(matches.count);
            case 'REG'
                result.reg = str2num(matches.count);
            case 'SRL16s'
                result.srl16s = str2num(matches.count);
            case 'Distributed RAM primitives'
                result.dist_ram = str2num(matches.count);
            case 'Block Memories'
                result.bram = str2num(matches.count);
            case 'Dedicated Multipliers'
                result.mults = str2num(matches.count);
            case 'HU_SETs'
                result.hu_sets = str2num(matches.count);
            otherwise
                disp(['Unkonwn core resource type: ',matches.type]);
        end
    end
end