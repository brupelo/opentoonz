/*********************************************************************/
/* Copyright 2009, 2010 The University of Texas at Austin.           */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/*   1. Redistributions of source code must retain the above         */
/*      copyright notice, this list of conditions and the following  */
/*      disclaimer.                                                  */
/*                                                                   */
/*   2. Redistributions in binary form must reproduce the above      */
/*      copyright notice, this list of conditions and the following  */
/*      disclaimer in the documentation and/or other materials       */
/*      provided with the distribution.                              */
/*                                                                   */
/*    THIS  SOFTWARE IS PROVIDED  BY THE  UNIVERSITY OF  TEXAS AT    */
/*    AUSTIN  ``AS IS''  AND ANY  EXPRESS OR  IMPLIED WARRANTIES,    */
/*    INCLUDING, BUT  NOT LIMITED  TO, THE IMPLIED  WARRANTIES OF    */
/*    MERCHANTABILITY  AND FITNESS FOR  A PARTICULAR  PURPOSE ARE    */
/*    DISCLAIMED.  IN  NO EVENT SHALL THE UNIVERSITY  OF TEXAS AT    */
/*    AUSTIN OR CONTRIBUTORS BE  LIABLE FOR ANY DIRECT, INDIRECT,    */
/*    INCIDENTAL,  SPECIAL, EXEMPLARY,  OR  CONSEQUENTIAL DAMAGES    */
/*    (INCLUDING, BUT  NOT LIMITED TO,  PROCUREMENT OF SUBSTITUTE    */
/*    GOODS  OR  SERVICES; LOSS  OF  USE,  DATA,  OR PROFITS;  OR    */
/*    BUSINESS INTERRUPTION) HOWEVER CAUSED  AND ON ANY THEORY OF    */
/*    LIABILITY, WHETHER  IN CONTRACT, STRICT  LIABILITY, OR TORT    */
/*    (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY WAY OUT    */
/*    OF  THE  USE OF  THIS  SOFTWARE,  EVEN  IF ADVISED  OF  THE    */
/*    POSSIBILITY OF SUCH DAMAGE.                                    */
/*                                                                   */
/* The views and conclusions contained in the software and           */
/* documentation are those of the authors and should not be          */
/* interpreted as representing official policies, either expressed   */
/* or implied, of The University of Texas at Austin.                 */
/*********************************************************************/

#include <stdio.h>
#include "common.h"

int CNAME(BLASLONG m, BLASLONG n, FLOAT *a, BLASLONG lda, BLASLONG offset, FLOAT *b){

  BLASLONG i, ii, j, jj;

  FLOAT data01, data02, data03, data04;
  FLOAT data05, data06, data07, data08;
  FLOAT data09, data10, data11, data12;
  FLOAT data13, data14, data15, data16;
  FLOAT data17, data18, data19, data20;
  FLOAT data21, data22, data23, data24;
  FLOAT data25, data26, data27, data28;
  FLOAT data29, data30, data31, data32;

  FLOAT *a1, *a2, *a3, *a4;

  lda *= 2;

  jj = offset;

  j = (n >> 2);
  while (j > 0){

    a1 = a + 0 * lda;
    a2 = a + 1 * lda;
    a3 = a + 2 * lda;
    a4 = a + 3 * lda;

    ii = 0;

    i = (m >> 2);
    while (i > 0) {

      if (ii == jj) {
#ifndef UNIT
	data01 = *(a1 + 0);
	data02 = *(a1 + 1);
#endif
	data03 = *(a1 + 2);
	data04 = *(a1 + 3);
	data05 = *(a1 + 4);
	data06 = *(a1 + 5);
	data07 = *(a1 + 6);
	data08 = *(a1 + 7);

#ifndef UNIT
	data11 = *(a2 + 2);
	data12 = *(a2 + 3);
#endif
	data13 = *(a2 + 4);
	data14 = *(a2 + 5);
	data15 = *(a2 + 6);
	data16 = *(a2 + 7);

#ifndef UNIT
	data21 = *(a3 + 4);
	data22 = *(a3 + 5);
#endif
	data23 = *(a3 + 6);
	data24 = *(a3 + 7);

#ifndef UNIT
	data31 = *(a4 + 6);
	data32 = *(a4 + 7);
#endif

	compinv(b +  0, data01, data02);
	*(b +  2) = data03;
	*(b +  3) = data04;
	*(b +  4) = data05;
	*(b +  5) = data06;
	*(b +  6) = data07;
	*(b +  7) = data08;

	compinv(b + 10, data11, data12);
	*(b + 12) = data13;
	*(b + 13) = data14;
	*(b + 14) = data15;
	*(b + 15) = data16;

	compinv(b + 20, data21, data22);
	*(b + 22) = data23;
	*(b + 23) = data24;

	compinv(b + 30, data31, data32);
      }

      if (ii < jj) {
	data01 = *(a1 + 0);
	data02 = *(a1 + 1);
	data03 = *(a1 + 2);
	data04 = *(a1 + 3);
	data05 = *(a1 + 4);
	data06 = *(a1 + 5);
	data07 = *(a1 + 6);
	data08 = *(a1 + 7);

	data09 = *(a2 + 0);
	data10 = *(a2 + 1);
	data11 = *(a2 + 2);
	data12 = *(a2 + 3);
	data13 = *(a2 + 4);
	data14 = *(a2 + 5);
	data15 = *(a2 + 6);
	data16 = *(a2 + 7);

	data17 = *(a3 + 0);
	data18 = *(a3 + 1);
	data19 = *(a3 + 2);
	data20 = *(a3 + 3);
	data21 = *(a3 + 4);
	data22 = *(a3 + 5);
	data23 = *(a3 + 6);
	data24 = *(a3 + 7);

	data25 = *(a4 + 0);
	data26 = *(a4 + 1);
	data27 = *(a4 + 2);
	data28 = *(a4 + 3);
	data29 = *(a4 + 4);
	data30 = *(a4 + 5);
	data31 = *(a4 + 6);
	data32 = *(a4 + 7);

	*(b +  0) = data01;
	*(b +  1) = data02;
	*(b +  2) = data03;
	*(b +  3) = data04;
	*(b +  4) = data05;
	*(b +  5) = data06;
	*(b +  6) = data07;
	*(b +  7) = data08;

	*(b +  8) = data09;
	*(b +  9) = data10;
	*(b + 10) = data11;
	*(b + 11) = data12;
	*(b + 12) = data13;
	*(b + 13) = data14;
	*(b + 14) = data15;
	*(b + 15) = data16;

	*(b + 16) = data17;
	*(b + 17) = data18;
	*(b + 18) = data19;
	*(b + 19) = data20;
	*(b + 20) = data21;
	*(b + 21) = data22;
	*(b + 22) = data23;
	*(b + 23) = data24;

	*(b + 24) = data25;
	*(b + 25) = data26;
	*(b + 26) = data27;
	*(b + 27) = data28;
	*(b + 28) = data29;
	*(b + 29) = data30;
	*(b + 30) = data31;
	*(b + 31) = data32;
      }
      
      a1 += 4 * lda;
      a2 += 4 * lda;
      a3 += 4 * lda;
      a4 += 4 * lda;
      b += 32;

      i  --;
      ii += 4;
    }

    if (m & 2) {

      if (ii == jj) {
#ifndef UNIT
	data01 = *(a1 + 0);
	data02 = *(a1 + 1);
#endif
	data03 = *(a1 + 2);
	data04 = *(a1 + 3);
	data05 = *(a1 + 4);
	data06 = *(a1 + 5);
	data07 = *(a1 + 6);
	data08 = *(a1 + 7);

#ifndef UNIT
	data11 = *(a2 + 2);
	data12 = *(a2 + 3);
#endif
	data13 = *(a2 + 4);
	data14 = *(a2 + 5);
	data15 = *(a2 + 6);
	data16 = *(a2 + 7);

	compinv(b +  0, data01, data02);
	*(b +  2) = data03;
	*(b +  3) = data04;
	*(b +  4) = data05;
	*(b +  5) = data06;
	*(b +  6) = data07;
	*(b +  7) = data08;

	compinv(b + 10, data11, data12);
	*(b + 12) = data13;
	*(b + 13) = data14;
	*(b + 14) = data15;
	*(b + 15) = data16;
      }

      if (ii < jj) {
	data01 = *(a1 + 0);
	data02 = *(a1 + 1);
	data03 = *(a1 + 2);
	data04 = *(a1 + 3);
	data05 = *(a1 + 4);
	data06 = *(a1 + 5);
	data07 = *(a1 + 6);
	data08 = *(a1 + 7);

	data09 = *(a2 + 0);
	data10 = *(a2 + 1);
	data11 = *(a2 + 2);
	data12 = *(a2 + 3);
	data13 = *(a2 + 4);
	data14 = *(a2 + 5);
	data15 = *(a2 + 6);
	data16 = *(a2 + 7);

	*(b +  0) = data01;
	*(b +  1) = data02;
	*(b +  2) = data03;
	*(b +  3) = data04;
	*(b +  4) = data05;
	*(b +  5) = data06;
	*(b +  6) = data07;
	*(b +  7) = data08;

	*(b +  8) = data09;
	*(b +  9) = data10;
	*(b + 10) = data11;
	*(b + 11) = data12;
	*(b + 12) = data13;
	*(b + 13) = data14;
	*(b + 14) = data15;
	*(b + 15) = data16;
      }
      
      a1 += 2 * lda;
      a2 += 2 * lda;
      b += 16;

      ii += 2;
    }

    if (m & 1) {

      if (ii == jj) {
#ifndef UNIT
	data01 = *(a1 + 0);
	data02 = *(a1 + 1);
#endif
	data03 = *(a1 + 2);
	data04 = *(a1 + 3);
	data05 = *(a1 + 4);
	data06 = *(a1 + 5);
	data07 = *(a1 + 6);
	data08 = *(a1 + 7);

	compinv(b +  0, data01, data02);
	*(b +  2) = data03;
	*(b +  3) = data04;
	*(b +  4) = data05;
	*(b +  5) = data06;
	*(b +  6) = data07;
	*(b +  7) = data08;
      }

      if (ii < jj) {
	data01 = *(a1 + 0);
	data02 = *(a1 + 1);
	data03 = *(a1 + 2);
	data04 = *(a1 + 3);
	data05 = *(a1 + 4);
	data06 = *(a1 + 5);
	data07 = *(a1 + 6);
	data08 = *(a1 + 7);

	*(b +  0) = data01;
	*(b +  1) = data02;
	*(b +  2) = data03;
	*(b +  3) = data04;
	*(b +  4) = data05;
	*(b +  5) = data06;
	*(b +  6) = data07;
	*(b +  7) = data08;
      }
      
      a1 += lda;
      b += 8;
      ii += 1;
    }

    a  += 8;
    jj += 4;
    j  --;
  }

  if (n & 2) {

    a1 = a + 0 * lda;
    a2 = a + 1 * lda;

    ii = 0;

    i = (m >> 1);
    while (i > 0) {

      if (ii == jj) {
#ifndef UNIT
	data01 = *(a1 + 0);
	data02 = *(a1 + 1);
#endif
	data03 = *(a1 + 2);
	data04 = *(a1 + 3);

#ifndef UNIT
	data11 = *(a2 + 2);
	data12 = *(a2 + 3);
#endif

	compinv(b +  0, data01, data02);
	*(b +  2) = data03;
	*(b +  3) = data04;
	compinv(b + 6, data11, data12);
      }

      if (ii < jj) {
	data01 = *(a1 + 0);
	data02 = *(a1 + 1);
	data03 = *(a1 + 2);
	data04 = *(a1 + 3);

	data09 = *(a2 + 0);
	data10 = *(a2 + 1);
	data11 = *(a2 + 2);
	data12 = *(a2 + 3);

	*(b +  0) = data01;
	*(b +  1) = data02;
	*(b +  2) = data03;
	*(b +  3) = data04;
	*(b +  4) = data09;
	*(b +  5) = data10;
	*(b +  6) = data11;
	*(b +  7) = data12;
      }
      
      a1 += 2 * lda;
      a2 += 2 * lda;
      b += 8;

      i  --;
      ii += 2;
    }

    if (m & 1) {

      if (ii == jj) {
#ifndef UNIT
	data01 = *(a1 + 0);
	data02 = *(a1 + 1);
#endif
	data03 = *(a1 + 2);
	data04 = *(a1 + 3);

	compinv(b +  0, data01, data02);
	*(b +  2) = data03;
	*(b +  3) = data04;
      }

      if (ii < jj) {
	data01 = *(a1 + 0);
	data02 = *(a1 + 1);
	data03 = *(a1 + 2);
	data04 = *(a1 + 3);

	*(b +  0) = data01;
	*(b +  1) = data02;
	*(b +  2) = data03;
	*(b +  3) = data04;
      }
      
      a1 += lda;
      b += 4;
      ii += 1;
    }

    a  += 4;
    jj += 2;
  }

  if (n & 1) {

    a1 = a + 0 * lda;

    ii = 0;

    i = m;
    while (i > 0) {

      if (ii == jj) {
#ifndef UNIT
	data01 = *(a1 + 0);
	data02 = *(a1 + 1);
#endif
	compinv(b +  0, data01, data02);
      }

      if (ii < jj) {
	data01 = *(a1 + 0);
	data02 = *(a1 + 1);

	*(b +  0) = data01;
	*(b +  1) = data02;
      }
      
      a1 += lda;
      b += 2;

      i  --;
      ii += 1;
    }

    a  += 2;
    jj += 1;
  }

  return 0;
}
