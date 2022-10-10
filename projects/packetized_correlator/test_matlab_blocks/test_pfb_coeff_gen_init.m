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

% Initialize and configure the Polyphase Filter Bank coefficient generator.
%
% pfb_coeff_gen_init(blk, varargin)
%
% blk = The block to configure.
% varargin = {'varname', 'value', ...} pairs
% 
% Valid varnames for this block are:
% PFBSize = Size of the FFT (2^FFTSize points).
% CoeffBitWidth = Bit width of coefficients.
% TotalTaps = Total number of taps in the PFB
% CoeffDistMem = Implement coefficients in distributed memory
% WindowType = The type of windowing function to use.
% bram_latency = The latency of BRAM in the system.
% n_inputs = Number of parallel input streams
% nput = Which input this is (of the n_inputs parallel).
% fwidth = The scaling of the bin width (1 is normal).

% Declare any default values for arguments you might like.

PFBSize = 11;
CoeffBitWidth = 8;
TotalTaps = 4;
CoeffDistMem = 0;
WindowType = 'hamming';
bram_latency = 2;
n_inputs = 0;
nput = 0;
fwidth = 1;

% Set coefficient vector
alltaps = TotalTaps*2^PFBSize;
windowval = transpose(window(WindowType, alltaps));
total_coeffs = windowval .* sinc(fwidth*([0:alltaps-1]/(2^PFBSize)-TotalTaps/2));
for i=1:alltaps/2^n_inputs,
    buf(i)=total_coeffs((i-1)*2^n_inputs + nput + 1);
end

% Add Dynamic Blocks
for a=1:TotalTaps,
    blkname = ['ROM', num2str(a)];
    %v = ['buf(', num2str(a-1), '*2^(PFBSize-n_inputs) + 1:', num2str(a), '*2^(PFBSize-n_inputs))'];
    v = mat2str(buf((a-1)*2^(PFBSize-n_inputs)+1 : a*2^(PFBSize-n_inputs)),CoeffBitWidth/2)
end