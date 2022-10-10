cursys = gcb;

stages = log2(num_inputs);
if (stages ~= round(stages) && stages ~= 0)
    error('The number of inputs must be a non-negative power of 2 interger');
end
inports = find_system(cursys, 'lookUnderMasks', 'on', 'FollowLinks','on','SearchDepth',1,'BlockType', 'Inport');
num_inports = length(inports);

if num_inports ~= num_inputs,
    delete_lines(cursys);

    % Add ports
    for i=1:num_inputs,
        reuse_block(cursys, ['din',num2str(i)], 'built-in/inport', {'Position', [30 i*40 60 15+40*i]});
    end
    reuse_block(cursys, 'dout', 'built-in/outport', {'Position', [30+(stages+1)*100 40 60+(stages+1)*100 55]});
    % Add Adders
    if stages==0
        add_line(cursys,'din1/1','dout/1');
    else
        for i=stages-1:-1:0,
            for j=1:2^i,
                blk = ['adder',num2str(stages-i),'_',num2str(j)];
                reuse_block(cursys, blk, 'xbsIndex_r3/AddSub', ...
                    {'latency', 'latency', 'pipeline', 'on', 'explicit_period', 'on',...
                    'Position', [30+(stages-i)*100 j*80-40 70+(stages-i)*100 j*80+20]});
                if i == stages-1,
                    add_line(cursys,['din',num2str((j*2-1)),'/1'],[blk,'/1']);
                    add_line(cursys,['din',num2str((j*2)),'/1'],[blk,'/2']);
                else,
                    add_line(cursys,['adder',num2str(stages-i-1),'_',num2str(j*2-1),'/1'],[blk,'/1']);
                    add_line(cursys,['adder',num2str(stages-i-1),'_',num2str(j*2),'/1'],[blk,'/2']);
                end
            end
        end
        add_line(cursys,['adder',num2str(stages),'_',num2str(1),'/1'],['dout/1']);
    end

    clean_blocks(cursys);
end