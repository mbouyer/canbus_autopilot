/* $Id: marg_filter.c,v 1.15 2017/06/05 11:00:18 bouyer Exp $ */
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

/*
 * Based on Sebastian O.H. Madgwick's paper
 *  An efficient orientation filter for inertial and
 *      inertial/magnetic sensor arrays
 */


#include <stdio.h>
#include <math.h>
#include "marg_filter.h"
#include "l3dg20reg.h"
/* System constants */
#define deltat 0.1f	/* sampling period in seconds (shown as 100 ms) */
/* define beta and zeta:
 * beta = sqrtf(3.0 / 4.0) * gyrooffset,
 *     with gyrooffset = 100 d/s at start for convergence
 *     with gyrooffset = 5 d/s running for stability
 * zeta = sqrtf(3.0 / 4.0) * gyrodrift, with gyrodrift = 0.1d/s/s (0.0015r/s/s)
 */
#define beta_start 1.56f
#define beta_converged 0.076f
static float beta;

#define zeta 0.0013f

#define deltatzeta 0.00013f

/* Global system variables */
float a_x, a_y, a_z;			/* accelerometer measurements */
float w_x, w_y, w_z;			/* gyroscope measurements in rad/s */
float m_x, m_y, m_z;			/* magnetometer measurements */

/* estimated orientation quaternion elements */
float SEq_1, SEq_2, SEq_3, SEq_4;

static float b_x, b_z;		/* reference direction of flux in earth frame */
float w_bx, w_by, w_bz;		/* estimate gyroscope biases error */

/* euler attitude */
int heading;
int pitch;
int roll;
float rate_of_turn;

/* auxiliary variables to avoid reapeated calculations */
static float twoSEq_1;
static float twoSEq_2;
static float twoSEq_3;
static float twoSEq_4;
static float twob_x;
static float twob_z;
static float twob_xSEq_1;
static float twob_xSEq_2;
static float twob_xSEq_3;
static float twob_xSEq_4;
static float twob_zSEq_1;
static float twob_zSEq_2;
static float twob_zSEq_3;
static float twob_zSEq_4;
static float SEq_1SEq_1;
static float SEq_1SEq_2;
static float SEq_1SEq_3;
static float SEq_1SEq_4;
static float SEq_2SEq_2;
static float SEq_2SEq_3;
static float SEq_2SEq_4;
static float SEq_3SEq_3;
static float SEq_3SEq_4;
static float SEq_4SEq_4;
static float twom_x;
static float twom_y;
static float twom_z;

static unsigned char filter_converge;

/* initial conditions */
void
filterInit(void)
{
	SEq_1 = 1;
	SEq_2 = 0;
	SEq_3 = 0;
	SEq_4 = 0;
	twoSEq_1 = 2.0f;
	twoSEq_2 = 0;
	twoSEq_3 = 0;
	twoSEq_4 = 0;
	SEq_1SEq_1 = 0;
	SEq_1SEq_2 = 0;
	SEq_1SEq_3 = 0;
	SEq_1SEq_4 = 0;
	SEq_2SEq_2 = 0;
	SEq_2SEq_3 = 0;
	SEq_2SEq_4 = 0;
	SEq_3SEq_3 = 0;
	SEq_3SEq_4 = 0;
	SEq_4SEq_4 = 0;

	b_x = 1; /* assume magnetic flux is horizontal (will be corected) */
	b_z = 0;
	twob_x = 2.0f;
	twob_z = 0;

	twob_xSEq_1 = 2.0f;
	twob_xSEq_2 = 0;
	twob_xSEq_3 = 0;
	twob_xSEq_4 = 0;
	twob_zSEq_1 = 0;
	twob_zSEq_2 = 0;
	twob_zSEq_3 = 0;
	twob_zSEq_4 = 0;
#if 0 /* read from eeprom */
	w_bx = 0; 
	w_by = 0;
	w_bz = 0;
#endif
	filter_converge = 255;
	beta = beta_start;
}

/* Function to compute one filter iteration */
void
filterUpdate(void)
{
	/* local system variables */
	static float norm;	/* vector norm */
	static float ftemp1, ftemp2;
	/* quaternion rate from gyroscopes elements */
	static float SEqDot_omega_1, SEqDot_omega_2, SEqDot_omega_3, SEqDot_omega_4;
	static float f_1, f_2, f_3, f_4, f_5, f_6; /* objective function elements */
	/* objective function Jacobian elements */
	static float J_32, J_33,
	  J_43, J_44, J_51, J_52, J_53, J_54, J_62, J_63;
	/* estimated direction of the gyroscope error */
	static float SEqHatDot_1, SEqHatDot_2, SEqHatDot_3, SEqHatDot_4;
	/* estimated direction of the gyroscope error (angular) */
	static float w_err_x, w_err_y, w_err_z;
	static float h_x, h_y, h_z;	/* computed flux in the earth frame */
	static float halfw_x, halfw_y, halfw_z;

#if 0
	/* normalise the accelerometer measurement */
	norm = 1 / sqrtf(a_x * a_x + a_y * a_y + a_z * a_z);
	a_x *= norm;
	a_y *= norm;
	a_z *= norm;
	/* normalise the magnetometer measurement */
	norm = 1 / sqrtf(m_x * m_x + m_y * m_y + m_z * m_z);
	m_x *= norm;
	twom_x = 2.0f * m_x;
	m_y *= norm;
	twom_y = 2.0f * m_y;
	m_z *= norm;
	twom_z = 2.0f * m_z;
#else
	twom_x = 2.0f * m_x;
	twom_y = 2.0f * m_y;
	twom_z = 2.0f * m_z;
#endif
	/* compute the objective function and Jacobian */
	f_1 = 2.0f * SEq_2SEq_4 - 2.0f * SEq_1SEq_3 - a_x;
	f_2 = 2.0f * SEq_1SEq_2 + 2.0f * SEq_3SEq_4 - a_y;
	f_3 = 1.0f - 2.0f * SEq_2SEq_2 - 2.0f * SEq_3SEq_3 - a_z;
	f_4 = twob_x * (0.5f - SEq_3SEq_3 - SEq_4SEq_4) + twob_z * (SEq_2SEq_4 - SEq_1SEq_3) - m_x;
	f_5 = twob_x * (SEq_2SEq_3 - SEq_1SEq_4) + twob_z * (SEq_1SEq_2 + SEq_3SEq_4) - m_y;
	f_6 = twob_x * (SEq_1SEq_3 + SEq_2SEq_4) + twob_z * (0.5f - SEq_2SEq_2 - SEq_3SEq_3) - m_z;
#define J_11or24  twoSEq_3	/* J_11 negated in matrix multiplication */
#define J_12or23  twoSEq_4	/* J_12 negated in matrix multiplication */
#define J_13or22  twoSEq_1
#define J_14or21  twoSEq_2
	J_32 = 2.0f * J_14or21;	/* negated in matrix multiplication */
	J_33 = 2.0f * J_11or24;	/* negated in matrix multiplication */
#define J_41  twob_zSEq_3	/* negated in matrix multiplication */
#define J_42  twob_zSEq_4
	J_43 = 2.0f * twob_xSEq_3 + twob_zSEq_1; /* negated in matrix multiplication */
	J_44 = 2.0f * twob_xSEq_4 - twob_zSEq_2; /* negated in matrix multiplication */
	J_51 = twob_xSEq_4 - twob_zSEq_2; /* negated in matrix multiplication */
	J_52 = twob_xSEq_3 + twob_zSEq_1;
	J_53 = twob_xSEq_2 + twob_zSEq_4;
	J_54 = twob_xSEq_1 - twob_zSEq_3; /* negated in matrix multiplication */
#define J_61  twob_xSEq_3
	J_62 = twob_xSEq_4 - 2.0f * twob_zSEq_2;
	J_63 = twob_xSEq_1 - 2.0f * twob_zSEq_3;
#define J_64  twob_xSEq_2

	get_data_w();
	/* compute the gradient (matrix multiplication) */
	SEqHatDot_1 = J_14or21 * f_2 - J_11or24 * f_1 - J_41	* f_4 - J_51 * f_5 +	J_61 * f_6;
	SEqHatDot_2 = J_12or23 * f_1 + J_13or22 * f_2 - J_32	* f_3 + J_42 * f_4 +	J_52 * f_5 + J_62 * f_6;
	SEqHatDot_3 = J_12or23 * f_2 - J_33 * f_3 - J_13or22	* f_1 - J_43 * f_4 +	J_53 * f_5 + J_63 * f_6;
	SEqHatDot_4 = J_14or21 * f_1 + J_11or24 * f_2 - J_44	* f_4 - J_54 * f_5 +	J_64 * f_6;
	/*
	 * normalise the gradient to estimate direction of the gyroscope error
	 */
	norm = 1 / sqrtf(SEqHatDot_1 * SEqHatDot_1 + SEqHatDot_2 * SEqHatDot_2 + SEqHatDot_3 * SEqHatDot_3 + SEqHatDot_4 * SEqHatDot_4);
	SEqHatDot_1 = SEqHatDot_1 * norm;
	SEqHatDot_2 = SEqHatDot_2 * norm;
	SEqHatDot_3 = SEqHatDot_3 * norm;
	SEqHatDot_4 = SEqHatDot_4 * norm;

	get_data_w();
	/* compute angular estimated direction of the gyroscope error */
	w_err_x = twoSEq_1 * SEqHatDot_2 - twoSEq_2 * SEqHatDot_1 - twoSEq_3 * SEqHatDot_4 + twoSEq_4 * SEqHatDot_3;
	w_err_y = twoSEq_1 * SEqHatDot_3 + twoSEq_2 * SEqHatDot_4 - twoSEq_3 * SEqHatDot_1 - twoSEq_4 * SEqHatDot_2;
	w_err_z = twoSEq_1 * SEqHatDot_4 - twoSEq_2 * SEqHatDot_3 + twoSEq_3 * SEqHatDot_2 - twoSEq_4 * SEqHatDot_1;
	/* compute and remove the gyroscope biases */
	w_bx += w_err_x * deltatzeta;
	w_by += w_err_y * deltatzeta;
	w_bz += w_err_z * deltatzeta;
	w_x -= w_bx;
	w_y -= w_by;
	w_z -= w_bz;

	get_data_w();

	/* compute the quaternion rate measured by gyroscopes */
#if 0
	SEqDot_omega_1 = -halfSEq_2 * w_x - halfSEq_3 * w_y - halfSEq_4 * w_z;
	SEqDot_omega_2 = halfSEq_1 * w_x + halfSEq_3 * w_z - halfSEq_4 * w_y;
	SEqDot_omega_3 = halfSEq_1 * w_y - halfSEq_2 * w_z + halfSEq_4 * w_x;
	SEqDot_omega_4 = halfSEq_1 * w_z + halfSEq_2 * w_y - halfSEq_3 * w_x;
#else
	/*
	 * more efficient way of doing the above, as halfSEq_* are not
	 * needed anywhere else
	 */
	halfw_x = w_x * 0.5f;
	halfw_y = w_y * 0.5f;
	halfw_z = w_z * 0.5f;
	SEqDot_omega_1 = -SEq_2 * halfw_x - SEq_3 * halfw_y - SEq_4 * halfw_z;
	SEqDot_omega_2 = SEq_1 * halfw_x + SEq_3 * halfw_z - SEq_4 * halfw_y;
	SEqDot_omega_3 = SEq_1 * halfw_y - SEq_2 * halfw_z + SEq_4 * halfw_x;
	SEqDot_omega_4 = SEq_1 * halfw_z + SEq_2 * halfw_y - SEq_3 * halfw_x;
#endif
	/* compute then integrate the estimated quaternion rate */
	SEq_1 += (SEqDot_omega_1 - (beta * SEqHatDot_1)) * deltat;
	SEq_2 += (SEqDot_omega_2 - (beta * SEqHatDot_2)) * deltat;
	SEq_3 += (SEqDot_omega_3 - (beta * SEqHatDot_3)) * deltat;
	SEq_4 += (SEqDot_omega_4 - (beta * SEqHatDot_4)) * deltat;
	/* normalise quaternion */
	norm = 1 / sqrtf(SEq_1 * SEq_1 + SEq_2 * SEq_2 + SEq_3 * SEq_3 + SEq_4 * SEq_4);
	SEq_1 *= norm;
	SEq_2 *= norm;
	SEq_3 *= norm;
	SEq_4 *= norm;

	get_data_w();

	/* recompute auxlirary variables */
#if 0 /* not needed any more */
	halfSEq_1 = 0.5f * SEq_1;
	halfSEq_2 = 0.5f * SEq_2;
	halfSEq_3 = 0.5f * SEq_3;
	halfSEq_4 = 0.5f * SEq_4;
#endif
	twoSEq_1 = 2.0f * SEq_1;
	twoSEq_2 = 2.0f * SEq_2;
	twoSEq_3 = 2.0f * SEq_3;
	twoSEq_4 = 2.0f * SEq_4;
	SEq_1SEq_1 = SEq_1 * SEq_1;
	SEq_1SEq_2 = SEq_1 * SEq_2;
	SEq_1SEq_3 = SEq_1 * SEq_3;
	SEq_1SEq_4 = SEq_1 * SEq_4;
	SEq_2SEq_2 = SEq_2 * SEq_2;
	SEq_2SEq_3 = SEq_2 * SEq_3;
	SEq_2SEq_4 = SEq_2 * SEq_4;
	SEq_3SEq_3 = SEq_3 * SEq_3;
	SEq_3SEq_4 = SEq_3 * SEq_4;
	SEq_4SEq_4 = SEq_4 * SEq_4;

	get_data_w();

	/* compute flux in the earth frame */
	h_x = twom_x * (0.5f - SEq_3SEq_3 - SEq_4SEq_4) + twom_y * (SEq_2SEq_3 - SEq_1SEq_4) + twom_z * (SEq_2SEq_4 + SEq_1SEq_3);
	h_y = twom_x * (SEq_2SEq_3 + SEq_1SEq_4) + twom_y * (0.5f - SEq_2SEq_2 - SEq_4SEq_4) + twom_z * (SEq_3SEq_4 - SEq_1SEq_2);
	h_z = twom_x * (SEq_2SEq_4 - SEq_1SEq_3) + twom_y * (SEq_3SEq_4 + SEq_1SEq_2) + twom_z * (0.5f - SEq_2SEq_2 - SEq_3SEq_3);
	/* normalise the flux vector to have only components in the x and z */
	b_x = sqrtf((h_x * h_x) + (h_y * h_y));
	b_z = h_z;

	twob_x = 2.0f * b_x;
	twob_z = 2.0f * b_z;

	twob_xSEq_1 = twob_x * SEq_1;
	twob_xSEq_2 = twob_x * SEq_2;
	twob_xSEq_3 = twob_x * SEq_3;
	twob_xSEq_4 = twob_x * SEq_4;
	twob_zSEq_1 = twob_z * SEq_1;
	twob_zSEq_2 = twob_z * SEq_2;
	twob_zSEq_3 = twob_z * SEq_3;
	twob_zSEq_4 = twob_z * SEq_4;

	get_data_w();

	/* compute interesting euler angles */
	/* heading: atan2f(2(q0*q3 + q1*q2), 1 - 2(q2*q2 + q3*q3)) */
	/* but negate ftemp1 because we're upside-down */
	ftemp1 = (SEq_1SEq_4 + SEq_2SEq_3) * -2.0F;
	ftemp2 = 1.0F - (SEq_3SEq_3 + SEq_4SEq_4) * 2.0F;
	heading = (atan2f(ftemp1, ftemp2) * 10000.0F);

	/*
	 * also compute rate of turn around z
	 * we have 4 different equations which can give us a w_z
	 * all have a possible divide by 0 possibility.
	 * Select 2 of them and use the largest divisor
	 */

	if (fabsf(SEq_4) > fabsf(SEq_1)) {
		ftemp2 = -2.0F * (
		    (SEq_4SEq_4 + SEq_1SEq_1) * SEqDot_omega_1 +
		    (SEq_1SEq_2 - SEq_3SEq_4) * SEqDot_omega_2 +
		    (SEq_2SEq_4 + SEq_1SEq_3) * SEqDot_omega_4);
		/*
		 this should be:
		 ftemp1 = SEq_4 * (SEq_1* SEq_1 + SEq_2SEq_2 + SEq_3SEq_3 + SEq_4SEq_4);
		 rate_of_turn = ftemp2 / ftemp1;
		 but we know SEq is normalized, to ftemp1 should be equal to SEq_4
		 */
		rate_of_turn = ftemp2 / SEq_4;
	} else {
		ftemp2 = 2.0F * (
		    (SEq_2SEq_4 + SEq_1SEq_3) * SEqDot_omega_2 +
		    (SEq_3SEq_4 - SEq_1SEq_2) * SEqDot_omega_3 +
		    (SEq_1SEq_1 + SEq_4SEq_4) * SEqDot_omega_4);
		rate_of_turn = ftemp2 / SEq_1;
	}

	get_data_w();

	/* pitch: arcsin(2(q0 * q2 - q3 * q1)) */
	ftemp1 = SEq_1SEq_3 - SEq_2SEq_4;
	pitch = (asinf(2 * ftemp1) * 10000.0F);

	/* roll: atan2f(2(q0 * q1 + q2 * q3, 1 - 2(q1 * q1 + q2 * q2)) */
	ftemp1 = (SEq_1SEq_2 + SEq_3SEq_4) * 2.0F;
	ftemp2 = 1.0F - (SEq_2SEq_2 + SEq_3SEq_3) * 2.0F;
	roll = (atan2f(ftemp1, ftemp2) * 10000.0F);

	get_data_w();
	if (filter_converge != 0) {
		filter_converge--;
		if (filter_converge == 0) {
			beta = beta_converged;
		}
	}
}
