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

function fix2string(value, width, binary_point, arith_type, file_name)
global bee_save_tb;
global bee_tb_dir;
if bee_save_tb
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
        otherwise
            error(['Unkown arith type passed to fix2string: ',num2str(arith_type)]);
    end
    
    result = dec2bin(int_value,width);
    result = strrep(result,'/','1');
    
    if ~isempty(bee_tb_dir)
        bee_tb_dir = [bee_tb_dir,'/'];
    end
    fid=fopen([bee_tb_dir,file_name],'a');
    fprintf(fid,[result,'\n']);
    fclose(fid);
end