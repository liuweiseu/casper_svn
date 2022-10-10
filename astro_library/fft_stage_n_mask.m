cursys = gcb;
disp(cursys);

% Implement delays normally or in BRAM
delays = {'delay_f', 'delay_b'};
for i=1:length(delays),
    full_path = [cursys,'/',delays{i}];
    if DelayBram,
        delay_type = 'delay_bram';
    else,
        delay_type = 'delay_slr';
    end
    if ~strcmp(get_param(full_path, 'BlockChoice'), delay_type),
        set_param(full_path, 'BlockChoice', delay_type)
    end
end

Coeffs = 0:2^min(MaxCoeffNum,FFTStage-1)-1;
StepPeriod = FFTSize-FFTStage+max(0, FFTStage-MaxCoeffNum);

br_indices = bit_rev(Coeffs, FFTSize-1);
br_indices = -2*pi*1j*br_indices/2^FFTSize;
ActualCoeffs = exp(br_indices);
vlen = length(ActualCoeffs);

% Propagate quantization behavior
if ~strcmp(get_param([cursys,'/butterfly/Convert'], 'quantization'), quantization),
    set_param([cursys,'/butterfly/Convert'], 'quantization', quantization);
    set_param([cursys,'/butterfly/Convert1'], 'quantization', quantization);
    set_param([cursys,'/butterfly/Convert2'], 'quantization', quantization);
    set_param([cursys,'/butterfly/Convert3'], 'quantization', quantization);
end

% Propagate overflow behavior
if ~strcmp(get_param([cursys,'/butterfly/Convert'], 'overflow'), overflow),
    set_param([cursys,'/butterfly/Convert'],  'overflow', overflow);
    set_param([cursys,'/butterfly/Convert1'], 'overflow', overflow);
    set_param([cursys,'/butterfly/Convert2'], 'overflow', overflow);
    set_param([cursys,'/butterfly/Convert3'], 'overflow', overflow);
end

% Take care of storing coefficients in BRAM
roms = find_system(cursys, 'lookUnderMasks', 'all', 'FollowLinks','on','masktype', 'Xilinx Single Port Read-Only Memory');
if strcmp(CoeffBram, 'on'),
    dist_mem = 'off';
else,
    dist_mem = 'on';
end
if length(roms) > 0 && ~strcmp(get_param(roms{1}, 'distributed_mem'), dist_mem),
    for i=1:length(roms),
        set_param(roms{i}, 'distributed_mem', dist_mem);
    end
end


