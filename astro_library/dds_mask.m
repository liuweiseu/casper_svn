cursys = gcb;
disp(cursys);

counter_width = ceil(log2(freq_div));
counter_step = mod(num_lo*freq,freq_div);

if num_lo < 1 | log2(num_lo) ~= round(log2(num_lo))
    error('The number of parallel LOs must be a power of 2 no less than 1');
end
if freq < 0 | freq ~= round(freq)
    error('The frequency factor must be a positive integer');
end

if freq_div <= 0 | freq_div < num_lo | freq_div ~= round(freq_div) | freq_div/num_lo ~= round(freq_div/num_lo) | log2(freq_div) ~= round(log2(freq_div))
    error('The frequency factor must be a positive power of 2 integer multiples of the number of LOs');
end

lo_osc = find_system(cursys, 'lookUnderMasks', 'on', 'FollowLinks','on','masktype', 'ddc_lo_osc');
lo_const = find_system(cursys, 'lookUnderMasks', 'on', 'FollowLinks','on','masktype', 'ddc_lo_const');
prev_lo_osc = length(lo_osc);
prev_lo_const = length(lo_const);
if (counter_step == 0 && prev_lo_const ~= num_lo) || (counter_step ~= 0 && prev_lo_osc ~= num_lo),
    delete_lines(cursys);
    for i=0:num_lo-1,
        sin_name = ['sin',num2str(i)];
        cos_name = ['cos',num2str(i)];
        lo_name = ['ddc_lo',num2str(i)];
        % Add ports
        reuse_block(cursys, sin_name, 'built-in/outport', {'Position', [135 35+i*100 175 55+100*i]});
        reuse_block(cursys, cos_name, 'built-in/outport', {'Position', [135 65+i*100 175 85+100*i]});
        % Add LOs
        reuse_block(cursys, lo_name, 'ddc_lo_lib/ddc_lo', {'Position', [30 i*100+30 70 i*100+90]});
        if counter_step == 0,
            set_param([cursys,'/',lo_name], 'BlockChoice', 'ddc_lo_const');
            set_param([cursys,'/',lo_name,'/ddc_lo_const'], 'BitWidth', 'BitWidth', ...
                'Phase', num2str(2*pi*freq*i/freq_div));
        else,
            set_param([cursys,'/',lo_name], 'BlockChoice', 'ddc_lo_osc');
            disp('hi');
            set_param([cursys,'/',lo_name,'/ddc_lo_osc'], 'BitWidth', 'BitWidth', ...
                'latency', 'latency', 'counter_width', 'counter_width', ...
                'counter_start', num2str(mod(i*freq,freq_div)), 'counter_step', num2str(counter_step));
            disp('lo');
        end
        add_line(cursys,[lo_name,'/1'],[sin_name,'/1']);
        add_line(cursys,[lo_name,'/2'],[cos_name,'/1']);
    end
    clean_blocks(cursys);
end
