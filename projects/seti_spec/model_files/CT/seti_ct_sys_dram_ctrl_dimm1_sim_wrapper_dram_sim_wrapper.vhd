
-------------------------------------------------------------------
-- System Generator VHDL source file.
-- Copyright (c) 2003 Xilinx, Inc.  All rights reserved.
-- Reproduction or reuse, in any form, without the explicit written
-- consent of Xilinx, Inc., is strictly prohibited.
-------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;

entity seti_ct_sys_dram_ctrl_dimm1_sim_wrapper_dram_sim_wrapper is
  port (
    rst: in std_logic_vector(0 downto 0);
    address: in std_logic_vector(31 downto 0);
    data_in: in std_logic_vector(143 downto 0);
    wr_be: in std_logic_vector(17 downto 0);
    RWn: in std_logic_vector(0 downto 0);
    cmd_tag: in std_logic_vector(31 downto 0);
    cmd_valid: in std_logic_vector(0 downto 0);
    rd_ack: in std_logic_vector(0 downto 0);
    clk: in std_logic;
    ce: in std_logic;
    cmd_ack: out std_logic_vector(0 downto 0);
    data_out: out std_logic_vector(143 downto 0);
    rd_tag: out std_logic_vector(31 downto 0);
    rd_valid: out std_logic_vector(0 downto 0);
    ddr_clock: out std_logic_vector(0 downto 0);
    ddr_clock90: out std_logic_vector(0 downto 0)
  );
end seti_ct_sys_dram_ctrl_dimm1_sim_wrapper_dram_sim_wrapper;

architecture structural of seti_ct_sys_dram_ctrl_dimm1_sim_wrapper_dram_sim_wrapper is
  component dram_sim
    generic (
      C_HALF_BURST: Integer := 0;
      C_WIDE_DATA: Integer := 0;
      IP_CLOCK: Integer := 200;
      SYSCLK_PER: Time := 500 ms
    );
    port (
      rst: in std_logic_vector(0 downto 0);
      address: in std_logic_vector(31 downto 0);
      data_in: in std_logic_vector(143 downto 0);
      wr_be: in std_logic_vector(17 downto 0);
      RWn: in std_logic_vector(0 downto 0);
      cmd_tag: in std_logic_vector(31 downto 0);
      cmd_valid: in std_logic_vector(0 downto 0);
      rd_ack: in std_logic_vector(0 downto 0);
      clk: in std_logic;
      ce: in std_logic;
      cmd_ack: out std_logic_vector(0 downto 0);
      data_out: out std_logic_vector(143 downto 0);
      rd_tag: out std_logic_vector(31 downto 0);
      rd_valid: out std_logic_vector(0 downto 0);
      ddr_clock: out std_logic_vector(0 downto 0);
      ddr_clock90: out std_logic_vector(0 downto 0)
    );
  end component;

  attribute syn_noprune: boolean;
  attribute dont_touch: boolean;
  attribute syn_noprune of dram_sim: component is true;
  attribute dont_touch of dram_sim: component is true;

  signal rst_net: std_logic_vector(0 downto 0);
  signal address_net: std_logic_vector(31 downto 0);
  signal data_in_net: std_logic_vector(143 downto 0);
  signal wr_be_net: std_logic_vector(17 downto 0);
  signal RWn_net: std_logic_vector(0 downto 0);
  signal cmd_tag_net: std_logic_vector(31 downto 0);
  signal cmd_valid_net: std_logic_vector(0 downto 0);
  signal rd_ack_net: std_logic_vector(0 downto 0);
  signal clk_net: std_logic;
  signal ce_net: std_logic;
  signal cmd_ack_net: std_logic_vector(0 downto 0);
  signal data_out_net: std_logic_vector(143 downto 0);
  signal rd_tag_net: std_logic_vector(31 downto 0);
  signal rd_valid_net: std_logic_vector(0 downto 0);
  signal ddr_clock_net: std_logic_vector(0 downto 0);
  signal ddr_clock90_net: std_logic_vector(0 downto 0);

begin
  rst_net <= rst;
  address_net <= address;
  data_in_net <= data_in;
  wr_be_net <= wr_be;
  RWn_net <= RWn;
  cmd_tag_net <= cmd_tag;
  cmd_valid_net <= cmd_valid;
  rd_ack_net <= rd_ack;
  clk_net <= clk;
  ce_net <= ce;
  cmd_ack <= cmd_ack_net;
  data_out <= data_out_net;
  rd_tag <= rd_tag_net;
  rd_valid <= rd_valid_net;
  ddr_clock <= ddr_clock_net;
  ddr_clock90 <= ddr_clock90_net;
  dram_sim_0: dram_sim
    generic map (
      C_HALF_BURST => 0,
      C_WIDE_DATA => 0,
      IP_CLOCK => 200,
      SYSCLK_PER => 500 ms
    )
    port map (
      rst => rst_net,
      address => address_net,
      data_in => data_in_net,
      wr_be => wr_be_net,
      RWn => RWn_net,
      cmd_tag => cmd_tag_net,
      cmd_valid => cmd_valid_net,
      rd_ack => rd_ack_net,
      clk => clk_net,
      ce => ce_net,
      cmd_ack => cmd_ack_net,
      data_out => data_out_net,
      rd_tag => rd_tag_net,
      rd_valid => rd_valid_net,
      ddr_clock => ddr_clock_net,
      ddr_clock90 => ddr_clock90_net
    );
end structural;
