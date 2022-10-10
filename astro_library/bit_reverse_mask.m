cursys = gcb;
disp(cursys);

try,
    prev_bits = length(get_param(cursys, 'Lines')) - 2;
catch,
    prev_bits = -1;
end
if n_bits ~= prev_bits,
    delete_lines(cursys);
    if n_bits <= 1,
        add_line(cursys,['din/1'],['dout/1']);
    else,
        reuse_block(cursys, 'Concat', 'xbsIndex_r3/Concat', ...
            {'Position',[300 100 400 100+n_bits*20],'num_inputs',num2str(n_bits)});  
        for i=0:n_bits-1,
            reuse_block(cursys, ['slice',num2str(i)], 'xbsIndex_r3/Slice', ...
                {'Position', [100 100+i*40 140 120+i*40], 'mode', 'Lower Bit Location + Width', ...
                'nbits', '1', 'bit0', num2str(i)});
            add_line(cursys,['din/1'],['slice',num2str(i),'/1']);
            add_line(cursys,['slice',num2str(i),'/1'],['Concat/',num2str(i+1)]);
        end
        add_line(cursys,['Concat/1'],['dout/1']);
    end
    clean_blocks(cursys);
end
