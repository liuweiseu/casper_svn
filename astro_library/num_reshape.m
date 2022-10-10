function out = num_reshape(data,bin_pt)
for i = 1:length(data)
    if data(i) > 2^31
        tmp(i) = data(i)-2^32;
    else
        tmp(i) = data(i);
    end
end
temp = reshape(tmp./2^bin_pt,8,[])';
for i = 1:4
    out(:,i) = temp(:,2*i-1)+temp(:,2*i)*1i;
end