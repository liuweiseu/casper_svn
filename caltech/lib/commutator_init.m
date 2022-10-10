function commutator_init(blk, varargin)
%n_inputs
%in_width

defaults = {};
if same_state(blk, 'defaults', defaults, varargin{:}), return, end
check_mask_type(blk, 'commutator');
munge_block(blk, varargin{:});
ninputs = get_var('ninputs', 'defaults', defaults, varargin{:});
in_width = get_var('in_width', 'defaults', defaults, varargin{:});
mux_latency = get_var('mux_latency', 'defaults', defaults, varargin{:});

muxsel_bits = nextpow2(ninputs);
if ninputs ~= 2^muxsel_bits,
    errordlg('Number of inputs must be power of 2 currently.');
end

delete_lines(blk);

set_param([blk, '/index_cnt'],'n_bits',num2str(muxsel_bits));
set_param([blk, '/uncram'], 'num_slice', num2str(ninputs));
set_param([blk, '/uncram'], 'slice_width', num2str(in_width));
set_param([blk, '/Mux'], 'inputs', num2str(ninputs), 'latency', num2str(mux_latency));

add_line(blk, 'reset/1', 'index_cnt/1');
add_line(blk, 'enable/1', 'index_cnt/2');
add_line(blk, 'din/1', 'uncram/1');
add_line(blk, 'index_cnt/1', 'index/1');
add_line(blk, 'index_cnt/1', 'Mux/1');
add_line(blk, 'Mux/1', 'dout/1');

for j = 1:ninputs
    add_line(blk, ['uncram/', num2str(j)], ['Mux/', num2str(j+1)]);
end

save_state(blk, 'defaults', defaults, varargin{:});
