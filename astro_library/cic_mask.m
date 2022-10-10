cursys = gcb;
disp(cursys)
delete_lines(cursys);
% determines whether to keep downsample block
if (R < 2), 
    decimation = 0;
else,
    decimation = 1;
end


% Add ports
% 'Position', [top left x, top left y, bottom right x, bottom right y],
reuse_block(cursys, 'sync', 'built-in/inport', {'Position', [0 470 30 484], 'Port', '1'});
reuse_block(cursys, 'syncout', 'built-in/outport', {'Position', [280+55+stages*(55+100+55+125) 470 280+55+30+stages*(55+100+55+125) 484], 'Port', '1'});
reuse_block(cursys, 'cic_out', 'built-in/outport', {'Position', [(900*stages) 113 (900*stages)+30 113+14], 'Port', '2'});
num_inputs = inputs;
while (num_inputs > 0),
    reuse_block(cursys, ['cic_in', num2str(num_inputs)], 'built-in/inport', ...
        {'Position', [0 (14+28)*num_inputs 30 (14+28)*num_inputs+14], 'Port', num2str(num_inputs+1)});
    num_inputs = num_inputs - 1;
end

% add blocks



%sync delay
if (pipeline),
    reuse_block(cursys, 'syncdelay', 'xbsIndex_r3/Delay', ...
    {'Position', [310 440 310+55 440+75],...
    'latency', ['add_latency*log2(inputs)+stages*(int_latency+comb_latency+D+2+', num2str(decimation), ')']});
else,
reuse_block(cursys, 'syncdelay', 'xbsIndex_r3/Delay', ...
    {'Position', [310 440 310+55 440+75],...
    'latency', ['add_latency*log2(inputs)+stages*(int_latency+comb_latency+D+', num2str(decimation), ')']});
end

% adder tree
reuse_block(cursys, 'adder_tree','cic_lib/adder_tree', ...
    {'Position', [155 100 125+85 100+100], 'num_inputs', 'inputs', 'latency', 'add_latency'});

if (decimation),

    reuse_block(cursys, 'downsample', 'xbsIndex_r3/Down Sample', {'Position', [420+stages*100 113 420+stages*100+55 113+75], ...
        'sample_ratio', 'R', 'sample_phase', 'Last Value of Frame', 'latency', '1'});
end

num_stages = stages;
while (num_stages > 0),
    


    reuse_block(cursys, ['integrator', num2str(num_stages)], 'xbsIndex_r3/AddSub', ...
        {'Mode', 'Addition', 'Precision', 'User Defined', 'n_bits', 'ceil(input_bitwidth+log2(inputs)+stages*log2(R*D))', ...
        'bin_pt', 'input_binary_pt', 'Quantization', 'truncate', 'arith_type', 'Signed',...
        'latency', 'int_latency', 'pipeline', 'on', 'explicit_period', 'on', 'period', '1'...
                    'Position', [310+100*num_stages 113 310+100*num_stages+55 113+75]});
                                
    %reuse_block(cursys, ['comb', num2str(num_stages)], 'xbsIndex_r3/AddSub', ...
    %    {'Mode', 'Subtraction', 'Precision', 'User Defined', 'n_bits', 'ceil(input_bitwidth+log2(inputs)+stages*log2(R*D))',  ...
    %    'bin_pt', 'input_binary_pt', 'Quantization', 'truncate', 'arith_type', 'Signed',...
    %    'latency', 'comb_latency', 'pipeline', 'on', 'explicit_period', 'on', 'period', '1',...
    %                'Position', [520*stages+100*num_stages 113 520*stages+100*num_stages+55 113+75]});
    %            
    reuse_block(cursys, ['comb', num2str(num_stages)], 'xbsIndex_r3/AddSub', ...
        {'Mode', 'Subtraction', 'Precision', 'User Defined', 'n_bits', 'ceil(input_bitwidth+log2(inputs)+stages*log2(R*D))',  ...
        'bin_pt', 'input_binary_pt', 'Quantization', 'truncate', 'arith_type', 'Signed',...
        'latency', 'comb_latency', 'pipeline', 'on', 'explicit_period', 'off',...
                    'Position', [520*stages+100*num_stages 113 520*stages+100*num_stages+55 113+75]});
                
        
    reuse_block(cursys, ['delay', num2str(num_stages)], 'xbsIndex_r3/Delay',...
        {'Position', [520*stages+100*num_stages 213 520*stages+100*num_stages+55 213+75], 'latency', 'D'});
    
    
    if (pipeline)
        %add integrator pipeline delays
        reuse_block(cursys, ['intpipeline', num2str(num_stages)], 'xbsIndex_r3/Delay',...
        {'Position', [310+100*num_stages 13 310+100*num_stages+55 13+75], 'latency', '1'});
    
        
        %add comb pipeline delays
        reuse_block(cursys, ['combpipeline', num2str(num_stages)], 'xbsIndex_r3/Delay',...
        {'Position', [520*stages+100*num_stages 13 520*stages+100*num_stages+55 13+75], 'latency', '1'});
    
    end
    num_stages = num_stages - 1;
                
end

% add lines
add_line(cursys, 'sync/1', 'syncdelay/1');
add_line(cursys, 'syncdelay/1', 'syncout/1');

num_inputs = inputs;
while (num_inputs > 0),

    add_line(cursys, ['cic_in', num2str(num_inputs), '/1'], ['adder_tree', '/', num2str(num_inputs)]);
    num_inputs = num_inputs - 1;
end



num_stages = stages;
while (num_stages > 0),
    if (decimation),
        if (num_stages == stages),
            % connect integrator to downsampler
            add_line(cursys, ['integrator', num2str(num_stages), '/1'], 'downsample/1');

        end
        if (num_stages == 1),         
            % connect downsampler to comb
            add_line(cursys, 'downsample/1', ['comb', num2str(num_stages), '/1']);
            add_line(cursys, 'downsample/1', ['delay', num2str(num_stages), '/1']);

            
        end
    else,
        %connect last integrator to first comb
        if (pipeline),
            if (num_stages == stages),
                add_line(cursys, ['integrator', num2str(num_stages), '/1'], ['intpipeline', num2str(num_stages), '/1']);
                add_line(cursys, ['intpipeline', num2str(num_stages), '/1'], 'comb1/1');
                add_line(cursys, ['intpipeline', num2str(num_stages), '/1'], 'delay1/1');
            end
        else,
            if (num_stages == stages),
            add_line(cursys, ['integrator', num2str(num_stages), '/1'], 'comb1/1');

            add_line(cursys, ['integrator', num2str(num_stages), '/1'], 'delay1/1');

            end
        end
    end
    %connect first integrator to adder tree
     if (num_stages == 1),
      add_line(cursys, 'adder_tree/1', ['integrator', num2str(num_stages),'/1']);

     end
        
    %connect output of integrator to its second input
    add_line(cursys, ['integrator', num2str(num_stages), '/1'], ['integrator', num2str(num_stages), '/2']);

    %connect output of integrator to next stage
    if (num_stages < stages),
        if (pipeline), 
            add_line(cursys, ['integrator', num2str(num_stages), '/1'], ['intpipeline', num2str(num_stages), '/1']);
            add_line(cursys, ['intpipeline', num2str(num_stages), '/1'], ['integrator', num2str(num_stages+1), '/1']);
        else,
            add_line(cursys, ['integrator', num2str(num_stages), '/1'], ['integrator', num2str(num_stages+1), '/1']);

        end
    end
           
            
    %connect output of comb to next stage
    if (num_stages == stages),
        if (pipeline == 1),
            add_line(cursys, ['comb', num2str(num_stages), '/1'], ['combpipeline', num2str(num_stages), '/1']);
            add_line(cursys, ['combpipeline', num2str(num_stages), '/1'], 'cic_out/1');
        else,
        add_line(cursys, ['comb', num2str(num_stages), '/1'], 'cic_out/1');

        end
        add_line(cursys, ['delay', num2str(num_stages), '/1'], ['comb', num2str(num_stages), '/2']);

    else,
        if (pipeline == 1),
            add_line(cursys, ['comb', num2str(num_stages), '/1'], ['combpipeline', num2str(num_stages), '/1']);
            add_line(cursys, ['combpipeline', num2str(num_stages), '/1'], ['comb', num2str(num_stages+1), '/1']);
        else,
            add_line(cursys, ['comb', num2str(num_stages), '/1'], ['comb', num2str(num_stages+1), '/1']);

        end
        add_line(cursys, ['comb', num2str(num_stages), '/1'], ['delay', num2str(num_stages+1), '/1']);

        add_line(cursys, ['delay', num2str(num_stages), '/1'], ['comb', num2str(num_stages), '/2']);

    end
    
    num_stages = num_stages - 1;
    
end
clean_blocks(cursys);
