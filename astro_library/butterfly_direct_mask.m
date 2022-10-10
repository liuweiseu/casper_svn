cursys = gcb;
disp(cursys);

twiddle = [cursys, '/twiddle'];
coeff_gen = [twiddle,'/twiddle_general_3mult/coeff_gen'];
br_coeff_gen = [coeff_gen,'/br_coeff_gen'];

% Compute the complex, bit-reversed values of the twiddle factors
br_indices = bit_rev(Coeffs, FFTSize-1);
br_indices = -2*pi*1j*br_indices/2^FFTSize;
ActualCoeffs = exp(br_indices);

% Optimize twiddler for coeff = 0, 1, or alternating 0-1
if length(Coeffs) == 1,
    if Coeffs(1) == 0,
        if ~strcmp(get_param(twiddle, 'BlockChoice'), 'twiddle_coeff_0'),
            set_param(twiddle, 'BlockChoice', 'twiddle_coeff_0');
        end
    elseif Coeffs(1) == 1,
        if ~strcmp(get_param(twiddle, 'BlockChoice'), 'twiddle_coeff_1'),
            set_param(twiddle, 'BlockChoice', 'twiddle_coeff_1');
        end
    else,
        if ~strcmp(get_param(twiddle, 'BlockChoice'), 'twiddle_general_3mult'),
            set_param(twiddle, 'BlockChoice', 'twiddle_general_3mult');
        end
        if ~strcmp(get_param(coeff_gen, 'BlockChoice'), 'static_coeff_gen'),
            set_param(coeff_gen, 'BlockChoice', 'static_coeff_gen');
        end
    end
elseif length(Coeffs)==2 && Coeffs(1)==0 && Coeffs(2)==1 && StepPeriod==FFTSize-2,
    if ~strcmp(get_param(twiddle, 'BlockChoice'), 'twiddle_stage_2'),
        set_param(twiddle, 'BlockChoice', 'twiddle_stage_2');
    end
else,
    if ~strcmp(get_param(twiddle, 'BlockChoice'), 'twiddle_general_3mult'),
        set_param(twiddle, 'BlockChoice', 'twiddle_general_3mult');
    end
    if ~strcmp(get_param(coeff_gen, 'BlockChoice'), 'br_coeff_gen'),
        set_param(coeff_gen, 'BlockChoice', 'br_coeff_gen');
    end
    if use_bram, dist_mem = 'off';
    else, dist_mem = 'on';
    end
    if ~strcmp(get_param([br_coeff_gen,'/ROM'], 'distributed_mem'), dist_mem),
        set_param([br_coeff_gen,'/ROM'], 'distributed_mem', dist_mem);
    end
    if ~strcmp(get_param([br_coeff_gen,'/ROM1'], 'distributed_mem'), dist_mem),
        set_param([br_coeff_gen,'/ROM1'], 'distributed_mem', dist_mem);
    end
end

% Propagate quantization behavior
if ~strcmp(get_param([cursys,'/Convert'], 'quantization'), quantization),
    set_param([cursys,'/Convert'], 'quantization', quantization);
    set_param([cursys,'/Convert1'], 'quantization', quantization);
    set_param([cursys,'/Convert2'], 'quantization', quantization);
    set_param([cursys,'/Convert3'], 'quantization', quantization);
end

% Propagate overflow behavior
if ~strcmp(get_param([cursys,'/Convert'], 'overflow'), overflow),
    set_param([cursys,'/Convert'],  'overflow', overflow);
    set_param([cursys,'/Convert1'], 'overflow', overflow);
    set_param([cursys,'/Convert2'], 'overflow', overflow);
    set_param([cursys,'/Convert3'], 'overflow', overflow);
end
