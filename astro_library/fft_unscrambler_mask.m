cursys = gcb;
disp(cursys);

if 2*n_inputs >= FFTSize,
    errordlg('FFT Unscrambler: Number of inputs must be >= FFT size / 2.');
end
part_mat = [0:2^(FFTSize-2*n_inputs)-1]*2^(n_inputs);
map_mat = [];
for i=0:2^n_inputs-1,
    map_mat = [map_mat, part_mat+i];
end
map_str = mat2str(map_mat);
old_map_str = get_param([cursys,'/reorder'], 'map');

if ~strcmp(map_str, old_map_str),
    delete_lines(cursys);
    
    % Add ports
    reuse_block(cursys, 'sync', 'built-in/inport', {'Position', [30 60 60 74], 'Port', '1'});
    reuse_block(cursys, 'sync_out', 'built-in/outport', {'Position', [500 40 530 54], 'Port', '1'});
    for i=1:2^n_inputs,
        reuse_block(cursys, ['In',num2str(i)], 'built-in/inport', {'Position', [30 20*i+60 60 20*i+74], 'Port', num2str(i+1)});
        reuse_block(cursys, ['Out',num2str(i)], 'built-in/outport', {'Position', [500 55*i+40 530 55*i+54], 'Port', num2str(i+1)});
    end

    % Add static blocks
    reuse_block(cursys, 'square_transposer', 'reorder_library/square_transposer', ...
        {'n_inputs', 'n_inputs', 'Position', [85 30 170 2^n_inputs*20+80]});
    reuse_block(cursys, 'reorder', 'reorder_library/reorder', ...
        {'map', map_str, 'bram_latency', 'bram_latency', ...
        'n_inputs', '2^n_inputs', 'map_latency', '1', 'Position', [265 37 360 93]});
    reuse_block(cursys, 'const', 'xbsIndex_r3/Constant', ...
        {'arith_type', 'Boolean', 'explicit_period', 'on', 'Position', [225 57 250 73]});
    
    % Add static lines
    add_line(cursys, 'sync/1', 'square_transposer/1');
    add_line(cursys, 'square_transposer/1', 'reorder/1');
    add_line(cursys, 'reorder/1', 'sync_out/1');
    add_line(cursys, 'const/1', 'reorder/2');
    
    % Add dynamic lines
    for i=1:2^n_inputs,
        in_name = ['In',num2str(i)];
        out_name = ['Out',num2str(i)];
        add_line(cursys, [in_name,'/1'], ['square_transposer/',num2str(i+1)]);
        add_line(cursys, ['square_transposer/',num2str(i+1)], ['reorder/',num2str(i+2)]);
        add_line(cursys, ['reorder/',num2str(i+2)], [out_name,'/1']);
    end

    clean_blocks(cursys);
end
