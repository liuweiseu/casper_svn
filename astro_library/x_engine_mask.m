cursys = gcb;
disp(cursys);

if AntNum < 2,
    errordlg('X Engine must have at least 2 antennas.');
    set_param(cursys, 'AntNum', '2');
    AntNum = 2;
end

BitGrowth = ceil(log2(AccLen));
AntBits = ceil(log2(AntNum));

current_taps = find_system(cursys, 'lookUnderMasks', 'all', 'FollowLinks','on', 'SearchDepth',1,...
    'masktype', 'x_engine_tap');
prev_taps = length(current_taps);

if prev_taps ~= floor(AntNum/2) + 1,
    delete_lines(cursys);
    % Add taps
    x = 275;
    for i=1:floor(AntNum/2),
        name = ['baseline_tap', num2str(i)];
        reuse_block(cursys, name, 'correlator_library/baseline_tap', ...
            {'AntSep', num2str(i), 'Position', [x, 52, x+95, 168]});
    	x = x + 135;
    end
    reuse_block(cursys, 'Term1', 'built-in/Terminator', {'Position', [x, 25, x + 20, 45]});
    reuse_block(cursys, 'Term2', 'built-in/Terminator', {'Position', [x, 65, x + 20, 85]});
    reuse_block(cursys, 'Term3', 'built-in/Terminator', {'Position', [x, 105, x + 20, 125]});
    % Set port positions
    set_param([cursys, '/acc'], 'Position', [x, 160, x + 30, 174]);
    set_param([cursys, '/valid'], 'Position', [x, 190, x + 30, 204]);
    set_param([cursys, '/sync_out'], 'Position', [x, 220, x + 30, 234]);
    % Add lines
    add_line(cursys, 'ant/1', 'auto_tap/1', 'autorouting', 'on');
    add_line(cursys, 'ant/1', 'auto_tap/2', 'autorouting', 'on');
    add_line(cursys, 'Constant/1', 'auto_tap/4', 'autorouting', 'on');
    add_line(cursys, 'Constant1/1', 'auto_tap/5', 'autorouting', 'on');
    add_line(cursys, 'sync_in/1', 'auto_tap/6', 'autorouting', 'on');
    for i=1:floor(AntNum / 2)
    	if i == 1
    		prevblk = 'auto_tap';
        else
    		prevblk = ['baseline_tap', num2str(i-1)];
        end
    	thisblk = ['baseline_tap', num2str(i)];
    	add_line(cursys, [prevblk, '/1'], [thisblk, '/1'], 'autorouting', 'on');
    	add_line(cursys, [prevblk, '/2'], [thisblk, '/2'], 'autorouting', 'on');
    	add_line(cursys, [prevblk, '/3'], [thisblk, '/3'], 'autorouting', 'on');
    	add_line(cursys, [prevblk, '/4'], [thisblk, '/4'], 'autorouting', 'on');
    	add_line(cursys, [prevblk, '/5'], [thisblk, '/5'], 'autorouting', 'on');
    	add_line(cursys, [prevblk, '/6'], [thisblk, '/6'], 'autorouting', 'on');
    	add_line(cursys, [prevblk, '/7'], [thisblk, '/7'], 'autorouting', 'on');
    end
    i = floor(AntNum / 2);
    thisblk = ['baseline_tap', num2str(i)];
    add_line(cursys, [thisblk, '/1'], 'auto_tap/3', 'autorouting', 'on');
    add_line(cursys, [thisblk, '/2'], 'Term1/1', 'autorouting', 'on');
    add_line(cursys, [thisblk, '/3'], 'Term2/1', 'autorouting', 'on');
    add_line(cursys, [thisblk, '/4'], 'acc/1', 'autorouting', 'on');
    add_line(cursys, [thisblk, '/5'], 'valid/1', 'autorouting', 'on');
    add_line(cursys, [thisblk, '/6'], 'Term3/1', 'autorouting', 'on');
    add_line(cursys, [thisblk, '/7'], 'sync_out/1', 'autorouting', 'on');
    clean_blocks(cursys);
end
% Propagate parameters
if use_mult_core, mc = 'on';
else, mc = 'off';
end
for i=0:floor(AntNum/2),
    if i == 0,
        name = 'auto_tap';
    else,
        name = ['baseline_tap', num2str(i)];
    end
    pathname = [cursys,'/',name];
    if ~strcmp(get_param(pathname, 'use_mult_core'), mc),
        set_param(pathname, 'use_mult_core', mc);
    end
end