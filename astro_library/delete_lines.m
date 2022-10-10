function delete_lines(cursys)
lines = get_param(cursys, 'Lines');
for i=1:length(lines),
    delete_line(lines(i).Handle);
end

