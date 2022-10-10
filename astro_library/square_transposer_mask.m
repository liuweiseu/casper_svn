cursys = gcb;
disp(cursys);

if n_inputs < 1,
    error('Number of inputs must be 2^1 or greater.');
end

outports = find_system(cursys, 'lookUnderMasks', 'on', 'FollowLinks','on','SearchDepth',1,'BlockType', 'Outport');
num_outports = length(outports);

if num_outports ~= 2^n_inputs + 1,
    delete_lines(cursys);
    % Add ports
    reuse_block(cursys, 'sync', 'built-in/inport', {'Position', [15 10 45 24], 'Port', '1'});
    reuse_block(cursys, 'sync_out', 'built-in/outport', {'Position', [315 10 345 24], 'Port', '1'});
    for i=1:2^n_inputs,
        reuse_block(cursys, ['In',num2str(i)], 'built-in/inport', {'Position', [15 i*80+95 45 109+80*i]});
        reuse_block(cursys, ['Out',num2str(i)], 'built-in/outport', {'Position', [315 95+i*80 345 109+80*i]});
    end
    % Add blocks
    reuse_block(cursys, 'counter', 'xbsIndex_r3/Counter', ...
       {'Position', [95 85 125 120], 'cnt_type', 'Free Running', 'n_bits', 'n_inputs', ...
        'arith_type', 'Unsigned', 'rst', 'on', 'operation', 'Down'});
    reuse_block(cursys, 'delay0', 'xbsIndex_r3/Delay', ...
        {'latency', '2^n_inputs - 1', 'Position', [270 0 300 30]});
    reuse_block(cursys, 'barrel_switcher', 'fft_library/barrel_switcher', ...
        {'n_inputs', 'n_inputs', 'Position', [120 150 240 300]});
    for j=1:2^n_inputs,
        reuse_block(cursys, ['Delayf', num2str(j)], 'xbsIndex_r3/Delay', ...
            {'latency', num2str(j-1), 'Position', [60 j*80+95 90 j*80+125]});
        reuse_block(cursys, ['Delayb', num2str(j)], 'xbsIndex_r3/Delay', ...
            {'latency', num2str(2^n_inputs-j), 'Position', [270 j*80+95 300 j*80+125]});
    end
    % Add lines
    add_line(cursys, 'sync/1', 'counter/1');
    add_line(cursys, 'sync/1', 'barrel_switcher/2');
    add_line(cursys, 'counter/1', 'barrel_switcher/1');
    add_line(cursys, 'barrel_switcher/1', 'delay0/1');
    add_line(cursys, 'delay0/1', 'sync_out/1');
    for j=1:2^n_inputs,
        if j == 1,
            dport = 3;
        else,
            dport = (2^n_inputs - j + 2) + 2;
        end
        add_line(cursys, ['In', num2str(j),'/1'], ['Delayf', num2str(j),'/1']);
        add_line(cursys, ['Delayf', num2str(j), '/1'], ['barrel_switcher/', num2str(dport)]);
        add_line(cursys, ['barrel_switcher/',num2str(j+1)], ['Delayb',num2str(j),'/1']);
        add_line(cursys, ['Delayb',num2str(j),'/1'], ['Out',num2str(j),'/1']);
    end
    
    clean_blocks(cursys);
end

