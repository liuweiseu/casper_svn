cursys = gcb;
disp(cursys);

outports = find_system(cursys, 'lookUnderMasks', 'on', 'FollowLinks','on','SearchDepth',1,'BlockType', 'Outport');
num_outports = length(outports);

if num_outports ~= 2^n_inputs + 2,
    delete_lines(cursys);
    reuse_block(cursys, 'en', 'built-in/inport', {'Position', [15 23 45 37], 'Port', '1'});
    reuse_block(cursys, 'sync_in', 'built-in/inport', {'Position', [15 100 45 114], 'Port', '2'});
    reuse_block(cursys, 'valid', 'built-in/outport', {'Position', [215 23 245 37], 'Port', '1'});
    reuse_block(cursys, 'sync_out', 'built-in/outport', {'Position', [215 100 245 114], 'Port', '2'});
    for i=1:2^n_inputs,
        reuse_block(cursys, ['In',num2str(i)], 'built-in/inport', {'Position', [15 i*80+145 45 159+80*i]});
        reuse_block(cursys, ['Out',num2str(i)], 'built-in/outport', {'Position', [215 i*80+145 245 159+80*i]});
    end
    reuse_block(cursys, 'Delay', 'xbsIndex_r3/Delay', {'Position', [95 5 125 50], 'latency', num2str(n_inputs + 1)});
    add_line(cursys, 'en/1', 'Delay/1');
    add_line(cursys, 'Delay/1', 'valid/1');
    reuse_block(cursys, 'Counter', 'xbsIndex_r3/Counter', ...
        {'Position', [95 85 125 120], 'cnt_type', 'Free Running', 'n_bits', 'n_inputs', ...
        'arith_type', 'Unsigned', 'rst', 'on', 'en', 'on'});
    add_line(cursys, 'en/1', 'Counter/2');
    add_line(cursys, 'sync_in/1', 'Counter/1');
    reuse_block(cursys, 'bs', 'fft_library/barrel_switcher', ...
        {'Position', [85 195 185 390], 'n_inputs', num2str(n_inputs)});
    add_line(cursys, 'sync_in/1', 'bs/2');
    add_line(cursys, 'Counter/1', 'bs/1');
    add_line(cursys, 'bs/1', 'sync_out/1');
    for i=1:2^n_inputs,
        add_line(cursys, ['In', num2str(i), '/1'], ['bs/', num2str(i+2)]);
        add_line(cursys, ['bs/', num2str(i+1)], ['Out', num2str(i), '/1']);
    end
    clean_blocks(cursys);
end
counter = [cursys,'/Counter'];
if ~strcmp(get_param(counter, 'operation'), step_dir),
    set_param(counter, 'operation', step_dir);
end
