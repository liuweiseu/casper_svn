function serialize_init(blk, varargin)
%n_inputs
%in_width
%depth

defaults = {};
%disp('hi from addracc_init')
if same_state(blk, 'defaults', defaults, varargin{:}), return, end
check_mask_type(blk, 'serialize');
munge_block(blk, varargin{:});
ninputs = get_var('ninputs', 'defaults', defaults, varargin{:});
in_width = get_var('in_width', 'defaults', defaults, varargin{:});
depth = get_var('depth', 'defaults', defaults, varargin{:});
mux_latency = get_var('mux_latency', 'defaults', defaults, varargin{:});

depth_bits = nextpow2(depth);
muxsel_bits = nextpow2(ninputs);
addr_bits = depth_bits + muxsel_bits;
if ninputs ~= 2^muxsel_bits,
    errordlg('Number of inputs must be power of 2 currently.');
end
if ninputs < 4
    errordlg('Currently only supports 4 or more inputs')
end

set_param([blk, '/commutator'], 'ninputs', num2str(ninputs));
set_param([blk, '/commutator'], 'in_width', num2str(in_width));
set_param([blk, '/commutator'], 'mux_latency', num2str(mux_latency));

set_param([blk, '/addr_cnt'], 'n_bits', num2str(depth_bits));

set_param([blk, '/Single Port RAM'], 'depth', num2str(depth));

set_param([blk, '/max_mux'], 'const', num2str(ninputs-3));
set_param([blk, '/max_mux'], 'n_bits', num2str(muxsel_bits));

set_param([blk, '/max_addr'], 'const', num2str((2^addr_bits)-4));
set_param([blk, '/max_addr'], 'n_bits', num2str(addr_bits));

save_state(blk, 'defaults', defaults, varargin{:});