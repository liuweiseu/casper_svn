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

function result = fix2string(value, width, binary_point, arith_type, dec_out)
% fix2string    Fixed-point number to STD_LOGIC_VECTOR string convertion.
%
%   fix2string(value, width, binary_point, arith_type)
%
% Value is the fixed point number value, width is the total bit width,
% binary_point is the binary point location, arith_type can be either '0'
% or '1' indication unsigned or signed number. When bit width is 1,
% STD_LOGIC is returned. The result include the "" or ''

int_value = round(value*(2^binary_point));
switch arith_type
    case {0 'beeUnsigned'} %unsigned
        if int_value < 0
            int_value = 0;
        elseif int_value > 2^width-1
            int_value = 2^width-1;
        end
    case {1 'beeSigned'} %unsigned
        if int_value < -2^(width-1)
            int_value = -2^(width-1);
        elseif int_value > 2^(width-1)-1
            int_value = 2^(width-1)-1;
        end
    case {2 'beeBool'} %boolean
        if int_value > 0
            int_value = 1;
        else
            int_value = 0;
        end
    otherwise
        error(['Unkown arith type passed to fix2string: ',num2str(arith_type)]);
end
if nargin == 5 & dec_out == 1
    result = num2str(int_value);
elseif nargin == 5 & dec_out == 2
    if int_value < 0
        result = dec2hex(2^width+int_value);
    else
        result = dec2hex(int_value);
    end
else
    result = dec2bin(int_value,width);
    result = strrep(result,'/','1');
    if width == 1
        result = [char(39),result,char(39)];
    else
        result = ['"',result,'"'];
    end
end