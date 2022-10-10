%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%   Center for Astronomy Signal Processing and Electronics Research           %
%   http://seti.ssl.berkeley.edu/casper/                                      %
%   Copyright (C) 2007 Terry Filiba, Aaron Parsons                            %
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

function fft_stage_n_init(blk, varargin)

% Initialize and configure a single FFT stage.
%
% fft_stage_n_init(blk, varargin)
%
% blk = The block to configure.
% varargin = {'varname', 'value', ...} pairs
% 
% Valid varnames for this block are:
% FFTSize = Size of the FFT (2^FFTSize points).
% FFTStage = Stage this block should be configured as.
% BitWidth = Bitwidth of input data.
% use_bram = Use bram or slr delays
% CoeffBram = Store coefficients in bram
% MaxCoeffNum = 
% quantization = Quantization behavior.
% overflow = Overflow behavior.
% add_latency = The latency of adders in the system.
% mult_latency = The latency of multipliers in the system.
% bram_latency = The latency of BRAM in the system.

% Declare any default values for arguments you might like.
defaults = {};
if same_state(blk, 'defaults', defaults, varargin{:}), return, end
check_mask_type(blk, 'fft_stage_n');
munge_block(blk, varargin{:});

FFTSize = get_var('FFTSize', 'defaults', defaults, varargin{:});
FFTStage = get_var('FFTStage', 'defaults', defaults, varargin{:});
BitWidth = get_var('BitWidth', 'defaults', defaults, varargin{:});
use_bram = get_var('use_bram', 'defaults', defaults, varargin{:});
CoeffBram = get_var('CoeffBram', 'defaults', defaults, varargin{:});
MaxCoeffNum = get_var('MaxCoeffNum', 'defaults', defaults, varargin{:});
quantization = get_var('quantization', 'defaults', defaults, varargin{:});
overflow = get_var('overflow', 'defaults', defaults, varargin{:});
add_latency = get_var('add_latency', 'defaults', defaults, varargin{:});
mult_latency = get_var('mult_latency', 'defaults', defaults, varargin{:});
bram_latency = get_var('bram_latency', 'defaults', defaults, varargin{:});

% Implement delays normally or in BRAM
delays = {'delay_f', 'delay_b'};
for i=1:length(delays),
    full_path = [blk,'/',delays{i}];
    if use_bram,
        set_param(full_path, 'BlockChoice', 'delay_bram');
	set_param([full_path,'/delay_bram'], 'bram_latency', 'bram_latency');
        set_param([full_path,'/delay_bram'], 'DelayLen', '2^(FFTSize-FFTStage)');
    else,
        set_param(full_path, 'BlockChoice', 'delay_slr');
        set_param([full_path,'/delay_slr'], 'DelayLen', '2^(FFTSize-FFTStage)');
    end
end

Coeffs = 0:2^min(MaxCoeffNum,FFTStage-1)-1;
StepPeriod = FFTSize-FFTStage+max(0, FFTStage-MaxCoeffNum);

if(FFTStage ~= 1),
    % Propagate parameters to the butterfly
    propagate_vars([blk,'/butterfly_direct'], 'defaults', defaults, varargin{:}); 
    set_param([blk,'/butterfly_direct'], 'Coeffs', mat2str(Coeffs));
    set_param([blk,'/butterfly_direct'], 'StepPeriod', num2str(StepPeriod));
end

% Take care of storing coefficients in BRAM
roms = find_system(blk, 'lookUnderMasks', 'all', 'FollowLinks','on','masktype', 'Xilinx Single Port Read-Only Memory');
if CoeffBram,
    dist_mem = 'off';
else,
    dist_mem = 'on';
end
if length(roms) > 0 && ~strcmp(get_param(roms{1}, 'distributed_mem'), dist_mem),
    for i=1:length(roms),
        set_param(roms{i}, 'distributed_mem', dist_mem);
    end
end

clean_blocks(blk);

fmtstr = sprintf('FFTSize=%d, FFTStage=%d, BitWidth=%d', FFTSize, FFTStage, BitWidth);
set_param(blk, 'AttributesFormatString', fmtstr);
save_state(blk, 'defaults', defaults, varargin{:});
