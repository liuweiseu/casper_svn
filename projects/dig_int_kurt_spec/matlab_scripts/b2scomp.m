function d = b2scomp(s)

% Converts twos complement binary integer strings to decimal
[m,n] = size(s);
if ~isstr(s)
 error('Input must be strings.')
end
x = abs(s) - abs('0'); % Convert from ASCII
if ~all(all(x >= 0) & all(x <= 1))
 error('Only ''0'' or ''1'' digits are allowed in the strings.')
end
d = -x(:,1); % Two's complement arithmetic
for i = 2:n
 d = 2*d + x(:,i);
end