/*
 * register definition for OCM interface
 * Xilinx bitfile.
 */

/*
 * registers 0 (PCD_CMD), 1 (PCD_DATA_PATH_STAT), 2 (OCD_FUNCT), 3 (PCD_STAT),  0xf (PCD_CONFIG),
 * 0x10 (SSD16_CHEN), 0x16 (SSD16_LSB), 0x18 (SSD16_UNDER) and 0x1b (SSD16_OVER) have
 * the regular address definition with the standard bit placement and definition as follows:
 * command - PCD_ENABLE bit 0x8 only, others are read/write but unused
 * data_path - bits are read/write but not used
 * funct - bits are read write but unused
 * status - bit 0 indicates the sysclk DCM on the Xilinx is locked to the clock from the OCM board
 */
#define	OCM_STAT_SYS_LOCKED		0x1

 /*
  * config - bit 0 is byteswap (PCD_BYTESWAP) and 3 is short swap (PCD_SHORTSWAP)
  * channel enable - bit 0 and 1 enable channel 0 and 1
  * lsb_first - bit 0 and 1 control channel 0 and 1
  * underflow - bit 0 and 1 report status, bit 2 to 15 are always 0.
  * overflow - bit 0 and 1 report status, bit 2 to 15 are always 0.
  */

 /*
  * OCM specific registers -
  * channel 0 registers are 0x20 - 0x24 have the same definitions as
  * channel 1 registers 0x30-0x34 except the channel does not have DDR memory fifo
  * so not bit to reset same.
  */
#define	OCM_CH0_CONFIGL	0x01010020
#define	OCM_CH0_CONFIGH	0x01010021
#define	OCM_CH0_STATUS	0x01010022
#define	OCM_CH0_XCVR	0x01010023
#define	OCM_CH0_ENABLE	0x01010024
#define	OCM_CH1_OFFSET	0x10


/* 
 * config register low bits
 * these bits directly control the SLK2511
 * the FRAME_EN bit also controls the framesync state machine for the channel
 */
#define	OCM_FRAME_EN	0x1
#define	OCM_LCKREF_L	0x2
#define	OCM_RSEL_MSK	0xC
#define	OCM_48			0x0
#define	OCM_24			0x4
#define	OCM_12			0x8
#define	OCM_3			0xC
#define OCM_AUTODETECT	0x10
#define	OCM_RLOOP		0x20
#define	OCM_LLOOP		0x40
#define	OCM_PRBS_EN		0x80
/*
 * config high bits
 */
#define	OCM_MODE_MSK	0x3
#define	OCM_MODE_FULL	0x0
#define	OCM_MODE_TX		0x1
#define	OCM_MODE_RX		0x2
#define	OCM_MODE_REP	0x3
#define	OCM_PRE_MSK		0xc
#define	OCM_PRE_DIS		0x0
#define	OCM_PRE_10		0x4
#define	OCM_PRE_20		0x8
#define	OCM_PRE_30		0xc
#define	OCM_LOOPTIME	0x10
/*
 * adjust PLL phase delay to center clock in data eye. DCM in XIlinx is
 * used for this the ONE bit increments or decrements the delay based on the
 * INC bit.
 */
#define	OCM_PLL_INC		0x20
#define OCM_PLL_ONE		0x40
#define	OCM_SPARE		0x80

/*
 * program the Xilinx for channel 0  and 1 registers
 * the data register is loaded with the bit file data. It is a fifo which holds 15 bytes
 * the number of bytes in the fifo is read in the top 4 bist of the status register
 * The CONT register controls the PROGL and INIT pins of each Xilinx, bit 4 controls which
 * xilinx gets the data being loaded in the data register
 * The STAT register reflects the status of the DONE and INIT pins.
 * The constant register is written and read to determine if the ocm.bit file is loaded before
 * attempting to load the individual channel xilinx.
 */
#define	OCM_X_DATA	0x01010040
#define OCM_X_CONT	0x01010041
#define	OCM_X_STAT	0x01010042
#define	OCM_X_CONST	0x01010043

/* control register bits */
#define	OCM_CONT_CH0_INIT	0x1	/* one tristates INIT pin */
#define OCM_CONT_CH0_PROG	0x2 /* one drives the PROG_L pin low (resets xilinx program) */
#define	OCM_CONT_CH1_INIT	0x4	
#define OCM_CONT_CH1_PROG	0x8 
#define	OCM_CONT_PRG_CH1	0x10	/* directs data to channel 1 */
#define	OCM_CONT_ENABLE		0x20	/* enables the fifo, cclk and bit serializer */
#define	OCM_CONT_EN_FIFO	0x40	/* enables the fifo, cclk and bit serializer */

/* status bits  */
#define	OCM_STAT_CH0_INIT	0x1	/* reads state of INIT pin */
#define	OCM_STAT_CH0_DONE	0x2	/* reads state of DONE pin */
#define	OCM_STAT_CH1_INIT	0x4	
#define	OCM_STAT_CH1_DONE	0x8	
#define	OCM_FCNT_MSK		0xf0
#define	OCM_FCNT_SHFT		4

#define	OCM_CONSTANT		0x0c	/* fixed pattern in OCM_X_CONST register */

