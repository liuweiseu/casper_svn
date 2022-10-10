function specacc_init(blk, varargin)

defaults = {};
disp('hi from datahalver_init')
L = get_var('input_length', 'defaults', defaults, varargin{:});

inlen = 2^L;
if inlen > 2^16,
    error('Input length must be < 2^16')
end
%       2^   0       1     2     3     4   5
fifolens = {'16', '16', '16', '16', '16', '32', '64', '128', '256', '512', '1K', '2K', '4K', '8K', '16K', '32K', '64K'};


set_param([blk, '/FIFO'], 'depth', fifolens{L+1});
set_param([blk, '/freeze_cntr'], 'CounterBits', num2str(L+1))
set_param([blk, '/sync_delay'], 'DelayLen', num2str(2^L+2))