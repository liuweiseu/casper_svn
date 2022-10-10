cursys = gcb;
disp(cursys);

% Set coefficient vector
%fwidth = 1.5;
alltaps = TotalTaps*2^PFBSize;
windowval = transpose(window(WindowType, alltaps));
total_coeffs = windowval .* sinc(fwidth*([0:alltaps-1]/(2^PFBSize)-TotalTaps/2));
%disp(['coeffs=',mat2str(total_coeffs)]);
for i=1:alltaps/2^n_inputs,
    buf(i)=total_coeffs((i-1)*2^n_inputs + nput + 1);
end
%disp(['inp',num2str(nput),'=',mat2str(buf)]);
prev_taps = length(find_system(cursys, 'lookUnderMasks', 'all', 'FollowLinks','on','masktype', 'Xilinx Single Port Read-Only Memory'));

if prev_taps ~= TotalTaps,
    delete_lines(cursys);
    % Add Ports
    reuse_block(cursys, 'din', 'built-in/inport', {'Position', [235 28 265 42], 'Port', '1'});
    reuse_block(cursys, 'sync', 'built-in/inport', {'Position', [15 93 45 107], 'Port', '2'});
    reuse_block(cursys, 'dout', 'built-in/outport', {'Position', [360 28 390 42], 'Port', '1'});
    reuse_block(cursys, 'sync_out', 'built-in/outport', {'Position', [130 28 160 42], 'Port', '2'});
    reuse_block(cursys, 'coeff', 'built-in/outport', {'Position', [450 343 480 357], 'Port', '3'});

    % Add Static Blocks
    reuse_block(cursys, 'Delay', 'xbsIndex_r3/Delay', ...
        {'latency', 'bram_latency+1', 'Position', [65 12 110 58]});
    reuse_block(cursys, 'Counter', 'xbsIndex_r3/Counter', ...
        {'cnt_type', 'Free Running', 'n_bits', 'PFBSize-n_inputs', 'arith_type', 'Unsigned', ...
        'rst', 'on', 'explicit_period', 'on', 'Position', [65 75 115 125]});
    reuse_block(cursys, 'Delay1', 'xbsIndex_r3/Delay', ...
        {'latency', 'bram_latency+1', 'Position', [290 12 335 58]});
    reuse_block(cursys, 'Concat', 'xbsIndex_r3/Concat', ...
        {'num_inputs', num2str(TotalTaps), 'Position', [310 97 365 643]});
    reuse_block(cursys, 'Register', 'xbsIndex_r3/Register', ...
        {'Position', [385 325 430 375]});

    add_line(cursys, 'din/1', 'Delay1/1');
    add_line(cursys, 'Delay1/1', 'dout/1');
    add_line(cursys, 'sync/1', 'Counter/1');
    add_line(cursys, 'sync/1', 'Delay/1');
    add_line(cursys, 'Delay/1', 'sync_out/1');
    add_line(cursys, 'Concat/1', 'Register/1');
    add_line(cursys, 'Register/1', 'coeff/1');

    % Add Dynamic Blocks
    for a=1:TotalTaps,
    	blkname = ['ROM', num2str(a)];
        v = ['buf(', num2str(a-1), '*2^(PFBSize-n_inputs) + 1:', num2str(a), '*2^(PFBSize-n_inputs))'];
    	reuse_block(cursys, blkname, 'xbsIndex_r3/ROM', ...
            {'depth', '2^(PFBSize-n_inputs)', 'initVector', v, 'arith_type', 'Signed  (2''s comp)', ...
            'n_bits', 'CoeffBitWidth', 'bin_pt', 'CoeffBitWidth-1', ...
            'latency', 'bram_latency', 'use_rpm','on', 'Position', [150 65*(a-1)+74 200 65*(a-1)+126]});
    	add_line(cursys, 'Counter/1', [blkname, '/1']);
    	reintname = ['Reinterpret', num2str(a)];
    	reuse_block(cursys, reintname, 'xbsIndex_r3/Reinterpret', ...
            {'Position', [220 65*(a-1)+84 260 65*(a-1)+116]});
    	add_line(cursys, [blkname, '/1'], [reintname, '/1']);
    	add_line(cursys, [reintname, '/1'], ['Concat/', num2str(a)]);
    end
    clean_blocks(cursys);
end

if ~strcmp(get_param([cursys,'/ROM1'], 'distributed_mem'), CoeffDistMem),
	for a=1:TotalTaps,
        blkname = ['ROM', num2str(a)];
        set_param([cursys,'/',blkname], 'distributed_mem', CoeffDistMem);
    end
end

