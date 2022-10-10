function [bramreal, bramimag] = comp2ri(ucdata, size)
% ucdata = unsigned complex array from BRAM dump
% size = size of ucdata array
u16mask = uint32(4294901760);
dum_real = zeros(size, 1);
dum_imag = zeros(size, 1);

for i = 1:size
%Break complex data into real (MSB) and imag (LSB)    
    dum_real(i,1) = uint16(bitand(ucdata(i,1), u16mask)/2^16) ;
    dum_imag(i,1) = uint16(bitand(ucdata(i,1), 65535));
    
    if dum_real(i,1) > intmax('int16')
        bramreal(i,1) = (int16(bitxor(dum_real(i,1), intmax('uint16'))+1))*-1;
    else
        bramreal(i,1) = dum_real(i,1);
    end
    
    if dum_imag(i,1) > intmax('int16')
        bramimag(i,1) = (int16(bitxor(dum_imag(i,1), intmax('uint16'))+1))*-1;
    else
        bramimag(i,1) = dum_imag(i,1);
    end
end