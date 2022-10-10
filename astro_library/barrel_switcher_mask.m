cursys = gcb;
disp(cursys);

outports = find_system(cursys, 'lookUnderMasks', 'on', 'FollowLinks','on','SearchDepth',1,'BlockType', 'Outport');
num_outports = length(outports);

if num_outports ~= 2^n_inputs + 1,
    delete_lines(cursys);
    reuse_block(cursys, 'sel', 'built-in/inport', {'Position', [15 23 45 37], 'Port', '1'});
    reuse_block(cursys, 'sync_in', 'built-in/inport', {'Position', [15 (2^n_inputs+1)*80+95 45 109+80*(2^n_inputs+1)], 'Port', '2'});
    reuse_block(cursys, 'sync_out', 'built-in/outport', {'Position', [95 (2^n_inputs+1)*80+95 125 109+80*(2^n_inputs+1)], 'Port', '1'});
    reuse_block(cursys, 'Delay_sync', 'xbsIndex_r3/Delay', ...
        {'Position', [55 (2^n_inputs+1)*80+95 85 109+80*(2^n_inputs+1)], 'latency', num2str(n_inputs)});
    add_line(cursys, 'sync_in/1', 'Delay_sync/1');
    add_line(cursys, 'Delay_sync/1', 'sync_out/1');
    for i=1:2^n_inputs,
        reuse_block(cursys, ['In',num2str(i)], 'built-in/inport', {'Position', [15 i*80+95 45 109+80*i]});
        reuse_block(cursys, ['Out',num2str(i)], 'built-in/outport', {'Position', [15+150*(n_inputs+1) 95+i*80 45+150*(n_inputs+1) 109+80*i]});
    	for j=1:n_inputs,
    		reuse_block(cursys, ['Mux', num2str(10*i + j)], 'xbsIndex_r3/Mux', ...
                {'latency', '1', 'Position', [15+150*j, 67+80*i, 40+150*j, 133+80*i]});
        end
    end
    for j=1:(n_inputs-1),
        reuse_block(cursys, ['Delay', num2str(j)], 'xbsIndex_r3/Delay', ...
            {'Position', [15+150*j 15 45+150*j 45]});
    end
    for j=1:n_inputs,
       	reuse_block(cursys, ['Slice', num2str(j)], 'xbsIndex_r3/Slice', ...
            {'Position', [85+150*(j-1) 91 130+150*(j-1) 119], 'bit1', ['-', num2str(j-1)]});
    end
    add_line(cursys, 'sel/1', 'Slice1/1');
    if n_inputs > 1, add_line(cursys, 'sel/1', 'Delay1/1');
    end
    for j=1:(n_inputs-2),
       	delayname = ['Delay', num2str(j)];
       	nextdelay = ['Delay', num2str(j+1)];
       	add_line(cursys, [delayname, '/1'], [nextdelay, '/1']);
    end
    for j=1:(n_inputs-1),
       	slicename = ['Slice', num2str(j+1)];
       	delayname = ['Delay', num2str(j)];
       	add_line(cursys, [delayname, '/1'], [slicename, '/1']);
    end
    for j=1:n_inputs,
       	slicename = ['Slice', num2str(j)];
       	for i=1:2^n_inputs
       		muxname = ['Mux', num2str(10*i + j)];
       		add_line(cursys, [slicename, '/1'], [muxname, '/1']);
        end
    end
    for i=1:2^n_inputs,
       	iport = ['In', num2str(i)];
       	oport = ['Out', num2str(i)];
       	firstmux = ['Mux', num2str(10*i + 1)];
       	if i > 2^n_inputs / 2
       		swmux = ['Mux', num2str(10*(i-2^n_inputs/2) + 1)];
        else
       		swmux = ['Mux', num2str(10*(i-2^n_inputs/2 + 2^n_inputs) + 1)];
        end
       	lastmux = ['Mux', num2str(10*i + n_inputs)];
       	add_line(cursys, [iport, '/1'], [firstmux, '/2']);
       	add_line(cursys, [iport, '/1'], [swmux, '/3']);
       	add_line(cursys, [lastmux, '/1'], [oport, '/1']);
    end
    for i=1:2^n_inputs,
       	for j=1:(n_inputs-1)
       		muxname = ['Mux', num2str(10*i + j)];
       		nextmuxname = ['Mux', num2str(10*i + j+1)];
       		add_line(cursys, [muxname, '/1'], [nextmuxname, '/2']);
       		if i > 2^n_inputs / (2^(j+1))
       			swmux = ['Mux', num2str(10*(i-2^n_inputs/(2^(j+1))) + j+1)];
            else
       			swmux = ['Mux', num2str(10*(i-2^n_inputs/(2^(j+1)) + 2^n_inputs) + j+1)];
            end
       		add_line(cursys, [muxname, '/1'], [swmux, '/3']);
        end
    end
    clean_blocks(cursys);
end

