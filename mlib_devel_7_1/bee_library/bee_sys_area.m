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

function [slices,luts,ffs,brams,sub_area] = bee_sys_area(sys, show_detail)
%BEE_SYS_AREA   estimates the FPGA area of the system using CoreGen statistics
%               and synthesis prediction. Only blocks belong to Xilnx System
%               Generator are calculated for total area.
%[slices,luts,ffs,brams] = bee_sys_area(sys)
%[slices,luts,ffs,brams, sub_area] = bee_sys_area(sys, show_detail)
%               Sys is the system reference, which can be a subsystem.
%               Show_detail is a true/false flag to indicate whether to show 
%               detailed block-by-block area of everything in the system.
%               The variable 'sub_area' stores block by block area in 
%               a structure array, with field "block_name" and "fpga_area".
%               Proper operation of this function requires the presence of the
%               System Generator block on the top level of the system. 
%               Area reported are in number of Xilinx Virtex Slices, Look-Up 
%               Tables (LUTs), single bit Registers (FFs), and 4 Kbit block
%               RAMs (BRAMs). Also by tagging any block/subsystem as "simonly"
%               the block/subsystem's area will not be estimated. This is 
%               recommanded for any non-Xilinx blocks/subsystems. Global level
%               variables are NOT passed to the subsystems unless the variable
%               name starts with "xsg_". This is a performace improving choice
%               to reduce the amount the unncessary variable scope copying.

% Author: Chen Chang
% Created on March 14th, 2002
% Last time modified: March 16th, 2002

debug = 0;
global sub_area;
sub_area = struct([]);
if nargin == 2 
    debug = 1;
end

[root_sys,root_dir,glb_ce,glb_clr,clk_period] = get_xsg_info(sys);
root = bdroot(sys);

core_dir = [root_dir,'/corework'];
ini_exist = 0;
tmp_d = dir(core_dir);
for i = 1:length(tmp_d)
    if strcmp(tmp_d(i).name, 'coregen.ini')
        ini_exist = 1;
        break;
    end
end
if ~exist(core_dir)
    disp(sprintf('Warning: core directory (%s) does not exist, please check if no cores used',core_dir));
elseif ~ini_exist
    fid = fopen (sprintf('%s/coregen.ini',core_dir),'w');
    if fid == -1
        error(sprintf('Error: cannot write to CoreGen initialization file at %s',core_dir));
    end
    fprintf(fid,'SET DumpCoreStatistics = true');
    fclose(fid);
    old_dir = pwd;
    eval(sprintf('cd %s',core_dir));
    dos(sprintf('c:/xilinx/bin/nt/coregen.bat -b sysgen.sgxco',core_dir));
    eval(sprintf('cd %s',old_dir));
end

vars = evalin('caller','who');
counter = 1;
ws = [];
for i = 1:length(vars)
    var_str = vars(i);
    var_str = var_str{1};
    xsg_pos = findstr(var_str,'xsg_');
    if length(xsg_pos) ~= 0 & xsg_pos(1) == 1
        ws(counter).Name = var_str;
        ws(counter).Value = evalin('caller',var_str);
        counter = counter + 1;
    end

end

%systems = find_system(sys,'FollowLinks', 'on','LookUnderMasks','graphical','RegExp','on','FunctionName','^xl');
eval(sprintf('%s([],[],[],%ccompile%c);',root,char(39),char(39)));
[slices,luts,ffs,brams] = subsys_area(sys,core_dir,ws,debug);
eval(sprintf('%s([],[],[],%cterm%c);',root,char(39),char(39)));
clear bee_blk_area;
clear value2str;

return;


function [slices,luts,ffs,brams] = subsys_area(sys,core_dir,ws,debug)
slices = 0;
luts = 0;   
ffs = 0;
brams = 0;
if strcmp(get_param(sys,'Tag'),'simonly')
    return;
end
clear vars_struct;
try
    mask_on = get_param(sys,'Mask');
catch
    mask_on = 'none';
end
if strcmp(mask_on,'on')
    vars_struct = get_param(sys,'MaskWSVariables');
    if length(vars_struct) ~= 0
        try
            ws = [ws,vars_struct];
        catch
            disp(size(ws));
            disp(size(vars_struct));
            disp(fieldnames(vars_struct));
        end
    end
end
for j = 1:length(ws)
    try
        eval([ws(j).Name,' = ',value2str(ws(j).Value),';']);
    catch
        disp(lasterr);
        disp(sprintf('Error: invalid mask parameter (%s) at %s',ws(j).Name,sys));
        return;
    end
end
xblks = find_system(sys,'SearchDepth',1,'FollowLinks', 'on','LookUnderMasks','all','RegExp','on','FunctionName','^xl');
for i=1:length(xblks)
    xblk = xblks(i);
    xblk = xblk{1,1};
    try
        [a,b,c,d] = bee_blk_area(xblk,core_dir);
    catch
        disp(lasterr);
        disp(sprintf('Error estimate area at block: %s',xblk));
        return;
    end
    if debug
        disp(sprintf('\t%s: %5.2f %5.2f %5.2f %5.2f',xblk,a,b,c,d));
        global sub_area;
        sub_area = [sub_area, struct('block_name',xblk,'fpga_area',[a,b,c,d])];
    end
    slices = slices + a;
    luts = luts + b;
    ffs = ffs + c;
    brams = brams + d;
end
subsystems = find_system(sys,'SearchDepth',1,'FollowLinks', 'on','LookUnderMasks','all','BlockType','SubSystem');
for i = 1:length(subsystems)
    subsys = subsystems(i);
    subsys = subsys{1,1};
    if strcmp(subsys,sys)
        continue;
    end
    if strcmp(get_param(subsys,'Tag'),'genX')
        continue;
    end
    try
        [a,b,c,d] = subsys_area(subsys,core_dir,ws,debug);
    catch
        disp(lasterr);
        disp(sprintf('Error estimate masked subsystem: %s',subsys));
        return;
    end
    if debug
        global sub_area;
        sub_area = [sub_area, struct('block_name',subsys,'fpga_area',[a,b,c,d])];
        disp(sprintf('%s: %5.2f %5.2f %5.2f %5.2f',subsys,a,b,c,d));
    end
    slices = slices + a;
    luts = luts + b;
    ffs = ffs + c;
    brams = brams + d;
end
return;

