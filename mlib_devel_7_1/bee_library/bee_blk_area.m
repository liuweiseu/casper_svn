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

function [slices,luts,ffs,brams] = bee_blk_area( blk,  core_dir)
%BEE_BLK_AREA estimate Xilinx System Generator block area using CoreGen statistics 
%       and synthesis predictions
%   [slices,luts,ffs,brams] = bee_blk_area(blk) 
%       return the total number of slices, LUTs, FFs, and Block RAMs (4 Kbits).

% Author: Chen Chang
% Created on March 14th, 2002
% Last time modified: March 16th, 2002

slices = 0;
luts = 0;
ffs = 0;
brams = 0;
use_core = 0;
try
    use_core_on = get_param(blk,'use_core');
catch
    use_core_on = 'none';
end
try
    gen_core_on = get_param(blk,'gen_core');
catch
    gen_core_on = 'none';
end

if strcmp(use_core_on,'on') & strcmp(gen_core_on,'on')
    use_core = 1;
elseif strcmp(use_core_on,'none') & strcmp(gen_core_on,'on')
    use_core = 1;
end;
    
if use_core;
    if nargin == 1
        root = bdroot(blk);
        sysgen = find_system(root, 'SearchDepth', 1,'Tag','genX');
        if length(sysgen) ~= 1
            error('Error: Systen Generator block need to be defined first');
            return;
        end
        sysgen = sysgen{1,1};
        root_dir = get_param(sprintf(sysgen,root),'directory');
        core_dir = [root_dir,'/corework'];
    end
    [slices, luts, ffs, brams] = get_blk_core_stat(core_dir,blk);
    return;
end

blk_type = get_param(blk, 'FunctionName');
switch blk_type
%Basic Elements
case 'xl_BlackBox'
    disp(sprintf('Warning: BlockBox element found, area not estimated at %s',get_param(blk,'Name')));
    return;
case 'xlconcat'
    return;
case 'xladdrsr'
    depth = evalin('caller',get_param(blk,'depth'));
    tmp = get_blk_inports(blk);
    d = tmp(1);
    width = get_xwidth(get_param(d,'CompiledPortDataType'));
    slices = width/2;
    luts = width;
    ffs = width;
    return;
case 'xlconstant'
    return;
case 'xlconvert'
    tmp = get_blk_outports(blk);
    d = tmp(1);
    width = get_xwidth(get_param(d,'CompiledPortDataType'));
    latency = evalin('caller',get_param(blk,'latency'));
    ffs = width*latency;
    return;
case 'xlcounter'
    width = evalin('caller',get_param(blk,'n_bits'));
    slices = width/2;
    luts = width;
    ffs = width;
    return;
case 'xldelay'
    tmp = get_blk_outports(blk);
    d = tmp(1);
    width = get_xwidth(get_param(d,'CompiledPortDataType'));
    latency = evalin('caller',get_param(blk,'latency'));
    ffs = width*latency;
    return;
case 'xldsamp'
    tmp = get_blk_inports(blk);
    d = tmp(1);
    width = get_xwidth(get_param(d,'CompiledPortDataType'));
    slices = width/2;
    luts = width;
    ffs = width;
    return;
case 'xlgetvalid'
    return;
case 'xlmux'
    tmp = get_blk_outports(blk);
    d = tmp(1);
    width = get_xwidth(get_param(d,'CompiledPortDataType'));
    latency = evalin('caller',get_param(blk,'latency'));
    inputs = evalin('caller',get_param(blk,'inputs'));
    tmp_area = sum(ceil(inputs/4).^[1:ceil(0.5*log2(inputs))])*width;
    slices = tmp_area/2;
    luts = tmp_area;
    ffs = width*latency;
    return;
case 'xlp2s'
    tmp = get_blk_outports(blk);
    d = tmp(1);
    width = get_xwidth(get_param(d,'CompiledPortDataType'));
    latency = evalin('caller',get_param(blk,'latency'));
    inbits = evalin('caller',get_param(blk,'input_bits'));
    luts = 1;
    ffs = inbits+width*latency; 
    slices = ffs/2;
    return;
case 'xlregister'
    tmp = get_blk_inports(blk);
    d = tmp(1);
    width = get_xwidth(get_param(d,'CompiledPortDataType'));
    slices = width/2;
    luts = 1;
    ffs = width;
    return;
case 'xlcast'
    return;
case 'xls2p'
    tmp = get_blk_outports(blk);
    d = tmp(1);
    width = get_xwidth(get_param(d,'CompiledPortDataType'));
    latency = evalin('caller',get_param(blk,'latency'));
    outbits = evalin('caller',get_param(blk,'output_bits'));
    luts = 1;
    ffs = outbits+width*latency;
    slices = ffs/2;
    return;
case 'xlsetvalid'
    slices = 0.5;
    luts = 1;
    return;
case 'xlslice'
    return;
case 'xlbalance'
    disp(sprintf('Warning: Sync block is used at %s, the actual area will depend on the simulation',blk));
    return;
case 'xlusamp'
    tmp = get_blk_inports(blk);
    d = tmp(1);
    width = get_xwidth(get_param(d,'CompiledPortDataType'));
    slices = width/2;
    luts = width;
    ffs = width;
    return;
case 'xlaccum'
    tmp = get_blk_outports(blk);
    d = tmp(1);
    width = get_xwidth(get_param(d,'CompiledPortDataType'));
    slices = width/2;
    luts = width;
    ffs = width;
    brams = 0;
    switch get_param(blk,'overflow')
    case 'Saturate'
        luts = luts + width;
        slices = luts/2;
    case 'Error'
        slices = slices + 1;
        ffs = ffs +1;
    end
    return;
case 'xladdsub'
    tmp = get_blk_outports(blk);
    d = tmp(1);
    width = get_xwidth(get_param(d,'CompiledPortDataType'));
    latency = evalin('caller',get_param(blk,'latency'));
    slices = width/2;
    luts = width;
    ffs = width*latency;
    return;
case 'xlcmult'
    disp(sprintf('Warning: Non-Core version of CMult is used at %s, area not estimated.',blk));
    return;
case 'xlmult'
    disp(sprintf('Warning: Non-Core version of Mult is used at %s, area not estimated.',blk));
    return;
case 'xlinv'
    tmp = get_blk_outports(blk);
    d = tmp(1);
    width = get_xwidth(get_param(d,'CompiledPortDataType'));
    latency = evalin('caller',get_param(blk,'latency'));
    slices = width/2;
    luts = width;
    ffs = width*latency;
    return;
case 'xllogical'
    tmp = get_blk_outports(blk);
    d = tmp(1);
    width = get_xwidth(get_param(d,'CompiledPortDataType'));
    latency = evalin('caller',get_param(blk,'latency'));
    slices = width/2;
    luts = width;
    ffs = width*latency;
    return;
case 'xlnegate'
    tmp = get_blk_outports(blk);
    d = tmp(1);
    width = get_xwidth(get_param(d,'CompiledPortDataType'));
    latency = evalin('caller',get_param(blk,'latency'));
    slices = width/2;
    luts = width;
    ffs = width*latency;
    return;
case 'xlrelational'
    tmp = get_blk_outports(blk);
    d = tmp(1);
    width = get_xwidth(get_param(d,'CompiledPortDataType'));
    latency = evalin('caller',get_param(blk,'latency'));
    slices = width/2;
    luts = width;
    ffs = width*latency;
    return;
case 'xlscale'
    return;
case 'xlshift'
    return;
case 'xlsincos'
    disp(sprintf('Warning: Non-Core version of SineCosine is used at %s, area not estimated.',blk));
    return;
case 'xlsgn'
    latency = evalin('caller',get_param(blk,'latency'));
    slices = 0;
    luts = 1;
    ffs = latency;
    return;
case 'xlcic'
    disp(sprintf('Warning: Non-Core version of CIC is used at %s, area not estimated.',blk));
    return;
case 'xldds'
    disp(sprintf('Warning: Non-Core version of DDS is used at %s, area not estimated.',blk));
    return;
case 'xlfir'
    disp(sprintf('Warning: Non-Core version of FIR is used at %s, area not estimated.',blk));
    return;
case 'xlfft'
    disp(sprintf('Warning: Non-Core version of FFT is used at %s, area not estimated.',blk));
    return;
case 'xldpram'
    disp(sprintf('Warning: Non-Core version of Dual-port RAM is used at %s, area not estimated.',blk));
    return;
case 'xlspram'
    disp(sprintf('Warning: Non-Core version of Single-port RAM is used at %s, area not estimated.',blk));
    return;
case 'xlfifo'
    disp(sprintf('Warning: Non-Core version of FIFO is used at %s, area not estimated.',blk));
    return;
case 'xlsprom'
    disp(sprintf('Warning: Non-Core version of Single-port ROM is used at %s, area not estimated.',blk));
    return;
case 'xlconvencoder'
    disp(sprintf('Warning: Non-Core version of Convolutional Encoder is used at %s, area not estimated.',blk));
    return;
case 'xlinsert'
    return;
case 'xlpuncture'
    return;
case 'xlinterleaver'
    disp(sprintf('Warning: Non-Core version of Interleaver/Deinterleaver is used at %s, area not estimated.',blk));
    return;
case 'xlrsdecode'
    disp(sprintf('Warning: Non-Core version of RS Decoder is used at %s, area not estimated.',blk));
    return;
case 'xlrsencode'
    disp(sprintf('Warning: Non-Core version of RS Encoder is used at %s, area not estimated.',blk));
    return;
case 'xlviterbi'
    disp(sprintf('Warning: Non-Core version of Viterbi Decoder is used at %s, area not estimated.',blk));
    return;
case 'xlqerr'
    return;
case 'xlperiod'
    return;
otherwise
    disp(sprintf('Warning: unknown area at block %s',blk));
    return;
end




function [slices,luts,ffs,brams] = get_blk_core_stat(core_dir, blk)
slices = 0;
luts = 0;
ffs = 0;
brams = 0;
blk_name = get_param(blk,'Name');
blk_type = get_param(blk,'FunctionName');
switch blk_type
case 'xladdsub'
    mode = get_param(blk,'mode');
    if strcmp(mode,'Addition or Subtraction');
        blk_type = 'xladdsubmode';
    end;
case 'xlmux'
    ports = str2num(get_param(blk,'inputs'));
    blk_type = [blk_type,ports];
case 'xllogical'
    ports = str2num(get_param(blk,'inputs'));
    blk_type = [blk_type,ports];
case 'xlddsreg'
    ports = str2num(get_param(blk,'inputs'));
    blk_type = [blk_type,ports];
case 'xlddsscmodereg'
    ports = str2num(get_param(blk,'inputs'));
    blk_type = [blk_type,ports];
case 'xlfir'
    chs = str2num(get_param(blk,'num_channels'));
    blk_type = [blk_type,'_',chs,'ch'];
case 'xlcounter'
    switch get_param(blk,'cnt_type')
        case 'Free Running'
            blk_type = 'xlcounter_free';
        case 'Count Limited'
            blk_type = 'xlcounter_limit';
        otherwise
            error(['Unknown counter type: ',get_param(blk,'cnt_type')]);
    end
end
tmp_index = findstr(blk,blk_name);
tmp_index = tmp_index(length(tmp_index));
parent_file = clear_name(blk(1:tmp_index-1));
parent_file = [parent_file,'.vhd'];
[fid,msg] = fopen([core_dir,'/../',parent_file],'r');
if fid == -1
    error(sprintf('Error: cannot open file %s',[core_dir,'/../',parent_file]));
    return;
end
core_name = '';
blk_name = lower(clear_name(blk_name));
while 1
    line = fgetl(fid);
    if ~ischar(line)
        break;
    else
        [T,R] = strtok(line);
        if strcmp(T,[blk_name,':'])
            [core_name,R] = strtok(R);
            break;
        end
    end
end
fclose(fid);
if strcmp(core_name,'')
    error(sprintf('Error: core component "%s" not found in VHDL file "%s"',blk_name,parent_file));
    return;
end

[fid,msg] = fopen([core_dir,'/../',core_name,'.vhd'],'r');
if fid == -1
    error(sprintf('Error: cannot open file %s',[core_dir,'/../',core_name,'.vhd']));
    return;
end
while 1
    line = fgetl(fid);
    if ~ischar(line)
        break;
    else
        tmp = findstr(line,'core : ');
        if ~isempty(tmp)
            tmp = deblank(line(tmp+7:length(line)));
            tmp_str = strtok(tmp);
            if strcmp(tmp,tmp_str)
                core_name = tmp;
            end
        end
    end
end
fclose(fid);

core_file = [core_name,'.xco'];

[fid,msg] = fopen([core_dir,'/',core_file],'r');
if fid == -1
    error(sprintf('Error: cannot open file %s',[core_dir,'/',core_file]));
    return;
end
while 1
    line = fgetl(fid);
    if ~ischar(line)
        break;
    else
        [T,R] = strtok(line,':');
        switch T
        % CoreGen 4.1i compatiable
        case '# Number of Slices used'
            slices = str2num(strtok(R,':'));
        case '# Number of LUTs used'
            luts = str2num(strtok(R,':'));
        case '# Number of REG used'
            ffs = str2num(strtok(R,':'));
        case '# Number of Block Memories used'
            brams = str2num(strtok(R,':'));
        
        % CoreGen 4.2i compatible
        case '# Number of Slices used in design'
            slices = str2num(strtok(R,':'));
        case '# Number of LUTs used in design'
            luts = str2num(strtok(R,':'));
        case '# Number of REG used in design'
            ffs = str2num(strtok(R,':'));
        case '# Number of Block Memories used in design'
            brams = str2num(strtok(R,':'));
        end
    end
end
fclose(fid);
if slices == 0
    slices = ceil(max(luts,ffs)/2);
end;

if slices ==0 & luts ==0 & ffs ==0 & brams ==0
    error(sprintf('Error: Core statistics not available for %s',blk));
end

return;
