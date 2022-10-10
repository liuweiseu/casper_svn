function pfb_coeff_gen_init(blk, varargin)
defaults = {};
if same_state(blk, 'defaults', defaults, varargin{:}), return, end
check_mask_type(blk, 'supercat');
munge_block(blk, varargin{:});
%disp('Running supercat init')

n = get_var('n', 'defaults', defaults, varargin{:});
lev1 = ceil(n/16);

delete_lines(blk);

reuse_block(blk, 'Out', 'built-in/outport', 'Position', [650 100 680 130]);

for k = 1:n
        reuse_block(blk, ['i',num2str(k)], 'built-in/inport', 'Position', [30 30+(50*(k-1)) 60 60+(50*(k-1))], 'Port', num2str(k));
end


for b = 1:lev1
    reuse_block(blk, ['Concat_',num2str(b)], 'xbsIndex_r3/Concat', ...
        'num_inputs', num2str(min(16,n-16*(b-1))), 'Position', [300 50+(250*(b-1)) 350 250+(250*(b-1))]);
    for k = (16*(b-1)+1):min(16*b,n)
        add_line(blk, ['i',num2str(k),'/1'], ['Concat_',num2str(b),'/',num2str(mod(k-1,16)+1)]);
    end
end
if lev1 == 1
    add_line(blk,'Concat_1/1','Out/1');
else
    reuse_block(blk,'OutConcat','xbsIndex_r3/Concat', 'num_inputs', num2str(lev1), ...
        'Position', [ 500 100 550 200]);
    add_line(blk,'OutConcat/1','Out/1');
    for b = 1:lev1
        add_line(blk, ['Concat_',num2str(b),'/1'],['OutConcat/',num2str(b)]);
    end
end
clean_blocks(blk);

save_state(blk, 'defaults', defaults, varargin{:});