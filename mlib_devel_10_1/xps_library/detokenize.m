function detokenize(in_fid, out_fid, xps_objs);

xsg_obj = xps_objs{1};

hw_sys       = get(xsg_obj,'hw_sys');
sw_os        = get(xsg_obj,'sw_os');
mpc_type     = get(xsg_obj,'mpc_type');
app_clk      = get(xsg_obj,'clk_src');
app_clk_rate = get(xsg_obj,'clk_rate');

while 1
    line = fgets(in_fid);
    if ~ischar(line)
        break;
    else
        toks = regexp(line,'(.*)#IF#(.*)#(.*)','tokens');
        if isempty(toks)
            fprintf(out_fid,line);
        else
            default   = toks{1}{1};
            condition = toks{1}{2};
            real_line = toks{1}{3};
            condition_met = 0;
            for i = 1:length(xps_objs)
                b = xps_objs{i};
                try
                    if eval(condition)
                        condition_met = 1;
                        try
                            real_line = eval(real_line);
                        end
                        fprintf(out_fid,real_line);
                        break;
                    end
                end
            end
            if ~condition_met & ~isempty(default)
                fprintf(out_fid, [default, '\n']);
            end
        end
    end
end

