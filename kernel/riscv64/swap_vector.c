/***************************************************************************
Copyright (c) 2020, The OpenBLAS Project
All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in
the documentation and/or other materials provided with the
distribution.
3. Neither the name of the OpenBLAS project nor the names of
its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE OPENBLAS PROJECT OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*****************************************************************************/

#include "common.h"
#include <stdio.h>

#define LMUL m8
#if defined(DOUBLE)
#        define ELEN 64
#        define MLEN 8
#else
#        define ELEN 32
#        define MLEN 4
#endif

#define _
#define JOIN2_X(x, y) x ## y
#define JOIN2(x, y) JOIN2_X(x, y)
#define JOIN(v, w, x, y, z) JOIN2( JOIN2( JOIN2( JOIN2( v, w ), x), y), z)

#define VSETVL          JOIN(vsetvl,    _e,     ELEN,   LMUL,   _)
#define FLOAT_V_T       JOIN(vfloat,    ELEN,   LMUL,   _t,     _)
#define VLEV_FLOAT      JOIN(vle,       ELEN,   _v_f,   ELEN,   LMUL)
#define VLSEV_FLOAT     JOIN(vlse,      ELEN,   _v_f,   ELEN,   LMUL)
#define VSEV_FLOAT      JOIN(vse,       ELEN,   _v_f,   ELEN,   LMUL)
#define VSSEV_FLOAT     JOIN(vsse,      ELEN,   _v_f,   ELEN,   LMUL)

int CNAME(BLASLONG n, BLASLONG dummy0, BLASLONG dummy1, FLOAT dummy3, FLOAT *x, BLASLONG inc_x, FLOAT *y, BLASLONG inc_y, FLOAT *dummy, BLASLONG dummy2)
{
	BLASLONG i = 0, j = 0;
	BLASLONG ix = 0,iy = 0;
        BLASLONG stride_x, stride_y;
        FLOAT_V_T vx0, vx1, vy0, vy1;

	if (n < 0)  return(0);

        unsigned int gvl = VSETVL((inc_x != 0 && inc_y != 0) ? n : 1);
        if( inc_x == 0 && inc_y == 0 ) { n = n & 1; }

        if(inc_x == 1 && inc_y == 1){
                if(gvl <= n/2){
                        for(i=0,j=0; i<n/(2*gvl); i++){
                                vx0 = VLEV_FLOAT(&x[j], gvl);
                                vy0 = VLEV_FLOAT(&y[j], gvl);
                                VSEV_FLOAT(&x[j], vy0, gvl);
                                VSEV_FLOAT(&y[j], vx0, gvl);

                                vx1 = VLEV_FLOAT(&x[j+gvl], gvl);
                                vy1 = VLEV_FLOAT(&y[j+gvl], gvl);
                                VSEV_FLOAT(&x[j+gvl], vy1, gvl);
                                VSEV_FLOAT(&y[j+gvl], vx1, gvl);
                                j+=gvl * 2;
                        }
                }
                for(;j<n;){
                        gvl = VSETVL(n-j);
                        vx0 = VLEV_FLOAT(&x[j], gvl);
                        vy0 = VLEV_FLOAT(&y[j], gvl);
                        VSEV_FLOAT(&x[j], vy0, gvl);
                        VSEV_FLOAT(&y[j], vx0, gvl);
                        j+=gvl;
                }
        }else if (inc_y == 1){
                stride_x = inc_x * sizeof(FLOAT);
                if(gvl <= n/2){
                        BLASLONG inc_xv = inc_x * gvl;
                        for(i=0,j=0; i<n/(2*gvl); i++){
                                vx0 = VLSEV_FLOAT(&x[ix], stride_x, gvl);
                                vy0 = VLEV_FLOAT(&y[j], gvl);
                                VSSEV_FLOAT(&x[ix], stride_x, vy0, gvl);
                                VSEV_FLOAT(&y[j], vx0, gvl);

                                vx1 = VLSEV_FLOAT(&x[ix+inc_xv], stride_x, gvl);
                                vy1 = VLEV_FLOAT(&y[j+gvl], gvl);
                                VSSEV_FLOAT(&x[ix+inc_xv], stride_x, vy1, gvl);
                                VSEV_FLOAT(&y[j+gvl], vx1, gvl);
                                j += gvl * 2;
                                ix += inc_xv * 2;
                        }
                }
                for(;j<n;){
                        gvl = VSETVL(n-j);
                        vx0 = VLSEV_FLOAT(&x[ix], stride_x, gvl);
                        vy0 = VLEV_FLOAT(&y[j], gvl);
                        VSSEV_FLOAT(&x[ix], stride_x, vy0, gvl);
                        VSEV_FLOAT(&y[j], vx0, gvl);
                        j += gvl;
                        ix += inc_x * gvl;
                }
        }else if(inc_x == 1){
                stride_y = inc_y * sizeof(FLOAT);
                if(gvl <= n/2){
                        BLASLONG inc_yv = inc_y * gvl;
                        for(i=0,j=0; i<n/(2*gvl); i++){
                                vx0 = VLEV_FLOAT(&x[j], gvl);
                                vy0 = VLSEV_FLOAT(&y[iy], stride_y, gvl);
                                VSEV_FLOAT(&x[j], vy0, gvl);
                                VSSEV_FLOAT(&y[iy], stride_y, vx0, gvl);

                                vx1 = VLEV_FLOAT(&x[j+gvl], gvl);
                                vy1 = VLSEV_FLOAT(&y[iy+inc_yv], stride_y, gvl);
                                VSEV_FLOAT(&x[j+gvl], vy1, gvl);
                                VSSEV_FLOAT(&y[iy+inc_yv], stride_y, vx1, gvl);
                                j += gvl * 2;
                                iy += inc_yv * 2;
                        }
                }
                for(;j<n;){
                        gvl = VSETVL(n-j);
                        vx0 = VLEV_FLOAT(&x[j], gvl);
                        vy0 = VLSEV_FLOAT(&y[iy], stride_y, gvl);
                        VSEV_FLOAT(&x[j], vy0, gvl);
                        VSSEV_FLOAT(&y[iy], stride_y, vx0, gvl);
                        j += gvl;
                        iy += inc_y * gvl;
                }
        }else{
                stride_x = inc_x * sizeof(FLOAT);
                stride_y = inc_y * sizeof(FLOAT);
                if(gvl <= n/2){
                        BLASLONG inc_xv = inc_x * gvl;
                        BLASLONG inc_yv = inc_y * gvl;
                        for(i=0,j=0; i<n/(2*gvl); i++){
                                vx0 = VLSEV_FLOAT(&x[ix], stride_x, gvl);
                                vy0 = VLSEV_FLOAT(&y[iy], stride_y, gvl);
                                VSSEV_FLOAT(&x[ix], stride_x, vy0, gvl);
                                VSSEV_FLOAT(&y[iy], stride_y, vx0, gvl);

                                vx1 = VLSEV_FLOAT(&x[ix+inc_xv], stride_x, gvl);
                                vy1 = VLSEV_FLOAT(&y[iy+inc_yv], stride_y, gvl);
                                VSSEV_FLOAT(&x[ix+inc_xv], stride_x, vy1, gvl);
                                VSSEV_FLOAT(&y[iy+inc_yv], stride_y, vx1, gvl);
                                j += gvl * 2;
                                ix += inc_xv * 2;
                                iy += inc_yv * 2;
                        }
                }
                for(;j<n;){
                        gvl = VSETVL(n-j);
                        vx0 = VLSEV_FLOAT(&x[ix], stride_x, gvl);
                        vy0 = VLSEV_FLOAT(&y[iy], stride_y, gvl);
                        VSSEV_FLOAT(&x[ix], stride_x, vy0, gvl);
                        VSSEV_FLOAT(&y[iy], stride_y, vx0, gvl);
                        j += gvl;
                        ix += inc_x * gvl;
                        iy += inc_y * gvl;
                }
        }
	return(0);
}


