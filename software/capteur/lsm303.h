/* $Id: lsm303.h,v 1.4 2017/06/05 11:00:18 bouyer Exp $ */
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

/* register definitions for the LSM303DLHC 3d-accel and 3d-magnetometer */

/* accellerometer */
#define LSM303_A_ADDR	0x19
#define LSM303_A_CTRL_REG1		0x20
#define LSM303_A_CTRL_REG1_ODR_DOWN		(char)0x00
#define LSM303_A_CTRL_REG1_ODR_1HZ		(char)0x10
#define LSM303_A_CTRL_REG1_ODR_10HZ		(char)0x20
#define LSM303_A_CTRL_REG1_ODR_25HZ		(char)0x30
#define LSM303_A_CTRL_REG1_ODR_50HZ		(char)0x40
#define LSM303_A_CTRL_REG1_ODR_100HZ		(char)0x50
#define LSM303_A_CTRL_REG1_ODR_200HZ		(char)0x60
#define LSM303_A_CTRL_REG1_ODR_400HZ		(char)0x70
#define LSM303_A_CTRL_REG1_LPEN			(char)0x08
#define LSM303_A_CTRL_REG1_ZEN			(char)0x04
#define LSM303_A_CTRL_REG1_YEN			(char)0x02
#define LSM303_A_CTRL_REG1_XEN			(char)0x01

#define LSM303_A_CTRL_REG2		0x21
#define LSM303_A_CTRL_REG2_FDS			(char)0x08
#define LSM303_A_CTRL_REG2_HPCLICK		(char)0x04
#define LSM303_A_CTRL_REG2_HPIS2		(char)0x02
#define LSM303_A_CTRL_REG2_HPIS1		(char)0x01

#define LSM303_A_CTRL_REG3		0x22
#define LSM303_A_CTRL_REG3_I1_CLICK		(char)0x80
#define LSM303_A_CTRL_REG3_I1_AOI1		(char)0x40
#define LSM303_A_CTRL_REG3_I1_AOI2		(char)0x20
#define LSM303_A_CTRL_REG3_I1_DRDY1		(char)0x10
#define LSM303_A_CTRL_REG3_I1_DRDY2		(char)0x08
#define LSM303_A_CTRL_REG3_I1_WTM		(char)0x04
#define LSM303_A_CTRL_REG3_I1_OVER		(char)0x02

#define LSM303_A_CTRL_REG4		0x23
#define LSM303_A_CTRL_REG4_BDU			(char)0x80
#define LSM303_A_CTRL_REG4_BLE			(char)0x40
#define LSM303_A_CTRL_REG4_FS2G			(char)0x00
#define LSM303_A_CTRL_REG4_FS4G			(char)0x10
#define LSM303_A_CTRL_REG4_FS8G			(char)0x20
#define LSM303_A_CTRL_REG4_FS16G		(char)0x30
#define LSM303_A_CTRL_REG4_HR			(char)0x08
#define LSM303_A_CTRL_REG4_SIM			(char)0x01

#define LSM303_A_CTRL_REG5		0x24
#define LSM303_A_CTRL_REG5_BOOT			(char)0x80
#define LSM303_A_CTRL_REG5_FIFOEN		(char)0x40
#define LSM303_A_CTRL_REG5_LIRINT1		(char)0x08
#define LSM303_A_CTRL_REG5_D4DINT1		(char)0x04
#define LSM303_A_CTRL_REG5_LIRINT2		(char)0x02
#define LSM303_A_CTRL_REG5_D4DINT2		(char)0x01

#define LSM303_A_CTRL_REG6		0x25
#define LSM303_A_CTRL_REG6_I2_CLICK		(char)0x80
#define LSM303_A_CTRL_REG6_I2_INT1		(char)0x40
#define LSM303_A_CTRL_REG6_I2_INT2		(char)0x20
#define LSM303_A_CTRL_REG6_BOOT_I1		(char)0x10
#define LSM303_A_CTRL_REG6_P2_ACT		(char)0x08
#define LSM303_A_CTRL_REG6_H_LACTIVE		(char)0x02

#define LSM303_A_REFERENCE		0x26

#define LSM303_A_STATUS_REG		0x27
#define LSM303_A_STATUS_REG_ZYXOR		(char)0x80
#define LSM303_A_STATUS_REG_ZOR			(char)0x40
#define LSM303_A_STATUS_REG_YOR			(char)0x20
#define LSM303_A_STATUS_REG_XOR			(char)0x10
#define LSM303_A_STATUS_REG_ZYXDA		(char)0x08
#define LSM303_A_STATUS_REG_ZDA			(char)0x04
#define LSM303_A_STATUS_REG_YDA			(char)0x02
#define LSM303_A_STATUS_REG_XDA			(char)0x01

#define LSM303_A_OUT_X_L		0x28
#define LSM303_A_OUT_X_H		0x29
#define LSM303_A_OUT_Y_L		0x2A
#define LSM303_A_OUT_Y_H		0x2B
#define LSM303_A_OUT_Z_L		0x2C
#define LSM303_A_OUT_Z_H		0x2D

#define LSM303_A_FIFO_CTRL_REG		0x2E
#define LSM303_A_FIFO_CTRL_REG_BYPASS		(char)0x00
#define LSM303_A_FIFO_CTRL_REG_FIFO		(char)0x40
#define LSM303_A_FIFO_CTRL_REG_STREAM		(char)0x80
#define LSM303_A_FIFO_CTRL_REG_TRIGGER		(char)0xc0
#define LSM303_A_FIFO_CTRL_REG_TR_INT1		(char)0x20

#define LSM303_A_FIFO_SRC_REG		0x2F
#define LSM303_A_FIFO_SRC_REG_WTM		(char)0x80
#define LSM303_A_FIFO_SRC_REG_OVR		(char)0x40
#define LSM303_A_FIFO_SRC_REG_EMPTY		(char)0x20

#define LSM303_A_INT1_CFG		0x30
#define LSM303_A_INT1_CFG_AOI			(char)0x80
#define LSM303_A_INT1_CFG_6D			(char)0x40
#define LSM303_A_INT1_CFG_ZHIE			(char)0x20
#define LSM303_A_INT1_CFG_ZLIE			(char)0x10
#define LSM303_A_INT1_CFG_YHIE			(char)0x08
#define LSM303_A_INT1_CFG_YLIE			(char)0x04
#define LSM303_A_INT1_CFG_XHIE			(char)0x02
#define LSM303_A_INT1_CFG_XLIE			(char)0x01

#define LSM303_A_INT1_SRC		0x31
#define LSM303_A_INT1_SRC_IA			(char)0x40
#define LSM303_A_INT1_SRC_ZH			(char)0x20
#define LSM303_A_INT1_SRC_ZL			(char)0x10
#define LSM303_A_INT1_SRC_YH			(char)0x08
#define LSM303_A_INT1_SRC_YL			(char)0x04
#define LSM303_A_INT1_SRC_XH			(char)0x02
#define LSM303_A_INT1_SRC_XL			(char)0x01

#define LSM303_A_INT1_THS		0x32

#define LSM303_A_INT1_DURATION		0x33

#define LSM303_A_INT2_CFG		0x34
#define LSM303_A_INT2_CFG_AOI			(char)0x80
#define LSM303_A_INT2_CFG_6D			(char)0x40
#define LSM303_A_INT2_CFG_ZHIE			(char)0x20
#define LSM303_A_INT2_CFG_ZLIE			(char)0x10
#define LSM303_A_INT2_CFG_YHIE			(char)0x08
#define LSM303_A_INT2_CFG_YLIE			(char)0x04
#define LSM303_A_INT2_CFG_XHIE			(char)0x02
#define LSM303_A_INT2_CFG_XLIE			(char)0x01

#define LSM303_A_INT2_SRC		0x35
#define LSM303_A_INT2_SRC_IA			(char)0x40
#define LSM303_A_INT2_SRC_ZH			(char)0x20
#define LSM303_A_INT2_SRC_ZL			(char)0x10
#define LSM303_A_INT2_SRC_YH			(char)0x08
#define LSM303_A_INT2_SRC_YL			(char)0x04
#define LSM303_A_INT2_SRC_XH			(char)0x02
#define LSM303_A_INT2_SRC_XL			(char)0x01

#define LSM303_A_INT2_THS		0x36

#define LSM303_A_INT2_DURATION		0x37

#define LSM303_A_CLICK_CFG		0x38
#define LSM303_A_CLICK_CFG_ZD			(char)0x20
#define LSM303_A_CLICK_CFG_ZS			(char)0x10
#define LSM303_A_CLICK_CFG_YD			(char)0x08
#define LSM303_A_CLICK_CFG_YS			(char)0x04
#define LSM303_A_CLICK_CFG_XD			(char)0x02
#define LSM303_A_CLICK_CFG_XS			(char)0x01

#define LSM303_A_CLICK_SRC		0x39
#define LSM303_A_CLICK_SRC_IA			(char)0x40
#define LSM303_A_CLICK_SRC_DCLICK		(char)0x20
#define LSM303_A_CLICK_SRC_SCLICK		(char)0x10
#define LSM303_A_CLICK_SRC_SIGN			(char)0x08
#define LSM303_A_CLICK_SRC_Z			(char)0x04
#define LSM303_A_CLICK_SRC_Y			(char)0x02
#define LSM303_A_CLICK_SRC_X			(char)0x01

#define LSM303_A_CLICK_THS		0x3A

#define LSM303_A_TIME_LIMIT		0x3B

#define LSM303_A_TIME_LATENCY		0x3C

#define LSM303_A_TIME_WINDOW		0x3D

struct accel_data {
	long accel_x;
	long accel_y;
	long accel_z;
};

/* magnetometer */
#define LSM303_M_ADDR	0x1e
#define LSM303_M_CRA_REG		0x00
#define LSM303_M_CRA_REG_TEMPEN			(char)0x80
#define LSM303_M_CRA_REG_DO_75			0x00
#define LSM303_M_CRA_REG_DO1_5			0x04
#define LSM303_M_CRA_REG_DO3			0x08
#define LSM303_M_CRA_REG_DO7_5			0x0c
#define LSM303_M_CRA_REG_DO15			0x10
#define LSM303_M_CRA_REG_DO30			0x14
#define LSM303_M_CRA_REG_DO75			0x18
#define LSM303_M_CRA_REG_DO220			0x1c

#define LSM303_M_CRB_REG		0x01
#define LSM303_M_CRB_REGG1_3			(char)0x20
#define LSM303_M_CRB_REGG1_9			(char)0x40
#define LSM303_M_CRB_REGG2_5			(char)0x60
#define LSM303_M_CRB_REGG4_0			(char)0x80
#define LSM303_M_CRB_REGG4_7			(char)0xa0
#define LSM303_M_CRB_REGG5_6			(char)0xc0
#define LSM303_M_CRB_REGG8_1			(char)0xe0

#define LSM303_M_MR_REG			0x02
#define LSM303_M_MR_REG_CONT			(char)0x00
#define LSM303_M_MR_REG_SINGLE			(char)0x01
#define LSM303_M_MR_REG_SLEEP			(char)0x02

#define LSM303_M_OUT_X_H		0x03

#define LSM303_M_OUT_X_L		0x04

#define LSM303_M_OUT_Z_H		0x05

#define LSM303_M_OUT_Z_L		0x06

#define LSM303_M_OUT_Y_H		0x07

#define LSM303_M_OUT_Y_L		0x08

#define LSM303_M_SR_REG			0x09
#define LSM303_M_SR_REG_LOCK			(char)0x02
#define LSM303_M_SR_REG_DRDY			(char)0x01

#define LSM303_M_IRA_REG		0x0A

#define LSM303_M_IRB_REG		0x0B

#define LSM303_M_IRC_REG		0x0C

#define LSM303_M_TEMP_OUT_H		0x31
#define LSM303_M_TEMP_OUT_L		0x32

struct magn_data {
	long magn_x;
	long magn_z;
	long magn_y;
};

