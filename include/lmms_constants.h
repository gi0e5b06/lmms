/*
 * lmms_constants.h - defines system constants
 *
 * Copyright (c) 2006 Danny McRae <khjklujn/at/users.sourceforge.net>
 *
 * This file is part of LMMS - https://lmms.io
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 */

#ifndef LMMS_CONSTANTS_H
#define LMMS_CONSTANTS_H

#include "lmms_basics.h"

// bc: scale=70; e(1); quit

const long double LD_PI
        = 3.1415926535897932384626433832795028841971693993751058209749445923078164;
const long double LD_2PI
        = 6.2831853071795864769252867665590057683943387987502116419498891846156328;
const long double LD_PI_2
        = 1.5707963267948966192313216916397514420985846996875529104874722961539082;
const long double LD_PI_R
        = 0.3183098861837906715377675267450287240689192914809128974953346881177935;
const long double LD_PI_SQR
        = 9.8696044010893586188344909998761511353136994072407906264133493762200447;
const long double LD_E
        = 2.7182818284590452353602874713526624977572470936999595749669676277240766;
const long double LD_E_R
        = 0.3678794411714423215955237701614608674458111310317678345078368016974614;
const long double LD_2_SQRT
        = 1.4142135623730950488016887242096980785696718753769480731766797379907324;
const long double LD_2_SQRT_R
        = 0.7071067811865475244008443621048490392848359376884740365883398689953662;
const long double LD_2_LOG
        = 0.6931471805599453094172321214581765680755001343602552541206800094933936;

const double D_PI       = (double)LD_PI;
const double D_2PI      = (double)LD_2PI;
const double D_PI_2     = (double)LD_PI_2;
const double D_PI_R     = (double)LD_PI_R;
const double D_PI_SQR   = (double)LD_PI_SQR;
const double D_E        = (double)LD_E;
const double D_E_R      = (double)LD_E_R;
const double D_2_SQRT   = (double)LD_2_SQRT;
const double D_2_SQRT_R = (double)LD_2_SQRT_R;
const double D_2_LOG    = (double)LD_2_LOG;

const float F_PI       = (float)LD_PI;
const float F_2PI      = (float)LD_2PI;
const float F_PI_2     = (float)LD_PI_2;
const float F_PI_R     = (float)LD_PI_R;
const float F_PI_SQR   = (float)LD_PI_SQR;
const float F_E        = (float)LD_E;
const float F_E_R      = (float)LD_E_R;
const float F_2_SQRT   = (float)LD_2_SQRT;
const float F_2_SQRT_R = (float)LD_2_SQRT_R;
const float F_2_LOG    = (float)LD_2_LOG;

const real_t R_PI       = (real_t)LD_PI;
const real_t R_2PI      = (real_t)LD_2PI;
const real_t R_PI_2     = (real_t)LD_PI_2;
const real_t R_PI_R     = (real_t)LD_PI_R;
const real_t R_PI_SQR   = (real_t)LD_PI_SQR;
const real_t R_E        = (real_t)LD_E;
const real_t R_E_R      = (real_t)LD_E_R;
const real_t R_2_SQRT   = (real_t)LD_2_SQRT;
const real_t R_2_SQRT_R = (real_t)LD_2_SQRT_R;
const real_t R_2_LOG    = (real_t)LD_2_LOG;

#endif
