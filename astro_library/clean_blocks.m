function clean_blocks(cursys)
% Removes any blocks from a subsystem which are not connected
% to the rest of the system.
blocks = get_param(cursys, 'Blocks');
for i=1:length(blocks),
    blk = [cursys,'/',blocks{i}];
    ports = get_param(blk, 'PortConnectivity');
    flag = 0;
    for j=1:length(ports),
        if ports(j).SrcBlock == -1,
            flag = 0;
            break
        elseif ~flag && (length(ports(j).SrcBlock) ~= 0 || length(ports(j).DstBlock) ~= 0),
            flag = 1;
        end
    end
    if flag == 0,
        delete_block(blk);
    end
end

