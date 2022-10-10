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

function fanout_init(blk, varargin)
% Initialize and configure a fanout block.
%
% fanout_init(blk, varargin)
%
% blk = The block to configure.
% varargin = {'varname', 'value', ...} pairs.
%
% Valid varnames for this block are:
% fanout = 
% delays = 

% Declare any default values for arguments you might like.
defaults = {};
if same_state(blk, 'defaults', defaults, varargin{:}), return, end
check_mask_type(blk, 'fanout');
munge_block(blk, varargin{:});

fanout = get_var('fanout', 'defaults', defaults, varargin{:});
delays = get_var('delays', 'defaults', defaults, varargin{:});

% Validate input fields.

if fanout < 1
	errordlg('fanout: Invalid fanout. Should be greater than zero.')
	return
end

%
% Start drawing!
%

delete_lines(blk)

levels = nextpow2(fanout);

cur_xpos = 0;
cur_ypos = 0;
cur_port = 1;

spacing = 2^levels;
for col = 1:levels+1

	% the number of the first pipeline delay block in this column.
	col_min = 2^(col-1);

	xpos = cur_xpos + 100;
	ypos = cur_ypos;

	% col_min happens to also be the number of items in the column.
	for row = 0:col_min-1
		delay_id = num2str(col_min+row);

		% always add a delay to the current column.
		name = ['pipeline', delay_id];
		ypos = cur_ypos + (row+.5)*spacing*50;
		position = [xpos, ypos, xpos+50, ypos+14];
		reuse_block(blk, name, 'xbsIndex_r4/Delay', ...
			'Position', position, ...
			'ShowName', 'off', ...
			'latency', 'delays', ...
			'reg_retiming', 'on');

		% if this is the first column, add an inport and wire it up.
		if col == 1
			name = 'in';
			position = [0, ypos, 30, ypos+14];
			reuse_block(blk, name, 'built-in/inport', ...
				'Position', position, ...
				'ShowName', 'on', ...
				'Port', '1');

			add_line(blk, [name, '/1'], 'pipeline1/1');
		% otherwise, wire the delay to one from the previous column.
		else
			src_id = num2str(floor((col_min+row)/2));
			dst_id = num2str(col_min+row);
			src = ['pipeline', src_id, '/1'];
			dst = ['pipeline', dst_id, '/1'];
			add_line(blk, src, dst);
		end

		% if this is the final column, add an outport or a terminator.
		if col == levels+1
			port_id = num2str(row);

			if row < fanout
				name = ['out', port_id];
				position = [xpos+100, ypos, xpos+130, ypos+14];
				reuse_block(blk, name, 'built-in/outport', ...
					'Position', position, ...
					'ShowName', 'on', ...
					'Port', num2str(row+1));
			else
				name = ['term', port_id];
				position = [xpos+100, ypos-6, xpos+120, ypos+14];
				reuse_block(blk, name, 'built-in/terminator', ...
					'Position', position, ...
					'ShowName', 'off');
			end

			delay_name = ['pipeline', num2str(col_min+row)];
			add_line(blk, [delay_name, '/1'], [name, '/1']);
		end
	end

	cur_xpos = xpos;
	spacing = spacing/2;
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
yend = ystart + 15*fanout;
position = [xstart, ystart, xend, yend];
set_param(blk, 'Position', position)

fmtstr = sprintf('delay=%d', (levels+1)*delays);
set_param(blk, 'AttributesFormatString', fmtstr);

save_state(blk, 'defaults', defaults, varargin{:});

