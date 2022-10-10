cursys = gcb;
disp(cursys);

current_taps = find_system(cursys, 'lookUnderMasks', 'all', 'FollowLinks','on', 'SearchDepth',1,...
    'masktype', 'Xilinx Slice Block');
prev_taps = length(current_taps);

if prev_taps ~= 2 * TotalTaps,
    delete_lines(cursys);
    % Add ports
    reuse_block(cursys, 'din', 'built-in/inport', {'Position', [15 123 45 137], 'Port', '1'});
    reuse_block(cursys, 'sync', 'built-in/inport', {'Position', [15 28 45 42], 'Port', '2'});
    reuse_block(cursys, 'dout', 'built-in/outport', {'Position', [500 25*TotalTaps+100 530 25*TotalTaps+115], 'Port', '1'});
    reuse_block(cursys, 'sync_out', 'built-in/outport', {'Position', [150 28 180 42], 'Port', '2'});
    % Add Static Blocks
    reuse_block(cursys, 'adder_tree1', 'pfb_library/adder_tree', ...
        {'num_inputs', 'TotalTaps', 'latency', 'add_latency', ...
        'Position', [200 114 350 50*TotalTaps+114]});
    reuse_block(cursys, 'adder_tree2', 'pfb_library/adder_tree', ...
        {'num_inputs', 'TotalTaps', 'latency', 'add_latency', ...
        'Position', [200 164+50*TotalTaps 350 164+100*TotalTaps]});
    reuse_block(cursys, 'convert1', 'xbsIndex_r3/Convert', ...
        {'arith_type', 'Signed  (2''s comp)', 'n_bits', 'BitWidthOut', ...
        'bin_pt', 'BitWidthOut-1', ...
        'overflow', 'Saturate', 'latency', 'add_latency', ...
        'Position', [400 25*TotalTaps+114 430 25*TotalTaps+128]});
    reuse_block(cursys, 'convert2', 'xbsIndex_r3/Convert', ...
        {'arith_type', 'Signed  (2''s comp)', 'n_bits', 'BitWidthOut', ...
        'bin_pt', 'BitWidthOut-1', ...
        'overflow', 'Saturate', 'latency', 'add_latency', ...
        'Position', [400 158+25*TotalTaps 430 172+25*TotalTaps]});
    % Add lines
    add_line(cursys, 'adder_tree1/1', 'convert1/1');
    add_line(cursys, 'adder_tree2/1', 'convert2/1');
    set_param([cursys,'/ri_to_c'], 'Position', [450 114+25*TotalTaps 480 144+25*TotalTaps]);
    add_line(cursys, 'convert1/1', 'ri_to_c/1');
    add_line(cursys, 'convert2/1', 'ri_to_c/2');
    add_line(cursys, 'ri_to_c/1', 'dout/1');
    reuse_block(cursys, 'delay', 'xbsIndex_r3/Delay', ...
        {'latency', '(log2(TotalTaps)+1)*add_latency', ...
        'Position', [80 14 120 56]});
    add_line(cursys, 'sync/1', 'delay/1');
    add_line(cursys, 'delay/1', 'sync_out/1');

    for i=0:TotalTaps-1,
        for j=1:2,
            slice_name = ['Slice', num2str(i),'_',num2str(j)];
            reuse_block(cursys, slice_name, 'xbsIndex_r3/Slice', ...
                {'mode', 'Upper Bit Location + Width', 'nbits', 'CoeffBitWidth + BitWidthIn', ...
                'base0', 'MSB of Input', 'base1', 'MSB of Input', ...
                'bit1', ['-',num2str(2*i+j-1),'*(CoeffBitWidth + BitWidthIn)'], 'Position', [70 50*i+25*j+116 115 50*i+25*j+128]});
            add_line(cursys, 'din/1', [slice_name, '/1']);
            reint_name = ['Reint',num2str(i),'_',num2str(j)];
            reuse_block(cursys, reint_name, 'xbsIndex_r3/Reinterpret', ...
                {'force_arith_type', 'on', 'arith_type', 'Signed  (2''s comp)', ...
                'force_bin_pt', 'on', 'bin_pt', 'CoeffBitWidth + BitWidthIn - 2', ...
                'Position', [130 50*i+25*j+116 160 50*i+25*j+128]});
        	add_line(cursys, [slice_name, '/1'], [reint_name, '/1']);
            add_line(cursys, [reint_name, '/1'], ['adder_tree',num2str(j),'/',num2str(i+1)]);
        end
    end
    clean_blocks(cursys);
end
 
% Set dynamic parameters
if ~strcmp(get_param([cursys,'/convert1'], 'quantization'), quantization),
    set_param([cursys,'/convert1'], 'quantization', quantization);
    set_param([cursys,'/convert2'], 'quantization', quantization);
end