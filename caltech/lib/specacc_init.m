function specacc_init(blk, varargin)
%spec_len
%num_inputs
%din_width
%max_spec
%add_latency
%bram_latency

defaults = {};
%disp('hi from specacc_init')
if same_state(blk, 'defaults', defaults, varargin{:}), return, end
check_mask_type(blk, 'specacc');
munge_block(blk, varargin{:});
spec_len = get_var('spec_len', 'defaults', defaults, varargin{:});
num_inputs = get_var('num_inputs', 'defaults', defaults, varargin{:});
din_width = get_var('din_width', 'defaults', defaults, varargin{:});
max_spec = get_var('max_spec', 'defaults', defaults, varargin{:});
add_latency = get_var('add_latency', 'defaults', defaults, varargin{:});
bram_latency = get_var('bram_latency', 'defaults', defaults, varargin{:});

spec_bits = nextpow2(spec_len);
cnt_width = nextpow2(max_spec);
acc_width = din_width + nextpow2(max_spec);
acc_bin_pt = din_width - 1;

addracc = [blk, '/addracc']
set_param(addracc,'ninputs',num2str(num_inputs));
set_param(addracc,'depth',num2str(spec_len));
set_param(addracc,'in_width',num2str(din_width));
set_param(addracc,'out_width',num2str(acc_width));
set_param(addracc,'bin_pt',num2str(acc_bin_pt));
set_param(addracc,'add_latency',num2str(add_latency));
set_param(addracc,'bram_latency',num2str(bram_latency));

set_param([blk, '/spec_zero'],'n_bits',num2str(cnt_width));
set_param([blk, '/spec_counter'],'n_bits',num2str(cnt_width));
set_param([blk, '/last_addr'],'n_bits',num2str(spec_bits),'const',num2str(spec_len-1));
set_param([blk, '/addr_counter'],'n_bits',num2str(spec_bits));
set_param([blk, '/sync_delay'],'DelayLen',num2str(spec_len+4));


fmtstr = sprintf('acc_widdth=%d, spec_bits=%d\nninputs=%d\ncount_width=%d', acc_width, spec_bits,num_inputs,cnt_width);
set_param(gcb, 'AttributesFormatString', fmtstr);

save_state(blk, 'defaults', defaults, varargin{:});