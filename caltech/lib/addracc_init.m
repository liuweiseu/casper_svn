function addracc_init(blk, varargin)
%ninputs
%in_width
%out_width
%bin_pt
%add_latency
%bram_latency

defaults = {};
%disp('hi from addracc_init')
if same_state(blk, 'defaults', defaults, varargin{:}), return, end
check_mask_type(blk, 'addracc');
munge_block(blk, varargin{:});
ninputs = get_var('ninputs', 'defaults', defaults, varargin{:});
in_width = get_var('in_width', 'defaults', defaults, varargin{:});
out_width = get_var('out_width', 'defaults', defaults, varargin{:});
bin_pt = get_var('bin_pt', 'defaults', defaults, varargin{:});
depth = get_var('depth', 'defaults', defaults, varargin{:});
add_latency = get_var('add_latency', 'defaults', defaults, varargin{:});
bram_latency = get_var('bram_latency', 'defaults', defaults, varargin{:});

delete_lines(blk);

% Inputs
reuse_block(blk, 'addr', 'built-in/inport', 'Position', [250 50 280 70]);
reuse_block(blk, 'din', 'built-in/inport', 'Position', [50 150 80 170]);
reuse_block(blk, 'accen', 'built-in/inport', 'Position', [120 550 150 570]);
reuse_block(blk, 'wren', 'built-in/inport', 'Position', [450 50 480 70]);
% Outputs
reuse_block(blk, 'dout', 'built-in/outport', 'Position', [800 250 830 270]);
reuse_block(blk, 'wraddr', 'built-in/outport', 'Position', [550 350 580 370]);
% Static Blocks
reuse_block(blk, 'AddrDelay', 'xbsIndex_r3/Delay', ... 
    'latency', num2str(add_latency+bram_latency), ...
    'reg_retiming', 'on', ...
    'Position', [300 50 330 70]);
add_line(blk, 'addr/1', 'AddrDelay/1');

reuse_block(blk, 'Uncram', 'gavrt_library/uncram', ...
    'num_slice', num2str(ninputs), 'slice_width', num2str(in_width), ...
    'bin_pt', num2str(bin_pt), 'Position', [90 100 150 400]);
add_line(blk, 'din/1', 'Uncram/1');

reuse_block(blk, 'Concat', 'xbsIndex_r3/Concat', 'num_inputs', num2str(ninputs), ...
    'Position', [690 100 720 400]);
add_line(blk, 'Concat/1', 'dout/1');

for j = 1:ninputs,
    reuse_block(blk, ['AddWithEnable', num2str(j)], 'gavrt_library/AddWithEnable', ...
        'acc_width', num2str(out_width), 'bin_pt', num2str(bin_pt), 'add_latency', num2str(add_latency), ...
        'Position', [250 200*j-50 300 200*j]);
    reuse_block(blk, ['DualPortRam', num2str(j)], 'xbsIndex_r3/Dual Port RAM', ...
        'depth', num2str(depth), 'write_mode_A', 'Read Before Write', 'write_mode_B', 'Read Before Write', ...
        'latency', num2str(bram_latency), 'explicit_period', 'on', 'period', num2str(1), ...
        'distributed_mem', 'off', ...
        'Position', [600 200*j-150 650 200*j]);
    reuse_block(blk, ['Dummy', num2str(j)], 'xbsIndex_r3/Constant', 'n_bits', num2str(out_width), ...
        'arith_type', 'Unsigned', ...
        'bin_pt', num2str(0), 'Position', [550 200*j-40 580 200*j-20]);
    reuse_block(blk, ['Zero', num2str(j)], 'xbsIndex_r3/Constant', 'arith_type', 'Boolean', ...
        'const', num2str(0), 'Position', [550 200*j-10 580 200*j+10]);

    add_line(blk, ['Uncram/', num2str(j)], ['AddWithEnable', num2str(j), '/1']);
    add_line(blk, 'accen/1', ['AddWithEnable', num2str(j), '/2']);
    add_line(blk, ['DualPortRam', num2str(j), '/2'], ['AddWithEnable', num2str(j), '/3']); %Change if allow dist ram
    add_line(blk, ['AddWithEnable', num2str(j), '/1'], ['DualPortRam', num2str(j), '/2']);
    add_line(blk, 'AddrDelay/1', ['DualPortRam', num2str(j), '/1']);
    add_line(blk, 'wren/1', ['DualPortRam', num2str(j), '/3']);
    add_line(blk, 'addr/1', ['DualPortRam', num2str(j), '/4']);
    add_line(blk, ['Dummy', num2str(j), '/1'], ['DualPortRam', num2str(j), '/5']); % Change if allow dist ram
    add_line(blk, ['Zero', num2str(j), '/1'], ['DualPortRam', num2str(j), '/6']); % Change if allow dist ram
    add_line(blk, ['DualPortRam', num2str(j), '/2'], ['Concat/' num2str(j)]); % Change if allow dist ram
end

clean_blocks(blk);
fmtstr = sprintf('depth=%d, inputs=%d', depth, ninputs);
set_param(blk, 'AttributesFormatString', fmtstr);
save_state(blk, 'defaults', defaults, varargin{:});