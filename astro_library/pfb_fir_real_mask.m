cursys = gcb;
disp(cursys);

if MakeBiplex, pols = 2;
else, pols = 1;
end

current_taps = find_system(cursys, 'lookUnderMasks', 'all', 'FollowLinks','on', 'SearchDepth',1,...
    'masktype', 'pfb_tap_real');
prev_taps = length(current_taps);

if prev_taps ~= pols * 2^n_inputs * TotalTaps,
    delete_lines(cursys);
    % Add ports
    portnum = 1;
    reuse_block(cursys, 'sync', 'built-in/inport', ...
        {'Position', [0 50*portnum 30 50*portnum+15], 'Port', num2str(portnum)});
    reuse_block(cursys, 'sync_out', 'built-in/outport', ...
        {'Position', [150*(TotalTaps+1) 50*portnum 150*(TotalTaps+1)+30 50*portnum+15], 'Port', num2str(portnum)});
    for p=1:pols,
        for i=1:2^n_inputs,
            portnum = portnum + 1; % Skip one to allow sync & sync_out to be 1
            in_name = ['pol',num2str(p),'_in',num2str(i)];
            out_name = ['pol',num2str(p),'_out',num2str(i)];
            reuse_block(cursys, in_name, 'built-in/inport', ...
                {'Position', [0 50*portnum 30 50*portnum+15], 'Port', num2str(portnum)});
            reuse_block(cursys, out_name, 'built-in/outport', ...
                {'Position', [150*(TotalTaps+1) 50*portnum 150*(TotalTaps+1)+30 50*portnum+15], 'Port', num2str(portnum)});
        end
    end

    % Add Blocks
    portnum = 0;
    for p=1:pols,
        for i=1:2^n_inputs,
            portnum = portnum + 1;
            for t=1:TotalTaps,
                if t==1,
                    src_blk = 'pfb_library/first_tap_real';
                    name = ['pol',num2str(p),'_in',num2str(i),'_first_tap'];
                elseif t==TotalTaps,
                    src_blk = 'pfb_library/last_tap_real';
                    name = ['pol',num2str(p),'_in',num2str(i),'_last_tap'];
                else,
                    src_blk = 'pfb_library/tap_real';
                    name = ['pol',num2str(p),'_in',num2str(i),'_tap',num2str(t)];
                end
                reuse_block(cursys, name, src_blk, ...
                    {'Position', [150*t 50*portnum 150*t+100 50*portnum+30]});
                if t==1,
                    set_param([cursys,'/',name], 'nput', num2str(i-1), 'fwidth', 'fwidth');
                end
            end
        end
    end

    % Add Lines
    for p=1:pols,
        for i=1:2^n_inputs,
            for t=1:TotalTaps,
                in_name = ['pol',num2str(p),'_in',num2str(i)];
                out_name = ['pol',num2str(p),'_out',num2str(i)];
                if t==1,
                    blk_name = ['pol',num2str(p),'_in',num2str(i),'_first_tap'];
                    add_line(cursys, [in_name,'/1'], [blk_name,'/1']);
                    add_line(cursys, 'sync/1', [blk_name,'/2']);
                elseif t==TotalTaps,
                    blk_name = ['pol',num2str(p),'_in',num2str(i),'_last_tap'];
                    if t==2,
                        prev_blk_name = ['pol',num2str(p),'_in',num2str(i),'_first_tap'];
                    else,
                        prev_blk_name = ['pol',num2str(p),'_in',num2str(i),'_tap',num2str(t-1)];
                    end
                    for n=1:4, add_line(cursys, [prev_blk_name,'/',num2str(n)], [blk_name,'/',num2str(n)]);
                    end
                    add_line(cursys, [blk_name,'/1'], [out_name,'/1']);
                    if i==1 && p==1, add_line(cursys, [blk_name,'/2'], 'sync_out/1');
                    end
                else,
                    blk_name = ['pol',num2str(p),'_in',num2str(i),'_tap',num2str(t)];
                    if t==2,
                        prev_blk_name = ['pol',num2str(p),'_in',num2str(i),'_first_tap'];
                    else,
                        prev_blk_name = ['pol',num2str(p),'_in',num2str(i),'_tap',num2str(t-1)];
                    end
                    for n=1:4, add_line(cursys, [prev_blk_name,'/',num2str(n)], [blk_name,'/',num2str(n)]);
                    end
                end
            end
        end
    end
    clean_blocks(cursys);
end

% Set dynamic parameters
if ~strcmp(get_param([cursys,'/pol1_in1_first_tap'], 'CoeffDistMem'), CoeffDistMem),
    for p=1:pols,
        for i=1:2^n_inputs,
            blk_name = ['pol',num2str(p),'_in',num2str(i),'_first_tap'];
            set_param([cursys,'/',blk_name], 'CoeffDistMem', CoeffDistMem);
        end
    end
end
if ~strcmp(get_param([cursys,'/pol1_in1_last_tap'], 'quantization'), quantization),
    for p=1:pols,
        for i=1:2^n_inputs,
            blk_name = ['pol',num2str(p),'_in',num2str(i),'_last_tap'];
            set_param([cursys,'/',blk_name], 'quantization', quantization);
        end
    end
end
