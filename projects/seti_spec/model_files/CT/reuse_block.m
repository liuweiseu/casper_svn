function reuse_block(cursys, name, refblk, parameters)
existing_blk = find_system(cursys, 'lookUnderMasks', 'all', 'FollowLinks','on', ...
    'SearchDepth', 1, 'Name', name);
if length(existing_blk) == 0,
    args = [{refblk, [cursys,'/',name], 'Name', name}, parameters];
    builtin('add_block', args{:});
else,
    args = [{[cursys,'/',name]}, parameters];
    builtin('set_param', args{:});
end

