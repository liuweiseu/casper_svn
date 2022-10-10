% testblk_uu/XSG core config
testblk_uu_XSG_core_config_type         = 'xps_xsg';
testblk_uu_XSG_core_config_param        = '';

% testblk_uu/data_in_mux/data_sel
testblk_uu_data_in_mux_data_sel_type         = 'xps_sw_reg';
testblk_uu_data_in_mux_data_sel_param        = 'in';
testblk_uu_data_in_mux_data_sel_ip_name      = 'opb_register_ppc2simulink';

% testblk_uu/data_in_mux/gpio1
testblk_uu_data_in_mux_gpio1_type         = 'xps_gpio';
testblk_uu_data_in_mux_gpio1_param        = '';
testblk_uu_data_in_mux_gpio1_ip_name      = 'gpio_ext2simulink';

% testblk_uu/data_in_mux/tvg_in/bram_wr_reg
testblk_uu_data_in_mux_tvg_in_bram_wr_reg_type         = 'xps_sw_reg';
testblk_uu_data_in_mux_tvg_in_bram_wr_reg_param        = 'in';
testblk_uu_data_in_mux_tvg_in_bram_wr_reg_ip_name      = 'opb_register_ppc2simulink';

% testblk_uu/data_in_mux/tvg_in/im_we
testblk_uu_data_in_mux_tvg_in_im_we_type         = 'xps_sw_reg';
testblk_uu_data_in_mux_tvg_in_im_we_param        = 'in';
testblk_uu_data_in_mux_tvg_in_im_we_ip_name      = 'opb_register_ppc2simulink';

% testblk_uu/data_in_mux/tvg_in/ram_im
testblk_uu_data_in_mux_tvg_in_ram_im_type         = 'xps_bram';
testblk_uu_data_in_mux_tvg_in_ram_im_param        = '2048';
testblk_uu_data_in_mux_tvg_in_ram_im_ip_name      = 'bram_block';

% testblk_uu/data_in_mux/tvg_in/ram_re
testblk_uu_data_in_mux_tvg_in_ram_re_type         = 'xps_bram';
testblk_uu_data_in_mux_tvg_in_ram_re_param        = '2048';
testblk_uu_data_in_mux_tvg_in_ram_re_ip_name      = 'bram_block';

% testblk_uu/data_in_mux/tvg_in/re_we
testblk_uu_data_in_mux_tvg_in_re_we_type         = 'xps_sw_reg';
testblk_uu_data_in_mux_tvg_in_re_we_param        = 'in';
testblk_uu_data_in_mux_tvg_in_re_we_ip_name      = 'opb_register_ppc2simulink';

% testblk_uu/data_in_mux/tvg_in/snap_im/addr
testblk_uu_data_in_mux_tvg_in_snap_im_addr_type         = 'xps_sw_reg';
testblk_uu_data_in_mux_tvg_in_snap_im_addr_param        = 'out';
testblk_uu_data_in_mux_tvg_in_snap_im_addr_ip_name      = 'opb_register_simulink2ppc';

% testblk_uu/data_in_mux/tvg_in/snap_im/bram
testblk_uu_data_in_mux_tvg_in_snap_im_bram_type         = 'xps_bram';
testblk_uu_data_in_mux_tvg_in_snap_im_bram_param        = '2048';
testblk_uu_data_in_mux_tvg_in_snap_im_bram_ip_name      = 'bram_block';

% testblk_uu/data_in_mux/tvg_in/snap_im/ctrl
testblk_uu_data_in_mux_tvg_in_snap_im_ctrl_type         = 'xps_sw_reg';
testblk_uu_data_in_mux_tvg_in_snap_im_ctrl_param        = 'in';
testblk_uu_data_in_mux_tvg_in_snap_im_ctrl_ip_name      = 'opb_register_ppc2simulink';

% testblk_uu/data_in_mux/tvg_in/snap_re/addr
testblk_uu_data_in_mux_tvg_in_snap_re_addr_type         = 'xps_sw_reg';
testblk_uu_data_in_mux_tvg_in_snap_re_addr_param        = 'out';
testblk_uu_data_in_mux_tvg_in_snap_re_addr_ip_name      = 'opb_register_simulink2ppc';

% testblk_uu/data_in_mux/tvg_in/snap_re/bram
testblk_uu_data_in_mux_tvg_in_snap_re_bram_type         = 'xps_bram';
testblk_uu_data_in_mux_tvg_in_snap_re_bram_param        = '2048';
testblk_uu_data_in_mux_tvg_in_snap_re_bram_ip_name      = 'bram_block';

% testblk_uu/data_in_mux/tvg_in/snap_re/ctrl
testblk_uu_data_in_mux_tvg_in_snap_re_ctrl_type         = 'xps_sw_reg';
testblk_uu_data_in_mux_tvg_in_snap_re_ctrl_param        = 'in';
testblk_uu_data_in_mux_tvg_in_snap_re_ctrl_ip_name      = 'opb_register_ppc2simulink';

% testblk_uu/data_in_mux/tvg_in/tvg_ctrl
testblk_uu_data_in_mux_tvg_in_tvg_ctrl_type         = 'xps_sw_reg';
testblk_uu_data_in_mux_tvg_in_tvg_ctrl_param        = 'in';
testblk_uu_data_in_mux_tvg_in_tvg_ctrl_ip_name      = 'opb_register_ppc2simulink';

% testblk_uu/data_out
testblk_uu_data_out_type         = 'xps_gpio';
testblk_uu_data_out_param        = '';
testblk_uu_data_out_ip_name      = 'gpio_simulink2ext';

% testblk_uu/data_out_mux/data_sel
testblk_uu_data_out_mux_data_sel_type         = 'xps_sw_reg';
testblk_uu_data_out_mux_data_sel_param        = 'in';
testblk_uu_data_out_mux_data_sel_ip_name      = 'opb_register_ppc2simulink';

% testblk_uu/data_out_mux/tvg_out/bram_wr_reg
testblk_uu_data_out_mux_tvg_out_bram_wr_reg_type         = 'xps_sw_reg';
testblk_uu_data_out_mux_tvg_out_bram_wr_reg_param        = 'in';
testblk_uu_data_out_mux_tvg_out_bram_wr_reg_ip_name      = 'opb_register_ppc2simulink';

% testblk_uu/data_out_mux/tvg_out/im_we
testblk_uu_data_out_mux_tvg_out_im_we_type         = 'xps_sw_reg';
testblk_uu_data_out_mux_tvg_out_im_we_param        = 'in';
testblk_uu_data_out_mux_tvg_out_im_we_ip_name      = 'opb_register_ppc2simulink';

% testblk_uu/data_out_mux/tvg_out/ram_im
testblk_uu_data_out_mux_tvg_out_ram_im_type         = 'xps_bram';
testblk_uu_data_out_mux_tvg_out_ram_im_param        = '2048';
testblk_uu_data_out_mux_tvg_out_ram_im_ip_name      = 'bram_block';

% testblk_uu/data_out_mux/tvg_out/ram_re
testblk_uu_data_out_mux_tvg_out_ram_re_type         = 'xps_bram';
testblk_uu_data_out_mux_tvg_out_ram_re_param        = '2048';
testblk_uu_data_out_mux_tvg_out_ram_re_ip_name      = 'bram_block';

% testblk_uu/data_out_mux/tvg_out/re_we
testblk_uu_data_out_mux_tvg_out_re_we_type         = 'xps_sw_reg';
testblk_uu_data_out_mux_tvg_out_re_we_param        = 'in';
testblk_uu_data_out_mux_tvg_out_re_we_ip_name      = 'opb_register_ppc2simulink';

% testblk_uu/data_out_mux/tvg_out/snap_im/addr
testblk_uu_data_out_mux_tvg_out_snap_im_addr_type         = 'xps_sw_reg';
testblk_uu_data_out_mux_tvg_out_snap_im_addr_param        = 'out';
testblk_uu_data_out_mux_tvg_out_snap_im_addr_ip_name      = 'opb_register_simulink2ppc';

% testblk_uu/data_out_mux/tvg_out/snap_im/bram
testblk_uu_data_out_mux_tvg_out_snap_im_bram_type         = 'xps_bram';
testblk_uu_data_out_mux_tvg_out_snap_im_bram_param        = '2048';
testblk_uu_data_out_mux_tvg_out_snap_im_bram_ip_name      = 'bram_block';

% testblk_uu/data_out_mux/tvg_out/snap_im/ctrl
testblk_uu_data_out_mux_tvg_out_snap_im_ctrl_type         = 'xps_sw_reg';
testblk_uu_data_out_mux_tvg_out_snap_im_ctrl_param        = 'in';
testblk_uu_data_out_mux_tvg_out_snap_im_ctrl_ip_name      = 'opb_register_ppc2simulink';

% testblk_uu/data_out_mux/tvg_out/snap_re/addr
testblk_uu_data_out_mux_tvg_out_snap_re_addr_type         = 'xps_sw_reg';
testblk_uu_data_out_mux_tvg_out_snap_re_addr_param        = 'out';
testblk_uu_data_out_mux_tvg_out_snap_re_addr_ip_name      = 'opb_register_simulink2ppc';

% testblk_uu/data_out_mux/tvg_out/snap_re/bram
testblk_uu_data_out_mux_tvg_out_snap_re_bram_type         = 'xps_bram';
testblk_uu_data_out_mux_tvg_out_snap_re_bram_param        = '2048';
testblk_uu_data_out_mux_tvg_out_snap_re_bram_ip_name      = 'bram_block';

% testblk_uu/data_out_mux/tvg_out/snap_re/ctrl
testblk_uu_data_out_mux_tvg_out_snap_re_ctrl_type         = 'xps_sw_reg';
testblk_uu_data_out_mux_tvg_out_snap_re_ctrl_param        = 'in';
testblk_uu_data_out_mux_tvg_out_snap_re_ctrl_ip_name      = 'opb_register_ppc2simulink';

% testblk_uu/data_out_mux/tvg_out/tvg_ctrl
testblk_uu_data_out_mux_tvg_out_tvg_ctrl_type         = 'xps_sw_reg';
testblk_uu_data_out_mux_tvg_out_tvg_ctrl_param        = 'in';
testblk_uu_data_out_mux_tvg_out_tvg_ctrl_ip_name      = 'opb_register_ppc2simulink';

% testblk_uu/snap64/addr
testblk_uu_snap64_addr_type         = 'xps_sw_reg';
testblk_uu_snap64_addr_param        = 'out';
testblk_uu_snap64_addr_ip_name      = 'opb_register_simulink2ppc';

% testblk_uu/snap64/bram_lsb
testblk_uu_snap64_bram_lsb_type         = 'xps_bram';
testblk_uu_snap64_bram_lsb_param        = '2048';
testblk_uu_snap64_bram_lsb_ip_name      = 'bram_block';

% testblk_uu/snap64/bram_msb
testblk_uu_snap64_bram_msb_type         = 'xps_bram';
testblk_uu_snap64_bram_msb_param        = '2048';
testblk_uu_snap64_bram_msb_ip_name      = 'bram_block';

% testblk_uu/snap64/ctrl
testblk_uu_snap64_ctrl_type         = 'xps_sw_reg';
testblk_uu_snap64_ctrl_param        = 'in';
testblk_uu_snap64_ctrl_ip_name      = 'opb_register_ppc2simulink';

% testblk_uu/snap_out_we
testblk_uu_snap_out_we_type         = 'xps_sw_reg';
testblk_uu_snap_out_we_param        = 'in';
testblk_uu_snap_out_we_ip_name      = 'opb_register_ppc2simulink';

% testblk_uu/sync_in_mux/gpio
testblk_uu_sync_in_mux_gpio_type         = 'xps_gpio';
testblk_uu_sync_in_mux_gpio_param        = '';
testblk_uu_sync_in_mux_gpio_ip_name      = 'gpio_ext2simulink';

% testblk_uu/sync_in_mux/sync_period
testblk_uu_sync_in_mux_sync_period_type         = 'xps_sw_reg';
testblk_uu_sync_in_mux_sync_period_param        = 'in';
testblk_uu_sync_in_mux_sync_period_ip_name      = 'opb_register_ppc2simulink';

% testblk_uu/sync_in_mux/sync_sel
testblk_uu_sync_in_mux_sync_sel_type         = 'xps_sw_reg';
testblk_uu_sync_in_mux_sync_sel_param        = 'in';
testblk_uu_sync_in_mux_sync_sel_ip_name      = 'opb_register_ppc2simulink';

% testblk_uu/sync_out
testblk_uu_sync_out_type         = 'xps_gpio';
testblk_uu_sync_out_param        = '';
testblk_uu_sync_out_ip_name      = 'gpio_simulink2ext';

% testblk_uu/sync_out_mux/sync_per
testblk_uu_sync_out_mux_sync_per_type         = 'xps_sw_reg';
testblk_uu_sync_out_mux_sync_per_param        = 'in';
testblk_uu_sync_out_mux_sync_per_ip_name      = 'opb_register_ppc2simulink';

% testblk_uu/sync_out_mux/sync_sel
testblk_uu_sync_out_mux_sync_sel_type         = 'xps_sw_reg';
testblk_uu_sync_out_mux_sync_sel_param        = 'in';
testblk_uu_sync_out_mux_sync_sel_ip_name      = 'opb_register_ppc2simulink';

% PLB to OPB bridge added at 0xD1000000

% PLB to OPB bridge added at 0xD2000000

