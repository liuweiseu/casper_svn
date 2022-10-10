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

function str = value2str(value)
if isempty(value)
    str = '';
    return;
elseif iscell(value)
    open_str = '{';
    close_str = '}';
elseif ischar(value)
    str = [char(39),value,char(39)];
    return;
elseif isnumeric(value) & length(value) == 1
    str = num2str(value);
    return;
else
    open_str = '[';
    close_str = ']';
end
[dimy,dimx] = size(value);
str = open_str;
if isstruct(value)
    for p = 1:length(value)
        str = [str,'struct('];
        names = fieldnames(value(p));
        for k=1:length(names)
            str = [str,char(39),names{k},char(39),',',value2str(getfield(value(p),names{k})),','];
        end
        str(length(str):length(str)+1) = '),';
    end
else
    for i =1:dimy
        for j=1:dimx
            if iscell(value)
                str = [str,value2str(value{i,j}),','];
            else
                str = [str,value2str(value(i,j)),','];
            end
        end
        str = [str(1:end-1),';'];
    end
end
str = [str(1:end-1),close_str];