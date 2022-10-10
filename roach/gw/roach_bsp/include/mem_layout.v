`ifndef MEMLAYOUT_H
`define MEMLAYOUT_H

`define DRAM_A_BASE      32'h0400_0000 //DRAM base          [  64MB]
`define QDR1_A_BASE      32'h0300_0000 //QDR1 base          [  48MB]
`define QDR0_A_BASE      32'h0200_0000 //QDR0 base          [  32MB]
`define BLOCKRAM_A_BASE  32'h0100_0000 //BlockRAM           [  16MB]
`define APP_A_BASE       32'h0080_0000 //Application Memory [   8MB]
`define TESTING_A_BASE   32'h0040_0000 //Testing            [   4MB]
`define TGE3_A_BASE      32'h0038_0000 //10 Gig Eth 3       [ 3.5MB]
`define TGE2_A_BASE      32'h0030_0000 //10 Gig Eth 2       [   3MB]
`define TGE1_A_BASE      32'h0028_0000 //10 Gig Eth 1       [ 2.5MB]
`define TGE0_A_BASE      32'h0020_0000 //10 Gig Eth 0       [   2MB]
`define RSRVD1_A_BASE    32'h0010_0000 //Reserved 1         [   1MB]
`define RSRVD0_A_BASE    32'h0006_0000 //Reserved 0         [ 192KB]
`define DRAMCONF_A_BASE  32'h0005_0000 //DRAM registers     [ 160KB]
`define QDR1CONF_A_BASE  32'h0004_0000 //QDR 1 registers    [ 128KB]
`define QDR0CONF_A_BASE  32'h0003_0000 //QDR 0 registers    [  96KB]
`define ADC1_A_BASE      32'h0002_0000 //ADC 1 registers    [  64KB]
`define ADC0_A_BASE      32'h0001_0000 //ADC 0 registers    [  32KB]
`define SYSBLOCK_A_BASE  32'h0000_0000 //System Block       [   0KB]

`define DRAM_A_HIGH      32'h07ff_ffff //DRAM base          [ 128MB]
`define QDR1_A_HIGH      32'h03ff_ffff //QDR1 base          [  64MB]
`define QDR0_A_HIGH      32'h02ff_ffff //QDR0 base          [  48MB]
`define BLOCKRAM_A_HIGH  32'h01ff_ffff //BlockRAM           [  32MB]
`define APP_A_HIGH       32'h00ff_ffff //Application Memory [  16MB]
`define TESTING_A_HIGH   32'h007f_ffff //Testing            [   8MB]
`define TGE3_A_HIGH      32'h003f_ffff //10 Gig Eth 3       [   4MB]
`define TGE2_A_HIGH      32'h0037_ffff //10 Gig Eth 2       [ 3.5MB]
`define TGE1_A_HIGH      32'h002f_ffff //10 Gig Eth 1       [   3MB]
`define TGE0_A_HIGH      32'h0027_ffff //10 Gig Eth 0       [ 2.5MB]
`define RSRVD1_A_HIGH    32'h001f_ffff //Reserved 1         [   2MB]
`define RSRVD0_A_HIGH    32'h000f_ffff //Reserved 0         [   1MB]
`define DRAMCONF_A_HIGH  32'h0005_ffff //DRAM registers     [ 192KB]
`define QDR1CONF_A_HIGH  32'h0004_ffff //QDR 1 registers    [ 160KB]
`define QDR0CONF_A_HIGH  32'h0003_ffff //QDR 0 registers    [ 128KB]
`define ADC1_A_HIGH      32'h0002_ffff //ADC 1 registers    [  96KB]
`define ADC0_A_HIGH      32'h0001_ffff //ADC 0 registers    [  64KB]
`define SYSBLOCK_A_HIGH  32'h0000_ffff //System Block       [  32KB]

`endif
