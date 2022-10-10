%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%   Center for Astronomy Signal Processing and Electronics Research           %
%   http://casper.berkeley.edu                                                %
%   Copyright (C)2010 Billy Mallard                                           %
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

function capture_init(blk, varargin)
% Initialize and configure a capture block.
%
% capture_init(blk, varargin)
%
% blk = The block to configure.
% varargin = {'varname', 'value', ...} pairs.
%
% Valid varnames for this block are:
% num_brams = 
% addr_width = 
% done_port = 
% logic_latency = 

% Declare any default values for arguments you might like.
defaults = {};
if same_state(blk, 'defaults', defaults, varargin{:}), return, end
check_mask_type(blk, 'capture');
munge_block(blk, varargin{:});

num_brams = get_var('num_brams', 'defaults', defaults, varargin{:});
addr_width = get_var('addr_width', 'defaults', defaults, varargin{:});
logic_latency = get_var('logic_latency', 'defaults', defaults, varargin{:});
fanout_latency = get_var('fanout_latency', 'defaults', defaults, varargin{:});
bram_latency = get_var('bram_latency', 'defaults', defaults, varargin{:});

% Validate input fields.

if num_brams < 1
	errordlg('capture: Invalid number of BRAMs. Should be greater than zero.')
end

if addr_width < 11
	errordlg('capture: Invalid address width. Must be at least 11 bits.')
end

%
% Start drawing!
%

delete_lines(blk)

cur_ypos = 0;
cur_port = 1;

% Add input delays and BRAMs.

for i = 1:num_brams
	id = num2str(i-1);

	name = ['din', id];
	ypos = cur_ypos + 28;
	position = [15, ypos, 45, ypos+14];
	reuse_block(blk, name, 'built-in/inport', ...
		'Position', position, ...
		'Port', num2str(cur_port));

	name = ['reinterp', id];
	xpos = 95;
	ypos = cur_ypos + 28;
	position = [xpos, ypos, xpos+50, ypos+14];
	reuse_block(blk, name, 'xbsIndex_r4/Reinterpret', ...
		'Position', position, ...
		'ShowName', 'off', ...
		'force_arith_type', 'on', ...
		'arith_type', 'Unsigned', ...
		'force_bin_pt', 'on', ...
		'bin_pt', '0');

	name = ['din_delay', id];
	xpos = 220;
	ypos = cur_ypos + 28;
	position = [xpos, ypos, xpos+50, ypos+14];
	reuse_block(blk, name, 'xbsIndex_r4/Delay', ...
		'Position', position, ...
		'ShowName', 'off', ...
		'latency', 'logic_latency + fanout_latency', ...
		'reg_retiming', 'on');

	name = ['trig_delay', id];
	xpos = 220;
	ypos = cur_ypos + 50;
	position = [xpos, ypos, xpos+50, ypos+14];
	reuse_block(blk, name, 'xbsIndex_r4/Delay', ...
		'Position', position, ...
		'ShowName', 'off', ...
		'latency', 'fanout_latency', ...
		'reg_retiming', 'on');

	name = ['cntr_delay', id];
	xpos = 295;
	ypos = cur_ypos + 28;
	position = [xpos, ypos, xpos+50, ypos+14];
	reuse_block(blk, name, 'xbsIndex_r4/Delay', ...
		'Position', position, ...
		'ShowName', 'off', ...
		'latency', 'cntr_latency + 1', ...
		'reg_retiming', 'on');

	name = ['trig_cntr', id];
	xpos = 295;
	ypos = cur_ypos + 43;
	position = [xpos, ypos, xpos+50, ypos+28];
	reuse_block(blk, name, 'isi_correlator_lib/trig_cntr', ...
		'Position', position, ...
		'bitwidth', 'addr_width', ...
		'latency', 'cntr_latency');

	name = ['bram', id, '_delay1'];
	xpos = 420;
	ypos = cur_ypos + 13;
	position = [xpos, ypos, xpos+50, ypos+14];
	reuse_block(blk, name, 'xbsIndex_r4/Delay', ...
		'Position', position, ...
		'ShowName', 'off', ...
		'latency', 'bram_latency', ...
		'reg_retiming', 'on');

	name = ['bram', id, '_delay2'];
	xpos = 420;
	ypos = cur_ypos + 28;
	position = [xpos, ypos, xpos+50, ypos+14];
	reuse_block(blk, name, 'xbsIndex_r4/Delay', ...
		'Position', position, ...
		'ShowName', 'off', ...
		'latency', 'bram_latency', ...
		'reg_retiming', 'on');

	name = ['bram', id, '_delay3'];
	xpos = 420;
	ypos = cur_ypos + 43;
	position = [xpos, ypos, xpos+50, ypos+14];
	reuse_block(blk, name, 'xbsIndex_r4/Delay', ...
		'Position', position, ...
		'ShowName', 'off', ...
		'latency', 'bram_latency', ...
		'reg_retiming', 'on');

	name = ['bram', id];
	xpos = 495;
	ypos = cur_ypos + 13;
	position = [xpos, ypos, xpos+100, ypos+44];
	reuse_block(blk, name, 'xps_library/Shared BRAM', ...
		'Position', position, ...
		'arith_type', 'Unsigned', ...
		'addr_width', 'addr_width', ...
		'data_bin_pt', '0', ...
		'init_vals', '0', ...
		'sample_rate', '1');

	name = ['terminator', id];
	xpos = 620;
	ypos = cur_ypos + 25;
	position = [xpos, ypos, xpos+20, ypos+20];
	reuse_block(blk, name, 'built-in/terminator', ...
		'Position', position, ...
		'ShowName', 'off');

	add_line(blk, ['bram', id, '_delay1/1'], ['bram', id, '/1']);
	add_line(blk, ['bram', id, '_delay3/1'], ['bram', id, '/3']);

	add_line(blk, ['din', id, '/1'], ['reinterp', id, '/1']);
	add_line(blk, ['reinterp', id, '/1'], ['din_delay', id, '/1']);
	add_line(blk, ['din_delay', id, '/1'], ['cntr_delay', id, '/1']);
	add_line(blk, ['cntr_delay', id, '/1'], ['bram', id, '_delay2/1']);
	add_line(blk, ['bram', id, '_delay2/1'], ['bram', id, '/2']);
	add_line(blk, ['bram', id, '/1'], ['terminator', id, '/1']);

	add_line(blk, ['trig_delay', id, '/1'], ['trig_cntr', id, '/1']);
	add_line(blk, ['trig_cntr', id, '/1'], ['bram', id, '_delay1/1']);
	add_line(blk, ['trig_cntr', id, '/2'], ['bram', id, '_delay3/1']);

	cur_port = cur_port + 1;
	cur_ypos = cur_ypos + 65;
end

% Add control logic blocks.

name = 'en';
ypos = cur_ypos + 28;
position = [15, ypos, 45, ypos+14];
reuse_block(blk, name, 'built-in/inport', ...
	'Position', position, ...
	'Port', num2str(cur_port))
cur_port = cur_port + 1;

name = 'trig';
ypos = cur_ypos + 68;
position = [15, ypos, 45, ypos+14];
reuse_block(blk, name, 'built-in/inport', ...
	'Position', position, ...
	'Port', num2str(cur_port))
cur_port = cur_port + 1;

name = 'Logical';
xpos = 95;
ypos = cur_ypos + 43;
position = [xpos, ypos, xpos+50, ypos+44];
reuse_block(blk, name, 'xbsIndex_r4/Logical', ...
	'Position', position, ...
	'ShowName', 'off', ...
	'logical_function', 'AND', ...
	'inputs', '2', ...
	'en', 'off', ...
	'latency', 'logic_latency', ...
	'precision', 'Full', ...
	'align_bp', 'on')

add_line(blk, 'en/1', 'Logical/1')
add_line(blk, 'trig/1', 'Logical/2')
for i = 1:num_brams
	add_line(blk, 'Logical/1', ['trig_delay', num2str(i-1), '/1'])
end

clean_blocks(blk)

%
% End drawing.
%

%
% Resize this block appropriately.
%
position = get_param(blk, 'Position');
xstart = position(1);
xend = xstart + 50;
ystart = position(2);
yend = ystart + 15*(num_brams+2);
position = [xstart, ystart, xend, yend];
set_param(blk, 'Position', position)

save_state(blk, 'defaults', defaults, varargin{:});

