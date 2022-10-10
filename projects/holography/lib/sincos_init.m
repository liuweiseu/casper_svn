% Generate sine/cos.
%
% sinecos_init(blk, varargin)
%
% blk = The block to be configured.
% varargin = {'varname', 'value', ...} pairs
%
% Valid varnames for this block are:

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%   The Karroo Array Telescope Project                                       %
%   www.kat.ac.za                                                            %
%   Copyright (C) 2010 Andrew Martens                          %
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

function sincos_init(blk,varargin)

check_mask_type(blk, 'sincos');

defaults = {};
if same_state(blk, 'defaults', defaults, varargin{:}), return, end
munge_block(blk, varargin{:});

func = get_var('func', 'defaults', defaults, varargin{:});
neg_sin = get_var('neg_sin', 'defaults', defaults, varargin{:});
neg_cos = get_var('neg_cos', 'defaults', defaults, varargin{:});
bit_width = get_var('bit_width', 'defaults', defaults, varargin{:});
symmetric = get_var('symmetric', 'defaults', defaults, varargin{:});
bram_latency = get_var('bram_latency', 'defaults', defaults, varargin{:});
depth_bits = get_var('depth_bits', 'defaults', defaults, varargin{:});
handle_sync = get_var('handle_sync', 'defaults', defaults, varargin{:});
prog_lookup = get_var('prog_lookup', 'defaults', defaults, varargin{:});

delete_lines(blk);

base = 0;
%handle the sync
if strcmp(handle_sync, 'on'),
    reuse_block(blk, 'sync_in', 'built-in/inport', 'Port', '1', 'Position', [80 70 110 90]);
    reuse_block(blk, 'delay', 'xbsIndex_r4/Delay', ...
        'latency', 'bram_latency', ... 
        'Position', [190 70 230 90]);

    add_line(blk, 'sync_in/1', 'delay/1');
    reuse_block(blk, 'sync_out', 'built-in/outport', 'Port', '1', 'Position', [500 70 530 90]);
    add_line(blk, 'delay/1', 'sync_out/1');
    base = 1;
end

%input and output ports
reuse_block(blk, 'theta', 'built-in/inport', 'Port', num2str(base+1), 'Position', [80 130 110 150]);

%draw first lookup
if( strcmp(func, 'sine and cosine') || strcmp(func, 'sine') ),
    if strcmp(neg_sin, 'on') , sin_name = '-sine'; else sin_name = 'sine'; end
    reuse_block(blk, sin_name, 'built-in/outport', 'Port', num2str(base+1), 'Position', [550 130 580 150]);
end
if( strcmp(func, 'sine and cosine') || strcmp(func, 'cosine')),
    if strcmp(neg_cos, 'on') , cos_name = '-cos'; else cos_name = 'cos'; end
    if strcmp(func, 'sine and cosine') pos = '3'; end
    reuse_block(blk, cos_name, 'built-in/outport', 'Port', num2str(base+2), 'Position', [550 190 580 210]);
end

%slice used to select bits from ROM
reuse_block(blk, 'slice0', 'xbsIndex_r4/Slice', ...
    'nbits', 'bit_width','mode', 'Upper Bit Location + Width', ...
    'bit1', '0', 'Position', [400 130 425 150]);

%lookup for sine/cos
s = '';
if( strcmp(func, 'sine') || strcmp(func, 'sine and cosine') ),
    if strcmp(neg_sin, 'on') , s = '-'; end    
    init_vec = sprintf('%ssin(2*pi*(0:(%s))/(%s))',s,'2^depth_bits-1','2^depth_bits');   
else 
    if( strcmp(neg_cos, 'on') ), s = '-'; end
    init_vec = sprintf('%scos(2*pi*(0:(%s))/(%s))',s,'2^depth_bits-1','2^depth_bits');   
end

if strcmp(prog_lookup, 'on'),
    bin_pt = '32-1';
    if(strcmp(symmetric, 'on')), bin_pt = '32-2'; end

    reuse_block(blk, 'rom0', 'xps_library/Shared BRAM', ...
            'arith_type', 'Signed  (2''s comp)', ...
            'addr_width', 'depth_bits', 'data_bin_pt', bin_pt, ...
            'init_vals', init_vec, 'sample_rate', '1', ...
            'Position', [260 120 340 160]);

    reuse_block(blk, 'dat_const', 'xbsIndex_r4/Constant', ...
            'arith_type', 'Signed (2''s comp)', 'const', '0', ...
            'n_bits', '32', 'bin_pt', bin_pt, ...
            'explicit_period', 'on', 'period', '1', ...
            'ShowName', 'off', ...
            'Position', [220 133 245 147]);
    add_line(gcb, 'dat_const/1', 'rom0/2');

    reuse_block(blk, 'we_const', 'xbsIndex_r4/Constant', ...
            'arith_type', 'Boolean', 'const', '0', ...
            'ShowName', 'off', ...
            'explicit_period', 'on', 'period', '1', ...
            'Position', [220 148 245 162]);
    add_line(gcb, 'we_const/1', 'rom0/3');

    if bram_latency > 1,
        %shared BRAM has fixed delay of 1 so need to compensate for extra
        reuse_block(blk, 'del0', 'xbsIndex_r4/Delay', ...
                'latency', 'bram_latency-1', ...
                'reg_retiming', 'on', ...
                'Position', [355 130 380 150]);
        add_line(blk, 'rom0/1', 'del0/1');
        add_line(blk, 'del0/1', 'slice0/1');
    else
        add_line(blk, 'rom0/1', 'slice0/1');
    end

else,
    bin_pt = 'bit_width-1';
    if(strcmp(symmetric, 'on')), bin_pt = 'bit_width-2'; end
    
    reuse_block(blk, 'rom0', 'xbsIndex_r4/ROM', ...
        'depth', '2^depth_bits', 'initVector', init_vec, ...
        'latency', 'bram_latency', 'n_bits', 'bit_width', ...
        'bin_pt', bin_pt, ...
        'Position', [260 120 340 160]);

    add_line(blk, 'rom0/1', 'slice0/1');
end
add_line(blk, 'theta/1', 'rom0/1'); 

bin_pt = 'bit_width-1';
if(strcmp(symmetric, 'on')), bin_pt = 'bit_width-2'; end

reuse_block(blk, 're0', 'xbsIndex_r4/Reinterpret', ...
    'force_arith_type', 'on' ,'arith_type', 'Signed  (2''s comp)', ...
    'force_bin_pt', 'on', 'bin_pt', bin_pt, ...
    'Position', [460 130 510 150]);

if strcmp(func, 'sine and cosine') || strcmp(func, 'sine'), dest = sin_name;
else dest = cos_name;
end
add_line(blk, 'slice0/1', 're0/1'); 
add_line(blk, 're0/1', [dest,'/1']);

%if we have 2 outputs add the cos block
if strcmp(func, 'sine and cosine'),

    reuse_block(blk, 'slice1', 'xbsIndex_r4/Slice', ...
        'nbits', 'bit_width', 'mode', 'Upper Bit Location + Width', ...
        'bit1', '0', 'Position', [400 190 425 210]);

    s = '';
    if( strcmp(neg_cos, 'on') ), s = '-'; end
    init_vec = sprintf('%scos(2*pi*(0:(%s))/(%s))',s,'2^depth_bits-1','2^depth_bits');   
        
    if strcmp(prog_lookup, 'on'),
    bin_pt = '32-1';
    if(strcmp(symmetric, 'on')), bin_pt = '32-2'; end

    reuse_block(blk, 'rom1', 'xps_library/Shared BRAM', ...
        'arith_type', 'Signed  (2''s comp)', ...
        'addr_width', 'depth_bits', 'data_bin_pt', bin_pt, ...
        'init_vals', init_vec, 'sample_rate', '1', ...
        'Position', [260 180 340 220]);

    reuse_block(blk, 'dat_const1', 'xbsIndex_r4/Constant', ...
        'arith_type', 'Signed (2''s comp)', 'const', '0', ...
        'n_bits', '32', 'bin_pt', bin_pt, ...
        'ShowName', 'off', ...
        'explicit_period', 'on', 'period', '1', ...
        'Position', [220 193 245 207]);
    add_line(gcb, 'dat_const1/1', 'rom1/2');

    reuse_block(blk, 'we_const1', 'xbsIndex_r4/Constant', ...
        'arith_type', 'Boolean', 'const', '0', ...
        'explicit_period', 'on', 'period', '1', ...
        'ShowName', 'off', ...
        'Position', [220 208 245 222]);
    add_line(gcb, 'we_const1/1', 'rom1/3');
        
        if bram_latency > 1,
            %shared BRAM has fixed delay of 1 so need to compensate for extra
            reuse_block(blk, 'del1', 'xbsIndex_r4/Delay', ...
                    'latency', 'bram_latency-1', ...
                    'reg_retiming', 'on', ...
                    'Position', [355 190 380 210]);
            add_line(blk, 'rom1/1', 'del1/1');
            add_line(blk, 'del1/1', 'slice1/1');
        else
            add_line(blk, 'rom1/1', 'slice1/1');
        end

    else,
        bin_pt = 'bit_width-1';
        if(strcmp(symmetric, 'on')), bin_pt = 'bit_width-2'; end

        reuse_block(blk, 'rom1', 'xbsIndex_r4/ROM', ...
            'depth', '2^depth_bits', 'initVector', init_vec, ...
            'latency', 'bram_latency', 'n_bits', 'bit_width', ...
                'bin_pt', bin_pt, ...
            'Position', [260 180 340 220]);
             
            add_line(blk, 'rom1/1', 'slice1/1');
    end
    
    add_line(blk, 'theta/1', 'rom1/1'); 
    dest = cos_name;
    
    bin_pt = 'bit_width-1';
    if(strcmp(symmetric, 'on')), bin_pt = 'bit_width-2'; end

    reuse_block(blk, 're1', 'xbsIndex_r4/Reinterpret', ...
        'force_arith_type', 'on' ,'arith_type', 'Signed  (2''s comp)' , ...
        'force_bin_pt', 'on', 'bin_pt', bin_pt, ...
        'Position', [460 190 510 210]);

    add_line(blk, 'slice1/1', 're1/1');
    add_line(blk, 're1/1', [dest,'/1']);
end

% When finished drawing blocks and lines, remove all unused blocks.
clean_blocks(blk);

% Set attribute format string (block annotation)
annotation=sprintf('');
set_param(blk,'AttributesFormatString',annotation);

save_state(blk, 'defaults', defaults, varargin{:});  % Save and back-populate mask parameter values


