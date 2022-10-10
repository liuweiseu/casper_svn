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

function [full_width, full_pt, full_arith] = get_full_precision(input_sigs)

uint_width = 0;
sint_width = 0;
frac_width = 0;
full_arith = '';
[a,b] = size(input_sigs);
for i=1:a
    if b == 3
        sig_width = input_sigs{i,1};
        sig_pt = input_sigs{i,2};
        sig_type = input_sigs{i,3};
    elseif b == 4
        sig_dim = input_sigs{i,1};
        sig_width = input_sigs{i,2}(1);
        sig_pt = input_sigs{i,3}(1);
        sig_type = input_sigs{i,4}{1};
    end
    
    int_width = sig_width-sig_pt;
    switch sig_type
        case 'beeSigned'
            sint_width = max(int_width-1,sint_width);
        case 'beeUnsigned'
            uint_width = max(int_width,uint_width);
        case 'beeBool'
            uint_width = max(int_width,uint_width);
        otherwise
            error(['Unsupported signal type: ',sig_type]);
    end
    if isempty(full_arith)
        full_arith = sig_type;
    elseif strcmp(sig_type,'beeSigned')
        full_arith = 'beeSigned';
    end
    frac_width = max(frac_width, sig_pt);
end
full_width = max(uint_width,sint_width)+frac_width;
full_pt = frac_width;
if strcmp(full_arith,'beeSigned')
    full_width = full_width + 1;
end