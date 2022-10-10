/****************************************************************************
 *        MB86290 Series Graphics Controller Access Library
 *        ALL RIGHTS RESERVED, COPYRIGHT (C) FUJITSU LIMITED 1999-2003
 *        LICENSED MATERIAL - PROGRAM PROPERTY OF FUJITSU LIMITED
 *        1.01.002
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 ****************************************************************************/
#ifndef _gdcreg_h_
#define _gdcreg_h_

/* Register Data */

/* I2C special registers */
#define GDC_I2C_REG_BUS_STATUS            0x00000000L	/* BSR */
#define GDC_I2C_REG_BUS_CONTROL           0x00000004L	/* BCR */
#define GDC_I2C_REG_CLOCK                 0x00000008L	/* CCR */
#define GDC_I2C_REG_SLAVE_ADDRESS         0x0000000CL	/* ADR */
#define GDC_I2C_REG_DATA                  0x00000010L	/* DAR */

/* Geometry special registers */
#define GDC_GEO_REG_CTR0                  0x00000000L	/* GCTR0 */
#define GDC_GEO_REG_INPUT_FIFO_STATUS     0x00000001L	/* GIFSR */
#define GDC_GEO_REG_INPUT_FIFO_COUNT      0x00000002L	/* GIFCNT */
#define GDC_GEO_REG_ENGINE_STATUS         0x00000003L	/* GGSR */
#define GDC_GEO_REG_SETUP_STATUS          0x00000004L	/* GSSR */
#define GDC_GEO_REG_PIXEL_STATUS          0x00000005L	/* GPSR */
#define GDC_GEO_REG_ERROR_STATUS          0x00000006L	/* GESR */
#define GDC_GEO_REG_RESERVED_1            0x00000007L	/* RESERVED */
/* 0x00002009 - 0x0000200F reserved */

/* Following macro are offset from draw_reg */
#define GDC_GEO_REG_MODE_GEOMETRY         0x00002010L	/* GMDR0 */
#define GDC_GEO_REG_MODE_LINE             0x00002011L	/* GMDR1 */
#define GDC_GEO_REG_MODE_TRIANGLE         0x00002012L	/* GMDR2 */

/* Following macro is offset from geo_reg */
#define GDC_GEO_REG_INPUT_FIFO            0x00000400L	/* DFIFOG */

/* Video capture special registers */
#define GDC_CAP_REG_MODE		0x00000000L	/* VCM */
#define GDC_CAP_REG_SCALE		0x00000004L	/* VSCALE */
#define GDC_CAP_REG_STATUS		0x00000008L	/* VCS */
#define GDC_CAP_REG_BUFFER_MODE		0x00000010L	/* CBM */
#define GDC_CAP_REG_BUFFER_SADDR	0x00000014L	/* CBOA */
#define GDC_CAP_REG_BUFFER_EADDR	0x00000018L	/* CBLA */
#define GDC_CAP_REG_IMAGE_START		0x0000001CL	/* CIVSTR,CIHSTR */
#define GDC_CAP_REG_IMAGE_END		0x00000020L	/* CIVEND,CIHEND */
#define GDC_CAP_REG_H_PIXEL		0x00000028L	/* CHP */
#define GDC_CAP_REG_V_PIXEL		0x0000002CL	/* CVP */
#define GDC_CAP_REG_DATA_COUNT_NTSC	0x00004000L	/* CDCN */
#define GDC_CAP_REG_DATA_COUNT_PAL	0x00004004L	/* CDCP */
#define GDC_CAP_REG_LPF			0x00000040L	/* CLPF */

/* for MB86294 or later */
#define GDC_CAP_REG_CMSS		0x00000048L	/* CMSS */
#define GDC_CAP_REG_CMDS		0x0000004CL	/* CMDS */

/* for MB86295 or later */
#define	GDC_CAP_REG_RGBHC		0x00000080L	/* RGBHC  */
#define	GDC_CAP_REG_RGBHEN		0x00000084L	/* RGBHEN */
#define	GDC_CAP_REG_RGBVEN		0x00000088L	/* RGBVEN */
#define	GDC_CAP_REG_RGBS		0x00000090L	/* RGBS   */
#define	GDC_CAP_REG_RGBCMY		0x000000C0L	/* RGBCMY  */
#define	GDC_CAP_REG_RGBCMCB		0x000000C4L	/* RGBCMCb */
#define	GDC_CAP_REG_RGBCMCR		0x000000C8L	/* RGBCMCr */
#define	GDC_CAP_REG_RGBCMB		0x000000CCL	/* RGBCMb  */

/* bit fields of Video capture registers */
#define GDC_CAP_REG_VCM_VS_MASK			0x00000002L
#define GDC_CAP_REG_VCM_VI_MASK			0x00100000L
#define GDC_CAP_REG_VCM_CM_MASK			0x03000000L
#define GDC_CAP_REG_VCM_VIS_MASK		0x40000000L
#define GDC_CAP_REG_VCM_VIE_MASK		0x80000000L
#define GDC_CAP_REG_CBM_CBST_MASK		0x00000001L
#define GDC_CAP_REG_CBM_CRGB_MASK		0x20000000L
#define GDC_CAP_REG_CBM_SBUF_MASK		0x40000000L
#define GDC_CAP_REG_BUFFER_MODE_MASK_OO		0x80000000L
#define GDC_CAP_REG_RGBHC_MASK_RGBHC		0x00000FFFL
#define GDC_CAP_REG_RGBHEN_MASK_RGBHST		0x0FFF0000L
#define GDC_CAP_REG_RGBHEN_MASK_RGBHEN		0x000007FFL
#define GDC_CAP_REG_RGBVEN_MASK_RGBVST		0x01FF0000L
#define GDC_CAP_REG_RGBVEN_MASK_RGBVEN		0x000003FFL
#define GDC_CAP_REG_RGBS_MASK_HP		0x00000002L
#define GDC_CAP_REG_RGBS_MASK_VP		0x00000001L

#define GDC_CAP_REG_RGBCMY_MASK_A11		0xFFC00000L
#define GDC_CAP_REG_RGBCMY_MASK_A12		0x001FF800L
#define GDC_CAP_REG_RGBCMY_MASK_A13		0x000003FFL
#define GDC_CAP_REG_RGBCMCB_MASK_A21		0xFFC00000L
#define GDC_CAP_REG_RGBCMCB_MASK_A22		0x001FF800L
#define GDC_CAP_REG_RGBCMCB_MASK_A23		0x000003FFL
#define GDC_CAP_REG_RGBCMCR_MASK_A31		0xFFC00000L
#define GDC_CAP_REG_RGBCMCR_MASK_A32		0x001FF800L
#define GDC_CAP_REG_RGBCMCR_MASK_A33		0x000003FFL
#define GDC_CAP_REG_RGBCMB_MASK_B1		0x7FC00000L
#define GDC_CAP_REG_RGBCMB_MASK_B2		0x000FF800L
#define GDC_CAP_REG_RGBCMB_MASK_B3		0x000001FFL

/* bit field offsets of video capture registers */
#define	GDC_CAP_REG_VCM_VIS_OFFSET		30
#define GDC_CAP_REG_RGBCMY_OFFSET_A11		22
#define GDC_CAP_REG_RGBCMY_OFFSET_A12		11
#define GDC_CAP_REG_RGBCMY_OFFSET_A13		 0
#define GDC_CAP_REG_RGBCMCB_OFFSET_A21		22
#define GDC_CAP_REG_RGBCMCB_OFFSET_A22		11
#define GDC_CAP_REG_RGBCMCB_OFFSET_A23		 0
#define GDC_CAP_REG_RGBCMCR_OFFSET_A31		22
#define GDC_CAP_REG_RGBCMCR_OFFSET_A32		11
#define GDC_CAP_REG_RGBCMCR_OFFSET_A33		 0
#define GDC_CAP_REG_RGBCMB_OFFSET_B1		22
#define GDC_CAP_REG_RGBCMB_OFFSET_B2		11
#define GDC_CAP_REG_RGBCMB_OFFSET_B3		 0

/* Mode Registers */
/* Mode Misc*/
#define GDC_BPP_FORMAT_MASK             0x00018000L	/* 8/16/24bpp support */
#define GDC_PATTERN_MODE_MASK           0x0000000FL
#define GDC_CLIP_MASK                   0x00000300L

/* for MB86293 or later */
#define GDC_MODE_Z_PRECISION_MASK	0x00100000

#define GDC_BPP_FORMAT_SHIFT                    15

#define GDC_BITPATTERN_ORDER_MASK       0x80000000L

/* Mode register for Line(MDR1) & Polygon(MDR2) */
#define GDC_SHADE_MODE_MASK             0x00000001L
#define GDC_DEPTH_TEST_ENABLE           0x00000004L
#define GDC_DEPTH_WRITE_ENABLE          0x00000040L
#define GDC_DEPTH_FUNC_MASK             0x00000038L
#define GDC_BLEND_MODE_MASK             0x00000180L
#define GDC_ROP_MASK                    0x00001E00L
#define GDC_TEXTURE_SELECT_MASK         0x30000000L
#define GDC_ALPHA_SHADE_MODE_MASK       0x00000002L	/* for MB86293 or later */

/* Mode register for extension(MDR7) */
#define GDC_SHADE_MODE_ENABLE_8BPP      0x00000050L	/* for MB86293 or later */
#define GDC_SHADE_MODE_DISABLE_8BPP     0x00000040L	/* for MB86293 or later */

#define GDC_DEPTH_FUNC_SHIFT                     3
#define GDC_BLEND_MODE_SHIFT                     7
#define GDC_ROP_SHIFT                            9
#define GDC_TEXTURE_SELECT_SHIFT                28

/* Mode Line */
#define GDC_BROKEN_LINE_ENABLE          0x00080000L
#define GDC_LINE_WIDTH_MASK             0x1F000000L

#define GDC_LINE_WIDTH_SHIFT                    24

#define GDC_POINT_HIDDEN_ATTR           0x00080000L

/* Broken line period */
#define GDC_BROKEN_LINE_PERIOD_24       0x00100000L

/* Mode Polygon */
#define GDC_TRIANGLE_HIDDEN_ATTR        0x00000001L
#define GDC_SET_POLYGON_TILE            0x10000000L
#define GDC_MODE_MISC_LOAD_TEXTURE      0x00008000L
#define GDC_MODE_MISC_LOAD_CURSOR       0x00000000L

/* for MB86293 or later */
#define GDC_MODE_MISC_LOAD_TEXTURE_8BPP  0x00000000L
#define GDC_MODE_MISC_LOAD_TEXTURE_24BPP 0x00010000L

/* Mode Bitmap Value for Clear Z */
#define GDC_MODE_BITMAP_CLEAR_Z         0x00000000L

/* Mode Bitmap */
#define GDC_MODE_BITMAP_LOAD_TEXTURE    0x00000700L
#define GDC_MODE_BITMAP_LOAD_CURSOR     0x00000700L

/* Mode texture */
#define GDC_TEXURE_BUFFER_INT           0x00000001L

#define GDC_TEX_PERSPECTIVE_ENABLE      0x00000008L

#define GDC_TEX_FILTER_MASK             0x00000020L
#define GDC_TEX_WRAP_T_MASK             0x00000300L
#define GDC_TEX_WRAP_S_MASK             0x00000C00L
#define GDC_TEX_BLEND_MASK              0x00030000L
#define GDC_TEX_ALPHA_MASK              0x00300000L

/* for MB86293 or later */
#define GDC_TEX_FAST_MODE_MASK          0x01000000L

#define GDC_TEX_FILTER_SHIFT                     5
#define GDC_TEX_WRAP_T_SHIFT                     8
#define GDC_TEX_WRAP_S_SHIFT                    10
#define GDC_TEX_BLEND_SHIFT                     16
#define GDC_TEX_ALPHA_SHIFT                     20

/* for MB86293 or later */
#define GDC_TEX_FAST_MODE_SHIFT                 24

/* Mode Bitmap */
#define GDC_BLT_TILE_MASK               0x00000001L

/* Mode register for BLT(MDR4) */
#define GDC_BLT_TRANS_MODE_SHIFT                 1
#define GDC_BLT_TRANS_MODE_MASK         0x00000002L

/* Coomand Code */
#define GDC_CMD_PIXEL                   0x00000000L
#define GDC_CMD_PIXEL_Z                 0x00000001L

#define GDC_CMD_X_VECTOR                0x00000020L
#define GDC_CMD_Y_VECTOR                0x00000021L
#define GDC_CMD_X_VECTOR_NOEND          0x00000022L
#define GDC_CMD_Y_VECTOR_NOEND          0x00000023L
#define GDC_CMD_X_VECTOR_BLPO           0x00000024L
#define GDC_CMD_Y_VECTOR_BLPO           0x00000025L
#define GDC_CMD_X_VECTOR_NOEND_BLPO     0x00000026L
#define GDC_CMD_Y_VECTOR_NOEND_BLPO     0x00000027L
#define GDC_CMD_AA_X_VECTOR             0x00000028L
#define GDC_CMD_AA_Y_VECTOR             0x00000029L
#define GDC_CMD_AA_X_VECTOR_NOEND       0x0000002AL
#define GDC_CMD_AA_Y_VECTOR_NOEND       0x0000002BL
#define GDC_CMD_AA_X_VECTOR_BLPO        0x0000002CL
#define GDC_CMD_AA_Y_VECTOR_BLPO        0x0000002DL
#define GDC_CMD_AA_X_VECTOR_NOEND_BLPO  0x0000002EL
#define GDC_CMD_AA_Y_VECTOR_NOEND_BLPO  0x0000002FL

#define GDC_CMD_0_VECTOR                0x00000030L
#define GDC_CMD_1_VECTOR                0x00000031L
#define GDC_CMD_0_VECTOR_NOEND          0x00000032L
#define GDC_CMD_1_VECTOR_NOEND          0x00000033L
#define GDC_CMD_0_VECTOR_BLPO           0x00000034L
#define GDC_CMD_1_VECTOR_BLPO           0x00000035L
#define GDC_CMD_0_VECTOR_NOEND_BLPO     0x00000036L
#define GDC_CMD_1_VECTOR_NOEND_BLPO     0x00000037L
#define GDC_CMD_AA_0_VECTOR             0x00000038L
#define GDC_CMD_AA_1_VECTOR             0x00000039L
#define GDC_CMD_AA_0_VECTOR_NOEND       0x0000003AL
#define GDC_CMD_AA_1_VECTOR_NOEND       0x0000003BL
#define GDC_CMD_AA_0_VECTOR_BLPO        0x0000003CL
#define GDC_CMD_AA_1_VECTOR_BLPO        0x0000003DL
#define GDC_CMD_AA_0_VECTOR_NOEND_BLPO  0x0000003EL
#define GDC_CMD_AA_1_VECTOR_NOEND_BLPO  0x0000003FL

#define GDC_CMD_BLT_FILL                0x00000041L
#define GDC_CMD_BLT_DRAW                0x00000042L
#define GDC_CMD_BITMAP                  0x00000043L
#define GDC_CMD_BLTCOPY_TOP_LEFT        0x00000044L
#define GDC_CMD_BLTCOPY_TOP_RIGHT       0x00000045L
#define GDC_CMD_BLTCOPY_BOTTOM_LEFT     0x00000046L
#define GDC_CMD_BLTCOPY_BOTTOM_RIGHT    0x00000047L
#define GDC_CMD_LOAD_TEXTURE            0x00000048L
#define GDC_CMD_LOAD_TILE               0x00000049L

#define GDC_CMD_TRAP_RIGHT              0x00000060L
#define GDC_CMD_TRAP_LEFT               0x00000061L
#define GDC_CMD_TRIANGLE_FAN            0x00000062L
#define GDC_CMD_FLAG_TRIANGLE_FAN       0x00000063L

#define GDC_CMD_FLUSH_FB                0x000000C1L
#define GDC_CMD_FLUSH_Z                 0x000000C2L

#define GDC_CMD_POLYGON_BEGIN           0x000000E0L
#define GDC_CMD_POLYGON_END             0x000000E1L
#define GDC_CMD_CLEAR_POLY_FLAG         0x000000E2L
#define GDC_CMD_NORMAL                  0x000000FFL

#define GDC_CMD_VECTOR_BLPO_FLAG        0x00040000L
#define GDC_CMD_FAST_VECTOR_BLPO_FLAG   0x00000004L

/* for MB86293 or later */
#define GDC_CMD_MDR1                            0x00000000L
#define GDC_CMD_MDR1S                           0x00000002L
#define GDC_CMD_MDR1B                           0x00000004L
#define GDC_CMD_MDR2                            0x00000001L
#define GDC_CMD_MDR2S                           0x00000003L
#define GDC_CMD_MDR2TL                          0x00000007L
#define GDC_CMD_GMDR1E                          0x00000010L
#define GDC_CMD_GMDR2E                          0x00000020L
#define GDC_CMD_OVERLAP_SHADOW_XY               0x00000000L
#define GDC_CMD_OVERLAP_SHADOW_XY_COMPOSITION   0x00000001L
#define GDC_CMD_OVERLAP_Z_PACKED_ONBS           0x00000007L
#define GDC_CMD_OVERLAP_Z_ORIGIN                0x00000000L
#define GDC_CMD_OVERLAP_Z_NON_TOPLEFT           0x00000001L
#define GDC_CMD_OVERLAP_Z_BORDER                0x00000002L
#define GDC_CMD_OVERLAP_Z_SHADOW                0x00000003L
#define GDC_CMD_BLTCOPY_ALT_ALPHA               0x00000000L	/* Reserverd */
#define GDC_CMD_DC_LOGOUT                       0x00000000L	/* Reserverd */
#define GDC_CMD_BODY_FORE_COLOR                 0x00000000L
#define GDC_CMD_BODY_BACK_COLOR                 0x00000001L
#define GDC_CMD_SHADOW_FORE_COLOR               0x00000002L
#define GDC_CMD_SHADOW_BACK_COLOR               0x00000003L
#define GDC_CMD_BORDER_FORE_COLOR               0x00000004L
#define GDC_CMD_BORDER_BACK_COLOR               0x00000005L

/* Status Registers */
#define GDC_ENGINE_BUSY_FLAG         0x00000001L
#define GDC_MAX_BANK                          2

/* Display Control Registers */
#define GDC_DISP_16_BPP_FLAG         0x80000000L

#define GDC_DISP_C_ENABLE_MASK       0x00010000L
#define GDC_DISP_W_ENABLE_MASK       0x00020000L
#define GDC_DISP_M_ENABLE_MASK       0x00040000L
#define GDC_DISP_B_ENABLE_MASK       0x00080000L
#define GDC_DISP_VIDEO_ENABLE_MASK   0x80000000L
#define GDC_DISP_CMODE_MASK                 0x3
#define GDC_DISP_BANK_MASK           0x60000000L
#define GDC_DISP_WIDTH_MASK              0x00FF
#define GDC_DISP_HEIGHT_MASK             0x0FFF
#define GDC_DISP_BLEND_RATIO_MASK    0x000000F0L

#define GDC_DISP_BLEND_SELECT_MASK   0x00008000L

#define GDC_DISP_BLEND_MODE_MASK     0x00010000L
#define GDC_DISP_TRANS_8_MASK        0x000000FFL
#define GDC_DISP_TRANS_16_MASK       0x00007FFFL
#define GDC_DISP_ZERO_MODE8_MASK     0x00000100L
#define GDC_DISP_ZERO_MODE16_MASK    0x00008000L

#define GDC_DISP_CKEY_MODE_MASK      0x00008000L
#define GDC_DISP_CKEY_SRC_MASK       0x00010000L
#define GDC_DISP_CKEY_COLOR_MASK     0x00007FFFL

#define GDC_CURSOR0_PRIORITY_MASK    0x00010000L
#define GDC_CURSOR1_PRIORITY_MASK    0x00020000L
#define GDC_CURSOR0_ENABLE_MASK      0x00100000L
#define GDC_CURSOR1_ENABLE_MASK      0x00200000L

#define GDC_DISP_W_8_BPP_SHIFT                6
#define GDC_DISP_W_16_BPP_SHIFT               5
#define GDC_DISP_W_24_BPP_SHIFT               4

#define GDC_DISP_C_ENABLE_SHIFT              16
#define GDC_DISP_W_ENABLE_SHIFT              17
#define GDC_DISP_M_ENABLE_SHIFT              18
#define GDC_DISP_B_ENABLE_SHIFT              19
#define GDC_DISP_CMODE_SHIFT                 31
#define GDC_DISP_BANK_SHIFT                  29
#define GDC_DISP_WIDTH_SHIFT                 16

#define GDC_DISP_BLEND_SELECT_SHIFT          15

#define GDC_DISP_BLEND_MODE_SHIFT            16
#define GDC_DISP_ZERO_MODE8_SHIFT             8
#define GDC_DISP_ZERO_MODE16_SHIFT           15

#define GDC_DISP_CKEY_MODE_SHIFT             15
#define GDC_DISP_CKEY_SRC_SHIFT              16

#define GDC_CURSOR0_PRIORITY_SHIFT           16
#define GDC_CURSOR1_PRIORITY_SHIFT           17
#define GDC_CURSOR0_ENABLE_SHIFT             20
#define GDC_CURSOR1_ENABLE_SHIFT             21

#define NUM_PALETTE                         256

/*
 * Register Address
 *   Internal Address = Long Word Address
 */

/* Host Interface Registers */
#define GDC_HOST_REG_DMA_COUNT			0x00000000L
#define GDC_HOST_REG_DMA_SETUP			0x00000001L
#define GDC_HOST_REG_DMA_TRANSFER_STOP		0x00000002L
/*	0x00000003L Reserved	*/
#define GDC_HOST_REG_DL_TRANSFER_STATUS		0x00000004L
/*	0x00000005L Reserved	*/

/* DMA request */
#define GDC_HOST_REG_DMA_REQ			0x00000006L
#define GDC_HOST_REG_INTERRUPT_STATUS		0x00000008L
#define GDC_HOST_REG_INTERRUPT_MASK		0x00000009L
/*	0x0000000AL Reserved	*/
#define GDC_HOST_REG_SOFTWARE_RESET		0x0000000BL
#define GDC_HOST_REG_DL_SRC_ADDR		0x00000010L
#define GDC_HOST_REG_DL_COUNT			0x00000011L
#define GDC_HOST_REG_DL_TRANSFER_REQ		0x00000012L

/* Memory I/F Mode Registers */
#define GDC_HOST_REG_MEMORY_MODE		0x00003fffL

/* for MB86293 or later */
#define GDC_HOST_REG_INTERNAL_CLOCK		0x0000000EL
#define GDC_HOST_REG_SID			0x0000000FL
#define GDC_HOST_REG_REGISTER_LOCATION_SWITCH	0x00000017L
#define GDC_HOST_REG_CHIP_ID			0x0000003CL
#define GDC_HOST_REG_DRAWING_BURST_MODE		0x00003FFEL

/* Interrupt Status */
#define	GDC_HOST_REG_IST_MASK_CERR	0x00000001
#define	GDC_HOST_REG_IST_MASK_CEND	0x00000002
#define	GDC_HOST_REG_IST_MASK_VSYNC	0x00000004
#define	GDC_HOST_REG_IST_MASK_FSYNC	0x00000008
#define	GDC_HOST_REG_IST_MASK_SYNCERR	0x00000010
#define	GDC_HOST_REG_IST_MASK_SII	0x04000000
#define	GDC_HOST_REG_IST_MASK_GI	0x08000000
#define	GDC_HOST_REG_IST_MASK_TC	0x20000000

/* Burst Controller(Coral LP) */
#define	GDC_HOST_REG_BSA		0x8000
#define	GDC_HOST_REG_BDA		0x8004
#define	GDC_HOST_REG_BCR		0x8008
#define	GDC_HOST_REG_BSR		0x800C
#define	GDC_HOST_REG_BER		0x8010
#define	GDC_HOST_REG_BST		0x8014
#define	GDC_HOST_REG_BCB		0x8040
#define	GDC_HOST_REG_BSR_MASK_TCM	0x00000020
#define	GDC_HOST_REG_SRBS		0x00A4

/* Serial/GPIO Interface (Coral LP) */
#define	GDC_HOST_REG_IOM		0x00A8
#define	GDC_HOST_REG_GD			0x00AC
#define	GDC_HOST_REG_SERIALCTL		0x00B0
#define	GDC_HOST_REG_SERIALDATA		0x00B4

#define GDC_HOST_REG_IOM_EEE_MASK	0x00000001
#define GDC_HOST_REG_IOM_BCE_MASK	0x00000002
#define GDC_HOST_REG_IOM_TCE_MASK	0x00000004
#define GDC_HOST_REG_IOM_SBE_MASK	0x00000008
#define GDC_HOST_REG_IOM_BEE_MASK	0x00000010
#define GDC_HOST_REG_IOM_RGB_MASK	0x00000020
#define GDC_HOST_REG_IOM_SER_MASK	0x00000040
#define	GDC_HOST_REG_IOM_GD_MASK	0x0000FF80
#define	GDC_HOST_REG_IOM_GIM_MASK	0x3FFF0000
#define	GDC_HOST_REG_IOM_GD_SHIFT	7
#define	GDC_HOST_REG_IOM_GIM_SHIFT	16

#define GDC_HOST_REG_GD_GD_MASK		0x00003FFF
#define GDC_HOST_REG_GD_GWE_MASK	0x01FF0000
#define GDC_HOST_REG_GD_GWE_SHIFT	16

/* Local Transfer */
#define GDC_HOST_REG_LSTA	0x00000010L
#define GDC_HOST_REG_LSA	0x00000040L
#define GDC_HOST_REG_LCO	0x00000044L
#define GDC_HOST_REG_LREQ	0x00000048L

/* Trapezoid Registers */
#define GDC_REG_TRAP_YS             0x00000000L
#define GDC_REG_TRAP_XS             0x00000001L
#define GDC_REG_TRAP_DXDY           0x00000002L
#define GDC_REG_TRAP_XUS            0x00000003L
#define GDC_REG_TRAP_DXUDY          0x00000004L
#define GDC_REG_TRAP_XLS            0x00000005L
#define GDC_REG_TRAP_DXLDY          0x00000006L
#define GDC_REG_TRAP_USN            0x00000007L
#define GDC_REG_TRAP_LSN            0x00000008L

/* Color DDA Registers */
#define GDC_REG_COL_DDA_RS          0x00000010L
#define GDC_REG_COL_DDA_DRDX        0x00000011L
#define GDC_REG_COL_DDA_DRDY        0x00000012L
#define GDC_REG_COL_DDA_GS          0x00000013L
#define GDC_REG_COL_DDA_DGDX        0x00000014L
#define GDC_REG_COL_DDA_DGDY        0x00000015L
#define GDC_REG_COL_DDA_BS          0x00000016L
#define GDC_REG_COL_DDA_DBDX        0x00000017L
#define GDC_REG_COL_DDA_DBDY        0x00000018L

/* Z DDA Registers */
#define GDC_REG_Z_DDA_ZS            0x00000020L
#define GDC_REG_Z_DDA_DZDX          0x00000021L
#define GDC_REG_Z_DDA_DZDY          0x00000022L

/* Texure DDA Registers */
#define GDC_REG_TEX_DDA_SS          0x00000030L
#define GDC_REG_TEX_DDA_DSDX        0x00000031L
#define GDC_REG_TEX_DDA_DSDY        0x00000032L
#define GDC_REG_TEX_DDA_TS          0x00000033L
#define GDC_REG_TEX_DDA_DTDX        0x00000034L
#define GDC_REG_TEX_DDA_DTDY        0x00000035L
#define GDC_REG_TEX_DDA_QS          0x00000036L
#define GDC_REG_TEX_DDA_DQDX        0x00000037L
#define GDC_REG_TEX_DDA_DQDY        0x00000038L

/* DDA Line Registers */
#define GDC_REG_DDA_LINE_LPN        0x00000050L
#define GDC_REG_DDA_LINE_LXS        0x00000051L
#define GDC_REG_DDA_LINE_LXDE       0x00000052L
#define GDC_REG_DDA_LINE_LYS        0x00000053L
#define GDC_REG_DDA_LINE_LYDE       0x00000054L
#define GDC_REG_DDA_LINE_LZS        0x00000055L
#define GDC_REG_DDA_LINE_LZDE       0x00000056L
#define GDC_REG_DDA_LINE_LRS        0x00000057L
#define GDC_REG_DDA_LINE_LRDE       0x00000058L
#define GDC_REG_DDA_LINE_LGS        0x00000059L
#define GDC_REG_DDA_LINE_LGDE       0x0000005AL
#define GDC_REG_DDA_LINE_LBS        0x0000005BL
#define GDC_REG_DDA_LINE_LBDE       0x0000005CL

/* Pixel Registers */
#define GDC_REG_PIXEL_PXDC          0x00000060L
#define GDC_REG_PIXEL_PYDC          0x00000061L
#define GDC_REG_PIXEL_PZDC          0x00000062L

/* BltFill Registers */
#define GDC_REG_BLTFILL_RECT_XS     0x00000080L
#define GDC_REG_BLTFILL_RECT_YS     0x00000081L
#define GDC_REG_BLTFILL_RSIZE_X     0x00000082L
#define GDC_REG_BLTFILL_RSIZE_Y     0x00000083L

/* BltCopy & LoadTexture Registers */
#define GDC_REG_BLTCOPY_SRC_ADDR    0x00000090L
#define GDC_REG_BLTCOPY_SRC_STRIDE  0x00000091L
#define GDC_REG_BLTCOPY_SRC_RECT_XS 0x00000092L
#define GDC_REG_BLTCOPY_SRC_RECT_YS 0x00000093L
#define GDC_REG_BLTCOPY_DES_ADDR    0x00000094L
#define GDC_REG_BLTCOPY_DES_STRIDE  0x00000095L
#define GDC_REG_BLTCOPY_DES_RECT_XS 0x00000096L
#define GDC_REG_BLTCOPY_DES_RECT_YS 0x00000097L
#define GDC_REG_BLTCOPY_DES_RSIZE_X 0x00000098L
#define GDC_REG_BLTCOPY_DES_RSIZE_Y 0x00000099L

#define GDC_REG_BLT_TRANS_COLOR     0x000000A0L

/* for MB86293 or later */
#define GDC_REG_BROKEN_LINE_CORRECT_LENGTH 0x000000A3L

#if 0

/* Special Registers */
#define GDC_REG_CTRL                0x00000100L
#define GDC_REG_FIFO_STATUS         0x00000101L
#define GDC_REG_FIFO_COUNT          0x00000102L
#define GDC_REG_SETUP_STATUS        0x00000103L
#define GDC_REG_DDA_STATUS          0x00000104L
#define GDC_REG_ENGINE_STATUS       0x00000105L
#define GDC_REG_ERROR_STATUS        0x00000106L
#define GDC_REG_RESERVE_0           0x00000107L
#define GDC_REG_MODE_MISC           0x00000108L	/* MDR0 */
#define GDC_REG_MODE_LINE           0x00000109L	/* MDR1 */
#define GDC_REG_MODE_POLYGON        0x0000010AL	/* MDR2 */
#define GDC_REG_MODE_TEXTURE        0x0000010BL	/* MDR3 */
#define GDC_REG_MODE_BITMAP         0x0000010CL	/* MDR4 */
#define GDC_REG_MODE_EXTENSION      0x0000010FL	/* MDR7 */

/* Configuration Registers */
#define GDC_REG_DRAW_BASE           0x00000110L
#define GDC_REG_X_RESOLUTION        0x00000111L
#define GDC_REG_Z_BASE              0x00000112L
#define GDC_REG_TEXTURE_BASE        0x00000113L
#define GDC_REG_POLYGON_FLAG_BASE   0x00000114L
#define GDC_REG_CLIP_XMIN           0x00000115L
#define GDC_REG_CLIP_XMAX           0x00000116L
#define GDC_REG_CLIP_YMIN           0x00000117L
#define GDC_REG_CLIP_YMAX           0x00000118L
#define GDC_REG_TEXURE_SIZE         0x00000119L
#define GDC_REG_TILE_SIZE           0x0000011AL
#define GDC_REG_TEX_BUF_OFFSET      0x0000011BL
#define GDC_REG_RESERVE_1           0x0000011CL

/* for MB86293 or later */
#define GDC_REG_ALPHA_MAP_BASE      0x0000011DL	/* ABR */

/* Constant Registers */
#define GDC_REG_FOREGROUND_COLOR    0x00000120L
#define GDC_REG_BACKGROUND_COLOR    0x00000121L
#define GDC_REG_ALPHA               0x00000122L
#define GDC_REG_LINE_PATTERN        0x00000123L
#define GDC_REG_RESERVE_3           0x00000124L
#define GDC_REG_TEX_BORDER_COLOR    0x00000125L

#else

/* Special Registers */
#define GDC_REG_CTRL                0x00000400L
#define GDC_REG_FIFO_STATUS         0x00000404L
#define GDC_REG_FIFO_COUNT          0x00000408L
#define GDC_REG_SETUP_STATUS        0x0000040CL
#define GDC_REG_DDA_STATUS          0x00000410L
#define GDC_REG_ENGINE_STATUS       0x00000414L
#define GDC_REG_ERROR_STATUS        0x00000418L
#define GDC_REG_MODE_MISC           0x00000420L	/* MDR0 */
#define GDC_REG_MODE_LINE           0x00000424L	/* MDR1 */
#define GDC_REG_MODE_POLYGON        0x00000428L	/* MDR2 */
#define GDC_REG_MODE_TEXTURE        0x0000042CL	/* MDR3 */
#define GDC_REG_MODE_BITMAP         0x00000430L	/* MDR4 */
#define GDC_REG_MODE_EXTENSION      0x0000043CL	/* MDR7 */

/* Configuration Registers */
#define GDC_REG_DRAW_BASE           0x00000440L
#define GDC_REG_X_RESOLUTION        0x00000444L
#define GDC_REG_Z_BASE              0x00000448L
#define GDC_REG_TEXTURE_BASE        0x0000044CL
#define GDC_REG_POLYGON_FLAG_BASE   0x00000450L
#define GDC_REG_CLIP_XMIN           0x00000454L
#define GDC_REG_CLIP_XMAX           0x00000458L
#define GDC_REG_CLIP_YMIN           0x0000045CL
#define GDC_REG_CLIP_YMAX           0x00000460L
#define GDC_REG_TEXURE_SIZE         0x00000464L
#define GDC_REG_TILE_SIZE           0x00000468L
#define GDC_REG_TEX_BUF_OFFSET      0x0000046CL

/* for MB86293 or later */
#define GDC_REG_ALPHA_MAP_BASE      0x00000474L	/* ABR */

/* Constant Registers */
#define GDC_REG_FOREGROUND_COLOR    0x00000480L
#define GDC_REG_BACKGROUND_COLOR    0x00000484L
#define GDC_REG_ALPHA               0x00000488L
#define GDC_REG_LINE_PATTERN        0x0000048CL
#define GDC_REG_TEX_BORDER_COLOR    0x00000494L
#define GDC_REG_LINE_PATTERN_OFFSET 0x000003E0L

#endif

/* FIFO */
#define GDC_REG_INPUT_FIFO          0x00000128L

/* General Purpose Registers */
#define GDC_REG_R0                  0x00000130L
#define GDC_REG_R1                  0x00000131L
#define GDC_REG_R2                  0x00000132L
#define GDC_REG_R3                  0x00000133L
#define GDC_REG_R4                  0x00000134L
#define GDC_REG_R5                  0x00000135L
#define GDC_REG_R6                  0x00000136L
#define GDC_REG_R7                  0x00000137L

/* Fast 2D Line Registers */
#define GDC_REG_FAST_LINE_LX0DC     0x00000150L
#define GDC_REG_FAST_LINE_LY0DC     0x00000151L
#define GDC_REG_FAST_LINE_LX1DC     0x00000152L
#define GDC_REG_FAST_LINE_LY1DC     0x00000153L

/* Fast 2D Triangle Registers */
#define GDC_REG_FAST_TRI_X0DC       0x00000160L
#define GDC_REG_FAST_TRI_Y0DC       0x00000161L
#define GDC_REG_FAST_TRI_X1DC       0x00000162L
#define GDC_REG_FAST_TRI_Y1DC       0x00000163L
#define GDC_REG_FAST_TRI_X2DC       0x00000164L
#define GDC_REG_FAST_TRI_Y2DC       0x00000165L

/* Display Controller Registers */
#define GDC_DISP_REG_MODE           0x0000L
#define GDC_DISP_REG_H_TOTAL        0x0004L
#define GDC_DISP_REG_H_PERIOD       0x0008L
#define GDC_DISP_REG_V_H_W_H_POS    0x000CL
#define GDC_DISP_REG_V_TOTAL        0x0010L
#define GDC_DISP_REG_V_PERIOD_POS   0x0014L
#define GDC_DISP_REG_WIN_POS        0x0018L
#define GDC_DISP_REG_WIN_SIZE       0x001CL
#define GDC_DISP_REG_C_MODE_W_H     0x0020L
#define GDC_DISP_REG_C_ORG_ADR      0x0024L
#define GDC_DISP_REG_C_DISP_ADR     0x0028L
#define GDC_DISP_REG_C_DISP_POS     0x002CL
#define GDC_DISP_REG_W_WIDTH        0x0030L
#define GDC_DISP_REG_W_ORG_ADR      0x0034L
/* 0x0038L reserved */
/* 0x003CL reserved */
#define GDC_DISP_REG_ML_MODE_W_H    0x0040L
#define GDC_DISP_REG_ML_ORG_ADR1    0x0044L
#define GDC_DISP_REG_ML_DISP_ADR1   0x0048L
#define GDC_DISP_REG_ML_ORG_ADR2    0x004CL
#define GDC_DISP_REG_ML_DISP_ADR2   0x0050L
#define GDC_DISP_REG_ML_DISP_POS    0x0054L
#define GDC_DISP_REG_MR_MODE_W_H    0x0058L
#define GDC_DISP_REG_MR_ORG_ADR1    0x005CL
#define GDC_DISP_REG_MR_DISP_ADR1   0x0060L
#define GDC_DISP_REG_MR_ORG_ADR2    0x0064L
#define GDC_DISP_REG_MR_DISP_ADR2   0x0068L
#define GDC_DISP_REG_MR_DISP_POS    0x006CL
#define GDC_DISP_REG_BL_MODE_W_H    0x0070L
#define GDC_DISP_REG_BL_ORG_ADR1    0x0074L
#define GDC_DISP_REG_BL_DISP_ADR1   0x0078L
#define GDC_DISP_REG_BL_ORG_ADR2    0x007CL
#define GDC_DISP_REG_BL_DISP_ADR2   0x0080L
#define GDC_DISP_REG_BL_DISP_POS    0x0084L
#define GDC_DISP_REG_BR_MODE_W_H    0x0088L
#define GDC_DISP_REG_BR_ORG_ADR1    0x008CL
#define GDC_DISP_REG_BR_DISP_ADR1   0x0090L
#define GDC_DISP_REG_BR_ORG_ADR2    0x0094L
#define GDC_DISP_REG_BR_DISP_ADR2   0x0098L
#define GDC_DISP_REG_BR_DISP_POS    0x009CL
#define GDC_DISP_REG_CURSOR_MODE    0x00A0L
#define GDC_DISP_REG_CUR1_ADR       0x00A4L
#define GDC_DISP_REG_CUR1_POS       0x00A8L
#define GDC_DISP_REG_CUR2_ADR       0x00ACL
#define GDC_DISP_REG_CUR2_POS       0x00B0L
#define GDC_DISP_REG_BLEND_MODE     0x00B4L
#define GDC_DISP_REG_KEY_COLOR      0x00B8L
#define GDC_DISP_REG_C_TRANS        0x00BCL

#define GDC_DISP_REG_ML_TRANS       0x00C0L
#define GDC_DISP_REG_MR_TRANS       0x00C4L
#define GDC_DISP_REG_MLMR_TRANS     0x00C0L

#define GDC_DISP_REG_PALLET_0       0x0400L
#define GDC_DISP_REG_PALLET_1       0x0800L

/* for MB86293 or later start */
#define GDC_DISP_REG_EXT_MODE       0x0100L

#define GDC_DISP_REG_L0_EXT_MODE    0x0110L
#define GDC_DISP_REG_L0_WIN_POS     0x0114L
#define GDC_DISP_REG_L0_WIN_SIZE    0x0118L

#define GDC_DISP_REG_L1_EXT_MODE    0x0120L
#define GDC_DISP_REG_L1_WIN_POS     0x0124L
#define GDC_DISP_REG_L1_WIN_SIZE    0x0128L

#define GDC_DISP_REG_L2_EXT_MODE    0x0130L
#define GDC_DISP_REG_L2_WIN_POS     0x0134L
#define GDC_DISP_REG_L2_WIN_SIZE    0x0138L

#define GDC_DISP_REG_L3_EXT_MODE    0x0140L
#define GDC_DISP_REG_L3_WIN_POS     0x0144L
#define GDC_DISP_REG_L3_WIN_SIZE    0x0148L

#define GDC_DISP_REG_L4_EXT_MODE    0x0150L
#define GDC_DISP_REG_L4_WIN_POS     0x0154L
#define GDC_DISP_REG_L4_WIN_SIZE    0x0158L

#define GDC_DISP_REG_L5_EXT_MODE    0x0160L
#define GDC_DISP_REG_L5_WIN_POS     0x0164L
#define GDC_DISP_REG_L5_WIN_SIZE    0x0168L

#define GDC_DISP_REG_LAYER_SELECT   0x0180L

#define GDC_DISP_REG_BG_COLOR       0x0184L

#define GDC_DISP_REG_BLEND_MODE_L0  0x00B4L
#define GDC_DISP_REG_BLEND_MODE_L1  0x0188L
#define GDC_DISP_REG_BLEND_MODE_L2  0x018CL
#define GDC_DISP_REG_BLEND_MODE_L3  0x0190L
#define GDC_DISP_REG_BLEND_MODE_L4  0x0194L
#define GDC_DISP_REG_BLEND_MODE_L5  0x0198L

#define GDC_DISP_REG_L0_TRANS       0x01A0L
#define GDC_DISP_REG_L1_TRANS       0x01A4L
#define GDC_DISP_REG_L2_TRANS       0x01A8L
#define GDC_DISP_REG_L3_TRANS       0x01ACL
#define GDC_DISP_REG_L4_TRANS       0x01B0L
#define GDC_DISP_REG_L5_TRANS       0x01B4L

#define GDC_DISP_REG_PALLET_2       0x1000L
#define GDC_DISP_REG_PALLET_3       0x1400L

#define GDC_DISP_L0_ENABLE_SHIFT             16
#define GDC_DISP_L1_ENABLE_SHIFT             17
#define GDC_DISP_L2_ENABLE_SHIFT             18
#define GDC_DISP_L3_ENABLE_SHIFT             19
#define GDC_DISP_L4_ENABLE_SHIFT             20
#define GDC_DISP_L5_ENABLE_SHIFT             21

#define GDC_DISP_L0_ENABLE_MASK      0x00010000L
#define GDC_DISP_L1_ENABLE_MASK      0x00020000L
#define GDC_DISP_L2_ENABLE_MASK      0x00040000L
#define GDC_DISP_L3_ENABLE_MASK      0x00080000L
#define GDC_DISP_L4_ENABLE_MASK      0x00100000L
#define GDC_DISP_L5_ENABLE_MASK      0x00200000L

#define GDC_EXTEND_MODE_MASK         0x00000003L	/* for L*EM register */
#define GDC_EXTEND_COLOR_MODE_MASK   0xC0000000L	/* for L*EM register */
#define GDC_EXTEND_COLOR_MODE_24BPP  0x40000000L	/* for L*EM register */
#define GDC_EXTEND_MAGNIFY_MASK      0x03000000L	/* for L1DM register */
#define GDC_EXTEND_MAGNIFY_MODE      0x02000000L	/* for L1DM register */

#define GDC_DISP_BLEND_SELECT_SHIFT          15
#define GDC_DISP_BLEND_CORRECT_SHIFT         14
#define GDC_DISP_BLEND_SOURCE_SHIFT          13

#define GDC_DISP_BLEND_SELECT_MASK   0x00008000L
#define GDC_DISP_BLEND_CORRECT_MASK  0x00004000L
#define GDC_DISP_BLEND_SOURCE_MASK   0x00002000L
#define GDC_DISP_BLEND_RATIO_MASK8   0x000000FFL

#define GDC_DISP_TRANS_24_MASK       0x00FFFFFFL

#define GDC_DISP_ZERO_MODE32_SHIFT           31
#define GDC_DISP_ZERO_MODE32_MASK    0x80000000L

#define GDC_DISP_LUT_OFFSET_SHIFT            20
#define GDC_DISP_LUT_OFFSET_MASK     0x00F00000L
/* for MB86293 or later end */

#endif /* _gdcreg_h_ */