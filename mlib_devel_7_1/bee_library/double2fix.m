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

function result = double2fix(value,type,width,bin_pt,round_mode,sat_mode)
switch round_mode
    case 'fix'
        int_value = fix(value*(2^bin_pt));
    otherwise
        int_value = round(value*(2^bin_pt));
end
if strcmp(sat_mode,'sat')
    switch type
        case 'ufix' %unsigned
            if int_value < 0
                int_value = 0;
            elseif int_value > 2^width-1
                int_value = 2^width-1;
            end
        case 'sfix' %unsigned
            if int_value < -2^(width-1)
                int_value = -2^(width-1);
            elseif int_value > 2^(width-1)-1
                int_value = 2^(width-1)-1;
            end
        otherwise
            error(['Unkown arith type: ',type]);
    end
end
result = int_value;