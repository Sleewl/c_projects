/* svd.c
   Singular value decomposition.
*/

/************************************************/
/*                                              */
/*  CMATH.  Copyright (c) 1989 Design Software  */
/*                                              */
/************************************************/


#include "cmath.h"
#if (STDLIBH)
#include <stdlib.h>
#endif
#include <stdio.h>
#include <math.h>

#ifndef NULL
#define  NULL  0
#endif


#define  zero  0.0
#define  one   1.0
#define  two   2.0
#define  MINDX(i,j,rowsize)  (((i)-1) * (rowsize) + (j)-1)
#define  VINDX(j)            ((j)-1)
#define  MIN(a,b)            (((a) < (b)) ? (a) : (b))
#define  MAX(a,b)            (((a) > (b)) ? (a) : (b))
#define  SIGN(a,b)           (((b) >= 0.0) ? fabs(a) : -fabs(a))

/*-----------------------------------------------------------------*/

#if (PROTOTYPE)

int svd (int nm, int m, int n,
         double a[], double w[],
         int matu, double u[],
         int matv, double v[],
         int *ierr)

#else

int svd (nm, m, n, a, w, matu, u, matv, v, ierr)
int nm; int m; int n;
double a[]; double w[];
int matu; double u[];
int matv; double v[];
int *ierr;

#endif

/* Purpose ...
   -------
   This subroutine determines the singular value decomposition
                  T
       a  =  u.s.v
   of a real m by n rectangular matrix.  (T represents the transpose.)
   Householder bidiagonalization and a variant of the qr algorithm
   are used to decompose the matrix a.

   Input ...
   -----
   nm    : must be set to the size of the rows of two-dimensional
           arrays a, u and v(transpose) as declared in the calling
           program.  Note that nm must be at least as large as the
           maximum of m and n.
   m     : is the number of rows of a and u and v(transpose).
   n     : is the number of columns of a and u and the order of v.
   a     : contains the rectangular input matrix to be decomposed.
           a[i*nm + j] i = 0 ... m-1, j = 0 ... n-1
   matu  : should be set to 1 if the u matrix in the decomposition
           is desired, and to 0 otherwise.
   matv  : should be set to 1 if the v matrix in the decomposition
           is desired, and to 0 otherwise.

   Output ...
   ------
   a     : is unaltered (unless overwritten by u or v).
   w     : contains the n (non-negative) singular values of a (the
           diagonal elements of s).  They are unordered.  If an
           error exit is made, the singular values should be correct
           for indices ierr, ierr+1, ..., n-1.
           w[j], j = 0 ... n-1
   u     : contains the matrix u (orthogonal column vectors) of the
           decomposition if matu == 1 otherwise u is used as a
           temporary array.  If you wish to save on storage space, the
           memory occupied by u may coincide that occupied by a.
           If an error exit is made, the columns of u corresponding
           to indices of correct singular values should be correct.
           u[i*nm + j] i = 0 ... m-1, j = 0 ... n-1
   v     : contains the matrix v (orthogonal) of the decomposition if
           matv == 1 otherwise v is not referenced.  The memory occupied
           by v may also coincide with a if u is not needed.  If an error
           exit is made, the columns of v corresponding to indices of
           correct singular values should be correct.
           v[i*nm + j] i = 0 ... m-1, j = 0 ... n-1
   ierr  : status flag
           ierr =  0, normal return
           ierr =  k, if the k-1-th singular value has not been
                      determined after 30 iterations.
           ierr = -1, could not allocate memory for workspace
           ierr = -2, invalid user input.

   Workspace ...
   ---------
   rv1 is a temporary storage array of n double elements.

   This C code written by ...  Peter & Nigel,
   ----------------------      Design Software,
                               42 Gubberley St,
                               Kenmore, 4069,
                               Australia.

   Version ... 1.0, 13 nov 1989
   -------

   Notes ...
   -----
   (1) This program has been adapted from "Computer Methods for
       Mathematical Computations" by Forsythe, Malcolm and Moler
       (1977),pp 229-235.
   (2) That FORTRAN program is in turn a translation of the algol
       procedure svd,
       Num. Math. 14, 403-420(1970) by Golub and Reinsch.
       Handbook for Auto. Comp., Vol II-Linear Algebra, 134-151(1971).

----------------------------------------------------------------------
*/

{  /* begin function svd() */

int i, j, k, L, i1, k1, L1, mn, its;
double c, f, g, h, s, x, y, z, scale, anorm, t;
double *rv1;

*ierr = 0;
rv1 = (double *) NULL;

if (n <= 1 || m <= 1 || (nm < n && nm < m) || a == NULL || w == NULL)
   {
   /* illegal user input */
   *ierr = -2;
   goto LeaveSvd;
   }

rv1 = (double *) malloc (n * sizeof(double));
if (rv1 == NULL)
   {
   *ierr = -1;
   goto LeaveSvd;
   }

for (i = 1; i <= m; ++i)
   {
   for (j = 1; j <= n; ++j) u[MINDX(i,j,nm)] = a[MINDX(i,j,nm)];
   }

/*  Householder reduction to bidiagonal form..... */

g = 0.0;
scale = 0.0;
anorm = 0.0;

for (i = 1; i <= n; ++i)
   {
   L = i + 1;
   rv1[VINDX(i)] = scale * g;
   g = 0.0;
   s = 0.0;
   scale = 0.0;
   if (i <= m)
      {
      for (k = i; k <= m; ++k) scale += fabs(u[MINDX(k,i,nm)]);

      if (scale != 0.0)
         {
         for (k = i; k <= m; ++k)
            {
            t = u[MINDX(k,i,nm)] / scale;
            s = s + t * t;
            u[MINDX(k,i,nm)] = t;
            }

         f = u[MINDX(i,i,nm)];
         g = -( SIGN(sqrt(s), f) );
         h = f * g - s;
         u[MINDX(i,i,nm)] = f - g;
         if (i != n)
            {
            for (j = L; j <= n; ++j)
               {
               s = 0.0;
               for (k = i; k <= m; ++k)
                  s += u[MINDX(k,i,nm)] * u[MINDX(k,j,nm)];
               f = s/h;
               for (k = i; k <= m; ++k)
                  u[MINDX(k,j,nm)] += f * u[MINDX(k,i,nm)];
               }
            }

         for (k = i; k <= m; ++k) u[MINDX(k,i,nm)] *= scale;
         }
      }

   w[VINDX(i)] = scale * g;
   g = 0.0;
   s = 0.0;
   scale = 0.0;
   if ((i <= m) && (i != n))
      {
      for (k = L; k <= n; ++k) scale += fabs(u[MINDX(i,k,nm)]);

      if (scale != 0.0)
         {
         for (k = L; k <= n; ++k)
            {
            t = u[MINDX(i,k,nm)] / scale;
            s = s + t * t;
            u[MINDX(i,k,nm)] = t;
            }
         f = u[MINDX(i,L,nm)];
         g = -( SIGN(sqrt(s), f) );
         h = f * g - s;
         u[MINDX(i,L,nm)] = f - g;
         for (k = L; k <= n; ++k) rv1[VINDX(k)] = u[MINDX(i,k,nm)] / h;
         if (i != m)
            {
            for (j = L; j <= m; ++j)
               {
               s = 0.0;
               for (k = L; k <= n; ++k)
                  s += u[MINDX(j,k,nm)] * u[MINDX(i,k,nm)];
               for (k = L; k <= n; ++k)
                  u[MINDX(j,k,nm)] += s * rv1[VINDX(k)];
               }
            }
         for (k = L; k <= n; ++k) u[MINDX(i,k,nm)] *= scale;
         }
      }

   anorm = MAX( anorm, fabs(w[VINDX(i)]) + fabs(rv1[VINDX(i)]) );
   }

/*  Accumulation of right-hand transformations.... */

if (matv)
   {
   for (i = n; i >= 1; --i)
      {
      if (i != n)
         {
         if (g != 0.0)
            {
            /* Double division avoids possible underflow.... */
            for (j = L; j <= n; ++j)
               v[MINDX(j,i,nm)] = (u[MINDX(i,j,nm)] / u[MINDX(i,L,nm)]) / g;
            for (j = L; j <= n; ++j)
               {
               s = 0.0;
               for (k = L; k <= n; ++k)
                  s += u[MINDX(i,k,nm)] * v[MINDX(k,j,nm)];
               for (k = L; k <= n; ++k)
                  v[MINDX(k,j,nm)] += s * v[MINDX(k,i,nm)];
               }
            }
            for (j = L; j <= n; ++j)
               {
               v[MINDX(i,j,nm)] = 0.0;
               v[MINDX(j,i,nm)] = 0.0;
               }
            }

      v[MINDX(i,i,nm)] = 1.0;
      g = rv1[VINDX(i)];
      L = i;
      }
   }

/*  Accumulation of Left-hand transformations.... */

if (matu)
   {
   /* for i = min(m,n) step -1 until 1 do.... */
   mn = MIN(m, n);
   for (i = mn; i >= 1; --i)
      {
      L = i + 1;
      g = w[VINDX(i)];
      if (i != n)
         {
         for (j = L; j <= n; ++j) u[MINDX(i,j,nm)] = 0.0;
         }

      if (g != 0.0)
         {
         if (i != mn)
            {
            for (j = L; j <= n; ++j)
               {
               s = 0.0;
               for (k = L; k <= m; ++k)
                  s += u[MINDX(k,i,nm)] * u[MINDX(k,j,nm)];
               /* double division avoids possible underflow. */
               f = (s / u[MINDX(i,i,nm)]) / g;
               for (k = i; k <= m; ++k)
                  u[MINDX(k,j,nm)] += f * u[MINDX(k,i,nm)];
               }
            }

         for (j = i; j <= m; ++j) u[MINDX(j,i,nm)] /= g;
         }
      else
         {
         for (j = i; j <= m; ++j)  u[MINDX(j,i,nm)] = 0.0;
         }

      u[MINDX(i,i,nm)] += 1.0;
      }
   }

/* Diagonalization of the bidiagonal form. */

/* for each singular value do... */
for (k = n; k >= 1; --k)
   {
   k1 = k - 1;
   its = 0;

   /* Start of REPEAT Loop ... */
   L520:

   /* Test for splitting. */

   for (L = k; L >= 1; --L)
      {
      L1 = L - 1;
      if ((fabs(rv1[VINDX(L)]) + anorm) == anorm) goto L565;
      /* rv1[VINDX(1)] is always zero, so there is no exit */
      /* through the bottom of the Loop.... */
      if ((fabs(w[VINDX(L1)]) + anorm) == anorm) goto L540;
      }

   /* Cancellation of rv1[VINDX(L)] if L greater than 1 .... */

   L540:
   c = 0.0;
   s = 1.0;

   for (i = L; i <= k; ++i)
      {
      f = s * rv1[VINDX(i)];
      rv1[VINDX(i)] *= c;
      /* Break from this Loop? ... */
      if ((fabs(f) + anorm) == anorm) goto L565;
      g = w[VINDX(i)];
      h = sqrt(f * f + g * g);
      w[VINDX(i)] = h;
      c = g / h;
      s = -f / h;
      if (matu)
         {
         for (j = 1; j <= m; ++j)
            {
            y = u[MINDX(j,L1,nm)];
            z = u[MINDX(j,i,nm)];
            u[MINDX(j,L1,nm)] = y * c + z * s;
            u[MINDX(j,i,nm)] = -y * s + z * c;
            }
         }
      }

   /* Test for convergence. */

   L565:
   z = w[VINDX(k)];
   if (L != k)
      {
      /* shift from bottom 2 by 2 minor.... */
      if (its == 30)
         {
         /* set error -- no convergence to a */
         /* singular value after 30 iterations.... */
         *ierr = k;
         goto LeaveSvd;
         }
      ++its;
      x = w[VINDX(L)];
      y = w[VINDX(k1)];
      g = rv1[VINDX(k1)];
      h = rv1[VINDX(k)];
      f = ((y - z) * (y + z) + (g - h) * (g + h)) / (2.0 * h * y);
      g = sqrt(f * f + 1.0);
      f = ((x - z) * (x + z) + h * (y / (f + SIGN(g,f)) - h)) / x;

      /* Next QR transformation. */
      c = 1.0;
      s = 1.0;
      for (i1 = L; i1 <= k1; ++i1)
         {
         i = i1 + 1;
         g = rv1[VINDX(i)];
         y = w[VINDX(i)];
         h = s * g;
         g = c * g;
         z = sqrt(f * f + h * h);
         rv1[VINDX(i1)] = z;
         c = f / z;
         s = h / z;
         f = x * c + g * s;
         g = -x * s + g * c;
         h = y * s;
         y = y * c;
         if (matv)
            {
            for (j = 1; j <= n; ++j)
               {
               x = v[MINDX(j,i1,nm)];
               z = v[MINDX(j,i,nm)];
               v[MINDX(j,i1,nm)] = x * c + z * s;
               v[MINDX(j,i,nm)] = -x * s + z * c;
               }
            }
         z = sqrt(f * f + h * h);
         w[VINDX(i1)] = z;
         /* Rotation can be arbitrary if z is zero. */
         if (z != 0.0)
            {
            c = f / z;
            s = h / z;
            }
         f = c * g + s * y;
         x = -s * g + c * y;
         if (matu)
            {
            for (j = 1; j <= m; ++j)
               {
               z = u[MINDX(j,i,nm)];
               y = u[MINDX(j,i1,nm)];
               u[MINDX(j,i1,nm)] = y * c + z * s;
               u[MINDX(j,i,nm)] = -y * s + z * c;
               }
            }
         }

      rv1[VINDX(L)] = 0.0;
      rv1[VINDX(k)] = f;
      w[VINDX(k)] = x;
      goto L520;
      }

   /* Convergence. */

   if (z < 0.0)
      {
      /* singular value, w(k) is made non-negative. */
      w[VINDX(k)] = -z;
      if (matv)
         {
         for (j = 1; j <= n; ++j) v[MINDX(j,k,nm)] = -v[MINDX(j,k,nm)];
         }
      }

   }

LeaveSvd:
if (rv1 != NULL) { free(rv1);  rv1 = NULL; }
return (0);
}

/*-----------------------------------------------------------------*/

#if (PROTOTYPE)

int svdsolve (int nm, int m, int n,
    double u[], double w[], double v[], double b[], double x[],
    double tol, int *ierr)

#else

int svdsolve (nm, m, n, u, w, v, b, x, tol, ierr)
int nm; int m; int n;
double u[]; double w[]; double v[];
double b[]; double x[];
double tol;
int *ierr;

#endif

/* Purpose ...
   -------
   Given the singular value decomposition
                  T
       a  =  u.s.v
   of a real m by n rectangular matrix.  (T represents the transpose.)
   This routine generates the solution to the linear system
       a.x = b
   as                 T
       x = v . 1/s . u . b

   Input ...
   -----
   nm    : must be set to the size of the rows of two-dimensional
           arrays a, u and v(transpose) as declared in the calling
           program.  Note that nm must be at least as large as the
           maximum of m and n.
   m     : is the number of rows of a and u and v(transpose).
   n     : is the number of columns of a and u and the order of v.
   u     : contains the matrix u (orthogonal column vectors) of the
           decomposition as returned by svd().
           u[i*nm + j] i = 0 ... m-1, j = 0 ... n-1
   w     : contains the n (non-negative) singular values of a (the
           diagonal elements of s) as returned by svd().
           w[j], j = 0 ... n-1
   v     : contains the matrix v (orthogonal) of the decomposition as
           returned by svd().
           v[i*nm + j] i = 0 ... m-1, j = 0 ... n-1
   b     : The right hand side vector, b[j], j = 0 ... m-1.
   tol   : the tolerance below which the singular values will be
           ignored (or set to zero)

   Output ...
   ------
   x     : The solution vector, x[j], j = 0 ... n-1.
   ierr  : status flag
           ierr =  0, normal return
           ierr = -1, could not allocate memory for workspace
           ierr = -2, invalid user input.

   Workspace ...
   ---------

   This C code written by ...  Peter & Nigel,
   ----------------------      Design Software,
                               42 Gubberley St,
                               Kenmore, 4069,
                               Australia.

   Version ... 1.0, 15 nov 1989
   -------

   Notes ...
   -----

----------------------------------------------------------------------
*/

{  /* begin function svdsolve() */

int i, j;
double s, *tv;

*ierr = 0;
tv = (double *) NULL;

if (n <= 1 || m <= 1 || (nm < n && nm < m) || u == NULL || w == NULL ||
    v == NULL || b == NULL || x == NULL || tol <= 0.0)
   {
   /* illegal user input */
   *ierr = -2;
   goto LeaveSolve;
   }

tv = (double *) malloc (n * sizeof(double));
if (tv == NULL)
   {
   *ierr = -1;
   goto LeaveSolve;
   }

/* Calculate u(transpose) . b. */
for (j = 0; j < n; ++j)
   {
   s = 0.0;
   if (w[j] >= tol)
      {
      /* nonzero result only if w[j] is significant. */
      for (i = 0; i < m; ++i) s += u[i * nm + j] * b[i];
      s /= w[j];  /* divide by singular value */
      }
   tv[j] = s;
   }

/* Matrix multiply by v to get x. */
for (j = 0; j < n; ++j)
   {
   s = 0.0;
   for (i = 0; i < n; ++i) s += v[j * nm + i] * tv[i];
   x[j] = s;
   }

LeaveSolve:
if (tv != NULL) { free(tv);  tv = NULL; }
return (0);
}  /* end of svdsolve() */


