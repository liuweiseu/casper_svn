cursys = gcb;
disp(cursys);
biplex_len = FFTSize - n_inputs;

outports = find_system(cursys, 'lookUnderMasks', 'on', 'FollowLinks','on','masktype', 'dbuf_unscr');
prev_inputs = length(outports);

if 2^n_inputs ~= prev_inputs,
    delete_lines(cursys);
    
    % Add ports
    reuse_block(cursys, 'sync', 'built-in/inport', {'Position', [30 60 60 74], 'Port', '1'});
    reuse_block(cursys, 'sync_out', 'built-in/inport', {'Position', [500 40 530 54], 'Port', '1'});
    for i=1:2^n_inputs,
        reuse_block(cursys, ['In',num2str(i)], 'built-in/inport', {'Position', [30 20*i+60 60 20*i+74], 'Port', num2str(i+1)});
        reuse_block(cursys, ['Out',num2str(i)], 'built-in/outport', {'Position', [500 55*i+40 530 55*i+54], 'Port', num2str(i+1)});
    end

    % Add static blocks
    reuse_block(cursys, 'Constant', 'xbsIndex_r3/Constant', ...
        {'const', '1', 'arith_type', 'Boolean', 'explicit_period', 'on', ...
        'Position', [25 35 60 55]});
    reuse_block(cursys, 'step_barrel_switcher', 'fft_library/step_barrel_switcher', ...
        {'step_dir', 'Down', 'n_inputs', 'n_inputs', ...
        'Position', [85 30 170 2^n_inputs*20+80]});
    reuse_block(cursys, 'sync_delay', 'fft_library/sync_delay', ...
        {'DelayLen', '2^biplex_len + bram_latency + 2', 'Position', [235 24 275 66]});
    reuse_block(cursys, 'Counter', 'xbsIndex_r3/Counter', ...
        {'Position', [290 75 325 95], 'cnt_type', 'Free Running', 'n_bits', 'biplex_len', ...
        'arith_type', 'Unsigned', 'rst', 'on'});
    reuse_block(cursys, 'slice', 'xbsIndex_r3/Slice', ...
        {'mode', 'Upper Bit Location + Width', 'nbits', 'n_inputs', ...
        'Position', [335 75 355 95]});
    reuse_block(cursys, 'barrel_switcher', 'fft_library/barrel_switcher', ...
        {'n_inputs', 'n_inputs', 'Position', [350 30 435 2^n_inputs*20+80]});
    
    % Add dynamic blocks
    for i=1:2^n_inputs,
        reuse_block(cursys, ['dbuf_unscr', num2str(i)], 'fft_library/dbuf_unscr', ...
            {'FFTSize', 'FFTSize', 'n_inputs', 'n_inputs', 'banknum', num2str(i), ...
            'bram_latency', 'bram_latency', 'Position', [240 65*i+12 275 65*i+48]});
    end

    % Add static lines
    add_line(cursys, 'Constant/1', 'step_barrel_switcher/1');
    add_line(cursys, 'Counter/1', 'slice/1');
    add_line(cursys, 'slice/1', 'barrel_switcher/1');
    add_line(cursys, 'sync/1', 'step_barrel_switcher/2');
    add_line(cursys, 'step_barrel_switcher/2', 'sync_delay/1');
    add_line(cursys, 'sync_delay/1', 'barrel_switcher/2');
    add_line(cursys, 'sync_delay/1', 'Counter/1');
    add_line(cursys, 'barrel_switcher/1', 'sync_out/1');

    % Add dynamic lines
    for i=1:2^n_inputs,
        in_name = ['In',num2str(i)];
        out_name = ['Out',num2str(i)];
        dbuf_name = ['dbuf_unscr', num2str(i)];
        add_line(cursys, [in_name,'/1'], ['step_barrel_switcher/',num2str(i+2)]);
        add_line(cursys, 'step_barrel_switcher/2', [dbuf_name,'/2']);
        add_line(cursys, ['step_barrel_switcher/',num2str(i+2)], [dbuf_name,'/1']);
        add_line(cursys, [dbuf_name,'/1'], ['barrel_switcher/',num2str(i+2)]);
        add_line(cursys, ['barrel_switcher/',num2str(i+1)], [out_name,'/1']);
    end

    clean_blocks(cursys);
end
