function r = celleq(c1,c2)
if length(c1) ~= length(c2)
    r = 0;
    return
end
for k = 1:length(c1)
    if strcmp(c1{k},c2{k}) ~= 1
        r = k-1;
        return
    end
end
r = k;
return