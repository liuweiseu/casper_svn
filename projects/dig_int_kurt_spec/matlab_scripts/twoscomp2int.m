function i = twoscomp2int(b)
% TWOSCOMP2INT(B) Convert a twos-complement binary number, B,
% represented as a row vector of zeros and ones, to an integer. Twoscomp2int
% operates on the rows of a matrix.
b=b';
[m, n] = size(b);
i = -b(:,1).*2.^(n-1) + b(:,2:end)*transpose(2.^(n-2:-1:0));