###################################################################################################
#                                                                                                 #
# This file is part of BLASFEO.                                                                   #
#                                                                                                 #
# BLASFEO -- BLAS For Embedded Optimization.                                                      #
# Copyright (C) 2016-2017 by Gianluca Frison.                                                     #
# Developed at IMTEK (University of Freiburg) under the supervision of Moritz Diehl.              #
# All rights reserved.                                                                            #
#                                                                                                 #
# HPMPC is free software; you can redistribute it and/or                                          #
# modify it under the terms of the GNU Lesser General Public                                      #
# License as published by the Free Software Foundation; either                                    #
# version 2.1 of the License, or (at your option) any later version.                              #
#                                                                                                 #
# HPMPC is distributed in the hope that it will be useful,                                        #
# but WITHOUT ANY WARRANTY; without even the implied warranty of                                  #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                                            #
# See the GNU Lesser General Public License for more details.                                     #
#                                                                                                 #
# You should have received a copy of the GNU Lesser General Public                                #
# License along with HPMPC; if not, write to the Free Software                                    #
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA                  #
#                                                                                                 #
# Author: Gianluca Frison, giaf (at) dtu.dk                                                       #
#                          gianluca.frison (at) imtek.uni-freiburg.de                             #
#                                                                                                 #
###################################################################################################

include ../Makefile.rule

ifeq ($(REF_BLAS), 0)
LIBS = -lm 
endif
ifeq ($(REF_BLAS), OPENBLAS)
LIBS = /opt/openblas/lib/libopenblas.a -pthread -lgfortran -lm
endif
ifeq ($(REF_BLAS), BLIS)
LIBS = /opt/netlib/liblapack.a /opt/blis/lib/libblis.a -lgfortran -lm -fopenmp
endif
ifeq ($(REF_BLAS), NETLIB)
LIBS = /opt/netlib/liblapack.a /opt/netlib/libblas.a -lgfortran -lm
endif
ifeq ($(REF_BLAS), MKL)
LIBS = -Wl,--start-group /opt/intel/mkl/lib/intel64/libmkl_gf_lp64.a /opt/intel/mkl/lib/intel64/libmkl_core.a /opt/intel/mkl/lib/intel64/libmkl_sequential.a -Wl,--end-group -ldl -lpthread -lm
endif
ifeq ($(REF_BLAS), ATLAS)
LIBS = /opt/atlas/lib/liblapack.a /opt/atlas/lib/libcblas.a /opt/atlas/lib/libf77blas.a /opt/atlas/lib/libatlas.a -lgfortran -lm
endif

#ifneq ($(NUM_THREAD), 1)
#LIBS += -pthread 
#endif

OBJS_TEST = test_blas_d.o
#OBJS_TEST = test_blas_s.o
#OBJS_TEST = test_d_strmat.o
#OBJS_TEST = test_s_strmat.o
#OBJS_TEST = kernel_assembly.o test_assembly.o

obj: $(OBJS_TEST)
	$(CC) -o test.out $(OBJS_TEST) -L. libblasfeo.a $(LIBS) #-pg

clean:
	rm -f *.o
	rm -f test.out
	rm -f libblasfeo.a

