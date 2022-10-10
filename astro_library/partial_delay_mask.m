cursys = gcb;
disp(cursys);

outports = find_system(cursys, 'lookUnderMasks', 'on', 'FollowLinks', 'on', 'SearchDepth', 1, 'BlockType', 'Outport');
num_outports = length(outports);

if num_outports ~= num_inputs,
	delete_lines(cursys);
	reuse_block(cursys, 'sel', 'built-in/inport', {'Position', [25 25 55 39], 'Port', '1'});
	
	for i = 1 : num_inputs,
		reuse_block(cursys, ['In', num2str(i)], 'built-in/inport', {'Position', [25 25+(i*75) 55 39+(i*75)], 'Port', num2str(1+i)});
		reuse_block(cursys, ['Delay', num2str(i)], 'xbsIndex_r3/Delay', {'Position', [100 20+(i*75) 130 50+(i*75)], 'latency', '1'});
		reuse_block(cursys, ['Mux', num2str(i)], 'xbsIndex_r3/Mux', {'Position', [175 20+(i*75) 200 86+(i*75)], 'inputs', num2str(num_inputs), 'latency', 'mux_latency'});
		reuse_block(cursys, ['Out', num2str(i)], 'built-in/outport', {'Position', [250 25+(i*75) 280 39+(i*75)], 'Port', num2str(i)});
		
		add_line(cursys, ['In', num2str(i), '/1'], ['Delay', num2str(i), '/1']);
		add_line(cursys, ['Mux', num2str(i), '/1'], ['Out', num2str(i), '/1']);
		add_line(cursys, 'sel/1', ['Mux', num2str(i), '/1']);
        add_line(cursys, ['Delay', num2str(i), '/1'], ['Mux', num2str(i), '/2']),
    end
    
    for j = 1 : num_inputs,
        for k = 2 : num_inputs,
            if j+k <= num_inputs+1,
                add_line(cursys, ['Delay', num2str(j+k-1), '/1'], ['Mux', num2str(j), '/', num2str(k+1)]);
            else
                add_line(cursys, ['In', num2str(j+k-num_inputs-1), '/1'], ['Mux', num2str(j), '/', num2str(k+1)]);
            end
        end
    end
    
    clean_blocks(cursys);
end
