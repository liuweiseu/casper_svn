% adcddcxaui_100/XSG core config
adcddcxaui_100_XSG_core_config_type         = 'xps_xsg';
adcddcxaui_100_XSG_core_config_param        = '';

% adcddcxaui_100/XAUI
adcddcxaui_100_XAUI_type         = 'xps_xaui';
adcddcxaui_100_XAUI_param        = '';
adcddcxaui_100_XAUI_ip_name      = 'XAUI_interface';

% adcddcxaui_100/adc
adcddcxaui_100_adc_type         = 'xps_adc';
adcddcxaui_100_adc_param        = 'adc = adc0 / interleave = off';
adcddcxaui_100_adc_ip_name      = 'adc_interface';

% adcddcxaui_100/data_sel
adcddcxaui_100_data_sel_type         = 'xps_sw_reg';
adcddcxaui_100_data_sel_param        = 'in';
adcddcxaui_100_data_sel_ip_name      = 'opb_register_ppc2simulink';

% adcddcxaui_100/gpio
adcddcxaui_100_gpio_type         = 'xps_gpio';
adcddcxaui_100_gpio_param        = '';
adcddcxaui_100_gpio_ip_name      = 'gpio_simulink2ext';

% adcddcxaui_100/snap_dec/addr
adcddcxaui_100_snap_dec_addr_type         = 'xps_sw_reg';
adcddcxaui_100_snap_dec_addr_param        = 'out';
adcddcxaui_100_snap_dec_addr_ip_name      = 'opb_register_simulink2ppc';

% adcddcxaui_100/snap_dec/bram
adcddcxaui_100_snap_dec_bram_type         = 'xps_bram';
adcddcxaui_100_snap_dec_bram_param        = '2048';
adcddcxaui_100_snap_dec_bram_ip_name      = 'bram_block';

% adcddcxaui_100/snap_dec/ctrl
adcddcxaui_100_snap_dec_ctrl_type         = 'xps_sw_reg';
adcddcxaui_100_snap_dec_ctrl_param        = 'in';
adcddcxaui_100_snap_dec_ctrl_ip_name      = 'opb_register_ppc2simulink';

% adcddcxaui_100/snap_dec_we
adcddcxaui_100_snap_dec_we_type         = 'xps_sw_reg';
adcddcxaui_100_snap_dec_we_param        = 'in';
adcddcxaui_100_snap_dec_we_ip_name      = 'opb_register_ppc2simulink';

% adcddcxaui_100/tvg/bram_wr_reg
adcddcxaui_100_tvg_bram_wr_reg_type         = 'xps_sw_reg';
adcddcxaui_100_tvg_bram_wr_reg_param        = 'in';
adcddcxaui_100_tvg_bram_wr_reg_ip_name      = 'opb_register_ppc2simulink';

% adcddcxaui_100/tvg/im_we
adcddcxaui_100_tvg_im_we_type         = 'xps_sw_reg';
adcddcxaui_100_tvg_im_we_param        = 'in';
adcddcxaui_100_tvg_im_we_ip_name      = 'opb_register_ppc2simulink';

% adcddcxaui_100/tvg/ram_im
adcddcxaui_100_tvg_ram_im_type         = 'xps_bram';
adcddcxaui_100_tvg_ram_im_param        = '2048';
adcddcxaui_100_tvg_ram_im_ip_name      = 'bram_block';

% adcddcxaui_100/tvg/ram_re
adcddcxaui_100_tvg_ram_re_type         = 'xps_bram';
adcddcxaui_100_tvg_ram_re_param        = '2048';
adcddcxaui_100_tvg_ram_re_ip_name      = 'bram_block';

% adcddcxaui_100/tvg/re_we
adcddcxaui_100_tvg_re_we_type         = 'xps_sw_reg';
adcddcxaui_100_tvg_re_we_param        = 'in';
adcddcxaui_100_tvg_re_we_ip_name      = 'opb_register_ppc2simulink';

% adcddcxaui_100/tvg/snap_im/addr
adcddcxaui_100_tvg_snap_im_addr_type         = 'xps_sw_reg';
adcddcxaui_100_tvg_snap_im_addr_param        = 'out';
adcddcxaui_100_tvg_snap_im_addr_ip_name      = 'opb_register_simulink2ppc';

% adcddcxaui_100/tvg/snap_im/bram
adcddcxaui_100_tvg_snap_im_bram_type         = 'xps_bram';
adcddcxaui_100_tvg_snap_im_bram_param        = '2048';
adcddcxaui_100_tvg_snap_im_bram_ip_name      = 'bram_block';

% adcddcxaui_100/tvg/snap_im/ctrl
adcddcxaui_100_tvg_snap_im_ctrl_type         = 'xps_sw_reg';
adcddcxaui_100_tvg_snap_im_ctrl_param        = 'in';
adcddcxaui_100_tvg_snap_im_ctrl_ip_name      = 'opb_register_ppc2simulink';

% adcddcxaui_100/tvg/snap_re/addr
adcddcxaui_100_tvg_snap_re_addr_type         = 'xps_sw_reg';
adcddcxaui_100_tvg_snap_re_addr_param        = 'out';
adcddcxaui_100_tvg_snap_re_addr_ip_name      = 'opb_register_simulink2ppc';

% adcddcxaui_100/tvg/snap_re/bram
adcddcxaui_100_tvg_snap_re_bram_type         = 'xps_bram';
adcddcxaui_100_tvg_snap_re_bram_param        = '2048';
adcddcxaui_100_tvg_snap_re_bram_ip_name      = 'bram_block';

% adcddcxaui_100/tvg/snap_re/ctrl
adcddcxaui_100_tvg_snap_re_ctrl_type         = 'xps_sw_reg';
adcddcxaui_100_tvg_snap_re_ctrl_param        = 'in';
adcddcxaui_100_tvg_snap_re_ctrl_ip_name      = 'opb_register_ppc2simulink';

% adcddcxaui_100/tvg/tvg_ctrl
adcddcxaui_100_tvg_tvg_ctrl_type         = 'xps_sw_reg';
adcddcxaui_100_tvg_tvg_ctrl_param        = 'in';
adcddcxaui_100_tvg_tvg_ctrl_ip_name      = 'opb_register_ppc2simulink';

% adcddcxaui_100/tx_nvalid
adcddcxaui_100_tx_nvalid_type         = 'xps_sw_reg';
adcddcxaui_100_tx_nvalid_param        = 'in';
adcddcxaui_100_tx_nvalid_ip_name      = 'opb_register_ppc2simulink';

% PLB to OPB bridge added at 0xD1000000

