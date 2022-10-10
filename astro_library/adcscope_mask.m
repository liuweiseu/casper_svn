cursys = gcb;
disp(cursys)
delete_lines(cursys);


% determines number of ADC boards
numadcs = num_adcs;

% Add ports
% 'Position', [top left x, top left y, bottom right x, bottom right y],
disp('adding ports')

if (interleave),
        num_inputs = 0;
        while (num_inputs < 8),
            reuse_block(cursys, ['ADC0o', num2str(num_inputs)], 'built-in/inport',...
            {'Position', [0 (14+28)*num_inputs 30 (14+28)*num_inputs+14], 'Port', num2str(num_inputs+1)});
            num_inputs = num_inputs + 1;
        end
else,
    num_inputs = 0;
    while (num_inputs < 4),
        reuse_block(cursys, ['ADC0i', num2str(num_inputs)], 'built-in/inport', ...
        {'Position', [0 (14+28)*num_inputs 30 (14+28)*num_inputs+14], 'Port', num2str(num_inputs+1)});
        reuse_block(cursys, ['ADC0q', num2str(num_inputs)], 'built-in/inport', ...
        {'Position', [0 (14+28)*(4+num_inputs) 30 (14+28)*(4+num_inputs)+14], 'Port', num2str(num_inputs+4+1)});
        num_inputs = num_inputs + 1;
    end
end


if (numadcs == 2),
    if (interleave),
        num_inputs = 0;
        while (num_inputs < 8),
            reuse_block(cursys, ['ADC1o', num2str(num_inputs)], 'built-in/inport',...
                {'Position', [0 (14+28)*(8+num_inputs) 30 (14+28)*(8+num_inputs)+14], 'Port', num2str(8+num_inputs+1)});
            num_inputs = num_inputs + 1;
        end
    else,
        num_inputs = 0;
        while (num_inputs < 4),
            reuse_block(cursys, ['ADC1i', num2str(num_inputs)], 'built-in/inport', ...
            {'Position', [0 (14+28)*(8+num_inputs) 30 (14+28)*(8+num_inputs)+14], 'Port', num2str(8+num_inputs+1)});
            reuse_block(cursys, ['ADC1q', num2str(num_inputs)], 'built-in/inport', ...
            {'Position', [0 (14+28)*(12+num_inputs) 30 (14+28)*(12+num_inputs)+14], 'Port', num2str(8+num_inputs+4+1)});
            num_inputs = num_inputs + 1;
        end
    end
end


reuse_block(cursys, 'sim_capture_en', 'built-in/inport', ...
    {'Position', [0 (14+28)*(17) 30 (14+28)*(17)+14], 'Port', num2str((numadcs-1)*8+4+4+1)});
reuse_block(cursys, 'adc0_sync0', 'built-in/inport',...
    {'Position', [0 (14+28)*18 30 (14+28)*(18)+14], 'Port', num2str((numadcs-1)*8+4+4+2)});
if (numadcs == 2),
    reuse_block(cursys, 'adc1_sync0', 'built-in/inport',...
        {'Position', [0 (14+28)*19 30 (14+28)*(19)+14], 'Port', num2str((numadcs-1)*8+4+4+3)});
end

if (interleave),
    reuse_block(cursys, 'adc0ooro1', 'built-in/inport',...
    {'Position', [0 (14+28)*21 30 (14+28)*21+14], 'Port', num2str((numadcs-1)*8+4+4+4+(numadcs-2))});
reuse_block(cursys, 'adc0ooro3', 'built-in/inport',...
    {'Position', [0 (14+28)*22 30 (14+28)*22+14], 'Port', num2str((numadcs-1)*8+4+4+5+(numadcs-2))});
reuse_block(cursys, 'adc0ooro0', 'built-in/inport',...
    {'Position', [0 (14+28)*23 30 (14+28)*23+14], 'Port', num2str((numadcs-1)*8+4+4+6+(numadcs-2))});
reuse_block(cursys, 'adc0ooro2', 'built-in/inport',...
    {'Position', [0 (14+28)*24 30 (14+28)*24+14], 'Port', num2str((numadcs-1)*8+4+4+7+(numadcs-2))});

else,
reuse_block(cursys, 'adc0oori0', 'built-in/inport',...
    {'Position', [0 (14+28)*21 30 (14+28)*21+14], 'Port', num2str((numadcs-1)*8+4+4+4+(numadcs-2))});
reuse_block(cursys, 'adc0oori1', 'built-in/inport',...
    {'Position', [0 (14+28)*22 30 (14+28)*22+14], 'Port', num2str((numadcs-1)*8+4+4+5+(numadcs-2))});
reuse_block(cursys, 'adc0oorq0', 'built-in/inport',...
    {'Position', [0 (14+28)*23 30 (14+28)*23+14], 'Port', num2str((numadcs-1)*8+4+4+6+(numadcs-2))});
reuse_block(cursys, 'adc0oorq1', 'built-in/inport',...
    {'Position', [0 (14+28)*24 30 (14+28)*24+14], 'Port', num2str((numadcs-1)*8+4+4+7+(numadcs-2))});
end

if (numadcs==2),
    if (interleave),
        reuse_block(cursys, 'adc1ooro1', 'built-in/inport',...
    {'Position', [0 (14+28)*25 30 (14+28)*25+14], 'Port', num2str((numadcs-1)*8+4+4+8)});
reuse_block(cursys, 'adc1ooro3', 'built-in/inport',...
    {'Position', [0 (14+28)*26 30 (14+28)*26+14], 'Port', num2str((numadcs-1)*8+4+4+9)});
reuse_block(cursys, 'adc1ooro0', 'built-in/inport',...
    {'Position', [0 (14+28)*27 30 (14+28)*27+14], 'Port', num2str((numadcs-1)*8+4+4+10)});
reuse_block(cursys, 'adc1ooro2', 'built-in/inport',...
    {'Position', [0 (14+28)*28 30 (14+28)*28+14], 'Port', num2str((numadcs-1)*8+4+4+11)});

    else,
    reuse_block(cursys, 'adc1oori0', 'built-in/inport',...
    {'Position', [0 (14+28)*25 30 (14+28)*25+14], 'Port', num2str((numadcs-1)*8+4+4+8)});
reuse_block(cursys, 'adc1oori1', 'built-in/inport',...
    {'Position', [0 (14+28)*26 30 (14+28)*26+14], 'Port', num2str((numadcs-1)*8+4+4+9)});
reuse_block(cursys, 'adc1oorq0', 'built-in/inport',...
    {'Position', [0 (14+28)*27 30 (14+28)*27+14], 'Port', num2str((numadcs-1)*8+4+4+10)});
reuse_block(cursys, 'adc1oorq1', 'built-in/inport',...
    {'Position', [0 (14+28)*28 30 (14+28)*28+14], 'Port', num2str((numadcs-1)*8+4+4+11)});
    end
end

reuse_block(cursys, 'bram0_i_out', 'built-in/outport',...
    {'Position', [1000 100 1000+30 100+14], 'Port', '1'});
reuse_block(cursys, 'bram0_q_out', 'built-in/outport',...
    {'Position', [1000 100+50 1000+30 100+50+14], 'Port', '2'});
if (numadcs == 2),
    reuse_block(cursys, 'bram1_i_out', 'built-in/outport',...
        {'Position', [1000 100+100 1000+30 100+100+14], 'Port', '3'});
    reuse_block(cursys, 'bram1_q_out', 'built-in/outport',...
        {'Position', [1000 100+150 1000+30 100+150+14], 'Port', '4'});
end
reuse_block(cursys, 'sim_adcs', 'built-in/outport',...
    {'Position', [500 500+(50*(numadcs+2)) 500+30 500+14+(50*(numadcs+2))], 'Port', num2str(3+(2*numadcs-2))});
reuse_block(cursys, 'sim_sync0', 'built-in/outport',...
    {'Position', [500 500+50*(numadcs+3) 500+30 500+14+50*(numadcs+3)], 'Port', num2str(4+(2*numadcs-2))});
if (numadcs == 2),
    reuse_block(cursys, 'sim_sync1', 'built-in/outport',...
        {'Position', [500 500+50*(numadcs+4) 500+30 500+14+50*(numadcs+4)], 'Port', num2str(5+(2*numadcs-2))});
end

disp('finished adding ports')
% add blocks
disp('adding blocks')
% force blocks
num_inputs = numadcs*8;
while(num_inputs > 0),
    reuse_block(cursys, ['Reinterpret', num2str(num_inputs)], 'xbsIndex_r3/Reinterpret',...
        {'force_arith_type', 'on','arith_type', 'Unsigned', 'bin_pt', '0','force_bin_pt', 'on',...
        'Position', [150 (14+28)*num_inputs 150+30 (14+28)*num_inputs+15]});
    num_inputs = num_inputs - 1;
end
% overflow cast blocks
%num_inputs = numadcs*4;
%while (num_inputs > 0),
%    reuse_block(cursys, ['oorReinterpret', num2str(num_inputs-1)], 'xbsIndex_r3/Reinterpret',...
%        {'force_arith_type', 'on','arith_type', 'Unsigned', 'bin_pt', '0','force_bin_pt', 'on',...
%        'Position', [150 (14+28)*(21+num_inputs) 150+30 (14+28)*(21+num_inputs)+15]});
%    num_inputs = num_inputs - 1;
%end
num_inputs = numadcs*4;
while (num_inputs > 0),
    reuse_block(cursys, ['oorConvert', num2str(num_inputs-1)], 'xbsIndex_r3/Convert',...
        {'arith_type', 'Unsigned', 'n_bits', '8', 'bin_pt', '0',...
        'Position', [150 (14+28)*(21+num_inputs) 150+30 (14+28)*(21+num_inputs)+15]});
    num_inputs = num_inputs - 1;
end
% cat blocks
num_inputs = num_adcs*2;
while (num_inputs > 0),
    reuse_block(cursys, ['Concat', num2str(num_inputs)], 'xbsIndex_r3/Concat',...
        {'num_inputs', '4', 'Position', [300 (14+28+28)*num_inputs 300+50 (14+28+28)*num_inputs+50]});
    num_inputs = num_inputs - 1;
end
%overflow cat blocks
num_inputs = numadcs;
while (num_inputs > 0),
    reuse_block(cursys, ['oorConcat', num2str(num_inputs - 1)], 'xbsIndex_r3/Concat',...
        {'num_inputs', '4', 'Position', [300 (14+28)*(21+num_inputs)+75*num_inputs 300+50 (14+28)*(21+num_inputs)+75*num_inputs+50]});
    num_inputs = num_inputs - 1;
end

% software registers
reuse_block(cursys, 'capture_en_reg', 'xps_library/software register',...
    {'io_dir', 'From Processor', 'arith_type', 'Unsigned', 'bitwidth', '32', 'bin_pt', '0', 'sample_period', '1',...
    'Position', [250 (14+28)*17 250+200 (14+28)*17+50]});

reuse_block(cursys, 'numADCs', 'xps_library/software register',...
    {'io_dir', 'To Processor', 'arith_type', 'Unsigned', 'bitwidth', '32', 'bin_pt', '0', 'sample_period', '1',...
    'Position', [250 (14+28)*18 250+200 (14+28)*18+50]});

reuse_block(cursys, 'sync0', 'xps_library/software register',...
    {'io_dir', 'To Processor', 'arith_type', 'Unsigned', 'bitwidth', '32', 'bin_pt', '0', 'sample_period', '1',...
    'Position', [250 (14+28)*19 250+200 (14+28)*19+50]});
if (numadcs == 2),
    reuse_block(cursys, 'sync1', 'xps_library/software register',...
    {'io_dir', 'To Processor', 'arith_type', 'Unsigned', 'bitwidth', '32', 'bin_pt', '0', 'sample_period', '1',...
    'Position', [250 (14+28)*20 250+200 (14+28)*20+50]});
end

reuse_block(cursys, 'interleave_reg', 'xps_library/software register',...
    {'io_dir', 'To Processor', 'arith_type', 'Unsigned', 'bitwidth', '32', 'bin_pt', '0', 'sample_period', '1',...
    'Position', [250 (14+28)*21 250+200 (14+28)*21+50]});

reuse_block(cursys, 'bramrows', 'xps_library/software register',...
    {'io_dir', 'To Processor', 'arith_type', 'Unsigned', 'bitwidth', '32', 'bin_pt', '0', 'sample_period', '1',...
    'Position', [250 (14+28)*22 250+200 (14+28)*22+50]});

%rows in bram setting
reuse_block(cursys, 'bram_rows', 'xbsIndex_r3/Constant',...
    {'const', num2str(2^address_width), 'arith_type', 'Unsigned', 'n_bits', '32', 'bin_pt', '0', 'explicit_period', 'on',...
    'Position', [200 (14+28)*22 200+20 (14+28)*22+20]});

%interleave setting
reuse_block(cursys, 'interleaved', 'xbsIndex_r3/Constant',...
    {'const', num2str(interleave), 'arith_type', 'Unsigned', 'n_bits', '1', 'bin_pt', '0', 'explicit_period', 'on',...
    'Position', [200 (14+28)*21 200+20 (14+28)*21+20]});

%numADCs setting
reuse_block(cursys, 'adcs', 'xbsIndex_r3/Constant',...
    {'const', num2str(numadcs), 'arith_type', 'Unsigned', 'n_bits', num2str(numadcs), 'bin_pt', '0','explicit_period', 'on',...
    'Position', [80 (14+28)*18 115 (14+28)*18+20]});

%capture enable setting
reuse_block(cursys, 'slice', 'xbsIndex_r3/Slice',...
    {'mode', 'Lower Bit Location + Width', 'nbits', '1', 'bit0', '0', 'base0', 'LSB of Input', 'boolean_output', 'on', ...
    'Position', [475 (14+28)*9 475+30 (14+28)*9+14]});

%address counter reset
reuse_block(cursys, 'posedge', 'adcscope_lib/posedge',...
    {'Position', [525 (14+28)*8 525+40 (14+28)*8+20]});

%write enable / address counter enable
reuse_block(cursys, 'pulse_extender', 'adcscope_lib/pulse_extender',...
    {'pulse_len', '2^address_width-1', 'Position', [525 (14+28+14)*8 525+40 (14+28+14)*8+20]});

reuse_block(cursys, 'we', 'xbsIndex_r3/Logical',...
    {'logical_function', 'AND', 'inputs', '2', 'precision', 'Full', 'align_bp', 'on', 'latency', '0',...
    'Position', [590 (14+28+14)*8 590+20 (14+28+14)*8+20]});

%address counter
reuse_block(cursys, 'bram_address', 'xbsIndex_r3/Counter',...
    {'cnt_type', 'Free Running', 'n_bits', 'address_width', 'bin_pt', '0', 'arith_type', 'Unsigned',...
    'start_count', '0', 'cnt_by_val', '1', 'operation', 'Up', 'rst', 'on', 'en', 'on',...
    'Position', [625 (14+28)*8 625+50 (14+28)*8+50]});

% bram

reuse_block(cursys, 'bram0_i', 'xps_library/Shared BRAM',...
    {'arith_type', 'Unsigned', 'addr_width', 'address_width', 'data_width', '32', 'data_bin_pt', '0',...
    'init_vals', 'zeros(1, 2^address_width)', 'sample_rate', '1',...
    'Position', [795 (14+28)*num_inputs 795+100 (14+28)*num_inputs+100]});
reuse_block(cursys, 'bram0_q', 'xps_library/Shared BRAM',...
    {'arith_type', 'Unsigned', 'addr_width', 'address_width', 'data_width', '32', 'data_bin_pt', '0',...
    'init_vals', 'zeros(1, 2^address_width)', 'sample_rate', '1',...
    'Position', [795 (14+28)*num_inputs+150 795+100 (14+28)*num_inputs+250]});

if (numadcs == 2), 
reuse_block(cursys, 'bram1_i', 'xps_library/Shared BRAM',...
    {'arith_type', 'Unsigned', 'addr_width', 'address_width', 'data_width', '32', 'data_bin_pt', '0',...
    'init_vals', 'zeros(1, 2^address_width)', 'sample_rate', '1',...
    'Position', [795 (14+28)*num_inputs+300 795+100 (14+28)*num_inputs+400]});
reuse_block(cursys, 'bram1_q', 'xps_library/Shared BRAM',...
    {'arith_type', 'Unsigned', 'addr_width', 'address_width', 'data_width', '32', 'data_bin_pt', '0',...
    'init_vals', 'zeros(1, 2^address_width)', 'sample_rate', '1',...
    'Position', [795 (14+28)*num_inputs+450 795+100 (14+28)*num_inputs+550]});
end

%overflow bram
num_inputs = numadcs;
while (num_inputs > 0),
    reuse_block(cursys, ['oorbram', num2str(num_inputs - 1)], 'xps_library/Shared BRAM',...
        {'arith_type', 'Unsigned', 'addr_width', 'address_width', 'data_width', '32', 'data_bin_pt', '0',...
    'init_vals', 'zeros(1, 2^address_width)', 'sample_rate', '1',...
    'Position', [795 (14+28)*(5+num_inputs)+75*num_inputs+450 795+100 (14+28)*(5+num_inputs)+75*num_inputs+550]});
num_inputs = num_inputs - 1;
end

  
% add lines (outport, inport)
disp('adding lines')
%register connections
add_line(cursys, 'sim_capture_en/1', 'capture_en_reg/1');
add_line(cursys, 'capture_en_reg/1', 'slice/1');
add_line(cursys, 'adc0_sync0/1', 'sync0/1');
add_line(cursys, 'sync0/1', 'sim_sync0/1');
if (numadcs==2),
    add_line(cursys, 'adc1_sync0/1', 'sync1/1');
    add_line(cursys, 'sync1/1', 'sim_sync1/1');
end
add_line(cursys, 'adcs/1', 'numADCs/1');
add_line(cursys, 'numADCs/1', 'sim_adcs/1');
add_line(cursys, 'interleaved/1', 'interleave_reg/1');
add_line(cursys, 'bram_rows/1', 'bramrows/1');
disp('done with registers')

%adc-force connections
if (interleave),
    disp('interleaving')
    num_inputs = numadcs*8;
    while (num_inputs > 0),
        if (num_inputs > 8),
            add_line(cursys, ['ADC1o', num2str(num_inputs-8-1), '/1'], ['Reinterpret', num2str(num_inputs), '/1']);
        else,
            add_line(cursys, ['ADC0o', num2str(num_inputs-1),'/1'], ['Reinterpret', num2str(num_inputs), '/1']);
        end
        num_inputs = num_inputs - 1;
    end
    
else,
num_inputs = numadcs*8;
while(num_inputs > 0),
    if(num_inputs > 12),
       
        add_line(cursys, ['ADC1q', num2str(mod(num_inputs-1, 4)), '/1'],['Reinterpret', num2str(num_inputs), '/1']);
    elseif (num_inputs > 8),
       
        add_line(cursys,['ADC1i', num2str(mod(num_inputs-1,4)), '/1'],['Reinterpret', num2str(num_inputs), '/1'] );
    elseif (num_inputs > 4),
       
        add_line(cursys,['ADC0q', num2str(mod(num_inputs-1, 4)), '/1'],['Reinterpret', num2str(num_inputs), '/1'] );
    else,
        add_line(cursys,['ADC0i', num2str(mod(num_inputs-1, 4)), '/1'],['Reinterpret', num2str(num_inputs), '/1'] );
    end
    num_inputs = num_inputs - 1;
    
end
end
disp('done with adc-force')
%force-cat connections
num_inputs = numadcs*8;
while (num_inputs > 0),
    if (num_inputs > 12),
        add_line(cursys, ['Reinterpret', num2str(num_inputs), '/1'], ['Concat4','/', num2str(1+mod(num_inputs-1, 4))]);
    elseif (num_inputs > 8),
        add_line(cursys, ['Reinterpret', num2str(num_inputs), '/1'], ['Concat3','/',num2str(1+mod(num_inputs-1, 4)) ]);
    elseif (num_inputs > 4),
        add_line(cursys, ['Reinterpret', num2str(num_inputs), '/1'], ['Concat2','/',num2str(1+mod(num_inputs-1, 4))]);
    else,
        add_line(cursys, ['Reinterpret', num2str(num_inputs), '/1'], ['Concat1','/',num2str(1+mod(num_inputs-1, 4))]);
    end
    num_inputs = num_inputs - 1;
end
disp('done with force-cat')
%cat-bram connections
add_line(cursys, 'Concat1/1', 'bram0_i/2');
add_line(cursys, 'Concat2/1', 'bram0_q/2');
if (numadcs == 2),
    add_line(cursys, 'Concat3/1', 'bram1_i/2');
    add_line(cursys, 'Concat4/1', 'bram1_q/2');
end
disp('done with cat-bram')
%bram-out connections
add_line(cursys, 'bram0_i/1', 'bram0_i_out/1');
add_line(cursys, 'bram0_q/1', 'bram0_q_out/1');
if (numadcs == 2),
    add_line(cursys, 'bram1_i/1', 'bram1_i_out/1');
    add_line(cursys, 'bram1_q/1', 'bram1_q_out/1');
end
disp('done with bram-out')
%bram_address-bram connections
add_line(cursys, 'bram_address/1', 'bram0_i/1');
add_line(cursys, 'bram_address/1', 'bram0_q/1');
add_line(cursys, 'bram_address/1', 'oorbram0/1');
if (numadcs == 2),
    add_line(cursys, 'bram_address/1', 'bram1_i/1');
    add_line(cursys, 'bram_address/1', 'bram1_q/1');
    add_line(cursys, 'bram_address/1', 'oorbram1/1');
end

%posedge and pulse_extender connections
add_line(cursys, 'posedge/1', 'bram_address/1');
add_line(cursys, 'pulse_extender/1', 'we/2');
add_line(cursys, 'slice/1', 'we/1');
%add_line(cursys, 'pulse_extender/1', 'bram_address/2');
add_line(cursys,'we/1', 'bram_address/2');
add_line(cursys, 'slice/1', 'posedge/1');
add_line(cursys, 'slice/1', 'pulse_extender/1');
%add_line(cursys, 'pulse_extender/1', 'bram0_i/3');
add_line(cursys, 'we/1', 'bram0_i/3');
%add_line(cursys, 'pulse_extender/1', 'bram0_q/3');
add_line(cursys, 'we/1', 'bram0_q/3');
add_line(cursys, 'we/1', 'oorbram0/3');
if (numadcs == 2),
%    add_line(cursys, 'pulse_extender/1', 'bram1_i/3');
add_line(cursys, 'we/1', 'bram1_i/3');
%    add_line(cursys, 'pulse_extender/1', 'bram1_q/3');
    add_line(cursys, 'we/1', 'bram1_q/3');
    add_line(cursys, 'we/1', 'oorbram1/3');
end

%overflow connections
if (interleave),
    add_line(cursys, 'adc0ooro1/1','oorConvert0/1');
    add_line(cursys, 'adc0ooro3/1','oorConvert1/1');
    add_line(cursys, 'adc0ooro0/1','oorConvert2/1');
    add_line(cursys, 'adc0ooro2/1','oorConvert3/1');
    
    if (numadcs == 2),
        add_line(cursys, 'adc1ooro1/1','oorConvert4/1');
        add_line(cursys, 'adc1ooro3/1','oorConvert5/1');
        add_line(cursys, 'adc1ooro0/1','oorConvert6/1');
        add_line(cursys, 'adc1ooro2/1','oorConvert7/1');
    end

    else,
        add_line(cursys, 'adc0oori0/1','oorConvert0/1');
        add_line(cursys, 'adc0oori1/1','oorConvert1/1');
        add_line(cursys, 'adc0oorq0/1','oorConvert2/1');
        add_line(cursys, 'adc0oorq1/1','oorConvert3/1');
        
        if (numadcs == 2),
            add_line(cursys, 'adc1oori0/1','oorConvert4/1');
            add_line(cursys, 'adc1oori1/1','oorConvert5/1');
            add_line(cursys, 'adc1oorq0/1','oorConvert6/1');
            add_line(cursys, 'adc1oorq1/1','oorConvert7/1');
        end
end

            



num_inputs = numadcs*4;
while (num_inputs > 0),
    if (num_inputs > 4),
        add_line(cursys, ['oorConvert', num2str(num_inputs-1), '/1'], ['oorConcat1','/',num2str(1+mod(num_inputs-1, 4))]);
    else,
        add_line(cursys, ['oorConvert', num2str(num_inputs-1), '/1'], ['oorConcat0','/',num2str(1+mod(num_inputs-1, 4))]);
    end
    num_inputs = num_inputs - 1;
end

add_line(cursys, 'oorConcat0/1', 'oorbram0/2');
if (numadcs == 2),
    add_line(cursys, 'oorConcat1/1', 'oorbram1/2');
end



clean_blocks(cursys);
