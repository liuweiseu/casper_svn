cursys = gcb;
disp(cursys);
try,
    prev_bits = (length(get_param(cursys, 'Lines')) - 2)/2;
catch,
    prev_bits = -1;
end
if n_bits ~= prev_bits || length(old_map) ~= length(map) || any(old_map ~= map),
    delete_lines(cursys);
    set_param(cursys, 'old_map', mat2str(map));
    if n_bits < 1,
        add_line(cursys,['din/1'],['dout/1']);
    elseif n_bits == 1,
        if map == [0 1],
            add_line(cursys,['din/1'],['dout/1']);
        else,
            reuse_block(cursys, 'Not', 'xbsIndex_r3/Inverter', ...
                {'latency', 'latency', 'Position', [100 100 140 140]});
            add_line(cursys,['din/1'],['Not/1']);
            add_line(cursys,['Not/1'],['dout/1']);
        end
    else,
        n_bits_digits = length(num2str(n_bits));
        reuse_block(cursys, 'Concat', 'xbsIndex_r3/Concat', ...
            {'Position',[300 100 400 100+n_bits*20],'num_inputs',num2str(n_bits)});  
        for i=0:n_bits-1,
            reuse_block(cursys, ['slice',num2str(i)], 'xbsIndex_r3/Slice', ...
                {'Position', [100 100+i*40 140 120+i*40], 'mode', 'Lower Bit Location + Width', ...
                'nbits', '1', 'bit0', num2str(i)});
            espresso_input = ['.i ',num2str(n_bits),'\n.o 1\n'];
            mapbits = bitget(map, i+1);
            % Create file for espresso to reduce the logic of
            for j=0:2^n_bits-1,
                espresso_input = [espresso_input, dec2bin(j, n_bits), ' ', dec2bin(mapbits(j+1)), '\n'];
            end
            espresso_input = [espresso_input, '.e\n'];
            fid = fopen('espresso_in.tmp', 'w');
            fprintf(fid, espresso_input);
            fclose(fid);
            dos('espresso.exe espresso_in.tmp > espresso_out.tmp');
            lgc = '';
            fid = fopen('espresso_out.tmp', 'r');
            % Translate espresso output into matlab logic string
            while 1,
                tline = fgetl(fid);
                if ~ischar(tline), break, end
                if ~strncmp(tline, '.', 1),
                    if length(lgc) > 0,
                        lgc = [lgc, '|'];
                    end
                    tok = strtok(tline);
                    lgc = [lgc, '('];
                    expr = '';
                    toklen = length(tok);
                    for j=1:toklen,
                        inname = num2str(toklen-j);
                        while length(inname) < n_bits_digits,
                            inname = ['0', inname];
                        end
                        if strcmp(tok(j), '0'),
                            if length(expr) > 0,
                                expr = [expr, '&'];
                            end
                            expr = [expr, '~in',inname];
                        elseif strcmp(tok(j), '1'),
                            if length(expr) > 0,
                                expr = [expr, '&'];
                            end
                            expr = [expr, 'in', inname];
                        end
                    end
                    lgc = [lgc, expr, ')'];
                end
            end
            fclose(fid);
            dos('del espresso_in.tmp');
            dos('del espresso_out.tmp');
            % Idiocy to force the number of ports of expr block to be
            % n_bits
            for j=1:n_bits,
                inname = num2str(j-1);
                while length(inname) < n_bits_digits,
                   inname = ['0', inname];
                end
                lgc = [lgc, '|(in',inname,'&~in',inname,')'];
            end
            % End idiocy
            reuse_block(cursys, ['expr',num2str(i)], 'xbsIndex_r3/Expression', ...
                {'Position', [200 100+i*40 240 120+i*40], 'latency', 'latency', ...
                'expression', lgc});
        end
        for i=0:n_bits-1,
            add_line(cursys,['din/1'],['slice',num2str(i),'/1']);
            for j=0:n_bits-1,
                add_line(cursys, ['slice',num2str(j),'/1'], ['expr',num2str(i),'/',num2str(j+1)]);
            end
            add_line(cursys,['expr',num2str(i),'/1'],['Concat/',num2str(n_bits-i)]);
        end
        add_line(cursys,['Concat/1'],['dout/1']);
    end
    clean_blocks(cursys);
end
