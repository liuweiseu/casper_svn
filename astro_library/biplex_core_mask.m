cursys = gcb;
disp(cursys);

BRAMSize = 18432;
MaxCoeffNum = 11;	        % This is the maximum that will fit in a BRAM
%DelayBramThresh = 1/8;      % Use bram when delays will fill this fraction of a BRAM
CoeffBramThresh = 1/8;      % Use bram when coefficients will fill this fraction of a BRAM

if FFTSize < 2,
    errordlg('Biplex FFT must have length of at least 2^2.');
    set_param(cursys, 'FFTSize', '2');
    FFTSize = 2;
end

current_stages = find_system(cursys, 'lookUnderMasks', 'all', 'FollowLinks','on', 'SearchDepth',1,...
    'masktype', 'fft_stage');
prev_stages = length(current_stages);

if FFTSize ~= prev_stages,
    outports = {'out1', 'out2', 'of', 'sync_out'};
    delete_lines(cursys);
    % Create/Delete Stages and set static parameters
    for a=3:FFTSize,
        stage_name = ['fft_stage_',num2str(a)];
        reuse_block(cursys, stage_name, 'fft_library/fft_stage_n', ...
            {'FFTSize', 'FFTSize', 'FFTStage', num2str(a), 'BitWidth', 'BitWidth', ...
            'MaxCoeffNum', 'MaxCoeffNum', 'add_latency', 'add_latency', ...
            'mult_latency', 'mult_latency', 'bram_latency', 'bram_latency', ...
            'Position', [110*a, 27, 110*a+95, 113]});
        prev_stage_name = ['fft_stage_',num2str(a-1)];
        add_line(cursys, [prev_stage_name,'/1'], [stage_name,'/1']);
        add_line(cursys, [prev_stage_name,'/2'], [stage_name,'/2']);
        add_line(cursys, [prev_stage_name,'/3'], [stage_name,'/3']);
        add_line(cursys, [prev_stage_name,'/4'], [stage_name,'/4']);
        add_line(cursys, 'shift/1', [stage_name,'/5']);
    end
    add_line(cursys, 'pol1/1', 'fft_stage_1/1');
    add_line(cursys, 'pol2/1', 'fft_stage_1/2');
    add_line(cursys, 'sync/1', 'fft_stage_1/3');
    add_line(cursys, 'shift/1', 'fft_stage_1/4');
    add_line(cursys, 'fft_stage_1/1', 'fft_stage_2/1');
    add_line(cursys, 'fft_stage_1/2', 'fft_stage_2/2');
    add_line(cursys, 'fft_stage_1/3', 'fft_stage_2/3');
    add_line(cursys, 'fft_stage_1/4', 'fft_stage_2/4');
    add_line(cursys, 'shift/1', 'fft_stage_2/5');
    % Reposition output ports
    last_stage = ['fft_stage_',num2str(FFTSize)];
    for a=1:length(outports),
    	x = 110*(FFTSize+1);
    	y = 33 + 20*(a-1);
        set_param([cursys,'/',outports{a}], 'Position', [x, y, x+30, y+14]);
        add_line(cursys, [last_stage,'/',num2str(a)], [outports{a},'/1']);
    end
    clean_blocks(cursys);
end

% Set Dynamic Parameters
for a=1:FFTSize,
    stage_name = [cursys,'/fft_stage_',num2str(a)];
    %if (2^(FFTSize - a) * BitWidth >= DelayBramThresh*BRAMSize), delay_bram = 'on';
    if (FFTSize - a >= 6), delay_bram = 'on';
    else, delay_bram = 'off';
    end
    if (min(2^(a-1), 2^MaxCoeffNum) * BitWidth >= CoeffBramThresh*BRAMSize), coeff_bram = 'on';
 	else, coeff_bram = 'off';
    end
    if ~strcmp(get_param(stage_name, 'DelayBram'), delay_bram),
        set_param(stage_name, 'DelayBram', delay_bram);
    end
    if ~strcmp(get_param(stage_name, 'CoeffBram'), coeff_bram),
        set_param(stage_name, 'CoeffBram', coeff_bram);
    end
    if ~strcmp(get_param(stage_name, 'quantization'), quantization),
        set_param(stage_name, 'quantization', quantization);
    end
    if ~strcmp(get_param(stage_name, 'overflow'), overflow),
        set_param(stage_name, 'overflow', overflow);
    end
end
