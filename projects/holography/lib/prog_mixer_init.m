%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%   Center for Astronomy Signal Processing and Electronics Research           %
%   http://seti.ssl.berkeley.edu/casper/                                      %
%   Copyright (C) 2010 Andrew Martens					      %	
%									      %
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

function prog_mixer_init(blk,varargin)
% mixer_init(blk, varargin)
%
% blk = The block to initialize.
% varargin = {'varname', 'value', ...} pairs
%
defaults = {};
if same_state(blk, 'defaults', defaults, varargin{:}), return, end
check_mask_type(blk, 'prog_mixer');
munge_block(blk, varargin{:});

lookup_bits = get_var('lookup_bits', 'defaults', defaults, varargin{:});
coeff_bits = get_var('coeff_bits', 'defaults', defaults, varargin{:});
streams = get_var('streams', 'defaults', defaults, varargin{:});
din_bits = get_var('din_bits', 'defaults', defaults, varargin{:});
add_latency = get_var('add_latency', 'defaults', defaults, varargin{:});
bram_latency = get_var('bram_latency', 'defaults', defaults, varargin{:});
mult_latency = get_var('mult_latency', 'defaults', defaults, varargin{:});
numerator = get_var('numerator', 'defaults', defaults, varargin{:});
denominator = get_var('denominator', 'defaults', defaults, varargin{:});
offset = get_var('offset', 'defaults', defaults, varargin{:});
dout_bits = get_var('dout_bits', 'defaults', defaults, varargin{:});
dout_bin_pt = get_var('dout_bin_pt', 'defaults', defaults, varargin{:});

set_param([blk, '/prog_dds'], ...
	'lookup_bits', tostring(lookup_bits), ...
	'streams', tostring(streams), ...
	'data_bits', tostring(coeff_bits), ...
	'numerator', tostring(numerator), ...
	'denominator', tostring(denominator), ...
	'offset', tostring(offset), ...
	'bram_latency', tostring(bram_latency));

% Set attribute format string (block annotation)
fstring = ['f - ', num2str(numerator), '/', num2str(denominator)];
set_param(blk,'AttributesFormatString',fstring);

save_state(blk, 'defaults', defaults, varargin{:});
