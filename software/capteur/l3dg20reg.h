/* $Id: l3dg20reg.h,v 1.5 2017/06/05 11:00:18 bouyer Exp $ */
/*
 * Copyright (c) 2017 Manuel Bouyer
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/* register definitions for the L3DG20 3-axis giro */

#define L3DG20_ADDRESS 0x6b

#define L3DG20_WHO_AM_I		0x0f

#define L3DG20_CTRL_REG1	0x20
#define L3DG20_CTRL_REG1_DR_MASK	(char)0xc0
#define L3DG20_CTRL_REG1_BW_MASK	(char)0x30
#define L3DG20_CTRL_REG1_PD		(char)0x08 /* power down */
#define L3DG20_CTRL_REG1_ZEN		(char)0x04 /* z-axis enable */
#define L3DG20_CTRL_REG1_XEN		(char)0x02 /* x-axis enable */
#define L3DG20_CTRL_REG1_YEN		(char)0x01 /* y-axis enable */

#define L3DG20_CTRL_REG2	0x21
#define L3DG20_CTRL_REG2_HPM_MASK	(char)0x30
#define L3DG20_CTRL_REG2_HPCF_MASK	(char)0x0f

#define L3DG20_CTRL_REG3	0x22
#define L3DG20_CTRL_REG3_I1_INT		(char)0x80 /* intr enable on int1 */
#define L3DG20_CTRL_REG3_I1_BOOT	(char)0x40 /* boot status on int1 */
#define L3DG20_CTRL_REG3_IACTL		(char)0x20 /* interrupt active low */
#define L3DG20_CTRL_REG3_PP_OD		(char)0x10 /* irq open drain */
#define L3DG20_CTRL_REG3_DRDY		(char)0x08 /* data ready on DRDY */
#define L3DG20_CTRL_REG3_WTM		(char)0x04 /* fifo watermark on DRDY */
#define L3DG20_CTRL_REG3_ORUN		(char)0x02 /* fifo overrun on DRDY */
#define L3DG20_CTRL_REG3_EMPTY		(char)0x01 /* fifo empty on DRDY */

#define L3DG20_CTRL_REG4	0x23
#define L3DG20_CTRL_REG4_BDU		(char)0x80 /* block data update */
#define L3DG20_CTRL_REG4_BLE		(char)0x40 /* big/little endian */
#define L3DG20_CTRL_REG4_FSE_250dps	(char)0x00
#define L3DG20_CTRL_REG4_FSE_500dps	(char)0x10
#define L3DG20_CTRL_REG4_FSE_2000dps	(char)0x20
#define L3DG20_CTRL_REG4_FSE_MASK	(char)0x30
#define L3DG20_CTRL_REG4_SPI		(char)0x01 /* SPI mode */

#define L3DG20_CTRL_REG5	0x24
#define L3DG20_CTRL_REG5_BOOT		(char)0x80 /* reboot mem content */
#define L3DG20_CTRL_REG5_FIFO_EN	(char)0x40 /* fifo enable */
#define L3DG20_CTRL_REG5_HPEN		(char)0x10 /* High-pass filter enable */
#define L3DG20_CTRL_REG5_INT_SELMASK	(char)0x0c /* int1 output selection */
#define L3DG20_CTRL_REG5_OUT_SELMASK	(char)0x03 /* OUT selection */

#define L3DG20_REF_DATA		0x25	/* reference value for interrupt */

#define L3DG20_OUT_TEMP		0x26	/* temperature data */

#define L3DG20_STATUS_REG	0x27
#define L3DG20_STATUS_REG_ZYXOR	0x80 /* x, y, z overrun */
#define L3DG20_STATUS_REG_ZOR		(char)0x40 /* z overrun */
#define L3DG20_STATUS_REG_YOR		(char)0x20 /* y overrun */
#define L3DG20_STATUS_REG_XOR		(char)0x10 /* x overrun */
#define L3DG20_STATUS_REG_ZYXDA	0x08 /* x, y, z data available */
#define L3DG20_STATUS_REG_ZDA		(char)0x40 /* z data available */
#define L3DG20_STATUS_REG_YDA		(char)0x20 /* y data available */
#define L3DG20_STATUS_REG_XDA		(char)0x10 /* x data available */

#define L3DG20_OUT_X_L		0x28
#define L3DG20_OUT_X_H		0x29

#define L3DG20_OUT_Y_L		0x2a
#define L3DG20_OUT_Y_H		0x2b

#define L3DG20_OUT_Z_L		0x2c
#define L3DG20_OUT_Z_H		0x2d

#define L3DG20_FIFO_CTRL	0x2e
#define L3DG20_FIFO_CTRL_FM_MASK	(char)0xe0
#define L3DG20_FIFO_CTRL_FM_BYPASS	(char)0x00
#define L3DG20_FIFO_CTRL_FM_FIFO	(char)0x20
#define L3DG20_FIFO_CTRL_FM_STREAM	(char)0x40
#define L3DG20_FIFO_CTRL_FM_STRTOFIO	(char)0x60
#define L3DG20_FIFO_CTRL_FM_BPTOSTR	(char)0x80
#define L3DG20_FIFO_CTRL_WTM_MASK	(char)0x1f

#define L3DG20_FIFO_SRC		0x2f
#define L3DG20_FIFO_SRC_WTM		(char)0x80 /* fifo at or above watermark */
#define L3DG20_FIFO_SRC_OVRN		(char)0x40 /* fifo full */
#define L3DG20_FIFO_SRC_EMPTY		(char)0x20 /* fifo empty */
#define L3DG20_FIFO_SRC_LVL_MASK	(char)0x1f /* fifo level */

#define L3DG20_INT1_CFG		0x30
#define L3DG20_INT1_CFG_AND		(char)0x80 /* AND combination of IRQ */
#define L3DG20_INT1_CFG_LIR		(char)0x40 /* IRQ latch enable */
#define L3DG20_INT1_CFG_ZHIE		(char)0x20 /* Z high interrupt enable */
#define L3DG20_INT1_CFG_ZLIE		(char)0x10 /* Z low interrupt enable */
#define L3DG20_INT1_CFG_YHIE		(char)0x08 /* Y high interrupt enable */
#define L3DG20_INT1_CFG_YLIE		(char)0x04 /* Y low interrupt enable */
#define L3DG20_INT1_CFG_XHIE		(char)0x02 /* X high interrupt enable */
#define L3DG20_INT1_CFG_XLIE		(char)0x01 /* X low interrupt enable */

#define L3DG20_INT1_SRC		0x31
#define L3DG20_INT1_SRC_IA		(char)0x40 /* interrupt active */
#define L3DG20_INT1_SRC_ZH		(char)0x20 /* Z high */
#define L3DG20_INT1_SRC_ZL		(char)0x10 /* Z low */
#define L3DG20_INT1_SRC_YH		(char)0x08 /* Y high */
#define L3DG20_INT1_SRC_YL		(char)0x04 /* Y low */
#define L3DG20_INT1_SRC_XH		(char)0x02 /* X high */
#define L3DG20_INT1_SRC_XL		(char)0x01 /* X low */

#define L3DG20_INT1_THRS_XH	0x32
#define L3DG20_INT1_THRS_XL	0x33

#define L3DG20_INT1_THRS_YH	0x34
#define L3DG20_INT1_THRS_YL	0x35

#define L3DG20_INT1_THRS_ZH	0x36
#define L3DG20_INT1_THRS_ZL	0x37

#define L3DG20_INT1_DURATION	0x38
#define L3DG20_INT1_DURATION_WAIT	(char)0x80
#define L3DG20_INT1_DURATION_VAL_MASK	(char)0x7f

struct gyro_data {
	long gyro_x;
	long gyro_y;
	long gyro_z;
};

void get_data_w(void);
