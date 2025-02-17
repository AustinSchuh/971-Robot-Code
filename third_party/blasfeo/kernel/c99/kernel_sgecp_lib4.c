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




// both A and B are aligned to 256-bit boundaries
void kernel_sgesc_4_lib4(int kmax, float *alphap, float *A)
	{

	if(kmax<=0)
		return;

	const int bs = 4;

	float alpha = alphap[0];

	int k;

	for(k=0; k<kmax; k++)
		{

		A[0+bs*0] *= alpha;
		A[1+bs*0] *= alpha;
		A[2+bs*0] *= alpha;
		A[3+bs*0] *= alpha;

		A += 4;

		}
	
	}



void kernel_sgesc_3_lib4(int kmax, float *alphap, float *A)
	{

	if(kmax<=0)
		return;

	const int bs = 4;

	float alpha = alphap[0];

	int k;

	for(k=0; k<kmax; k++)
		{

		A[0+bs*0] *= alpha;
		A[1+bs*0] *= alpha;
		A[2+bs*0] *= alpha;

		A += 4;

		}
	
	}



void kernel_sgesc_2_lib4(int kmax, float *alphap, float *A)
	{

	if(kmax<=0)
		return;

	const int bs = 4;

	float alpha = alphap[0];

	int k;

	for(k=0; k<kmax; k++)
		{

		A[0+bs*0] *= alpha;
		A[1+bs*0] *= alpha;

		A += 4;

		}
	
	}



void kernel_sgesc_1_lib4(int kmax, float *alphap, float *A)
	{

	if(kmax<=0)
		return;

	const int bs = 4;

	float alpha = alphap[0];

	int k;

	for(k=0; k<kmax; k++)
		{

		A[0+bs*0] *= alpha;

		A += 4;

		}
	
	}



// both A and B are aligned to 256-bit boundaries
void kernel_sgecp_4_0_lib4(int kmax, float *A, float *B)
	{

	if(kmax<=0)
		return;

	const int bs = 4;

	int k;

	for(k=0; k<kmax; k++)
		{

		B[0+bs*0] = A[0+bs*0];
		B[1+bs*0] = A[1+bs*0];
		B[2+bs*0] = A[2+bs*0];
		B[3+bs*0] = A[3+bs*0];

		A += 4;
		B += 4;

		}
	
	}



// both A and B are aligned to 256-bit boundaries, 1 element of A must be skipped
void kernel_sgecp_4_1_lib4(int kmax, float *A0, int sda, float *B)
	{

	if(kmax<=0)
		return;

	const int bs = 4;

	float *A1 = A0 + bs*sda;

	int k;

	for(k=0; k<kmax; k++)
		{

		B[0+bs*0] = A0[1+bs*0];
		B[1+bs*0] = A0[2+bs*0];
		B[2+bs*0] = A0[3+bs*0];
		B[3+bs*0] = A1[0+bs*0];

		A0 += 4;
		A1 += 4;
		B  += 4;

		}
	
	}



// both A and B are aligned to 256-bit boundaries, 2 elements of A must be skipped
void kernel_sgecp_4_2_lib4(int kmax, float *A0, int sda, float *B)
	{

	if(kmax<=0)
		return;

	const int bs = 4;

	float *A1 = A0 + bs*sda;

	int k;

	for(k=0; k<kmax; k++)
		{

		B[0+bs*0] = A0[2+bs*0];
		B[1+bs*0] = A0[3+bs*0];
		B[2+bs*0] = A1[0+bs*0];
		B[3+bs*0] = A1[1+bs*0];

		A0 += 4;
		A1 += 4;
		B  += 4;

		}
	
	}



// both A and B are aligned to 256-bit boundaries, 3 elements of A must be skipped
void kernel_sgecp_4_3_lib4(int kmax, float *A0, int sda, float *B)
	{

	if(kmax<=0)
		return;

	const int bs = 4;

	float *A1 = A0 + bs*sda;

	int k;

	for(k=0; k<kmax; k++)
		{

		B[0+bs*0] = A0[3+bs*0];
		B[1+bs*0] = A1[0+bs*0];
		B[2+bs*0] = A1[1+bs*0];
		B[3+bs*0] = A1[2+bs*0];

		A0 += 4;
		A1 += 4;
		B  += 4;

		}
	
	}



// both A and B are aligned to 64-bit boundaries
void kernel_sgecp_3_0_lib4(int kmax, float *A, float *B)
	{

	if(kmax<=0)
		return;

	const int bs = 4;

	int k;

	for(k=0; k<kmax; k++)
		{

		B[0+bs*0] = A[0+bs*0];
		B[1+bs*0] = A[1+bs*0];
		B[2+bs*0] = A[2+bs*0];

		A += 4;
		B += 4;

		}
	
	}



// both A and B are aligned to 256-bit boundaries, 2 elements of A must be skipped
void kernel_sgecp_3_2_lib4(int kmax, float *A0, int sda, float *B)
	{

	if(kmax<=0)
		return;

	const int bs = 4;

	float *A1 = A0 + bs*sda;

	int k;

	for(k=0; k<kmax; k++)
		{

		B[0+bs*0] = A0[2+bs*0];
		B[1+bs*0] = A0[3+bs*0];
		B[2+bs*0] = A1[0+bs*0];

		A0 += 4;
		A1 += 4;
		B  += 4;

		}
	
	}



// both A and B are aligned to 256-bit boundaries, 3 elements of A must be skipped
void kernel_sgecp_3_3_lib4(int kmax, float *A0, int sda, float *B)
	{

	if(kmax<=0)
		return;

	const int bs = 4;

	float *A1 = A0 + bs*sda;

	int k;

	for(k=0; k<kmax; k++)
		{

		B[0+bs*0] = A0[3+bs*0];
		B[1+bs*0] = A1[0+bs*0];
		B[2+bs*0] = A1[1+bs*0];

		A0 += 4;
		A1 += 4;
		B  += 4;

		}
	
	}



// both A and B are aligned to 64-bit boundaries
void kernel_sgecp_2_0_lib4(int kmax, float *A, float *B)
	{

	if(kmax<=0)
		return;

	const int bs = 4;

	int k;

	for(k=0; k<kmax; k++)
		{

		B[0+bs*0] = A[0+bs*0];
		B[1+bs*0] = A[1+bs*0];

		A += 4;
		B += 4;

		}
	
	}



// both A and B are aligned to 128-bit boundaries, 3 elements of A must be skipped
void kernel_sgecp_2_3_lib4(int kmax, float *A0, int sda, float *B)
	{

	if(kmax<=0)
		return;

	const int bs = 4;

	float *A1 = A0 + bs*sda;

	int k;

	for(k=0; k<kmax; k++)
		{

		B[0+bs*0] = A0[3+bs*0];
		B[1+bs*0] = A1[0+bs*0];

		A0 += 4;
		A1 += 4;
		B  += 4;

		}
	
	}



// both A and B are aligned 64-bit boundaries
void kernel_sgecp_1_0_lib4(int kmax, float *A, float *B)
	{

	if(kmax<=0)
		return;

	const int bs = 4;

	int k;

	for(k=0; k<kmax; k++)
		{

		B[0+bs*0] = A[0+bs*0];

		A += 4;
		B += 4;

		}

	}



// both A and B are aligned to 256-bit boundaries
void kernel_strcp_l_4_0_lib4(int kmax, float *A, float *B)
	{

	// A and C are lower triangular
	// kmax+1 4-wide + end 3x3 triangle

	kmax += 1;

	if(kmax<=0)
		return;

	const int bs = 4;

	int k;

	for(k=0; k<kmax; k++)
		{

		B[0+bs*0] = A[0+bs*0];
		B[1+bs*0] = A[1+bs*0];
		B[2+bs*0] = A[2+bs*0];
		B[3+bs*0] = A[3+bs*0];

		A += 4;
		B += 4;

		}
	
	// 3x3 triangle

	B[1+bs*0] = A[1+bs*0];
	B[2+bs*0] = A[2+bs*0];
	B[3+bs*0] = A[3+bs*0];

	B[2+bs*1] = A[2+bs*1];
	B[3+bs*1] = A[3+bs*1];

	B[3+bs*2] = A[3+bs*2];

	}



// both A and B are aligned to 256-bit boundaries, 1 element of A must be skipped
void kernel_strcp_l_4_1_lib4(int kmax, float *A0, int sda, float *B)
	{

	// A and C are lower triangular
	// kmax+1 4-wide + end 3x3 triangle

	kmax += 1;

	if(kmax<=0)
		return;

	const int bs = 4;

	float *A1 = A0 + bs*sda;

	int k;

	for(k=0; k<kmax; k++)
		{

		B[0+bs*0] = A0[1+bs*0];
		B[1+bs*0] = A0[2+bs*0];
		B[2+bs*0] = A0[3+bs*0];
		B[3+bs*0] = A1[0+bs*0];

		A0 += 4;
		A1 += 4;
		B  += 4;

		}
	
	// 3x3 triangle

	B[1+0*bs] = A0[2+0*bs];
	B[2+0*bs] = A0[3+0*bs];
	B[3+0*bs] = A1[0+0*bs];

	B[2+1*bs] = A0[3+1*bs];
	B[3+1*bs] = A1[0+1*bs];

	B[3+2*bs] = A1[0+2*bs];

	}



// both A and B are aligned to 256-bit boundaries, 2 elements of A must be skipped
void kernel_strcp_l_4_2_lib4(int kmax, float *A0, int sda, float *B)
	{

	// A and C are lower triangular
	// kmax+1 4-wide + end 3x3 triangle

	kmax += 1;

	if(kmax<=0)
		return;

	const int bs = 4;

	float *A1 = A0 + bs*sda;

	int k;

	for(k=0; k<kmax; k++)
		{

		B[0+bs*0] = A0[2+bs*0];
		B[1+bs*0] = A0[3+bs*0];
		B[2+bs*0] = A1[0+bs*0];
		B[3+bs*0] = A1[1+bs*0];

		A0 += 4;
		A1 += 4;
		B  += 4;

		}
	
	// 3x3 triangle}

	B[1+bs*0] = A0[3+bs*0];
	B[2+bs*0] = A1[0+bs*0];
	B[3+bs*0] = A1[1+bs*0];

	B[2+bs*1] = A1[0+bs*1];
	B[3+bs*1] = A1[1+bs*1];

	B[3+bs*2] = A1[1+bs*2];

	}



// both A and B are aligned to 256-bit boundaries, 3 elements of A must be skipped
void kernel_strcp_l_4_3_lib4(int kmax, float *A0, int sda, float *B)
	{

	// A and C are lower triangular
	// kmax+1 4-wide + end 3x3 triangle

	kmax += 1;

	if(kmax<=0)
		return;

	const int bs = 4;

	float *A1 = A0 + bs*sda;

	int k;

	for(k=0; k<kmax; k++)
		{

		B[0+bs*0] = A0[3+bs*0];
		B[1+bs*0] = A1[0+bs*0];
		B[2+bs*0] = A1[1+bs*0];
		B[3+bs*0] = A1[2+bs*0];

		A0 += 4;
		A1 += 4;
		B  += 4;

		}
	
	// 3x3 triangle

	B[1+bs*0] = A1[0+bs*0];
	B[2+bs*0] = A1[1+bs*0];
	B[3+bs*0] = A1[2+bs*0];

	B[2+bs*1] = A1[1+bs*1];
	B[3+bs*1] = A1[2+bs*1];

	B[3+bs*2] = A1[2+bs*2];

	}



// both A and B are aligned to 64-bit boundaries
void kernel_strcp_l_3_0_lib4(int kmax, float *A, float *B)
	{

	// A and C are lower triangular
	// kmax+1 3-wide + end 2x2 triangle

	kmax += 1;

	if(kmax<=0)
		return;

	const int bs = 4;

	int k;

	for(k=0; k<kmax; k++)
		{

		B[0+bs*0] = A[0+bs*0];
		B[1+bs*0] = A[1+bs*0];
		B[2+bs*0] = A[2+bs*0];

		A += 4;
		B += 4;

		}
	
	// 2x2 triangle

	B[1+bs*0] = A[1+bs*0];
	B[2+bs*0] = A[2+bs*0];

	B[2+bs*1] = A[2+bs*1];

	}



// both A and B are aligned to 256-bit boundaries, 2 elements of A must be skipped
void kernel_strcp_l_3_2_lib4(int kmax, float *A0, int sda, float *B)
	{

	// A and C are lower triangular
	// kmax+1 3-wide + end 2x2 triangle

	kmax += 1;

	if(kmax<=0)
		return;

	const int bs = 4;

	float *A1 = A0 + bs*sda;

	int k;

	for(k=0; k<kmax; k++)
		{

		B[0+bs*0] = A0[2+bs*0];
		B[1+bs*0] = A0[3+bs*0];
		B[2+bs*0] = A1[0+bs*0];

		A0 += 4;
		A1 += 4;
		B  += 4;

		}
	
	// 2x2 triangle

	B[1+bs*0] = A0[3+bs*0];
	B[2+bs*0] = A1[0+bs*0];

	B[2+bs*1] = A1[0+bs*1];

	}



// both A and B are aligned to 256-bit boundaries, 3 elements of A must be skipped
void kernel_strcp_l_3_3_lib4(int kmax, float *A0, int sda, float *B)
	{

	// A and C are lower triangular
	// kmax+1 3-wide + end 2x2 triangle

	kmax += 1;

	if(kmax<=0)
		return;

	const int bs = 4;

	float *A1 = A0 + bs*sda;

	int k;

	for(k=0; k<kmax; k++)
		{

		B[0+bs*0] = A0[3+bs*0];
		B[1+bs*0] = A1[0+bs*0];
		B[2+bs*0] = A1[1+bs*0];

		A0 += 4;
		A1 += 4;
		B  += 4;

		}
	
	// 2x2 triangle

	B[1+bs*0] = A1[0+bs*0];
	B[2+bs*0] = A1[1+bs*0];

	B[2+bs*1] = A1[1+bs*1];

	}



// both A and B are aligned to 64-bit boundaries
void kernel_strcp_l_2_0_lib4(int kmax, float alpha, float *A, float *B)
	{

	// A and C are lower triangular
	// kmax+1 2-wide + end 1x1 triangle

	kmax += 1;

	if(kmax<=0)
		return;

	const int bs = 4;

	int k;

	for(k=0; k<kmax; k++)
		{

		B[0+bs*0] = A[0+bs*0];
		B[1+bs*0] = A[1+bs*0];

		A += 4;
		B += 4;

		}
	
	// 1x1 triangle

	B[1+bs*0] = A[1+bs*0];

	}



// both A and B are aligned to 128-bit boundaries, 3 elements of A must be skipped
void kernel_strcp_l_2_3_lib4(int kmax, float *A0, int sda, float *B)
	{

	// A and C are lower triangular
	// kmax+1 2-wide + end 1x1 triangle

	kmax += 1;

	if(kmax<=0)
		return;

	const int bs = 4;

	float *A1 = A0 + bs*sda;

	int k;

	for(k=0; k<kmax; k++)
		{

		B[0+bs*0] = A0[3+bs*0];
		B[1+bs*0] = A1[0+bs*0];

		A0 += 4;
		A1 += 4;
		B  += 4;

		}
	
	// 1x1 triangle

	B[1+bs*0] = A1[0+bs*0];

	}



// both A and B are aligned 64-bit boundaries
void kernel_strcp_l_1_0_lib4(int kmax, float *A, float *B)
	{

	// A and C are lower triangular
	// kmax+1 1-wide

	kmax += 1;

	if(kmax<=0)
		return;

	const int bs = 4;

	int k;

	for(k=0; k<kmax; k++)
		{

		B[0+bs*0] = A[0+bs*0];

		A += 4;
		B += 4;

		}

	}




// both A and B are aligned to 256-bit boundaries
void kernel_sgead_4_0_lib4(int kmax, float *alphap, float *A, float *B)
	{

	if(kmax<=0)
		return;

	const int bs = 4;

	float alpha = alphap[0];

	int k;

	for(k=0; k<kmax; k++)
		{

		B[0+bs*0] += alpha * A[0+bs*0];
		B[1+bs*0] += alpha * A[1+bs*0];
		B[2+bs*0] += alpha * A[2+bs*0];
		B[3+bs*0] += alpha * A[3+bs*0];

		A += 4;
		B += 4;

		}

	}



// both A and B are aligned to 256-bit boundaries, 1 element of A must be skipped
void kernel_sgead_4_1_lib4(int kmax, float *alphap, float *A0, int sda, float *B)
	{

	if(kmax<=0)
		return;

	const int bs = 4;

	float alpha = alphap[0];

	float *A1 = A0 + bs*sda;

	int k;

	for(k=0; k<kmax; k++)
		{

		B[0+bs*0] += alpha * A0[1+bs*0];
		B[1+bs*0] += alpha * A0[2+bs*0];
		B[2+bs*0] += alpha * A0[3+bs*0];
		B[3+bs*0] += alpha * A1[0+bs*0];

		A0 += 4;
		A1 += 4;
		B  += 4;

		}

	}



// both A and B are aligned to 256-bit boundaries, 2 elements of A must be skipped
void kernel_sgead_4_2_lib4(int kmax, float *alphap, float *A0, int sda, float *B)
	{

	if(kmax<=0)
		return;

	const int bs = 4;

	float alpha = alphap[0];

	float *A1 = A0 + bs*sda;

	int k;

	for(k=0; k<kmax; k++)
		{

		B[0+bs*0] += alpha * A0[2+bs*0];
		B[1+bs*0] += alpha * A0[3+bs*0];
		B[2+bs*0] += alpha * A1[0+bs*0];
		B[3+bs*0] += alpha * A1[1+bs*0];

		A0 += 4;
		A1 += 4;
		B  += 4;

		}

	}



// both A and B are aligned to 256-bit boundaries, 3 elements of A must be skipped
void kernel_sgead_4_3_lib4(int kmax, float *alphap, float *A0, int sda, float *B)
	{

	if(kmax<=0)
		return;

	const int bs = 4;

	float alpha = alphap[0];

	float *A1 = A0 + bs*sda;

	int k;

	for(k=0; k<kmax; k++)
		{

		B[0+bs*0] += alpha * A0[3+bs*0];
		B[1+bs*0] += alpha * A1[0+bs*0];
		B[2+bs*0] += alpha * A1[1+bs*0];
		B[3+bs*0] += alpha * A1[2+bs*0];

		A0 += 4;
		A1 += 4;
		B  += 4;

		}

	}



// both A and B are aligned to 64-bit boundaries
void kernel_sgead_3_0_lib4(int kmax, float *alphap, float *A, float *B)
	{

	if(kmax<=0)
		return;

	const int bs = 4;

	float alpha = alphap[0];

	int k;

	for(k=0; k<kmax; k++)
		{

		B[0+bs*0] += alpha * A[0+bs*0];
		B[1+bs*0] += alpha * A[1+bs*0];
		B[2+bs*0] += alpha * A[2+bs*0];

		A += 4;
		B += 4;

		}

	}



// both A and B are aligned to 256-bit boundaries, 2 elements of A must be skipped
void kernel_sgead_3_2_lib4(int kmax, float *alphap, float *A0, int sda, float *B)
	{

	if(kmax<=0)
		return;

	const int bs = 4;

	float alpha = alphap[0];

	float *A1 = A0 + bs*sda;

	int k;

	for(k=0; k<kmax; k++)
		{

		B[0+bs*0] += alpha * A0[2+bs*0];
		B[1+bs*0] += alpha * A0[3+bs*0];
		B[2+bs*0] += alpha * A1[0+bs*0];

		A0 += 4;
		A1 += 4;
		B  += 4;

		}

	}



// both A and B are aligned to 256-bit boundaries, 3 elements of A must be skipped
void kernel_sgead_3_3_lib4(int kmax, float *alphap, float *A0, int sda, float *B)
	{

	if(kmax<=0)
		return;

	const int bs = 4;

	float alpha = alphap[0];

	float *A1 = A0 + bs*sda;

	int k;

	for(k=0; k<kmax; k++)
		{

		B[0+bs*0] += alpha * A0[3+bs*0];
		B[1+bs*0] += alpha * A1[0+bs*0];
		B[2+bs*0] += alpha * A1[1+bs*0];

		A0 += 4;
		A1 += 4;
		B  += 4;

		}

	}



// both A and B are aligned to 64-bit boundaries
void kernel_sgead_2_0_lib4(int kmax, float *alphap, float *A, float *B)
	{

	if(kmax<=0)
		return;

	const int bs = 4;

	float alpha = alphap[0];

	int k;

	for(k=0; k<kmax; k++)
		{

		B[0+bs*0] += alpha * A[0+bs*0];
		B[1+bs*0] += alpha * A[1+bs*0];

		A += 4;
		B += 4;

		}

	}



// both A and B are aligned to 128-bit boundaries, 3 elements of A must be skipped
void kernel_sgead_2_3_lib4(int kmax, float *alphap, float *A0, int sda, float *B)
	{

	if(kmax<=0)
		return;

	const int bs = 4;

	float alpha = alphap[0];

	float *A1 = A0 + bs*sda;

	int k;

	for(k=0; k<kmax; k++)
		{

		B[0+bs*0] += alpha * A0[3+bs*0];
		B[1+bs*0] += alpha * A1[0+bs*0];

		A0 += 4;
		A1 += 4;
		B  += 4;

		}

	}



// both A and B are aligned 64-bit boundaries
void kernel_sgead_1_0_lib4(int kmax, float *alphap, float *A, float *B)
	{

	if(kmax<=0)
		return;

	const int bs = 4;

	float alpha = alphap[0];

	int k;

	for(k=0; k<kmax; k++)
		{

		B[0+bs*0] += alpha * A[0+bs*0];

		A += 4;
		B += 4;

		}

	}





