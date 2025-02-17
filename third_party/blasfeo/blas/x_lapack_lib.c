/**************************************************************************************************
*                                                                                                 *
* This file is part of BLASFEO.                                                                   *
*                                                                                                 *
* BLASFEO -- BLAS For Embedded Optimization.                                                      *
* Copyright (C) 2016-2017 by Gianluca Frison.                                                     *
* Developed at IMTEK (University of Freiburg) under the supervision of Moritz Diehl.              *
* All rights reserved.                                                                            *
*                                                                                                 *
* HPMPC is free software; you can redistribute it and/or                                          *
* modify it under the terms of the GNU Lesser General Public                                      *
* License as published by the Free Software Foundation; either                                    *
* version 2.1 of the License, or (at your option) any later version.                              *
*                                                                                                 *
* HPMPC is distributed in the hope that it will be useful,                                        *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                                  *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                                            *
* See the GNU Lesser General Public License for more details.                                     *
*                                                                                                 *
* You should have received a copy of the GNU Lesser General Public                                *
* License along with HPMPC; if not, write to the Free Software                                    *
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA                  *
*                                                                                                 *
* Author: Gianluca Frison, giaf (at) dtu.dk                                                       *
*                          gianluca.frison (at) imtek.uni-freiburg.de                             *
*                                                                                                 *
**************************************************************************************************/



#if defined(LA_REFERENCE)



// dpotrf
void POTRF_L_LIBSTR(int m, struct STRMAT *sC, int ci, int cj, struct STRMAT *sD, int di, int dj)
	{
	if(m<=0)
		return;
	int ii, jj, kk;
	REAL
		f_00_inv, 
		f_10, f_11_inv,
		c_00, c_01,
		c_10, c_11;
	int ldc = sC->m;
	int ldd = sD->m;
	REAL *pC = sC->pA + ci + cj*ldc;
	REAL *pD = sD->pA + di + dj*ldd;
	REAL *dD = sD->dA;
	if(di==0 & dj==0)
		sD->use_dA = 1;
	else
		sD->use_dA = 0;
	jj = 0;
	for(; jj<m-1; jj+=2)
		{
		// factorize diagonal
		c_00 = pC[jj+0+ldc*(jj+0)];;
		c_10 = pC[jj+1+ldc*(jj+0)];;
		c_11 = pC[jj+1+ldc*(jj+1)];;
		for(kk=0; kk<jj; kk++)
			{
			c_00 -= pD[jj+0+ldd*kk] * pD[jj+0+ldd*kk];
			c_10 -= pD[jj+1+ldd*kk] * pD[jj+0+ldd*kk];
			c_11 -= pD[jj+1+ldd*kk] * pD[jj+1+ldd*kk];
			}
		if(c_00>0)
			{
			f_00_inv = 1.0/sqrt(c_00);
			}
		else
			{
			f_00_inv = 0.0;
			}
		dD[jj+0] = f_00_inv;
		pD[jj+0+ldd*(jj+0)] = c_00 * f_00_inv;
		f_10 = c_10 * f_00_inv;
		pD[jj+1+ldd*(jj+0)] = f_10;
		c_11 -= f_10 * f_10;
		if(c_11>0)
			{
			f_11_inv = 1.0/sqrt(c_11);
			}
		else
			{
			f_11_inv = 0.0;
			}
		dD[jj+1] = f_11_inv;
		pD[jj+1+ldd*(jj+1)] = c_11 * f_11_inv;
		// solve lower
		ii = jj+2;
		for(; ii<m-1; ii+=2)
			{
			c_00 = pC[ii+0+ldc*(jj+0)];
			c_10 = pC[ii+1+ldc*(jj+0)];
			c_01 = pC[ii+0+ldc*(jj+1)];
			c_11 = pC[ii+1+ldc*(jj+1)];
			for(kk=0; kk<jj; kk++)
				{
				c_00 -= pD[ii+0+ldd*kk] * pD[jj+0+ldd*kk];
				c_10 -= pD[ii+1+ldd*kk] * pD[jj+0+ldd*kk];
				c_01 -= pD[ii+0+ldd*kk] * pD[jj+1+ldd*kk];
				c_11 -= pD[ii+1+ldd*kk] * pD[jj+1+ldd*kk];
				}
			c_00 *= f_00_inv;
			c_10 *= f_00_inv;
			pD[ii+0+ldd*(jj+0)] = c_00;
			pD[ii+1+ldd*(jj+0)] = c_10;
			c_01 -= c_00 * f_10;
			c_11 -= c_10 * f_10;
			pD[ii+0+ldd*(jj+1)] = c_01 * f_11_inv;
			pD[ii+1+ldd*(jj+1)] = c_11 * f_11_inv;
			}
		for(; ii<m; ii++)
			{
			c_00 = pC[ii+0+ldc*(jj+0)];
			c_01 = pC[ii+0+ldc*(jj+1)];
			for(kk=0; kk<jj; kk++)
				{
				c_00 -= pD[ii+0+ldd*kk] * pD[jj+0+ldd*kk];
				c_01 -= pD[ii+0+ldd*kk] * pD[jj+1+ldd*kk];
				}
			c_00 *= f_00_inv;
			pD[ii+0+ldd*(jj+0)] = c_00;
			c_01 -= c_00 * f_10;
			pD[ii+0+ldd*(jj+1)] = c_01 * f_11_inv;
			}
		}
	for(; jj<m; jj++)
		{
		// factorize diagonal
		c_00 = pC[jj+ldc*jj];;
		for(kk=0; kk<jj; kk++)
			{
			c_00 -= pD[jj+ldd*kk] * pD[jj+ldd*kk];
			}
		if(c_00>0)
			{
			f_00_inv = 1.0/sqrt(c_00);
			}
		else
			{
			f_00_inv = 0.0;
			}
		dD[jj] = f_00_inv;
		pD[jj+ldd*jj] = c_00 * f_00_inv;
		// solve lower
//		for(ii=jj+1; ii<m; ii++)
//			{
//			c_00 = pC[ii+ldc*jj];
//			for(kk=0; kk<jj; kk++)
//				{
//				c_00 -= pD[ii+ldd*kk] * pD[jj+ldd*kk];
//				}
//			pD[ii+ldd*jj] = c_00 * f_00_inv;
//			}
		}
	return;
	}



// dpotrf
void POTRF_L_MN_LIBSTR(int m, int n, struct STRMAT *sC, int ci, int cj, struct STRMAT *sD, int di, int dj)
	{
	if(m<=0 | n<=0)
		return;
	int ii, jj, kk;
	REAL
		f_00_inv, 
		f_10, f_11_inv,
		c_00, c_01,
		c_10, c_11;
	int ldc = sC->m;
	int ldd = sD->m;
	REAL *pC = sC->pA + ci + cj*ldc;
	REAL *pD = sD->pA + di + dj*ldd;
	REAL *dD = sD->dA;
	if(di==0 & dj==0)
		sD->use_dA = 1;
	else
		sD->use_dA = 0;
	jj = 0;
	for(; jj<n-1; jj+=2)
		{
		// factorize diagonal
		c_00 = pC[jj+0+ldc*(jj+0)];;
		c_10 = pC[jj+1+ldc*(jj+0)];;
		c_11 = pC[jj+1+ldc*(jj+1)];;
		for(kk=0; kk<jj; kk++)
			{
			c_00 -= pD[jj+0+ldd*kk] * pD[jj+0+ldd*kk];
			c_10 -= pD[jj+1+ldd*kk] * pD[jj+0+ldd*kk];
			c_11 -= pD[jj+1+ldd*kk] * pD[jj+1+ldd*kk];
			}
		if(c_00>0)
			{
			f_00_inv = 1.0/sqrt(c_00);
			}
		else
			{
			f_00_inv = 0.0;
			}
		dD[jj+0] = f_00_inv;
		pD[jj+0+ldd*(jj+0)] = c_00 * f_00_inv;
		f_10 = c_10 * f_00_inv;
		pD[jj+1+ldd*(jj+0)] = f_10;
		c_11 -= f_10 * f_10;
		if(c_11>0)
			{
			f_11_inv = 1.0/sqrt(c_11);
			}
		else
			{
			f_11_inv = 0.0;
			}
		dD[jj+1] = f_11_inv;
		pD[jj+1+ldd*(jj+1)] = c_11 * f_11_inv;
		// solve lower
		ii = jj+2;
		for(; ii<m-1; ii+=2)
			{
			c_00 = pC[ii+0+ldc*(jj+0)];
			c_10 = pC[ii+1+ldc*(jj+0)];
			c_01 = pC[ii+0+ldc*(jj+1)];
			c_11 = pC[ii+1+ldc*(jj+1)];
			for(kk=0; kk<jj; kk++)
				{
				c_00 -= pD[ii+0+ldd*kk] * pD[jj+0+ldd*kk];
				c_10 -= pD[ii+1+ldd*kk] * pD[jj+0+ldd*kk];
				c_01 -= pD[ii+0+ldd*kk] * pD[jj+1+ldd*kk];
				c_11 -= pD[ii+1+ldd*kk] * pD[jj+1+ldd*kk];
				}
			c_00 *= f_00_inv;
			c_10 *= f_00_inv;
			pD[ii+0+ldd*(jj+0)] = c_00;
			pD[ii+1+ldd*(jj+0)] = c_10;
			c_01 -= c_00 * f_10;
			c_11 -= c_10 * f_10;
			pD[ii+0+ldd*(jj+1)] = c_01 * f_11_inv;
			pD[ii+1+ldd*(jj+1)] = c_11 * f_11_inv;
			}
		for(; ii<m; ii++)
			{
			c_00 = pC[ii+0+ldc*(jj+0)];
			c_01 = pC[ii+0+ldc*(jj+1)];
			for(kk=0; kk<jj; kk++)
				{
				c_00 -= pD[ii+0+ldd*kk] * pD[jj+0+ldd*kk];
				c_01 -= pD[ii+0+ldd*kk] * pD[jj+1+ldd*kk];
				}
			c_00 *= f_00_inv;
			pD[ii+0+ldd*(jj+0)] = c_00;
			c_01 -= c_00 * f_10;
			pD[ii+0+ldd*(jj+1)] = c_01 * f_11_inv;
			}
		}
	for(; jj<n; jj++)
		{
		// factorize diagonal
		c_00 = pC[jj+ldc*jj];;
		for(kk=0; kk<jj; kk++)
			{
			c_00 -= pD[jj+ldd*kk] * pD[jj+ldd*kk];
			}
		if(c_00>0)
			{
			f_00_inv = 1.0/sqrt(c_00);
			}
		else
			{
			f_00_inv = 0.0;
			}
		dD[jj] = f_00_inv;
		pD[jj+ldd*jj] = c_00 * f_00_inv;
		// solve lower
		for(ii=jj+1; ii<m; ii++)
			{
			c_00 = pC[ii+ldc*jj];
			for(kk=0; kk<jj; kk++)
				{
				c_00 -= pD[ii+ldd*kk] * pD[jj+ldd*kk];
				}
			pD[ii+ldd*jj] = c_00 * f_00_inv;
			}
		}
	return;
	}



// dsyrk dpotrf
void SYRK_POTRF_LN_LIBSTR(int m, int n, int k, struct STRMAT *sA, int ai, int aj, struct STRMAT *sB, int bi, int bj, struct STRMAT *sC, int ci, int cj, struct STRMAT *sD, int di, int dj)
	{
	int ii, jj, kk;
	REAL
		f_00_inv, 
		f_10, f_11_inv,
		c_00, c_01,
		c_10, c_11;
	int lda = sA->m;
	int ldb = sB->m;
	int ldc = sC->m;
	int ldd = sD->m;
	REAL *pA = sA->pA + ai + aj*lda;
	REAL *pB = sB->pA + bi + bj*ldb;
	REAL *pC = sC->pA + ci + cj*ldc;
	REAL *pD = sD->pA + di + dj*ldd;
	REAL *dD = sD->dA;
	if(di==0 & dj==0)
		sD->use_dA = 1;
	else
		sD->use_dA = 0;
	jj = 0;
	for(; jj<n-1; jj+=2)
		{
		// factorize diagonal
		c_00 = pC[jj+0+ldc*(jj+0)];;
		c_10 = pC[jj+1+ldc*(jj+0)];;
		c_11 = pC[jj+1+ldc*(jj+1)];;
		for(kk=0; kk<k; kk++)
			{
			c_00 += pA[jj+0+lda*kk] * pB[jj+0+ldb*kk];
			c_10 += pA[jj+1+lda*kk] * pB[jj+0+ldb*kk];
			c_11 += pA[jj+1+lda*kk] * pB[jj+1+ldb*kk];
			}
		for(kk=0; kk<jj; kk++)
			{
			c_00 -= pD[jj+0+ldd*kk] * pD[jj+0+ldd*kk];
			c_10 -= pD[jj+1+ldd*kk] * pD[jj+0+ldd*kk];
			c_11 -= pD[jj+1+ldd*kk] * pD[jj+1+ldd*kk];
			}
		if(c_00>0)
			{
			f_00_inv = 1.0/sqrt(c_00);
			}
		else
			{
			f_00_inv = 0.0;
			}
		dD[jj+0] = f_00_inv;
		pD[jj+0+ldd*(jj+0)] = c_00 * f_00_inv;
		f_10 = c_10 * f_00_inv;
		pD[jj+1+ldd*(jj+0)] = f_10;
		c_11 -= f_10 * f_10;
		if(c_11>0)
			{
			f_11_inv = 1.0/sqrt(c_11);
			}
		else
			{
			f_11_inv = 0.0;
			}
		dD[jj+1] = f_11_inv;
		pD[jj+1+ldd*(jj+1)] = c_11 * f_11_inv;
		// solve lower
		ii = jj+2;
		for(; ii<m-1; ii+=2)
			{
			c_00 = pC[ii+0+ldc*(jj+0)];
			c_10 = pC[ii+1+ldc*(jj+0)];
			c_01 = pC[ii+0+ldc*(jj+1)];
			c_11 = pC[ii+1+ldc*(jj+1)];
			for(kk=0; kk<k; kk++)
				{
				c_00 += pA[ii+0+lda*kk] * pB[jj+0+ldb*kk];
				c_10 += pA[ii+1+lda*kk] * pB[jj+0+ldb*kk];
				c_01 += pA[ii+0+lda*kk] * pB[jj+1+ldb*kk];
				c_11 += pA[ii+1+lda*kk] * pB[jj+1+ldb*kk];
				}
			for(kk=0; kk<jj; kk++)
				{
				c_00 -= pD[ii+0+ldd*kk] * pD[jj+0+ldd*kk];
				c_10 -= pD[ii+1+ldd*kk] * pD[jj+0+ldd*kk];
				c_01 -= pD[ii+0+ldd*kk] * pD[jj+1+ldd*kk];
				c_11 -= pD[ii+1+ldd*kk] * pD[jj+1+ldd*kk];
				}
			c_00 *= f_00_inv;
			c_10 *= f_00_inv;
			pD[ii+0+ldd*(jj+0)] = c_00;
			pD[ii+1+ldd*(jj+0)] = c_10;
			c_01 -= c_00 * f_10;
			c_11 -= c_10 * f_10;
			pD[ii+0+ldd*(jj+1)] = c_01 * f_11_inv;
			pD[ii+1+ldd*(jj+1)] = c_11 * f_11_inv;
			}
		for(; ii<m; ii++)
			{
			c_00 = pC[ii+0+ldc*(jj+0)];
			c_01 = pC[ii+0+ldc*(jj+1)];
			for(kk=0; kk<k; kk++)
				{
				c_00 += pA[ii+0+lda*kk] * pB[jj+0+ldb*kk];
				c_01 += pA[ii+0+lda*kk] * pB[jj+1+ldb*kk];
				}
			for(kk=0; kk<jj; kk++)
				{
				c_00 -= pD[ii+0+ldd*kk] * pD[jj+0+ldd*kk];
				c_01 -= pD[ii+0+ldd*kk] * pD[jj+1+ldd*kk];
				}
			c_00 *= f_00_inv;
			pD[ii+0+ldd*(jj+0)] = c_00;
			c_01 -= c_00 * f_10;
			pD[ii+0+ldd*(jj+1)] = c_01 * f_11_inv;
			}
		}
	for(; jj<n; jj++)
		{
		// factorize diagonal
		c_00 = pC[jj+ldc*jj];;
		for(kk=0; kk<k; kk++)
			{
			c_00 += pA[jj+lda*kk] * pB[jj+ldb*kk];
			}
		for(kk=0; kk<jj; kk++)
			{
			c_00 -= pD[jj+ldd*kk] * pD[jj+ldd*kk];
			}
		if(c_00>0)
			{
			f_00_inv = 1.0/sqrt(c_00);
			}
		else
			{
			f_00_inv = 0.0;
			}
		dD[jj] = f_00_inv;
		pD[jj+ldd*jj] = c_00 * f_00_inv;
		// solve lower
		for(ii=jj+1; ii<m; ii++)
			{
			c_00 = pC[ii+ldc*jj];
			for(kk=0; kk<k; kk++)
				{
				c_00 += pA[ii+lda*kk] * pB[jj+ldb*kk];
				}
			for(kk=0; kk<jj; kk++)
				{
				c_00 -= pD[ii+ldd*kk] * pD[jj+ldd*kk];
				}
			pD[ii+ldd*jj] = c_00 * f_00_inv;
			}
		}
	return;
	}



// dgetrf without pivoting
void GETF2_NOPIVOT(int m, int n, REAL *A, int lda, REAL *dA)
	{
	int ii, jj, kk, itmp0, itmp1;
	int iimax = m<n ? m : n;
	int i1 = 1;
	REAL dtmp;
	REAL dm1 = -1.0;

	for(ii=0; ii<iimax; ii++)
		{
		itmp0 = m-ii-1;
		dtmp = 1.0/A[ii+lda*ii];
		dA[ii] = dtmp;
		for(jj=0; jj<itmp0; jj++)
			{
			A[ii+1+jj+lda*ii] *= dtmp;
			}
		itmp1 = n-ii-1;
		for(jj=0; jj<itmp1; jj++)
			{
			for(kk=0; kk<itmp0; kk++)
				{
				A[(ii+1+kk)+lda*(ii+1+jj)] -= A[(ii+1+kk)+lda*ii] * A[ii+lda*(ii+1+jj)];
				}
			}
		}
	return;
	}



// dgetrf without pivoting
void GETRF_NOPIVOT_LIBSTR(int m, int n, struct STRMAT *sC, int ci, int cj, struct STRMAT *sD, int di, int dj)
	{
	int ii, jj, kk;
//	int i1 = 1;
//	REAL d1 = 1.0;
	REAL
		d_00_inv, d_11_inv,
		d_00, d_01,
		d_10, d_11;
	int ldc = sC->m;
	int ldd = sD->m;
	REAL *pC = sC->pA + ci + cj*ldc;
	REAL *pD = sD->pA + di + dj*ldd;
	REAL *dD = sD->dA;
	if(di==0 & dj==0)
		sD->use_dA = 1;
	else
		sD->use_dA = 0;
#if 1
	jj = 0;
	for(; jj<n-1; jj+=2)
		{
		// upper
		ii = 0;
		for(; ii<jj-1; ii+=2)
			{
			// correct upper
			d_00 = pC[(ii+0)+ldc*(jj+0)];
			d_10 = pC[(ii+1)+ldc*(jj+0)];
			d_01 = pC[(ii+0)+ldc*(jj+1)];
			d_11 = pC[(ii+1)+ldc*(jj+1)];
			for(kk=0; kk<ii; kk++)
				{
				d_00 -= pD[(ii+0)+ldd*kk] * pD[kk+ldd*(jj+0)];
				d_10 -= pD[(ii+1)+ldd*kk] * pD[kk+ldd*(jj+0)];
				d_01 -= pD[(ii+0)+ldd*kk] * pD[kk+ldd*(jj+1)];
				d_11 -= pD[(ii+1)+ldd*kk] * pD[kk+ldd*(jj+1)];
				}
			// solve upper
			d_10 -= pD[(ii+1)+ldd*kk] * d_00;
			d_11 -= pD[(ii+1)+ldd*kk] * d_01;
			pD[(ii+0)+ldd*(jj+0)] = d_00;
			pD[(ii+1)+ldd*(jj+0)] = d_10;
			pD[(ii+0)+ldd*(jj+1)] = d_01;
			pD[(ii+1)+ldd*(jj+1)] = d_11;
			}
		for(; ii<jj; ii++)
			{
			// correct upper
			d_00 = pC[(ii+0)+ldc*(jj+0)];
			d_01 = pC[(ii+0)+ldc*(jj+1)];
			for(kk=0; kk<ii; kk++)
				{
				d_00 -= pD[(ii+0)+ldd*kk] * pD[kk+ldd*(jj+0)];
				d_01 -= pD[(ii+0)+ldd*kk] * pD[kk+ldd*(jj+1)];
				}
			// solve upper
			pD[(ii+0)+ldd*(jj+0)] = d_00;
			pD[(ii+0)+ldd*(jj+1)] = d_01;
			}
		// diagonal
		ii = jj;
		if(ii<m-1)
			{
			// correct diagonal
			d_00 = pC[(ii+0)+ldc*(jj+0)];
			d_10 = pC[(ii+1)+ldc*(jj+0)];
			d_01 = pC[(ii+0)+ldc*(jj+1)];
			d_11 = pC[(ii+1)+ldc*(jj+1)];
			for(kk=0; kk<ii; kk++)
				{
				d_00 -= pD[(ii+0)+ldd*kk] * pD[kk+ldd*(jj+0)];
				d_10 -= pD[(ii+1)+ldd*kk] * pD[kk+ldd*(jj+0)];
				d_01 -= pD[(ii+0)+ldd*kk] * pD[kk+ldd*(jj+1)];
				d_11 -= pD[(ii+1)+ldd*kk] * pD[kk+ldd*(jj+1)];
				}
			// factorize diagonal
			d_00_inv = 1.0/d_00;
			d_10 *= d_00_inv;
			d_11 -= d_10 * d_01;
			d_11_inv = 1.0/d_11;
			pD[(ii+0)+ldd*(jj+0)] = d_00;
			pD[(ii+1)+ldd*(jj+0)] = d_10;
			pD[(ii+0)+ldd*(jj+1)] = d_01;
			pD[(ii+1)+ldd*(jj+1)] = d_11;
			dD[ii+0] = d_00_inv;
			dD[ii+1] = d_11_inv;
			ii += 2;
			}
		else if(ii<m)
			{
			// correct diagonal
			d_00 = pC[(ii+0)+ldc*(jj+0)];
			d_01 = pC[(ii+0)+ldc*(jj+1)];
			for(kk=0; kk<ii; kk++)
				{
				d_00 -= pD[(ii+0)+ldd*kk] * pD[kk+ldd*(jj+0)];
				d_01 -= pD[(ii+0)+ldd*kk] * pD[kk+ldd*(jj+1)];
				}
			// factorize diagonal
			d_00_inv = 1.0/d_00;
			pD[(ii+0)+ldd*(jj+0)] = d_00;
			pD[(ii+0)+ldd*(jj+1)] = d_01;
			dD[ii+0] = d_00_inv;
			ii += 1;
			}
		// lower
		for(; ii<m-1; ii+=2)
			{
			// correct lower
			d_00 = pC[(ii+0)+ldc*(jj+0)];
			d_10 = pC[(ii+1)+ldc*(jj+0)];
			d_01 = pC[(ii+0)+ldc*(jj+1)];
			d_11 = pC[(ii+1)+ldc*(jj+1)];
			for(kk=0; kk<jj; kk++)
				{
				d_00 -= pD[(ii+0)+ldd*kk] * pD[kk+ldd*(jj+0)];
				d_10 -= pD[(ii+1)+ldd*kk] * pD[kk+ldd*(jj+0)];
				d_01 -= pD[(ii+0)+ldd*kk] * pD[kk+ldd*(jj+1)];
				d_11 -= pD[(ii+1)+ldd*kk] * pD[kk+ldd*(jj+1)];
				}
			// solve lower
			d_00 *= d_00_inv;
			d_10 *= d_00_inv;
			d_01 -= d_00 * pD[kk+ldd*(jj+1)];
			d_11 -= d_10 * pD[kk+ldd*(jj+1)];
			d_01 *= d_11_inv;
			d_11 *= d_11_inv;
			pD[(ii+0)+ldd*(jj+0)] = d_00;
			pD[(ii+1)+ldd*(jj+0)] = d_10;
			pD[(ii+0)+ldd*(jj+1)] = d_01;
			pD[(ii+1)+ldd*(jj+1)] = d_11;
			}
		for(; ii<m; ii++)
			{
			// correct lower
			d_00 = pC[(ii+0)+ldc*(jj+0)];
			d_01 = pC[(ii+0)+ldc*(jj+1)];
			for(kk=0; kk<jj; kk++)
				{
				d_00 -= pD[(ii+0)+ldd*kk] * pD[kk+ldd*(jj+0)];
				d_01 -= pD[(ii+0)+ldd*kk] * pD[kk+ldd*(jj+1)];
				}
			// solve lower
			d_00 *= d_00_inv;
			d_01 -= d_00 * pD[kk+ldd*(jj+1)];
			d_01 *= d_11_inv;
			pD[(ii+0)+ldd*(jj+0)] = d_00;
			pD[(ii+0)+ldd*(jj+1)] = d_01;
			}
		}
	for(; jj<n; jj++)
		{
		// upper
		ii = 0;
		for(; ii<jj-1; ii+=2)
			{
			// correct upper
			d_00 = pC[(ii+0)+ldc*jj];
			d_10 = pC[(ii+1)+ldc*jj];
			for(kk=0; kk<ii; kk++)
				{
				d_00 -= pD[(ii+0)+ldd*kk] * pD[kk+ldd*jj];
				d_10 -= pD[(ii+1)+ldd*kk] * pD[kk+ldd*jj];
				}
			// solve upper
			d_10 -= pD[(ii+1)+ldd*kk] * d_00;
			pD[(ii+0)+ldd*jj] = d_00;
			pD[(ii+1)+ldd*jj] = d_10;
			}
		for(; ii<jj; ii++)
			{
			// correct upper
			d_00 = pC[(ii+0)+ldc*jj];
			for(kk=0; kk<ii; kk++)
				{
				d_00 -= pD[(ii+0)+ldd*kk] * pD[kk+ldd*jj];
				}
			// solve upper
			pD[(ii+0)+ldd*jj] = d_00;
			}
		// diagonal
		ii = jj;
		if(ii<m-1)
			{
			// correct diagonal
			d_00 = pC[(ii+0)+ldc*jj];
			d_10 = pC[(ii+1)+ldc*jj];
			for(kk=0; kk<ii; kk++)
				{
				d_00 -= pD[(ii+0)+ldd*kk] * pD[kk+ldd*jj];
				d_10 -= pD[(ii+1)+ldd*kk] * pD[kk+ldd*jj];
				}
			// factorize diagonal
			d_00_inv = 1.0/d_00;
			d_10 *= d_00_inv;
			pD[(ii+0)+ldd*jj] = d_00;
			pD[(ii+1)+ldd*jj] = d_10;
			dD[ii+0] = d_00_inv;
			ii += 2;
			}
		else if(ii<m)
			{
			// correct diagonal
			d_00 = pC[(ii+0)+ldc*jj];
			for(kk=0; kk<ii; kk++)
				{
				d_00 -= pD[(ii+0)+ldd*kk] * pD[kk+ldd*jj];
				}
			// factorize diagonal
			d_00_inv = 1.0/d_00;
			pD[(ii+0)+ldd*jj] = d_00;
			dD[ii+0] = d_00_inv;
			ii += 1;
			}
		// lower
		for(; ii<m-1; ii+=2)
			{
			// correct lower
			d_00 = pC[(ii+0)+ldc*jj];
			d_10 = pC[(ii+1)+ldc*jj];
			for(kk=0; kk<jj; kk++)
				{
				d_00 -= pD[(ii+0)+ldd*kk] * pD[kk+ldd*jj];
				d_10 -= pD[(ii+1)+ldd*kk] * pD[kk+ldd*jj];
				}
			// solve lower
			d_00 *= d_00_inv;
			d_10 *= d_00_inv;
			pD[(ii+0)+ldd*jj] = d_00;
			pD[(ii+1)+ldd*jj] = d_10;
			}
		for(; ii<m; ii++)
			{
			// correct lower
			d_00 = pC[(ii+0)+ldc*jj];
			for(kk=0; kk<jj; kk++)
				{
				d_00 -= pD[(ii+0)+ldd*kk] * pD[kk+ldd*jj];
				}
			// solve lower
			d_00 *= d_00_inv;
			pD[(ii+0)+ldd*jj] = d_00;
			}
		}
#else
	if(pC!=pD)
		{
		for(jj=0; jj<n; jj++)
			{
			for(ii=0; ii<m; ii++)
				{
				pD[ii+ldd*jj] = pC[ii+ldc*jj];
				}
			}
		}
	GETF2_NOPIVOT(m, n, pD, ldd, dD);
#endif
	return;
	}



// dgetrf pivoting
void GETRF_LIBSTR(int m, int n, struct STRMAT *sC, int ci, int cj, struct STRMAT *sD, int di, int dj, int *ipiv)
	{
	int ii, i0, jj, kk, ip, itmp0, itmp1;
	REAL dtmp, dmax;
	REAL
		d_00_inv, d_11_inv,
		d_00, d_01,
		d_10, d_11;
	int i1 = 1;
	REAL d1 = 1.0;
	int ldc = sC->m;
	int ldd = sD->m;
	REAL *pC = sC->pA+ci+cj*ldc;
	REAL *pD = sD->pA+di+dj*ldd;
	REAL *dD = sD->dA;
	if(di==0 & dj==0)
		sD->use_dA = 1;
	else
		sD->use_dA = 0;
	// copy if needed
	if(pC!=pD)
		{
		for(jj=0; jj<n; jj++)
			{
			for(ii=0; ii<m; ii++)
				{
				pD[ii+ldd*jj] = pC[ii+ldc*jj];
				}
			}
		}
	// factorize
#if 1
	jj = 0;
	for(; jj<n-1; jj+=2)
		{
		ii = 0;
		for(; ii<jj-1; ii+=2)
			{
			// correct upper
			d_00 = pD[(ii+0)+ldd*(jj+0)];
			d_10 = pD[(ii+1)+ldd*(jj+0)];
			d_01 = pD[(ii+0)+ldd*(jj+1)];
			d_11 = pD[(ii+1)+ldd*(jj+1)];
			for(kk=0; kk<ii; kk++)
				{
				d_00 -= pD[(ii+0)+ldd*kk] * pD[kk+ldd*(jj+0)];
				d_10 -= pD[(ii+1)+ldd*kk] * pD[kk+ldd*(jj+0)];
				d_01 -= pD[(ii+0)+ldd*kk] * pD[kk+ldd*(jj+1)];
				d_11 -= pD[(ii+1)+ldd*kk] * pD[kk+ldd*(jj+1)];
				}
			// solve upper
			d_10 -= pD[(ii+1)+ldd*kk] * d_00;
			d_11 -= pD[(ii+1)+ldd*kk] * d_01;
			pD[(ii+0)+ldd*(jj+0)] = d_00;
			pD[(ii+1)+ldd*(jj+0)] = d_10;
			pD[(ii+0)+ldd*(jj+1)] = d_01;
			pD[(ii+1)+ldd*(jj+1)] = d_11;
			}
		for(; ii<jj; ii++)
			{
			// correct upper
			d_00 = pD[(ii+0)+ldd*(jj+0)];
			d_01 = pD[(ii+0)+ldd*(jj+1)];
			for(kk=0; kk<ii; kk++)
				{
				d_00 -= pD[(ii+0)+ldd*kk] * pD[kk+ldd*(jj+0)];
				d_01 -= pD[(ii+0)+ldd*kk] * pD[kk+ldd*(jj+1)];
				}
			// solve upper
			pD[(ii+0)+ldd*(jj+0)] = d_00;
			pD[(ii+0)+ldd*(jj+1)] = d_01;
			}
		// correct diagonal and lower and look for pivot
		// correct
		ii = jj;
		i0 = ii;
		for(; ii<m-1; ii+=2)
			{
			d_00 = pD[(ii+0)+ldd*(jj+0)];
			d_10 = pD[(ii+1)+ldd*(jj+0)];
			d_01 = pD[(ii+0)+ldd*(jj+1)];
			d_11 = pD[(ii+1)+ldd*(jj+1)];
			for(kk=0; kk<jj; kk++)
				{
				d_00 -= pD[(ii+0)+ldd*kk] * pD[kk+ldd*(jj+0)];
				d_10 -= pD[(ii+1)+ldd*kk] * pD[kk+ldd*(jj+0)];
				d_01 -= pD[(ii+0)+ldd*kk] * pD[kk+ldd*(jj+1)];
				d_11 -= pD[(ii+1)+ldd*kk] * pD[kk+ldd*(jj+1)];
				}
			pD[(ii+0)+ldd*(jj+0)] = d_00;
			pD[(ii+1)+ldd*(jj+0)] = d_10;
			pD[(ii+0)+ldd*(jj+1)] = d_01;
			pD[(ii+1)+ldd*(jj+1)] = d_11;
			}
		for(; ii<m; ii++)
			{
			d_00 = pD[(ii+0)+ldd*(jj+0)];
			d_01 = pD[(ii+0)+ldd*(jj+1)];
			for(kk=0; kk<jj; kk++)
				{
				d_00 -= pD[(ii+0)+ldd*kk] * pD[kk+ldd*(jj+0)];
				d_01 -= pD[(ii+0)+ldd*kk] * pD[kk+ldd*(jj+1)];
				}
			pD[(ii+0)+ldd*(jj+0)] = d_00;
			pD[(ii+0)+ldd*(jj+1)] = d_01;
			}
		// look for pivot & solve
		// left column
		ii = i0;
		dmax = 0;
		ip = ii;
		for(; ii<m-1; ii+=2)
			{
			d_00 = pD[(ii+0)+ldd*jj];
			d_10 = pD[(ii+1)+ldd*jj];
			dtmp = d_00>0.0 ? d_00 : -d_00;
			if(dtmp>dmax)
				{
				dmax = dtmp;
				ip = ii+0;
				}
			dtmp = d_10>0.0 ? d_10 : -d_10;
			if(dtmp>dmax)
				{
				dmax = dtmp;
				ip = ii+1;
				}
			}
		for(; ii<m; ii++)
			{
			d_00 = pD[(ii+0)+ldd*jj];
			dtmp = d_00>0.0 ? d_00 : -d_00;
			if(dtmp>dmax)
				{
				dmax = dtmp;
				ip = ii+0;
				}
			}
		// row swap
		ii = i0;
		ipiv[ii] = ip;
		if(ip!=ii)
			{
			for(kk=0; kk<n; kk++)
				{
				dtmp = pD[ii+ldd*kk];
				pD[ii+ldd*kk] = pD[ip+ldd*kk];
				pD[ip+ldd*kk] = dtmp;
				}
			}
		// factorize diagonal
		d_00 = pD[(ii+0)+ldd*(jj+0)];
		d_00_inv = 1.0/d_00;
		pD[(ii+0)+ldd*(jj+0)] = d_00;
		dD[ii] = d_00_inv;
		ii += 1;
		// solve & compute next pivot
		dmax = 0;
		ip = ii;
		for(; ii<m-1; ii+=2)
			{
			d_00 = pD[(ii+0)+ldd*(jj+0)];
			d_10 = pD[(ii+1)+ldd*(jj+0)];
			d_00 *= d_00_inv;
			d_10 *= d_00_inv;
			d_01 = pD[(ii+0)+ldd*(jj+1)];
			d_11 = pD[(ii+1)+ldd*(jj+1)];
			d_01 -= d_00 * pD[jj+ldd*(jj+1)];
			d_11 -= d_10 * pD[jj+ldd*(jj+1)];
			dtmp = d_01>0.0 ? d_01 : -d_01;
			if(dtmp>dmax)
				{
				dmax = dtmp;
				ip = ii+0;
				}
			dtmp = d_11>0.0 ? d_11 : -d_11;
			if(dtmp>dmax)
				{
				dmax = dtmp;
				ip = ii+1;
				}
			pD[(ii+0)+ldd*(jj+0)] = d_00;
			pD[(ii+1)+ldd*(jj+0)] = d_10;
			pD[(ii+0)+ldd*(jj+1)] = d_01;
			pD[(ii+1)+ldd*(jj+1)] = d_11;
			}
		for(; ii<m; ii++)
			{
			d_00 = pD[(ii+0)+ldd*(jj+0)];
			d_00 *= d_00_inv;
			d_01 = pD[(ii+0)+ldd*(jj+1)];
			d_01 -= d_00 * pD[jj+ldd*(jj+1)];
			dtmp = d_01>0.0 ? d_01 : -d_01;
			if(dtmp>dmax)
				{
				dmax = dtmp;
				ip = ii+0;
				}
			pD[(ii+0)+ldd*(jj+0)] = d_00;
			pD[(ii+0)+ldd*(jj+1)] = d_01;
			}
		// row swap
		ii = i0+1;
		ipiv[ii] = ip;
		if(ip!=ii)
			{
			for(kk=0; kk<n; kk++)
				{
				dtmp = pD[ii+ldd*kk];
				pD[ii+ldd*kk] = pD[ip+ldd*kk];
				pD[ip+ldd*kk] = dtmp;
				}
			}
		// factorize diagonal
		d_00 = pD[ii+ldd*(jj+1)];
		d_00_inv = 1.0/d_00;
		pD[ii+ldd*(jj+1)] = d_00;
		dD[ii] = d_00_inv;
		ii += 1;
		// solve lower
		for(; ii<m; ii++)
			{
			d_00 = pD[ii+ldd*(jj+1)];
			d_00 *= d_00_inv;
			pD[ii+ldd*(jj+1)] = d_00;
			}
		}
	for(; jj<n; jj++)
		{
		ii = 0;
		for(; ii<jj-1; ii+=2)
			{
			// correct upper
			d_00 = pD[(ii+0)+ldd*jj];
			d_10 = pD[(ii+1)+ldd*jj];
			for(kk=0; kk<ii; kk++)
				{
				d_00 -= pD[(ii+0)+ldd*kk] * pD[kk+ldd*jj];
				d_10 -= pD[(ii+1)+ldd*kk] * pD[kk+ldd*jj];
				}
			// solve upper
			d_10 -= pD[(ii+1)+ldd*kk] * d_00;
			pD[(ii+0)+ldd*jj] = d_00;
			pD[(ii+1)+ldd*jj] = d_10;
			}
		for(; ii<jj; ii++)
			{
			// correct upper
			d_00 = pD[ii+ldd*jj];
			for(kk=0; kk<ii; kk++)
				{
				d_00 -= pD[ii+ldd*kk] * pD[kk+ldd*jj];
				}
			// solve upper
			pD[ii+ldd*jj] = d_00;
			}
		i0 = ii;
		ii = jj;
		// correct diagonal and lower and look for pivot
		dmax = 0;
		ip = ii;
		for(; ii<m-1; ii+=2)
			{
			d_00 = pD[(ii+0)+ldd*jj];
			d_10 = pD[(ii+1)+ldd*jj];
			for(kk=0; kk<jj; kk++)
				{
				d_00 -= pD[(ii+0)+ldd*kk] * pD[kk+ldd*jj];
				d_10 -= pD[(ii+1)+ldd*kk] * pD[kk+ldd*jj];
				}
			dtmp = d_00>0.0 ? d_00 : -d_00;
			if(dtmp>dmax)
				{
				dmax = dtmp;
				ip = ii+0;
				}
			dtmp = d_10>0.0 ? d_10 : -d_10;
			if(dtmp>dmax)
				{
				dmax = dtmp;
				ip = ii+1;
				}
			pD[(ii+0)+ldd*jj] = d_00;
			pD[(ii+1)+ldd*jj] = d_10;
			}
		for(; ii<m; ii++)
			{
			d_00 = pD[(ii+0)+ldd*jj];
			for(kk=0; kk<jj; kk++)
				{
				d_00 -= pD[(ii+0)+ldd*kk] * pD[kk+ldd*jj];
				}
			dtmp = d_00>0.0 ? d_00 : -d_00;
			if(dtmp>dmax)
				{
				dmax = dtmp;
				ip = ii+0;
				}
			pD[(ii+0)+ldd*jj] = d_00;
			}
		// row swap
		ii = i0;
		ipiv[ii] = ip;
		if(ip!=ii)
			{
			for(kk=0; kk<n; kk++)
				{
				dtmp = pD[ii+ldd*kk];
				pD[ii+ldd*kk] = pD[ip+ldd*kk];
				pD[ip+ldd*kk] = dtmp;
				}
			}
		// factorize diagonal
		d_00 = pD[ii+ldd*jj];
		d_00_inv = 1.0/d_00;
		pD[ii+ldd*jj] = d_00;
		dD[ii] = d_00_inv;
		ii += 1;
		for(; ii<m; ii++)
			{
			// correct lower
			d_00 = pD[ii+ldd*jj];
			// solve lower
			d_00 *= d_00_inv;
			pD[ii+ldd*jj] = d_00;
			}
		}
#else
	int iimax = m<n ? m : n;
	for(ii=0; ii<iimax; ii++)
		{
		dmax = (pD[ii+ldd*ii]>0 ? pD[ii+ldd*ii] : -pD[ii+ldd*ii]);
		ip = ii;
		for(jj=1; jj<m-ii; jj++)
			{
			dtmp = pD[ii+jj+ldd*ii]>0 ? pD[ii+jj+ldd*ii] : -pD[ii+jj+ldd*ii];
			if(dtmp>dmax)
				{
				dmax = dtmp;
				ip = ii+jj;
				}
			}
		ipiv[ii] = ip;
		if(ip!=ii)
			{
			for(jj=0; jj<n; jj++)
				{
				dtmp = pD[ii+jj*ldd];
				pD[ii+jj*ldd] = pD[ip+jj*ldd];
				pD[ip+jj*ldd] = dtmp;
				}
			}
		itmp0 = m-ii-1;
		dtmp = 1.0/pD[ii+ldd*ii];
		dD[ii] = dtmp;
		for(jj=0; jj<itmp0; jj++)
			{
			pD[ii+1+jj+ldd*ii] *= dtmp;
			}
		itmp1 = n-ii-1;
		for(jj=0; jj<itmp1; jj++)
			{
			for(kk=0; kk<itmp0; kk++)
				{
				pD[(ii+1+kk)+ldd*(ii+1+jj)] -= pD[(ii+1+kk)+ldd*ii] * pD[ii+ldd*(ii+1+jj)];
				}
			}
		}
#endif
	return;	
	}



int GEQRF_WORK_SIZE_LIBSTR(int m, int n)
	{
	return 0;
	}



void GEQRF_LIBSTR(int m, int n, struct STRMAT *sA, int ai, int aj, struct STRMAT *sD, int di, int dj, void *work)
	{
	if(m<=0 | n<=0)
		return;
	int ii, jj, kk;
	int lda = sA->m;
	int ldd = sD->m;
	REAL *pA = sA->pA+ai+aj*lda;
	REAL *pD = sD->pA+di+dj*ldd; // matrix of QR
	REAL *dD = sD->dA+di; // vectors of tau
	REAL alpha, beta, tmp, w0, w1;
	REAL *pC00, *pC01, *pC11, *pv0, *pv1;
	REAL pW[4] = {0.0, 0.0, 0.0, 0.0};
	int ldw = 2;
	REAL pT[4] = {0.0, 0.0, 0.0, 0.0};
	int ldb = 2;
	int imax, jmax, kmax;
	// copy if needed
	if(pA!=pD)
		{
		for(jj=0; jj<n; jj++)
			{
			for(ii=0; ii<m; ii++)
				{
				pD[ii+ldd*jj] = pA[ii+lda*jj];
				}
			}
		}
	imax = m<n ? m : n;
	ii = 0;
#if 1
	for(; ii<imax-1; ii+=2)
		{
		// first column
		pC00 = &pD[ii+ldd*ii];
		beta = 0.0;
		for(jj=1; jj<m-ii; jj++)
			{
			tmp = pC00[jj+ldd*0];
			beta += tmp*tmp;
			}
		if(beta==0.0)
			{
			// tau0
			dD[ii] = 0.0;
			}
		else
			{
			alpha = pC00[0+ldd*0];
			beta += alpha*alpha;
			beta = sqrt(beta);
			if(alpha>0)
				beta = -beta;
			// tau0
			dD[ii] = (beta-alpha) / beta;
			tmp = 1.0 / (alpha-beta);
			// compute v0
			pC00[0+ldd*0] = beta;
			for(jj=1; jj<m-ii; jj++)
				{
				pC00[jj+ldd*0] *= tmp;
				}
			}
		// gemv_t & ger
		pC01 = &pC00[0+ldd*1];
		pv0 = &pC00[0+ldd*0];
		kmax = m-ii;
		w0 = pC01[0+ldd*0]; // pv0[0] = 1.0
		for(kk=1; kk<kmax; kk++)
			{
			w0 += pC01[kk+ldd*0] * pv0[kk];
			}
		w0 = - dD[ii] * w0;
		pC01[0+ldd*0] += w0; // pv0[0] = 1.0
		for(kk=1; kk<kmax; kk++)
			{
			pC01[kk+ldd*0] += w0 * pv0[kk];
			}
		// second column
		pC11 = &pD[(ii+1)+ldd*(ii+1)];
		beta = 0.0;
		for(jj=1; jj<m-(ii+1); jj++)
			{
			tmp = pC11[jj+ldd*0];
			beta += tmp*tmp;
			}
		if(beta==0.0)
			{
			// tau1
			dD[(ii+1)] = 0.0;
			}
		else
			{
			alpha = pC11[0+ldd*0];
			beta += alpha*alpha;
			beta = sqrt(beta);
			if(alpha>0)
				beta = -beta;
			// tau1
			dD[(ii+1)] = (beta-alpha) / beta;
			tmp = 1.0 / (alpha-beta);
			// compute v1
			pC11[0+ldd*0] = beta;
			for(jj=1; jj<m-(ii+1); jj++)
				pC11[jj+ldd*0] *= tmp;
			}
		// compute lower triangular T containing tau for matrix update
		pv0 = &pC00[0+ldd*0];
		pv1 = &pC00[0+ldd*1];
		kmax = m-ii;
		tmp = pv0[1];
		for(kk=2; kk<kmax; kk++)
			tmp += pv0[kk]*pv1[kk];
		pT[0+ldb*0] = dD[ii+0];
		pT[1+ldb*0] = - dD[ii+1] * tmp * dD[ii+0];
		pT[1+ldb*1] = dD[ii+1];
		jmax = n-ii-2;
		jj = 0;
		for(; jj<jmax-1; jj+=2)
			{
			// compute W^T = C^T * V
			pW[0+ldw*0] = pC00[0+ldd*(jj+0+2)] + pC00[1+ldd*(jj+0+2)] * pv0[1];
			pW[1+ldw*0] = pC00[0+ldd*(jj+1+2)] + pC00[1+ldd*(jj+1+2)] * pv0[1];
			pW[0+ldw*1] =                        pC00[1+ldd*(jj+0+2)];
			pW[1+ldw*1] =                        pC00[1+ldd*(jj+1+2)];
			kk = 2;
			for(; kk<kmax; kk++)
				{
				tmp = pC00[kk+ldd*(jj+0+2)];
				pW[0+ldw*0] += tmp * pv0[kk];
				pW[0+ldw*1] += tmp * pv1[kk];
				tmp = pC00[kk+ldd*(jj+1+2)];
				pW[1+ldw*0] += tmp * pv0[kk];
				pW[1+ldw*1] += tmp * pv1[kk];
				}
			// compute W^T *= T
			pW[0+ldw*1] = pT[1+ldb*0]*pW[0+ldw*0] + pT[1+ldb*1]*pW[0+ldw*1];
			pW[1+ldw*1] = pT[1+ldb*0]*pW[1+ldw*0] + pT[1+ldb*1]*pW[1+ldw*1];
			pW[0+ldw*0] = pT[0+ldb*0]*pW[0+ldw*0];
			pW[1+ldw*0] = pT[0+ldb*0]*pW[1+ldw*0];
			// compute C -= V * W^T
			pC00[0+ldd*(jj+0+2)] -= pW[0+ldw*0];
			pC00[0+ldd*(jj+1+2)] -= pW[1+ldw*0];
			pC00[1+ldd*(jj+0+2)] -= pv0[1]*pW[0+ldw*0] + pW[0+ldw*1];
			pC00[1+ldd*(jj+1+2)] -= pv0[1]*pW[1+ldw*0] + pW[1+ldw*1];
			kk = 2;
			for(; kk<kmax-1; kk+=2)
				{
				pC00[kk+0+ldd*(jj+0+2)] -= pv0[kk+0]*pW[0+ldw*0] + pv1[kk+0]*pW[0+ldw*1];
				pC00[kk+1+ldd*(jj+0+2)] -= pv0[kk+1]*pW[0+ldw*0] + pv1[kk+1]*pW[0+ldw*1];
				pC00[kk+0+ldd*(jj+1+2)] -= pv0[kk+0]*pW[1+ldw*0] + pv1[kk+0]*pW[1+ldw*1];
				pC00[kk+1+ldd*(jj+1+2)] -= pv0[kk+1]*pW[1+ldw*0] + pv1[kk+1]*pW[1+ldw*1];
				}
			for(; kk<kmax; kk++)
				{
				pC00[kk+ldd*(jj+0+2)] -= pv0[kk]*pW[0+ldw*0] + pv1[kk]*pW[0+ldw*1];
				pC00[kk+ldd*(jj+1+2)] -= pv0[kk]*pW[1+ldw*0] + pv1[kk]*pW[1+ldw*1];
				}
			}
		for(; jj<jmax; jj++)
			{
			// compute W = T * V^T * C
			pW[0+ldw*0] = pC00[0+ldd*(jj+0+2)] + pC00[1+ldd*(jj+0+2)] * pv0[1];
			pW[0+ldw*1] =                        pC00[1+ldd*(jj+0+2)];
			for(kk=2; kk<kmax; kk++)
				{
				tmp = pC00[kk+ldd*(jj+0+2)];
				pW[0+ldw*0] += tmp * pv0[kk];
				pW[0+ldw*1] += tmp * pv1[kk];
				}
			pW[0+ldw*1] = pT[1+ldb*0]*pW[0+ldw*0] + pT[1+ldb*1]*pW[0+ldw*1];
			pW[0+ldw*0] = pT[0+ldb*0]*pW[0+ldw*0];
			// compute C -= V * W^T
			pC00[0+ldd*(jj+0+2)] -= pW[0+ldw*0];
			pC00[1+ldd*(jj+0+2)] -= pv0[1]*pW[0+ldw*0] + pW[0+ldw*1];
			for(kk=2; kk<kmax; kk++)
				{
				pC00[kk+ldd*(jj+0+2)] -= pv0[kk]*pW[0+ldw*0] + pv1[kk]*pW[0+ldw*1];
				}
			}
		}
#endif
	for(; ii<imax; ii++)
		{
		pC00 = &pD[ii+ldd*ii];
		beta = 0.0;
		for(jj=1; jj<m-ii; jj++)
			{
			tmp = pC00[jj+ldd*0];
			beta += tmp*tmp;
			}
		if(beta==0.0)
			{
			dD[ii] = 0.0;
			}
		else
			{
			alpha = pC00[0+ldd*0];
			beta += alpha*alpha;
			beta = sqrt(beta);
			if(alpha>0)
				beta = -beta;
			dD[ii] = (beta-alpha) / beta;
			tmp = 1.0 / (alpha-beta);
			for(jj=1; jj<m-ii; jj++)
				pC00[jj+ldd*0] *= tmp;
			pC00[0+ldd*0] = beta;
			}
		if(ii<n)
			{
			// gemv_t & ger
			pC01 = &pC00[0+ldd*1];
			pv0 = &pC00[0+ldd*0];
			jmax = n-ii-1;
			kmax = m-ii;
			jj = 0;
			for(; jj<jmax-1; jj+=2)
				{
				w0 = pC01[0+ldd*(jj+0)]; // pv0[0] = 1.0
				w1 = pC01[0+ldd*(jj+1)]; // pv0[0] = 1.0
				for(kk=1; kk<kmax; kk++)
					{
					w0 += pC01[kk+ldd*(jj+0)] * pv0[kk];
					w1 += pC01[kk+ldd*(jj+1)] * pv0[kk];
					}
				w0 = - dD[ii] * w0;
				w1 = - dD[ii] * w1;
				pC01[0+ldd*(jj+0)] += w0; // pv0[0] = 1.0
				pC01[0+ldd*(jj+1)] += w1; // pv0[0] = 1.0
				for(kk=1; kk<kmax; kk++)
					{
					pC01[kk+ldd*(jj+0)] += w0 * pv0[kk];
					pC01[kk+ldd*(jj+1)] += w1 * pv0[kk];
					}
				}
			for(; jj<jmax; jj++)
				{
				w0 = pC01[0+ldd*jj]; // pv0[0] = 1.0
				for(kk=1; kk<kmax; kk++)
					{
					w0 += pC01[kk+ldd*jj] * pv0[kk];
					}
				w0 = - dD[ii] * w0;
				pC01[0+ldd*jj] += w0; // pv0[0] = 1.0
				for(kk=1; kk<kmax; kk++)
					{
					pC01[kk+ldd*jj] += w0 * pv0[kk];
					}
				}
			}
		}
	return;
	}



int GELQF_WORK_SIZE_LIBSTR(int m, int n)
	{
	return 0;
	}



void GELQF_LIBSTR(int m, int n, struct STRMAT *sA, int ai, int aj, struct STRMAT *sD, int di, int dj, void *work)
	{
	if(m<=0 | n<=0)
		return;
	int ii, jj, kk;
	int lda = sA->m;
	int ldd = sD->m;
	REAL *pA = sA->pA+ai+aj*lda;
	REAL *pD = sD->pA+di+dj*ldd; // matrix of QR
	REAL *dD = sD->dA+di; // vectors of tau
	REAL alpha, beta, tmp, w0, w1;
	REAL *pC00, *pC10, *pC11, *pv0, *pv1;
	REAL pW[4] = {0.0, 0.0, 0.0, 0.0};
	int ldw = 2;
	REAL pT[4] = {0.0, 0.0, 0.0, 0.0};
	int ldb = 2;
	int imax, jmax, kmax;
	// copy if needed
	if(pA!=pD)
		{
		for(jj=0; jj<n; jj++)
			{
			for(ii=0; ii<m; ii++)
				{
				pD[ii+ldd*jj] = pA[ii+lda*jj];
				}
			}
		}
	imax = m<n ? m : n;
	ii = 0;
#if 1
	for(; ii<imax-1; ii+=2)
		{
		// first column
		pC00 = &pD[ii+ldd*ii];
		beta = 0.0;
		for(jj=1; jj<n-ii; jj++)
			{
			tmp = pC00[0+ldd*jj];
			beta += tmp*tmp;
			}
		if(beta==0.0)
			{
			// tau0
			dD[ii] = 0.0;
			}
		else
			{
			alpha = pC00[0+ldd*0];
			beta += alpha*alpha;
			beta = sqrt(beta);
			if(alpha>0)
				beta = -beta;
			// tau0
			dD[ii] = (beta-alpha) / beta;
			tmp = 1.0 / (alpha-beta);
			// compute v0
			pC00[0+ldd*0] = beta;
			for(jj=1; jj<n-ii; jj++)
				{
				pC00[0+ldd*jj] *= tmp;
				}
			}
		// gemv_t & ger
		pC10 = &pC00[1+ldd*0];
		pv0 = &pC00[0+ldd*0];
		kmax = n-ii;
		w0 = pC10[0+ldd*0]; // pv0[0] = 1.0
		for(kk=1; kk<kmax; kk++)
			{
			w0 += pC10[0+ldd*kk] * pv0[0+ldd*kk];
			}
		w0 = - dD[ii] * w0;
		pC10[0+ldd*0] += w0; // pv0[0] = 1.0
		for(kk=1; kk<kmax; kk++)
			{
			pC10[0+ldd*kk] += w0 * pv0[0+ldd*kk];
			}
		// second row
		pC11 = &pD[(ii+1)+ldd*(ii+1)];
		beta = 0.0;
		for(jj=1; jj<n-(ii+1); jj++)
			{
			tmp = pC11[0+ldd*jj];
			beta += tmp*tmp;
			}
		if(beta==0.0)
			{
			// tau1
			dD[(ii+1)] = 0.0;
			}
		else
			{
			alpha = pC11[0+ldd*0];
			beta += alpha*alpha;
			beta = sqrt(beta);
			if(alpha>0)
				beta = -beta;
			// tau1
			dD[(ii+1)] = (beta-alpha) / beta;
			tmp = 1.0 / (alpha-beta);
			// compute v1
			pC11[0+ldd*0] = beta;
			for(jj=1; jj<n-(ii+1); jj++)
				pC11[0+ldd*jj] *= tmp;
			}
		// compute lower triangular T containing tau for matrix update
		pv0 = &pC00[0+ldd*0];
		pv1 = &pC00[1+ldd*0];
		kmax = n-ii;
		tmp = pv0[0+ldd*1];
		for(kk=2; kk<kmax; kk++)
			tmp += pv0[0+ldd*kk]*pv1[0+ldd*kk];
		pT[0+ldb*0] = dD[ii+0];
		pT[1+ldb*0] = - dD[ii+1] * tmp * dD[ii+0];
		pT[1+ldb*1] = dD[ii+1];
		// downgrade
		jmax = m-ii-2;
		jj = 0;
		for(; jj<jmax-1; jj+=2)
			{
			// compute W^T = C^T * V
			pW[0+ldw*0] = pC00[jj+0+2+ldd*0] + pC00[jj+0+2+ldd*1] * pv0[0+ldd*1];
			pW[1+ldw*0] = pC00[jj+1+2+ldd*0] + pC00[jj+1+2+ldd*1] * pv0[0+ldd*1];
			pW[0+ldw*1] =                      pC00[jj+0+2+ldd*1];
			pW[1+ldw*1] =                      pC00[jj+1+2+ldd*1];
			kk = 2;
			for(; kk<kmax; kk++)
				{
				tmp = pC00[jj+0+2+ldd*kk];
				pW[0+ldw*0] += tmp * pv0[0+ldd*kk];
				pW[0+ldw*1] += tmp * pv1[0+ldd*kk];
				tmp = pC00[jj+1+2+ldd*kk];
				pW[1+ldw*0] += tmp * pv0[0+ldd*kk];
				pW[1+ldw*1] += tmp * pv1[0+ldd*kk];
				}
			// compute W^T *= T
			pW[0+ldw*1] = pT[1+ldb*0]*pW[0+ldw*0] + pT[1+ldb*1]*pW[0+ldw*1];
			pW[1+ldw*1] = pT[1+ldb*0]*pW[1+ldw*0] + pT[1+ldb*1]*pW[1+ldw*1];
			pW[0+ldw*0] = pT[0+ldb*0]*pW[0+ldw*0];
			pW[1+ldw*0] = pT[0+ldb*0]*pW[1+ldw*0];
			// compute C -= V * W^T
			pC00[jj+0+2+ldd*0] -= pW[0+ldw*0];
			pC00[jj+1+2+ldd*0] -= pW[1+ldw*0];
			pC00[jj+0+2+ldd*1] -= pv0[0+ldd*1]*pW[0+ldw*0] + pW[0+ldw*1];
			pC00[jj+1+2+ldd*1] -= pv0[0+ldd*1]*pW[1+ldw*0] + pW[1+ldw*1];
			kk = 2;
			for(; kk<kmax-1; kk+=2)
				{
				pC00[jj+0+2+ldd*(kk+0)] -= pv0[0+ldd*(kk+0)]*pW[0+ldw*0] + pv1[0+ldd*(kk+0)]*pW[0+ldw*1];
				pC00[jj+0+2+ldd*(kk+1)] -= pv0[0+ldd*(kk+1)]*pW[0+ldw*0] + pv1[0+ldd*(kk+1)]*pW[0+ldw*1];
				pC00[jj+1+2+ldd*(kk+0)] -= pv0[0+ldd*(kk+0)]*pW[1+ldw*0] + pv1[0+ldd*(kk+0)]*pW[1+ldw*1];
				pC00[jj+1+2+ldd*(kk+1)] -= pv0[0+ldd*(kk+1)]*pW[1+ldw*0] + pv1[0+ldd*(kk+1)]*pW[1+ldw*1];
				}
			for(; kk<kmax; kk++)
				{
				pC00[jj+0+2+ldd*kk] -= pv0[0+ldd*kk]*pW[0+ldw*0] + pv1[0+ldd*kk]*pW[0+ldw*1];
				pC00[jj+1+2+ldd*kk] -= pv0[0+ldd*kk]*pW[1+ldw*0] + pv1[0+ldd*kk]*pW[1+ldw*1];
				}
			}
		for(; jj<jmax; jj++)
			{
			// compute W = T * V^T * C
			pW[0+ldw*0] = pC00[jj+0+2+ldd*0] + pC00[jj+0+2+ldd*1] * pv0[0+ldd*1];
			pW[0+ldw*1] =                      pC00[jj+0+2+ldd*1];
			for(kk=2; kk<kmax; kk++)
				{
				tmp = pC00[jj+0+2+ldd*kk];
				pW[0+ldw*0] += tmp * pv0[0+ldd*kk];
				pW[0+ldw*1] += tmp * pv1[0+ldd*kk];
				}
			pW[0+ldw*1] = pT[1+ldb*0]*pW[0+ldw*0] + pT[1+ldb*1]*pW[0+ldw*1];
			pW[0+ldw*0] = pT[0+ldb*0]*pW[0+ldw*0];
			// compute C -= V * W^T
			pC00[jj+0+2+ldd*0] -= pW[0+ldw*0];
			pC00[jj+0+2+ldd*1] -= pv0[0+ldd*1]*pW[0+ldw*0] + pW[0+ldw*1];
			for(kk=2; kk<kmax; kk++)
				{
				pC00[jj+0+2+ldd*kk] -= pv0[0+ldd*kk]*pW[0+ldw*0] + pv1[0+ldd*kk]*pW[0+ldw*1];
				}
			}
		}
#endif
	for(; ii<imax; ii++)
		{
		pC00 = &pD[ii+ldd*ii];
		beta = 0.0;
		for(jj=1; jj<n-ii; jj++)
			{
			tmp = pC00[0+ldd*jj];
			beta += tmp*tmp;
			}
		if(beta==0.0)
			{
			dD[ii] = 0.0;
			}
		else
			{
			alpha = pC00[0+ldd*0];
			beta += alpha*alpha;
			beta = sqrt(beta);
			if(alpha>0)
				beta = -beta;
			dD[ii] = (beta-alpha) / beta;
			tmp = 1.0 / (alpha-beta);
			for(jj=1; jj<n-ii; jj++)
				pC00[0+ldd*jj] *= tmp;
			pC00[0+ldd*0] = beta;
			}
		if(ii<n)
			{
			// gemv_t & ger
			pC10 = &pC00[1+ldd*0];
			pv0 = &pC00[0+ldd*0];
			jmax = m-ii-1;
			kmax = n-ii;
			jj = 0;
			for(; jj<jmax-1; jj+=2)
				{
				w0 = pC10[jj+0+ldd*0]; // pv0[0] = 1.0
				w1 = pC10[jj+1+ldd*0]; // pv0[0] = 1.0
				for(kk=1; kk<kmax; kk++)
					{
					w0 += pC10[jj+0+ldd*kk] * pv0[0+ldd*kk];
					w1 += pC10[jj+1+ldd*kk] * pv0[0+ldd*kk];
					}
				w0 = - dD[ii] * w0;
				w1 = - dD[ii] * w1;
				pC10[jj+0+ldd*0] += w0; // pv0[0] = 1.0
				pC10[jj+1+ldd*0] += w1; // pv0[0] = 1.0
				for(kk=1; kk<kmax; kk++)
					{
					pC10[jj+0+ldd*kk] += w0 * pv0[0+ldd*kk];
					pC10[jj+1+ldd*kk] += w1 * pv0[0+ldd*kk];
					}
				}
			for(; jj<jmax; jj++)
				{
				w0 = pC10[jj+ldd*0]; // pv0[0] = 1.0
				for(kk=1; kk<kmax; kk++)
					{
					w0 += pC10[jj+ldd*kk] * pv0[0+ldd*kk];
					}
				w0 = - dD[ii] * w0;
				pC10[jj+ldd*0] += w0; // pv0[0] = 1.0
				for(kk=1; kk<kmax; kk++)
					{
					pC10[jj+ldd*kk] += w0 * pv0[0+ldd*kk];
					}
				}
			}
		}
	return;
	}



#elif defined(LA_BLAS)



// dpotrf
void POTRF_L_LIBSTR(int m, struct STRMAT *sC, int ci, int cj, struct STRMAT *sD, int di, int dj)
	{
	if(m<=0)
		return;
	int jj;
	char cl = 'l';
	char cn = 'n';
	char cr = 'r';
	char ct = 't';
	REAL d1 = 1.0;
	REAL *pC = sC->pA+ci+cj*sC->m;
	REAL *pD = sD->pA+di+dj*sD->m;
#if defined(REF_BLAS_BLIS)
	long long i1 = 1;
	long long mm = m;
	long long info;
	long long tmp;
	long long ldc = sC->m;
	long long ldd = sD->m;
	if(!(pC==pD))
		{
		for(jj=0; jj<m; jj++)
			{
			tmp = m-jj;
			COPY(&tmp, pC+jj*ldc+jj, &i1, pD+jj*ldd+jj, &i1);
			}
		}
	POTRF(&cl, &mm, pD, &ldd, &info);
#else
	int i1 = 1;
	int info;
	int tmp;
	int ldc = sC->m;
	int ldd = sD->m;
	if(!(pC==pD))
		{
		for(jj=0; jj<m; jj++)
			{
			tmp = m-jj;
			COPY(&tmp, pC+jj*ldc+jj, &i1, pD+jj*ldd+jj, &i1);
			}
		}
	POTRF(&cl, &m, pD, &ldd, &info);
#endif
	return;
	}



// dpotrf
void POTRF_L_MN_LIBSTR(int m, int n, struct STRMAT *sC, int ci, int cj, struct STRMAT *sD, int di, int dj)
	{
	if(m<=0 | n<=0)
		return;
	int jj;
	char cl = 'l';
	char cn = 'n';
	char cr = 'r';
	char ct = 't';
	REAL d1 = 1.0;
	REAL *pC = sC->pA+ci+cj*sC->m;
	REAL *pD = sD->pA+di+dj*sD->m;
#if defined(REF_BLAS_BLIS)
	long long i1 = 1;
	long long mm = m;
	long long nn = n;
	long long mmn = mm-nn;
	long long info;
	long long tmp;
	long long ldc = sC->m;
	long long ldd = sD->m;
	if(!(pC==pD))
		{
		for(jj=0; jj<n; jj++)
			{
			tmp = m-jj;
			COPY(&tmp, pC+jj*ldc+jj, &i1, pD+jj*ldd+jj, &i1);
			}
		}
	POTRF(&cl, &nn, pD, &ldd, &info);
	TRSM(&cr, &cl, &ct, &cn, &mmn, &nn, &d1, pD, &ldd, pD+n, &ldd);
#else
	int i1 = 1;
	int mmn = m-n;
	int info;
	int tmp;
	int ldc = sC->m;
	int ldd = sD->m;
	if(!(pC==pD))
		{
		for(jj=0; jj<n; jj++)
			{
			tmp = m-jj;
			COPY(&tmp, pC+jj*ldc+jj, &i1, pD+jj*ldd+jj, &i1);
			}
		}
	POTRF(&cl, &n, pD, &ldd, &info);
	TRSM(&cr, &cl, &ct, &cn, &mmn, &n, &d1, pD, &ldd, pD+n, &ldd);
#endif
	return;
	}



// dsyrk dpotrf
void SYRK_POTRF_LN_LIBSTR(int m, int n, int k, struct STRMAT *sA, int ai, int aj, struct STRMAT *sB, int bi, int bj, struct STRMAT *sC, int ci, int cj, struct STRMAT *sD, int di, int dj)
	{
	if(m<=0 | n<=0)
		return;
	int jj;
	char cl = 'l';
	char cn = 'n';
	char cr = 'r';
	char ct = 't';
	char cu = 'u';
	REAL d1 = 1.0;
	REAL *pA = sA->pA + ai + aj*sA->m;
	REAL *pB = sB->pA + bi + bj*sB->m;
	REAL *pC = sC->pA + ci + cj*sC->m;
	REAL *pD = sD->pA + di + dj*sD->m;
#if defined(REF_BLAS_BLIS)
	long long i1 = 1;
	long long mm = m;
	long long nn = n;
	long long kk = k;
	long long mmn = mm-nn;
	long long info;
	long long lda = sA->m;
	long long ldb = sB->m;
	long long ldc = sC->m;
	long long ldd = sD->m;
	if(!(pC==pD))
		{
		for(jj=0; jj<n; jj++)
			COPY(&mm, pC+jj*ldc, &i1, pD+jj*ldd, &i1);
		}
	if(pA==pB)
		{
		SYRK(&cl, &cn, &nn, &kk, &d1, pA, &lda, &d1, pD, &ldd);
		GEMM(&cn, &ct, &mmn, &nn, &kk, &d1, pA+n, &lda, pB, &ldb, &d1, pD+n, &ldd);
		POTRF(&cl, &nn, pD, &ldd, &info);
		TRSM(&cr, &cl, &ct, &cn, &mmn, &nn, &d1, pD, &ldd, pD+n, &ldd);
		}
	else
		{
		GEMM(&cn, &ct, &mm, &nn, &kk, &d1, pA, &lda, pB, &ldb, &d1, pD, &ldd);
		POTRF(&cl, &nn, pD, &ldd, &info);
		TRSM(&cr, &cl, &ct, &cn, &mmn, &nn, &d1, pD, &ldd, pD+n, &ldd);
		}
#else
	int i1 = 1;
	int mmn = m-n;
	int info;
	int lda = sA->m;
	int ldb = sB->m;
	int ldc = sC->m;
	int ldd = sD->m;
	if(!(pC==pD))
		{
		for(jj=0; jj<n; jj++)
			COPY(&m, pC+jj*ldc, &i1, pD+jj*ldd, &i1);
		}
	if(pA==pB)
		{
		SYRK(&cl, &cn, &n, &k, &d1, pA, &lda, &d1, pD, &ldd);
		GEMM(&cn, &ct, &mmn, &n, &k, &d1, pA+n, &lda, pB, &ldb, &d1, pD+n, &ldd);
		POTRF(&cl, &n, pD, &ldd, &info);
		TRSM(&cr, &cl, &ct, &cn, &mmn, &n, &d1, pD, &ldd, pD+n, &ldd);
		}
	else
		{
		GEMM(&cn, &ct, &m, &n, &k, &d1, pA, &lda, pB, &ldb, &d1, pD, &ldd);
		POTRF(&cl, &n, pD, &ldd, &info);
		TRSM(&cr, &cl, &ct, &cn, &mmn, &n, &d1, pD, &ldd, pD+n, &ldd);
		}
#endif
	return;
	}



// dgetrf without pivoting
#if defined(REF_BLAS_BLIS)
static void GETF2_NOPIVOT(long long m, long long n, REAL *A, long long lda)
	{
	if(m<=0 | n<=0)
		return;
	long long i, j;
	long long jmax = m<n ? m : n;
	REAL dtmp;
	REAL dm1 = -1.0;
	long long itmp0, itmp1;
	long long i1 = 1;
	for(j=0; j<jmax; j++)
		{
		itmp0 = m-j-1;
		dtmp = 1.0/A[j+lda*j];
		SCAL(&itmp0, &dtmp, &A[(j+1)+lda*j], &i1);
		itmp1 = n-j-1;
		GER(&itmp0, &itmp1, &dm1, &A[(j+1)+lda*j], &i1, &A[j+lda*(j+1)], &lda, &A[(j+1)+lda*(j+1)], &lda);
		}
	return;
	}
#else
static void GETF2_NOPIVOT(int m, int n, REAL *A, int lda)
	{
	if(m<=0 | n<=0)
		return;
	int i, j;
	int jmax = m<n ? m : n;
	REAL dtmp;
	REAL dm1 = -1.0;
	int itmp0, itmp1;
	int i1 = 1;
	for(j=0; j<jmax; j++)
		{
		itmp0 = m-j-1;
		dtmp = 1.0/A[j+lda*j];
		SCAL(&itmp0, &dtmp, &A[(j+1)+lda*j], &i1);
		itmp1 = n-j-1;
		GER(&itmp0, &itmp1, &dm1, &A[(j+1)+lda*j], &i1, &A[j+lda*(j+1)], &lda, &A[(j+1)+lda*(j+1)], &lda);
		}
	return;
	}
#endif



// dgetrf without pivoting
void GETRF_NOPIVOT_LIBSTR(int m, int n, struct STRMAT *sC, int ci, int cj, struct STRMAT *sD, int di, int dj)
	{
	// TODO with custom level 2 LAPACK + level 3 BLAS
	if(m<=0 | n<=0)
		return;
	int jj;
	REAL d1 = 1.0;
	REAL *pC = sC->pA+ci+cj*sC->m;
	REAL *pD = sD->pA+di+dj*sD->m;
#if defined(REF_BLAS_BLIS)
	long long i1 = 1;
	long long mm = m;
	long long nn = n;
	long long ldc = sC->m;
	long long ldd = sD->m;
	if(!(pC==pD))
		{
		for(jj=0; jj<n; jj++)
			COPY(&mm, pC+jj*ldc, &i1, pD+jj*ldd, &i1);
		}
	GETF2_NOPIVOT(mm, nn, pD, ldd);
#else
	int i1 = 1;
	int ldc = sC->m;
	int ldd = sD->m;
	if(!(pC==pD))
		{
		for(jj=0; jj<n; jj++)
			COPY(&m, pC+jj*ldc, &i1, pD+jj*ldd, &i1);
		}
	GETF2_NOPIVOT(m, n, pD, ldd);
#endif
	return;
	}



// dgetrf pivoting
void GETRF_LIBSTR(int m, int n, struct STRMAT *sC, int ci, int cj, struct STRMAT *sD, int di, int dj, int *ipiv)
	{
	// TODO with custom level 2 LAPACK + level 3 BLAS
	if(m<=0 | n<=0)
		return;
	int jj;
	int tmp = m<n ? m : n;
	REAL d1 = 1.0;
	REAL *pC = sC->pA+ci+cj*sC->m;
	REAL *pD = sD->pA+di+dj*sD->m;
#if defined(REF_BLAS_BLIS)
	long long i1 = 1;
	long long info;
	long long mm = m;
	long long nn = n;
	long long ldc = sC->m;
	long long ldd = sD->m;
	if(!(pC==pD))
		{
		for(jj=0; jj<n; jj++)
			COPY(&mm, pC+jj*ldc, &i1, pD+jj*ldd, &i1);
		}
	GETRF(&mm, &nn, pD, &ldd, (long long *) ipiv, &info);
	for(jj=0; jj<tmp; jj++)
		ipiv[jj] -= 1;
#else
	int i1 = 1;
	int info;
	int ldc = sC->m;
	int ldd = sD->m;
	if(!(pC==pD))
		{
		for(jj=0; jj<n; jj++)
			COPY(&m, pC+jj*ldc, &i1, pD+jj*ldd, &i1);
		}
	GETRF(&m, &n, pD, &ldd, ipiv, &info);
	for(jj=0; jj<tmp; jj++)
		ipiv[jj] -= 1;
#endif
	return;
	}



int GEQRF_WORK_SIZE_LIBSTR(int m, int n)
	{
	REAL dwork;
	REAL *pD, *dD;
#if defined(REF_BLAS_BLIS)
	long long mm = m;
	long long nn = n;
	long long lwork = -1;
	long long info;
	long long ldd = mm;
	GEQRF(&mm, &nn, pD, &ldd, dD, &dwork, &lwork, &info);
#else
	int lwork = -1;
	int info;
	int ldd = m;
	GEQRF(&m, &n, pD, &ldd, dD, &dwork, &lwork, &info);
#endif
	int size = dwork;
	return size*sizeof(REAL);
	}



void GEQRF_LIBSTR(int m, int n, struct STRMAT *sC, int ci, int cj, struct STRMAT *sD, int di, int dj, void *work)
	{
	if(m<=0 | n<=0)
		return;
	int jj;
	REAL *pC = sC->pA+ci+cj*sC->m;
	REAL *pD = sD->pA+di+dj*sD->m;
	REAL *dD = sD->dA+di;
	REAL *dwork = (REAL *) work;
#if defined(REF_BLAS_BLIS)
	long long i1 = 1;
	long long info = -1;
	long long mm = m;
	long long nn = n;
	long long ldc = sC->m;
	long long ldd = sD->m;
	if(!(pC==pD))
		{
		for(jj=0; jj<n; jj++)
			COPY(&mm, pC+jj*ldc, &i1, pD+jj*ldd, &i1);
		}
//	GEQR2(&mm, &nn, pD, &ldd, dD, dwork, &info);
	long long lwork = -1;
	GEQRF(&mm, &nn, pD, &ldd, dD, dwork, &lwork, &info);
	lwork = dwork[0];
	GEQRF(&mm, &nn, pD, &ldd, dD, dwork, &lwork, &info);
#else
	int i1 = 1;
	int info = -1;
	int ldc = sC->m;
	int ldd = sD->m;
	if(!(pC==pD))
		{
		for(jj=0; jj<n; jj++)
			COPY(&m, pC+jj*ldc, &i1, pD+jj*ldd, &i1);
		}
//	GEQR2(&m, &n, pD, &ldd, dD, dwork, &info);
	int lwork = -1;
	GEQRF(&m, &n, pD, &ldd, dD, dwork, &lwork, &info);
	lwork = dwork[0];
	GEQRF(&m, &n, pD, &ldd, dD, dwork, &lwork, &info);
#endif
	return;
	}



int GELQF_WORK_SIZE_LIBSTR(int m, int n)
	{
	REAL dwork;
	REAL *pD, *dD;
#if defined(REF_BLAS_BLIS)
	long long mm = m;
	long long nn = n;
	long long lwork = -1;
	long long info;
	long long ldd = mm;
	GELQF(&mm, &nn, pD, &ldd, dD, &dwork, &lwork, &info);
#else
	int lwork = -1;
	int info;
	int ldd = m;
	GELQF(&m, &n, pD, &ldd, dD, &dwork, &lwork, &info);
#endif
	int size = dwork;
	return size*sizeof(REAL);
	}



void GELQF_LIBSTR(int m, int n, struct STRMAT *sC, int ci, int cj, struct STRMAT *sD, int di, int dj, void *work)
	{
	if(m<=0 | n<=0)
		return;
	int jj;
	REAL *pC = sC->pA+ci+cj*sC->m;
	REAL *pD = sD->pA+di+dj*sD->m;
	REAL *dD = sD->dA+di;
	REAL *dwork = (REAL *) work;
#if defined(REF_BLAS_BLIS)
	long long i1 = 1;
	long long info = -1;
	long long mm = m;
	long long nn = n;
	long long ldc = sC->m;
	long long ldd = sD->m;
	if(!(pC==pD))
		{
		for(jj=0; jj<n; jj++)
			COPY(&mm, pC+jj*ldc, &i1, pD+jj*ldd, &i1);
		}
//	GEQR2(&mm, &nn, pD, &ldd, dD, dwork, &info);
	long long lwork = -1;
	GELQF(&mm, &nn, pD, &ldd, dD, dwork, &lwork, &info);
	lwork = dwork[0];
	GELQF(&mm, &nn, pD, &ldd, dD, dwork, &lwork, &info);
#else
	int i1 = 1;
	int info = -1;
	int ldc = sC->m;
	int ldd = sD->m;
	if(!(pC==pD))
		{
		for(jj=0; jj<n; jj++)
			COPY(&m, pC+jj*ldc, &i1, pD+jj*ldd, &i1);
		}
//	GEQR2(&m, &n, pD, &ldd, dD, dwork, &info);
	int lwork = -1;
	GELQF(&m, &n, pD, &ldd, dD, dwork, &lwork, &info);
	lwork = dwork[0];
	GELQF(&m, &n, pD, &ldd, dD, dwork, &lwork, &info);
#endif
	return;
	}



#else

#error : wrong LA choice

#endif




