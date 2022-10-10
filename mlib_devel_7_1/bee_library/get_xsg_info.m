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

function [root_sys,root_dir,glb_ce,glb_clr,clk_period,chip,xilinx_family, synth, part, speed, package] = get_xsg_info(sys)
%if isempty(gcs) | ~strcmp(gcs,design_name)
%    try 
%        open_system(design_name);
%    catch
%        errordlg(sprintf('Error cannot open system %c%s%c',39, design_name, 39));
%        return;
%end
%end

root = bdroot(sys);
while 1
    sysgen_blk = find_system(sys, 'SearchDepth', 1,'FollowLinks','on','LookUnderMasks','all','Tag','genX');
    if length(sysgen_blk) == 1 | strcmp(sys,root)
        root_sys = sys;
        break;
    else
        sys = get_param(sys,'Parent');
    end
end
    
if length(sysgen_blk) ~= 1
    disp('Error: Systen Generator block need to be defined first');
    return;
end
sysgen_blk = sysgen_blk{1};
root_dir = strrep(get_param(sysgen_blk,'directory'),'\','/');
[dir_prefix, R] = strtok(root_dir,'/');
switch dir_prefix
    case '.'
        root_dir = [strrep(pwd,'\','/'),R];
end
try 
    glb_ce = get_param(sysgen_blk,'create_ce');
catch
    glb_ce = '';
end
try 
    glb_clr = get_param(sysgen_blk,'create_clr');
catch
    glb_clr = '';
end
clk_period = get_param(sysgen_blk,'sysclk_period');

try 
    synth = get_param(sysgen_blk,'synthesis_tool');
catch
    synth = '';
end
try 
    xilinx_family = get_param(sysgen_blk,'xilinxfamily');
catch
    xilinx_family = '';
end
try
    part = get_param(sysgen_blk,'part');
    speed = get_param(sysgen_blk,'speed');
    package = get_param(sysgen_blk,'package');
    chip = [part,speed,'-',package];
catch
    chip = '';
    part = '';
    speed = '';
    package = '';
end
return;