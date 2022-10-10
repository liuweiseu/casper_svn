function sharedrom_init(blk, varargin)
%outwidth
%depth

defaults = {};
%if same_state(blk, 'defaults', defaults, varargin{:}), return, end
check_mask_type(blk, 'sharedrom');
munge_block(blk, varargin{:});

outwidth = get_var('outwidth', 'defaults', defaults, varargin{:});
depth = get_var('depth', 'defaults', defaults, varargin{:});

numbrams = ceil(outwidth/32);   % Each BRAM is 32 bits wide, so we need at least this many
bramdepth = max(depth,2^11);    % BRAM must have depth at least 2^11
bram_addr_width = nextpow2(bramdepth);
addrwidth = nextpow2(depth);    % Input address width
leftover = mod(outwidth,32);
if leftover==0,
  leftover = 32;
end

delete_lines(blk);

reuse_block(blk, 'addr', 'built-in/inport', 'Position', [100 100 130 170]);
reuse_block(blk, 'dout', 'built-in/outport', 'Position', [700 100 730 170]);

reuse_block(blk, 'Convert', 'xbsIndex_r3/Convert', 'arith_type', 'Unsigned', 'n_bits', num2str(addrwidth),...
    'bin_pt', num2str(0), 'quantization', 'Truncate', 'overflow', 'Wrap', 'latency', num2str(0), ...
    'Position', [150 100 180 170]);
add_line(blk, 'addr/1', 'Convert/1');   

reuse_block(blk, 'dinz','xbsIndex_r3/Constant', ...
    'const', num2str(0), 'arith_type', 'Unsigned', 'n_bits', num2str(32), ...
    'bin_pt', num2str(0), ...
    'Position', [150 280 170 300]);
reuse_block(blk, 'wez','xbsIndex_r3/Constant', ...
    'const', num2str(0), 'arith_type', 'Boolean', ...
    'Position', [150 310 170 330]);

if numbrams > 1,
  reuse_block(blk, 'Concat', 'xbsIndex_r3/Concat', 'num_inputs', num2str(numbrams),...
      'Position', [650 100 680 550]);
  add_line(blk, 'Concat/1', 'dout/1');
  for j = 2:numbrams,
      romnum = numbrams-j;
      reuse_block(blk, ['SharedROM', num2str(romnum)], 'xps_library/Shared BRAM', ...
          'arith_type', 'Unsigned', 'addr_width', num2str(bram_addr_width), 'data_bin_pt', num2str(0), ...
          'Position', [300 50+j*200 400 170+j*200]);
      add_line(blk, 'Convert/1', ['SharedROM', num2str(romnum), '/1']);
      add_line(blk, 'dinz/1', ['SharedROM', num2str(romnum), '/2']);
      add_line(blk, 'wez/1', ['SharedROM', num2str(romnum), '/3']);
      add_line(blk, ['SharedROM', num2str(romnum), '/1'], ['Concat/', num2str(j)]);
  end
end

reuse_block(blk, ['SharedROM', num2str(numbrams-1)], 'xps_library/Shared BRAM', ...
    'arith_type', 'Unsigned', 'addr_width', num2str(bram_addr_width), 'data_bin_pt', num2str(0), ...
    'Position', [300 50 400 170]);
reuse_block(blk, 'Slice', 'xbsIndex_r3/Slice', 'mode', 'Lower Bit Location + Width', ...
    'nbits', num2str(leftover), 'bit0', num2str(0), 'base0', 'LSB of Input', ...
    'Position', [420 70 480 100]);
    
add_line(blk, 'Convert/1', ['SharedROM', num2str(numbrams-1), '/1']);
add_line(blk, 'dinz/1', ['SharedROM', num2str(numbrams-1), '/2']);
add_line(blk, 'wez/1', ['SharedROM', num2str(numbrams-1), '/3']);
add_line(blk, ['SharedROM', num2str(numbrams-1), '/1'], 'Slice/1');

if numbrams == 1,
  add_line(blk, 'Slice/1', 'dout/1');
else,
  add_line(blk, 'Slice/1', 'Concat/1');
end



clean_blocks(blk);

save_state(blk, 'defaults', defaults, varargin{:});