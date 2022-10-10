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

function [bit_width,dec_pt,type] = get_xwidth(data_type);
switch data_type
    case 'double'
        bit_width = 64;
        dec_pt = 11;
        type = 'double';
    case 'single'
        bit_width = 32;
        dec_pt = 8;
        type = 'single';
    case 'Bool'
        bit_width = 1;
        dec_pt = 0;
        type = 'beeBool';
    case 'auto'
        bit_width = -1;
        dec_pt = -1;
        type = ''; %unknown
    otherwise
        Utype = sscanf(data_type,'UFix_%d_%d');
        Stype = sscanf(data_type,'Fix_%d_%d');
        SFix_type = sscanf(data_type,'sfix%d_En%d');
        UFix_type = sscanf(data_type,'ufix%d_En%d');
        int_type = sscanf(data_type,'int%d');
        uint_type = sscanf(data_type,'uint%d');
        if ~isempty(Utype)
            bit_width = Utype(1);
            dec_pt = Utype(2);
            type = 'beeUnsigned';
        elseif ~isempty(Stype)
            bit_width = Stype(1);
            dec_pt = Stype(2);
            type = 'beeSigned';
        elseif ~isempty(UFix_type)
            bit_width = UFix_type(1);
            dec_pt = UFix_type(2);
            type = 'beeUnsigned';
        elseif ~isempty(SFix_type)
            bit_width = SFix_type(1);
            dec_pt = SFix_type(2);
            type = 'beeSigned';
        elseif ~isempty(uint_type)
            bit_width = uint_type(1);
            dec_pt = 0;
            type = 'beeUnsigned';
        elseif ~isempty(int_type)
            bit_width = int_type(1);
            dec_pt = 0;
            type = 'beeSigned';
        else
            error(['Wrong Simulink data type: ',data_type]);
            return;
        end
end