cursys = gcb;
disp(cursys);

if log2(num_inputs) ~= round(log2(num_inputs))
    error('The number of inputs must be a positive power of 2 interger');
end

taps = find_system(cursys, 'lookUnderMasks', 'on', 'FollowLinks','on','SearchDepth',1,'masktype', 'fir_double_tap');
num_taps = length(taps);

if num_taps ~= num_inputs,
    delete_lines(cursys);    
    % Make sure these go through Ports in strictly increasing order, or
    % port assignments don't "stick".
    for i=1:num_inputs,
        reuse_block(cursys, ['real',num2str(i)], 'built-in/inport', {'Position', [30 i*80 60 15+80*i], 'Port', num2str(2*i-1)});
        reuse_block(cursys, ['imag',num2str(i)], 'built-in/inport', {'Position', [30 i*80+30 60 45+80*i], 'Port', num2str(2*i)});

        reuse_block(cursys, ['real_out',num2str(i)], 'built-in/outport', {'Position', [350 i*80 380 15+80*i], 'Port', num2str(2*i-1)});
        reuse_block(cursys, ['imag_out',num2str(i)], 'built-in/outport', {'Position', [350 i*80+30 380 45+80*i], 'Port', num2str(2*i)});
    end
    for i=1:num_inputs,
        reuse_block(cursys, ['real_back',num2str(i)], 'built-in/inport', {'Position', [30 num_inputs*80+i*80 60 num_inputs*80+15+80*i], 'Port', num2str(2*i-1+2*num_inputs)});
        reuse_block(cursys, ['imag_back',num2str(i)], 'built-in/inport', {'Position', [30 num_inputs*80+i*80+30 60 num_inputs*80+45+80*i], 'Port', num2str(2*i+2*num_inputs)});
        
        reuse_block(cursys, ['real_back_out',num2str(i)], 'built-in/outport', {'Position', [350 num_inputs*80+i*80 380 num_inputs*80+15+80*i], 'Port', num2str(2*i-1+2*num_inputs)});
        reuse_block(cursys, ['imag_back_out',num2str(i)], 'built-in/outport', {'Position', [350 num_inputs*80+i*80+30 380 num_inputs*80+45+80*i], 'Port', num2str(2*i+2*num_inputs)});
    end
    reuse_block(cursys, 'real_sum', 'built-in/outport', {'Position', [600 10+20*num_inputs 630 30+20*num_inputs], 'Port', num2str(4*num_inputs+1)});
    reuse_block(cursys, 'imag_sum', 'built-in/outport', {'Position', [600 110+20*num_inputs 630 130+20*num_inputs], 'Port', num2str(4*num_inputs+2)});

    for i=1:num_inputs,
        reuse_block(cursys, ['fir_double_tap',num2str(i)], 'ddc_library/fir_double_tap', ...
            {'Position', [180 i*160-70 230 50+160*i], 'mult_latency', 'mult_latency',...
            'add_latency', 'add_latency', 'factor', ['coeff(',num2str(i),')']});
    end

    if num_inputs > 1,
        reuse_block(cursys, 'adder_tree1', 'ddc_library/adder_tree', ...
            {'Position', [500 100 550 100+20*num_inputs], 'num_inputs', 'num_inputs',...
            'latency', 'add_latency'});
        reuse_block(cursys, 'adder_tree2', 'ddc_library/adder_tree', ...
            {'Position', [500 200+20*num_inputs 550 200+20*num_inputs+20*num_inputs], 'num_inputs', 'num_inputs',...
            'latency', 'add_latency'});
    end

    if num_inputs > 1,
        add_line(cursys,['adder_tree1/1'],'real_sum/1');
        add_line(cursys,['adder_tree2/1'],'imag_sum/1');
    end

    for i=1:num_inputs,
        add_line(cursys,['real',num2str(i),'/1'],['fir_double_tap',num2str(i),'/1']);
        add_line(cursys,['imag',num2str(i),'/1'],['fir_double_tap',num2str(i),'/2']);
        add_line(cursys,['real_back',num2str(num_inputs+1-i),'/1'],['fir_double_tap',num2str(i),'/3']);
        add_line(cursys,['imag_back',num2str(num_inputs+1-i),'/1'],['fir_double_tap',num2str(i),'/4']);
        add_line(cursys,['fir_double_tap',num2str(i),'/1'],['real_out',num2str(i),'/1']);
        add_line(cursys,['fir_double_tap',num2str(i),'/2'],['imag_out',num2str(i),'/1']);
        add_line(cursys,['fir_double_tap',num2str(i),'/3'],['real_back_out',num2str(num_inputs+1-i),'/1']);
        add_line(cursys,['fir_double_tap',num2str(i),'/4'],['imag_back_out',num2str(num_inputs+1-i),'/1']);
        if num_inputs > 1,
            add_line(cursys,['fir_double_tap',num2str(i),'/5'],['adder_tree1/',num2str(i)]);
            add_line(cursys,['fir_double_tap',num2str(i),'/6'],['adder_tree2/',num2str(i)]);
        else,
            add_line(cursys,['fir_double_tap',num2str(i),'/5'],['real_sum/1']);
            add_line(cursys,['fir_double_tap',num2str(i),'/6'],['imag_sum/1']);
        end
    end
    clean_blocks(cursys);
end
