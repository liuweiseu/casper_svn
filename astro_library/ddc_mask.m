cursys = gcb;
disp(cursys);

% round coefficients to make sure rounding error doesn't prevent us from
% detecting symmetric coefficients
lpfir_coeff = round(lpfir_coeff * 1e16) * 1e-16;

if log2(num_inputs) ~= round(log2(num_inputs)),
    error('The number of inputs must be a positive power of 2 integer');
end
if mod(length(lpfir_coeff)/num_inputs,1) ~= 0,
    error('The number of coefficients must be integer multiples of the number of inputs');
end

num_fir_col = length(lpfir_coeff)/num_inputs;
if lpfir_coeff(1:length(lpfir_coeff)/2) == lpfir_coeff(length(lpfir_coeff):-1:length(lpfir_coeff)/2+1),
    coeff_sym = 1;
else,
    coeff_sym = 0;
end

fir_col = find_system(cursys, 'lookUnderMasks', 'on', 'FollowLinks','on', 'SearchDepth', 1, 'masktype', 'fir_column');
fir_double_col = find_system(cursys, 'lookUnderMasks', 'on', 'FollowLinks','on', 'SeachDepth', 1, 'masktype', 'fir_double_column');
prev_fir_col = length(fir_col);
prev_fir_double_col = length(fir_double_col);

if (coeff_sym && prev_fir_double_col ~= num_fir_col/2) || (~coeff_sym && prev_fir_col ~= num_fir_col),
    delete_lines(cursys);    
    for i=1:num_inputs,
        reuse_block(cursys, ['din',num2str(i)], 'built-in/inport', {'Position', [130 i*80-10 160 5+80*i]});
        reuse_block(cursys, ['mixer',num2str(i)], 'ddc_library/mixer', ...
            {'Position', [230 i*80-10 280 50+80*i], 'latency', 'mult_latency',...
            'BitWidth', 'lo_bitwidth'});
    end
    reuse_block(cursys, 'dds', 'ddc_library/DDS', ...
        {'Position', [20 100 80 100+30*num_inputs], 'num_lo', 'num_inputs', 'freq', 'lo_freq',...
        'freq_div', 'freq_div', 'BitWidth', 'lo_bitwidth', 'latency','2'});    
    if coeff_sym,
        for i=1:num_fir_col/2,
            reuse_block(cursys, ['fir_double_column',num2str(i)], 'ddc_library/fir_double_column', ...
                {'Position', [i*300+200 50 i*300+300 50+80*num_inputs+40], 'num_inputs', 'num_inputs',...
                'coeff',['lpfir_coeff(',num2str(i*num_inputs),':-1:',num2str((i-1)*num_inputs+1),')'],...
                'mult_latency', 'mult_latency', 'add_latency', 'add_latency'});
        end
    else,
        for i=1:num_fir_col,
            reuse_block(cursys, ['fir_column',num2str(i)], 'ddc_library/fir_column', ...
                {'Position', [i*200+200 50 i*200+300 50+40*num_inputs+40], 'num_inputs', 'num_inputs',...
                'coeff',['lpfir_coeff(',num2str(i*num_inputs),':-1:',num2str((i-1)*num_inputs+1),')'],...
                'mult_latency', 'mult_latency'});
        end
    end

    if coeff_sym,
        for i=1:2*num_inputs,
            reuse_block(cursys, ['t',num2str(i)], 'built-in/Terminator', {'Position',[300+400 num_inputs*40+30+20*i 300+420 num_inputs*40+50+20*i]});
        end
    else,
        for i=1:2*num_inputs,
            reuse_block(cursys, ['t',num2str(i)], 'built-in/Terminator', {'Position',[num_fir_col*200+350 30+20*i num_fir_col*200+370 50+20*i]});
        end
    end

    reuse_block(cursys, 'real_sum', 'ddc_library/adder_tree', {'Position', [num_fir_col*200+400 150+40*num_inputs num_fir_col*200+460 150+40*num_inputs+20*num_fir_col]});
    reuse_block(cursys, 'imag_sum', 'ddc_library/adder_tree', {'Position', [num_fir_col*200+400 200+40*num_inputs+20*num_fir_col num_fir_col*200+460 200+40*num_inputs+20*num_fir_col+20*num_fir_col]});
    set_param([cursys, '/convert1'], 'Position', [num_fir_col*200+540 150+40*num_inputs num_fir_col*200+580 150+40*num_inputs+40]);
    set_param([cursys, '/convert2'], 'Position', [num_fir_col*200+540 200+40*num_inputs+20*num_fir_col num_fir_col*200+580 200+40*num_inputs+20*num_fir_col+40]);
    set_param([cursys, '/Shift'], 'Position', [num_fir_col*200+480 150+40*num_inputs num_fir_col*200+520 150+40*num_inputs+40]);
    set_param([cursys, '/Shift1'], 'Position', [num_fir_col*200+480 200+40*num_inputs+20*num_fir_col num_fir_col*200+520 200+40*num_inputs+20*num_fir_col+40]);
    set_param([cursys, '/ri_to_c'], 'Position', [num_fir_col*200+600 150+40*num_inputs num_fir_col*200+630 150+40*num_inputs+40]);
    
    reuse_block(cursys, 'dout', 'built-in/outport', {'Position', [num_fir_col*200+640 140+40*num_inputs+10*num_fir_col num_fir_col*200+655 150+40*num_inputs+10*num_fir_col]});
    
    if coeff_sym,
        set_param([cursys,'/imag_sum'],'num_inputs',num2str(num_fir_col/2),'latency',num2str(add_latency));
        set_param([cursys,'/real_sum'],'num_inputs',num2str(num_fir_col/2),'latency',num2str(add_latency));
    else,
        set_param([cursys,'/imag_sum'],'num_inputs',num2str(num_fir_col),'latency',num2str(add_latency));
        set_param([cursys,'/real_sum'],'num_inputs',num2str(num_fir_col),'latency',num2str(add_latency));
    end

    reuse_block(cursys, 'sync_in', 'built-in/inport', {'Position', [20 num_inputs*80+100 50 num_inputs*80+120], 'Port', num2str(num_inputs+1)});
    reuse_block(cursys, 'sync_out', 'built-in/outport', {'Position', [220 num_inputs*80+100 250 num_inputs*80+120]});
    reuse_block(cursys, 'delay', 'xbsIndex_r3/Delay', {'Position', [110 num_inputs*80+90 150 num_inputs*80+130]});
    if coeff_sym,
        sync_latency = 'mult_latency*2+2*add_latency+ceil(log2(num_inputs))*add_latency+ceil(log2(num_fir_col/2))*add_latency';
    else,
        sync_latency = 'mult_latency*2+ceil(log2(num_inputs))*add_latency+ceil(log2(num_fir_col))*add_latency+add_latency';
    end
    set_param([cursys,'/delay'],'latency', sync_latency);

    add_line(cursys,'real_sum/1','Shift/1');
    add_line(cursys,'imag_sum/1','Shift1/1');
    add_line(cursys,'Shift/1','convert1/1');
    add_line(cursys,'Shift1/1','convert2/1');
    add_line(cursys, 'convert1/1', 'ri_to_c/1');
    add_line(cursys, 'convert2/1', 'ri_to_c/2');
    add_line(cursys, 'ri_to_c/1', 'dout/1');
    add_line(cursys,'sync_in/1','delay/1');
    add_line(cursys,'delay/1','sync_out/1');

    for i=1:num_inputs,
        add_line(cursys,['din',num2str(i),'/1'],['mixer',num2str(i),'/1']);
        add_line(cursys,['dds/',num2str(i*2-1)],['mixer',num2str(i),'/2']);
        add_line(cursys,['dds/',num2str(i*2)],['mixer',num2str(i),'/3']);
    end

    if coeff_sym,
        for i=1:num_fir_col/2,
            for j=1:num_inputs,
                if i == 1,
                    add_line(cursys,['mixer',num2str(j),'/1'],['fir_double_column',num2str(i),'/',num2str(j*2-1)]);
                    add_line(cursys,['mixer',num2str(j),'/2'],['fir_double_column',num2str(i),'/',num2str(j*2)]);
                else,
                    add_line(cursys,['fir_double_column',num2str(i-1),'/',num2str(j*2-1)],['fir_double_column',num2str(i),'/',num2str(j*2-1)]);
                    add_line(cursys,['fir_double_column',num2str(i-1),'/',num2str(j*2)],['fir_double_column',num2str(i),'/',num2str(j*2)]);
                end
                if i ~= 1,
                    add_line(cursys,['fir_double_column',num2str(i),'/',num2str(2*num_inputs+j*2-1)],['fir_double_column',num2str(i-1),'/',num2str(2*num_inputs+j*2-1)]);
                    add_line(cursys,['fir_double_column',num2str(i),'/',num2str(2*num_inputs+j*2)],['fir_double_column',num2str(i-1),'/',num2str(2*num_inputs+j*2)]);
                end                   
            end
            add_line(cursys,['fir_double_column',num2str(i),'/',num2str(num_inputs*4+1)],['real_sum/',num2str(i)]);
            add_line(cursys,['fir_double_column',num2str(i),'/',num2str(num_inputs*4+2)],['imag_sum/',num2str(i)]);
        end
        for j=1:2*num_inputs,
            add_line(cursys,['fir_double_column1/',num2str(j+num_inputs*2)],['t',num2str(j),'/1']);
        end
        for j=1:num_inputs,
            add_line(cursys,['fir_double_column',num2str(num_fir_col/2),'/',num2str(j*2-1)],['fir_double_column',num2str(num_fir_col/2),'/',num2str(2*num_inputs+j*2-1)]);
            add_line(cursys,['fir_double_column',num2str(num_fir_col/2),'/',num2str(j*2)],['fir_double_column',num2str(num_fir_col/2),'/',num2str(2*num_inputs+j*2)]);
        end
    else,
        for i=1:num_fir_col,
            for j=1:num_inputs,
                if i == 1,
                    add_line(cursys,['mixer',num2str(j),'/1'],['fir_column',num2str(i),'/',num2str(j*2-1)]);
                    add_line(cursys,['mixer',num2str(j),'/2'],['fir_column',num2str(i),'/',num2str(j*2)]);
                else,
                    add_line(cursys,['fir_column',num2str(i-1),'/',num2str(j*2-1)],['fir_column',num2str(i),'/',num2str(j*2-1)]);
                    add_line(cursys,['fir_column',num2str(i-1),'/',num2str(j*2)],['fir_column',num2str(i),'/',num2str(j*2)]);
                end
            end
            add_line(cursys,['fir_column',num2str(i),'/',num2str(num_inputs*2+1)],['real_sum/',num2str(i)]);
            add_line(cursys,['fir_column',num2str(i),'/',num2str(num_inputs*2+2)],['imag_sum/',num2str(i)]);
        end
        for i=1:2*num_inputs,
            add_line(cursys,['fir_column',num2str(num_fir_col),'/',num2str(i)],['t',num2str(i),'/1']);
        end
    end
    clean_blocks(cursys);
end

if ~strcmp(get_param([cursys,'/convert1'], 'quantization'), quantization),
    set_param([cursys,'/convert1'], 'quantization', quantization);
    set_param([cursys,'/convert2'], 'quantization', quantization);
end