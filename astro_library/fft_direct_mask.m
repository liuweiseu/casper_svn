cursys = gcb;
disp(cursys);

current_stages = find_system(cursys, 'lookUnderMasks', 'all', 'FollowLinks','on',...
    'SearchDepth',1,'masktype', 'butterfly_direct');
prev_stages = length(current_stages);

if FFTSize*(2^(FFTSize-1)) ~= prev_stages,
    delete_lines(cursys);
    % Add ports
    reuse_block(cursys, 'sync', 'built-in/inport', {'Position', [30 0 60 15], 'Port', '1'});
    reuse_block(cursys, 'shift', 'built-in/inport', {'Position', [30 45 60 60], 'Port', '2'});
    reuse_block(cursys, 'sync_out', 'built-in/outport', {'Position', [300*FFTSize+150 0 300*FFTSize+180 15], 'Port', '1'});
    for i=0:2^FFTSize-1,
        reuse_block(cursys, ['in',num2str(i)], 'built-in/inport', {'Position', [30 100*i+100 60 100*i+115]});
        reuse_block(cursys, ['out',num2str(i)], 'built-in/outport', {'Position', [300*FFTSize+150 100*i+100 300*FFTSize+180 100*i+115]});
    end

    % Add nodes
    for stage=0:FFTSize,
        for i=0:2^FFTSize-1,
            name = ['node',num2str(stage),'_',num2str(i)];
            reuse_block(cursys, name, 'xbsIndex_r3/Delay', ...
                {'latency', '0', 'Position', [300*stage+90 100*i+100 300*stage+120 100*i+130]});
            if stage == 0,
                add_line(cursys, ['in',num2str(i),'/1'], [name,'/1']);
            end
            if stage == FFTSize,
                add_line(cursys, [name,'/1'], ['out',num2str(bit_reverse(i, FFTSize)),'/1']);
            end
        end
        if stage ~= FFTSize,
            name = ['slice',num2str(stage)];
            reuse_block(cursys, name, 'xbsIndex_r3/Slice', ...
                {'mode', 'Lower Bit Location + Width', 'nbits', '1', 'bit0', num2str(stage), ...
                'boolean_output', 'on', 'Position', [300*stage+90 70 300*stage+120 85]});
            add_line(cursys,'shift/1', [name,'/1']);
        end
    end
    % Add butterflies
    for stage=1:FFTSize,
        for i=0:2^(FFTSize-1)-1,
            name = ['butterfly',num2str(stage),'_',num2str(i)];
            reuse_block(cursys, name, 'fft_library/butterfly_direct', ...
                {'StepPeriod', '0', 'BitWidth', 'BitWidth', 'mult_latency', 'mult_latency', ...
                'add_latency', 'add_latency', 'bram_latency', 'bram_latency', ...
                'use_bram', 'on', 'Position', [300*(stage-1)+220 200*i+100 300*(stage-1)+300 200*i+175]});
            node_one_num = 2^(FFTSize-stage+1)*floor(i/2^(FFTSize-stage)) + mod(i, 2^(FFTSize-stage));
            node_two_num = node_one_num+2^(FFTSize-stage);
            input_one = ['node',num2str(stage-1),'_',num2str(node_one_num),'/1'];
            input_two = ['node',num2str(stage-1),'_',num2str(node_two_num),'/1'];
            output_one = ['node',num2str(stage),'_',num2str(node_one_num),'/1'];
            output_two = ['node',num2str(stage),'_',num2str(node_two_num),'/1'];
            add_line(cursys, input_one, [name,'/1']);
            add_line(cursys, input_two, [name,'/2']);
            add_line(cursys, [name,'/1'], output_one);
            add_line(cursys, [name,'/2'], output_two);
            if stage == 1, add_line(cursys, 'sync/1', [name,'/3']);
            else, add_line(cursys, ['butterfly',num2str(stage-1),'_',num2str(i),'/4'], [name,'/3']);
            end
            if stage == FFTSize && i == 0, add_line(cursys, [name,'/4'], 'sync_out/1');
            end
            add_line(cursys, ['slice',num2str(stage-1),'/1'], [name, '/4']);
        end
    end
    clean_blocks(cursys);
end

% Check dynamic settings
for stage=1:FFTSize,
    for i=0:2^(FFTSize-1)-1,
        butterfly = [cursys,'/butterfly',num2str(stage),'_',num2str(i)];
        % Implement a normal FFT or the tail end of a larger FFT
        if ~MapTail,
            coeffs = ['[',num2str(floor(i/2^(FFTSize-stage))),']'];
            actual_fft_size = FFTSize;
        else,
            redundancy = 2^(LargerFFTSize - FFTSize);
            coeffs = '[';
            for r=0:redundancy-1,
                n = bit_reverse(r, LargerFFTSize - FFTSize);
                coeffs = [coeffs,' ',num2str(floor((i+n*2^(FFTSize-1))/2^(LargerFFTSize-(StartStage+stage-1))))];
            end
            coeffs = [coeffs, ']'];
            actual_fft_size = LargerFFTSize;
        end
        if get_param(butterfly, 'FFTSize') ~= actual_fft_size,
            set_param(butterfly, 'FFTSize', num2str(actual_fft_size));
        end
        if ~strcmp(get_param(butterfly, 'Coeffs'), coeffs),
            set_param(butterfly, 'Coeffs', coeffs);
        end
        if ~strcmp(get_param(butterfly, 'quantization'), quantization),
            set_param(butterfly, 'quantization', quantization);
        end
        if ~strcmp(get_param(butterfly, 'overflow'), overflow),
            set_param(butterfly, 'overflow', overflow);
        end
    end
end