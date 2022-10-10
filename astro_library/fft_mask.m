cursys = gcb;
disp(cursys);

biplexes = find_system(cursys, 'lookUnderMasks', 'all', 'FollowLinks','on','masktype', 'fft_biplex');
outports = find_system(cursys, 'lookUnderMasks', 'on', 'FollowLinks','on','SearchDepth',1,'BlockType', 'Outport');
num_biplexes = length(biplexes);
num_outports = length(outports);

if (n_inputs ~= FFTSize && num_biplexes ~= 2^(n_inputs-1)) || (n_inputs == FFTSize && num_biplexes ~= 0) || num_outports ~= 2^(n_inputs) + 1,
    delete_lines(cursys);

    % Add Ports
    reuse_block(cursys, 'sync', 'built-in/inport', {'Position', [30 0 60 15], 'Port', '1'});
    reuse_block(cursys, 'shift', 'built-in/inport', {'Position', [30 45 60 60], 'Port', '2'});
    reuse_block(cursys, 'sync_out', 'built-in/outport', {'Position', [700 0 730 15], 'Port', '1'});
    if n_inputs < 1,
        reuse_block(cursys, 'pol0', 'built-in/inport', {'Position', [30 100 60 115]});
        reuse_block(cursys, 'pol1', 'built-in/inport', {'Position', [30 200 60 215]});
        reuse_block(cursys, 'out0', 'built-in/outport', {'Position', [700 100 730 115]});
        reuse_block(cursys, 'out1', 'built-in/outport', {'Position', [700 200 730 215]});
    else,
        for i=0:2^n_inputs-1,
            reuse_block(cursys, ['in',num2str(i)], 'built-in/inport', {'Position', [30 100*i+100 60 100*i+115], 'Port', num2str(i+3)});
            reuse_block(cursys, ['out',num2str(i)], 'built-in/outport', {'Position', [700 100*i+100 730 100*i+115], 'Port', num2str(i+2)});
        end
    end

    % Add biplex FFTs
    if n_inputs < 1,
        pos = [100 0 220 220];
        name = 'fft_biplex0';
        reuse_block(cursys, name, 'fft_library/fft_biplex', ...
            {'FFTSize', 'FFTSize-n_inputs', 'BitWidth', 'BitWidth', ...
            'add_latency', 'add_latency', 'mult_latency', 'mult_latency', ...
            'bram_latency', 'bram_latency', 'Position', pos});
        add_line(cursys, 'pol0/1', [name,'/1']);
        add_line(cursys, 'pol1/1', [name,'/2']);
        add_line(cursys, 'shift/1', [name,'/4']);
        add_line(cursys, 'sync/1', [name,'/3']);
    elseif n_inputs ~= FFTSize,
        for i=0:2^(n_inputs-1)-1,
            pos = [100 200*i+100 220 200*i+220];
            name = ['fft_biplex',num2str(i)];
            reuse_block(cursys, name, 'fft_library/fft_biplex', ...
                {'FFTSize', 'FFTSize-n_inputs', 'BitWidth', 'BitWidth', ...
                'add_latency', 'add_latency', 'mult_latency', 'mult_latency', ...
                'bram_latency', 'bram_latency', 'Position', pos});
            add_line(cursys, ['in',num2str(2*i),'/1'], [name,'/1']);
            add_line(cursys, ['in',num2str(2*i+1),'/1'], [name,'/2']);
            add_line(cursys, 'shift/1', [name,'/4']);
            add_line(cursys, 'sync/1', [name,'/3']);
        end
    end
    % Add direct FFTs
    if n_inputs < 1,
        add_line(cursys, 'fft_biplex0/1', 'out0/1');
        add_line(cursys, 'fft_biplex0/2', 'out1/1');
        add_line(cursys, 'fft_biplex0/4', 'sync_out/1');
    elseif n_inputs == FFTSize,
        pos = [400 0 520 120];
        reuse_block(cursys, 'fft_direct', 'fft_library/fft_direct', ...
            {'FFTSize', 'n_inputs', 'BitWidth', 'BitWidth', ...
            'add_latency', 'add_latency', 'mult_latency', 'mult_latency', ...
            'bram_latency', 'bram_latency', 'MapTail', 'off', ...
            'Position', pos});
        add_line(cursys, 'sync/1', 'fft_direct/1');
        add_line(cursys, 'shift/1', 'fft_direct/2');
        add_line(cursys, 'fft_direct/1', 'sync_out/1');
        for i=0:2^n_inputs-1,
            add_line(cursys, ['in',num2str(i),'/1'], ['fft_direct/',num2str(i+3)]);
            add_line(cursys, ['fft_direct/',num2str(i+2)], ['out',num2str(i),'/1']);
        end
    else,
        pos = [400 0 520 120];
        reuse_block(cursys, 'fft_direct', 'fft_library/fft_direct', ...
            {'FFTSize', 'n_inputs', 'BitWidth', 'BitWidth', ...
            'add_latency', 'add_latency', 'mult_latency', 'mult_latency', ...
            'bram_latency', 'bram_latency', 'MapTail', 'on', ...
            'LargerFFTSize', 'FFTSize', 'StartStage', 'FFTSize-n_inputs+1', 'Position', pos});
        pos = [400 200 520 200+120];
        reuse_block(cursys, 'slice', 'xbsIndex_r3/Slice', ...
            {'mode', 'Lower Bit Location + Width', 'bit0', 'FFTSize-n_inputs', 'nbits', 'n_inputs', ...
            'Position', [100 0 130 15]});
        add_line(cursys, 'shift/1', 'slice/1');
        add_line(cursys, 'slice/1', 'fft_direct/2');
        add_line(cursys, 'fft_biplex0/4', 'fft_direct/1');
        for i=0:2^(n_inputs-1)-1,
            bi_name = ['fft_biplex',num2str(i)];
            add_line(cursys, [bi_name,'/1'], ['fft_direct/',num2str(3+2*i)]);
            add_line(cursys, [bi_name,'/2'], ['fft_direct/',num2str(3+2*i+1)]);
        end
        % Add Unscrambler
        reuse_block(cursys, 'fft_unscrambler', 'fft_library/fft_unscrambler', ...
            {'FFTSize', 'FFTSize', 'n_inputs', 'n_inputs', 'bram_latency', 'bram_latency', ...
            'Position', [550 0 670 120]});
        for i=1:2^n_inputs+1,
            add_line(cursys, ['fft_direct/',num2str(i)], ['fft_unscrambler/',num2str(i)]);
            if i==1, add_line(cursys, ['fft_unscrambler/',num2str(i)], 'sync_out/1');
            else, add_line(cursys, ['fft_unscrambler/',num2str(i)], ['out',num2str(i-2),'/1']);
            end
        end
    end
    clean_blocks(cursys);
end

% Propagate dynamic variables
if n_inputs < 1,
    name = [cursys,'/fft_biplex0'];
    if ~strcmp(get_param(name, 'quantization'), quantization),
        set_param(name, 'quantization', quantization);
    end
    if ~strcmp(get_param(name, 'overflow'), overflow),
        set_param(name, 'overflow', overflow);
    end
else,
    if n_inputs ~= FFTSize,
        for i=0:2^(n_inputs-1)-1,
            name = [cursys,'/fft_biplex',num2str(i)];
            if ~strcmp(get_param(name, 'quantization'), quantization),
                set_param(name, 'quantization', quantization);
            end
            if ~strcmp(get_param(name, 'overflow'), overflow),
                set_param(name, 'overflow', overflow);
            end
        end
    end
    name = [cursys,'/fft_direct'];
    if ~strcmp(get_param(name, 'quantization'), quantization),
        set_param(name, 'quantization', quantization);
    end
    if ~strcmp(get_param(name, 'overflow'), overflow),
        set_param(name, 'overflow', overflow);
    end
end
