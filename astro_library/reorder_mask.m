cursys = gcb;
disp(cursys);

if n_inputs < 1
    error('Number of inputs cannot be less than 1.');
end

n_bits = log2(length(map));
order = compute_order(map);
order_bits = ceil(log2(order));

maps = find_system(cursys, 'lookUnderMasks', 'on', 'FollowLinks', 'on', 'SearchDepth', 1, 'masktype', 'cnt_map');
inports = find_system(cursys, 'lookUnderMasks','on', 'FollowLinks','on', 'SearchDepth',1, 'BlockType','Inport');
outports = find_system(cursys, 'lookUnderMasks','on', 'FollowLinks','on', 'SearchDepth',1, 'BlockType','Outport');
num_maps = length(maps);

if order > 16
    error('Reorder can only support a max mapping order of 16.');
end

if (order ~= num_maps + 1) || (length(inports) ~= n_inputs+2) || (length(outports) ~= n_inputs+2),
    delete_lines(cursys);
    % Add Static Ports
    reuse_block(cursys, 'sync', 'built-in/inport', {'Position', [40    65    70    79], 'Port', '1'});
    reuse_block(cursys, 'en', 'built-in/inport', {'Position', [25   120    55   134], 'Port', '2'});
    reuse_block(cursys, 'sync_out', 'built-in/outport', {'Position', [165   173   195   187], 'Port', '1'});
    reuse_block(cursys, 'valid', 'built-in/outport', {'Position', [705   13   735   27], 'Port', '2'});

    % Add Static Blocks
    reuse_block(cursys, 'pre_sync_delay', 'xbsIndex_r3/Delay', ...
        {'Position', [ 55   159    75   201], 'latency', '(order-1)*map_latency'});
    reuse_block(cursys, 'sync_en_delay', 'delay_lib/sync_en_delay', ...
        {'Position', [ 85   159   125   201], 'DelayLen', 'length(map)'});
    reuse_block(cursys, 'post_sync_delay', 'xbsIndex_r3/Delay', ...
        {'Position', [135   159   155   201], 'latency', 'bram_latency+1'});
    reuse_block(cursys, 'delay_valid', 'xbsIndex_r3/Delay', ...
        {'Position', [495    13    525    27], 'latency', 'bram_latency+1'});
    reuse_block(cursys, 'delay_we', 'xbsIndex_r3/Delay', ...
        {'Position', [305   115    345   135], 'latency', '(order-1)*map_latency'});

    if order == 1
        add_line(cursys, 'sync/1', 'pre_sync_delay/1');
        add_line(cursys, 'pre_sync_delay/1', 'sync_en_delay/1');
        add_line(cursys, 'sync_en_delay/1', 'post_sync_delay/1');
        add_line(cursys, 'en/1', 'sync_en_delay/2');
        add_line(cursys, 'post_sync_delay/1', 'sync_out/1');
        add_line(cursys, 'en/1', 'delay_we/1');
        add_line(cursys, 'delay_we/1', 'delay_valid/1');
        add_line(cursys, 'delay_valid/1', 'valid/1');

        for i=1:n_inputs,
            % Ports
            reuse_block(cursys, ['din',num2str(i-1)], 'built-in/inport', ...
                {'Position', [495    80*i+3   525    80*i+17], 'Port', num2str(2+i)});
            reuse_block(cursys, ['dout', num2str(i-1)], 'built-in/outport', ...
                {'Position', [705    80*i+3   735    80*i+17], 'Port', num2str(2+i)});

            % Delays
            reuse_block(cursys, ['delay_din',num2str(i-1)], 'xbsIndex_r3/Delay', ...
                {'Position', [550    80*i    590    80*i+20], 'latency', '(order-1)*map_latency+1'});
            reuse_block(cursys, ['delay_din_bram',num2str(i-1)], 'delay_lib/bram_en_delay_x', ...
                {'Position', [620    80*i    660    80*i+20], 'DelayLen', 'length(map)', 'bram_latency', 'bram_latency'});

            % Wires
            add_line(cursys, ['din',num2str(i-1),'/1'], ['delay_din',num2str(i-1),'/1']);
            add_line(cursys, ['delay_din',num2str(i-1),'/1'], ['delay_din_bram',num2str(i-1),'/1']);
            add_line(cursys, ['delay_din_bram',num2str(i-1),'/1'], ['dout',num2str(i-1),'/1']);
            add_line(cursys, 'delay_we/1', ['delay_din_bram',num2str(i-1),'/2']);
        end

        clean_blocks(cursys);

        return;
    end

    reuse_block(cursys, 'Counter', 'xbsIndex_r3/Counter', ...
        {'Position', [95    56   145   109],'n_bits', 'n_bits + order_bits', 'cnt_type', 'Count Limited', ...
        'arith_type', 'Unsigned', 'cnt_to', '2^n_bits * order - 1', ...
        'en', 'on', 'rst', 'on'});
    reuse_block(cursys, 'Slice1', 'xbsIndex_r3/Slice', ...
        {'Position', [170    37   200    53], 'mode', 'Upper Bit Location + Width', ...
        'nbits', 'order_bits'});
    reuse_block(cursys, 'Slice2', 'xbsIndex_r3/Slice', ...
        {'Position', [170    77   200    93], 'mode', 'Lower Bit Location + Width', ...
        'nbits', 'n_bits'});
    reuse_block(cursys, 'Mux', 'xbsIndex_r3/Mux', ...
        {'Position', [415    34   440    62+20*order], 'inputs', num2str(order), 'latency', '1'});
    reuse_block(cursys, 'delay_sel', 'xbsIndex_r3/Delay', ...
        {'Position', [305    37    345    53], 'latency', '(order-1)*map_latency'});
    reuse_block(cursys, 'delay_d0', 'xbsIndex_r3/Delay', ...
        {'Position', [305    77    345    93], 'latency', '(order-1)*map_latency'});


    % Add Dynamic Ports and Blocks
    for i=1:n_inputs,
        % Ports
        reuse_block(cursys, ['din',num2str(i-1)], 'built-in/inport', ...
            {'Position', [495    80*i+3   525    80*i+17], 'Port', num2str(2+i)});
        reuse_block(cursys, ['dout', num2str(i-1)], 'built-in/outport', ...
            {'Position', [705    80*i+3   735    80*i+17], 'Port', num2str(2+i)});

        % BRAMS
        reuse_block(cursys, ['delay_din',num2str(i-1)], 'xbsIndex_r3/Delay', ...
            {'Position', [550    80*i    590    80*i+20], 'latency', '(order-1)*map_latency+1'});
        reuse_block(cursys, ['bram',num2str(i-1)], 'xbsIndex_r3/Single Port RAM', ...
            {'Position', [615    80*i-17   680   80*i+37], 'depth', '2^n_bits', 'write_mode', 'Read Before Write', ...
            'latency', 'bram_latency'});
    end

    % Add Maps
    for i=1:order-1,
        mapname = ['cnt_map', num2str(i)];
        reuse_block(cursys, mapname, 'reorder_library/cnt_map', ...
            {'Position', [230  125+50*i   270    145+50*i], 'n_bits', 'n_bits', 'map', 'map', 'latency', 'map_latency'});
        reuse_block(cursys, ['delay_',mapname], 'xbsIndex_r3/Delay', ...
            {'Position', [305   125+50*i    345   145+50*i], 'latency', [num2str(order-(i+1)),'*map_latency']});
    end

    % Add static wires
    add_line(cursys, 'sync/1', 'Counter/1');
    add_line(cursys, 'en/1', 'Counter/2');
    add_line(cursys, 'Counter/1', 'Slice1/1');
    add_line(cursys, 'Counter/1', 'Slice2/1');
    add_line(cursys, 'Slice1/1', 'delay_sel/1');
    add_line(cursys, 'delay_sel/1', 'Mux/1');
    add_line(cursys, 'Slice2/1', 'delay_d0/1');
    add_line(cursys, 'delay_d0/1', 'Mux/2');
    add_line(cursys, 'sync/1', 'pre_sync_delay/1');
    add_line(cursys, 'pre_sync_delay/1', 'sync_en_delay/1');
    add_line(cursys, 'sync_en_delay/1', 'post_sync_delay/1');
    add_line(cursys, 'en/1', 'sync_en_delay/2');
    add_line(cursys, 'post_sync_delay/1', 'sync_out/1');
    add_line(cursys, 'en/1', 'delay_we/1');
    add_line(cursys, 'delay_we/1', 'delay_valid/1');
    add_line(cursys, 'delay_valid/1', 'valid/1');

    % Add dynamic wires
    for i=1:n_inputs
        add_line(cursys, 'delay_we/1', ['bram',num2str(i-1),'/3']);
        add_line(cursys, 'Mux/1', ['bram',num2str(i-1),'/1']);
        add_line(cursys, ['din',num2str(i-1),'/1'], ['delay_din',num2str(i-1),'/1']);
        add_line(cursys, ['delay_din',num2str(i-1),'/1'], ['bram',num2str(i-1),'/2']);
        add_line(cursys, ['bram',num2str(i-1),'/1'], ['dout',num2str(i-1),'/1']);
    end

    for i=1:order-1,
        mapname = ['cnt_map',num2str(i)];
        prevmapname = ['cnt_map',num2str(i-1)];
        if i == 1,
            add_line(cursys, 'Slice2/1', 'cnt_map1/1');
        else,
            add_line(cursys, [prevmapname,'/1'], [mapname,'/1'], 'autorouting', 'on');
        end
        add_line(cursys, [mapname,'/1'], ['delay_',mapname,'/1']);
        add_line(cursys, ['delay_',mapname,'/1'], ['Mux/',num2str(i+2)]);
    end

    clean_blocks(cursys);
end
