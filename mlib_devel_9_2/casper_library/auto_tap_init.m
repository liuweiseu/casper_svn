function auto_tap_init(blk, varargin)
% Initialize and configure a auto_tap block in the CASPER X Engine.
%
% auto_tap_init(blk, varargin)
%
% blk = The baseline_tap block
% varargin = {'varname', 'value', ...} pairs
% 
% Valid varnames for this block are:
% use_ded_mult = Use dedicated cores for multiplying (otherwise, use
% slices).

if same_state(blk, varargin{:}), return, end
check_mask_type(blk, 'auto_tap');
munge_block(blk, varargin{:});
use_ded_mult = get_var('use_ded_mult', varargin{:});

% Configure all multipliers in this block to use dedicated multipliers 
%(or not)
multipliers = find_system(blk, 'lookUnderMasks', 'all', 'FollowLinks','on', 'masktype', 'cmult*');
for i=1:length(multipliers),
    if use_ded_mult==2,
        set_param(multipliers{i}, 'BlockChoice', 'cmult_4bit_br*');
    elseif use_ded_mult==1,
        set_param(multipliers{i}, 'BlockChoice', 'cmult_4bit_em*');
    else,
        set_param(multipliers{i}, 'BlockChoice', 'cmult_4bit_sl*');
    end
end
% multipliers = find_system(blk, 'lookUnderMasks', 'all', 'FollowLinks','on', 'masktype', 'Xilinx Multiplier');
% for i=1:length(multipliers),
%     if use_ded_mult,
%         set_param(multipliers{i}, 'use_embedded', 'on');
%     else,
%         set_param(multipliers{i}, 'use_embedded', 'off');
%     end
% end

fmtstr = sprintf('mult=%d', use_ded_mult);
set_param(blk, 'AttributesFormatString', fmtstr);
save_state(blk, varargin{:});

