cursys = gcb;
disp(cursys);
disp(streams);

current_stages = find_system(cursys, 'lookUnderMasks', 'all', 'FollowLinks','on', 'SearchDepth',1,...
    'masktype', 'vacc_fifo');
prev_stages = length(current_stages);

if streams < 1,
    errordlg('Number of streams must be at least 1.');
    set_param(cursys, 'streams', '1');
    streams = 1;
end

if streams ~= prev_stages,
    outports = {'dout', 'valid', 'empty'};
    delete_lines(cursys);
    % Create/Delete Stages and set static parameters
    for a=2:streams,
        stage_name = ['vacc_fifo',num2str(a)];
        reuse_block(cursys, stage_name, 'correlator_library/vacc_fifo', ...
            {'vlen', 'vlen', 'n_bits', 'n_bits', 'bin_pt', 'bin_pt', ...
            'BitWidthIn', 'BitWidthIn', 'BinPtIn', 'BinPtIn', ...
            'add_latency', 'add_latency', 'streams', 'streams', ...
            'stream', num2str(a), 'Position', [125*a+5, 75, 125*a+105, 165]});
        prev_stage_name = ['vacc_fifo',num2str(a-1)];
        add_line(cursys, [prev_stage_name,'/1'], [stage_name,'/1']);
        add_line(cursys, [prev_stage_name,'/2'], [stage_name,'/2']);
        add_line(cursys, [prev_stage_name,'/3'], [stage_name,'/3']);
        add_line(cursys, [prev_stage_name,'/4'], [stage_name,'/4']);
        add_line(cursys, [prev_stage_name,'/5'], [stage_name,'/5']);
        add_line(cursys, [prev_stage_name,'/6'], [stage_name,'/6']);
        add_line(cursys, [prev_stage_name,'/7'], [stage_name,'/7']);
    end
    add_line(cursys, 'din/1', 'vacc_fifo1/1');
    add_line(cursys, 'we/1', 'vacc_fifo1/2');
    add_line(cursys, 'const0/1', 'vacc_fifo1/3');
    add_line(cursys, 'const1/1', 'vacc_fifo1/4');
    add_line(cursys, 'rst/1', 'vacc_fifo1/5');
    add_line(cursys, 're/1', 'vacc_fifo1/6');
    add_line(cursys, 'acc_en/1', 'vacc_fifo1/7');
    % Reposition output ports
    last_stage = ['vacc_fifo',num2str(streams)];
    for a=1:length(outports),
    	x = 125*(streams+1);
    	y = 103 + 20*(a-1);
        set_param([cursys,'/',outports{a}], 'Position', [x, y, x+30, y+14]);
    end
    add_line(cursys, [last_stage,'/',num2str(3)], [outports{1},'/1']);
    add_line(cursys, [last_stage,'/',num2str(4)], [outports{2},'/1']);
    add_line(cursys, [last_stage,'/',num2str(8)], [outports{3},'/1']);
    clean_blocks(cursys);
end

% Set Dynamic Parameters
for a=1:streams,
    stage_name = [cursys,'/vacc_fifo',num2str(a)];
    if ~strcmp(get_param(stage_name, 'arith_type'), arith_type),
        set_param(stage_name, 'arith_type', arith_type);
    end
end
const = [cursys,'/const0'];
if ~strcmp(get_param(const, 'arith_type'), arith_type),
    set_param(const, 'arith_type', arith_type);
end

