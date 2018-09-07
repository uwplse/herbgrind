#include <stdio.h>
#include <math.h>

double nulltime, TimeArray[3];   /* Variables needed for 'dtime()'.     */
double TLimit;                   /* Threshold to determine Number of    */

double T[36];                    /* Global Array used to hold timing    */

double sa,sb,sc,sd,one,two,three;
double four,five,piref,piprg;
double scale,pierr;

double A0 = 1.0;
double A1 = -0.1666666666671334;
double A2 = 0.833333333809067E-2;
double A3 = 0.198412715551283E-3;
double A4 = 0.27557589750762E-5;
double A5 = 0.2507059876207E-7;
double A6 = 0.164105986683E-9;

double B0 = 1.0;
double B1 = -0.4999999999982;
double B2 = 0.4166666664651E-1;
double B3 = -0.1388888805755E-2;
double B4 = 0.24801428034E-4;
double B5 = -0.2754213324E-6;
double B6 = 0.20189405E-8;

double C0 = 1.0;
double C1 = 0.99999999668;
double C2 = 0.49999995173;
double C3 = 0.16666704243;
double C4 = 0.4166685027E-1;
double C5 = 0.832672635E-2;
double C6 = 0.140836136E-2;
double C7 = 0.17358267E-3;
double C8 = 0.3931683E-4;

double D1 = 0.3999999946405E-1;
double D2 = 0.96E-3;
double D3 = 0.1233153E-5;

double E2 = 0.48E-3;
double E3 = 0.411051E-6;


int main()
{
   double s,u,v,w,x;

   long loops;
   register long i, n;

   printf("\n");

   loops = 15625;        /* Initial number of loops. */


   T[1] = 1.0E+06/(double)loops;

   TLimit = 1.0;

   piref = 3.14159265358979324;
   one   = 1.0;
   two   = 2.0;
   three = 3.0;
   four  = 4.0;
   five  = 5.0;
   scale = one;
   n = 100000;
   sa = 0.0;

   n = 2 * n;
   x = one / (double)n;                            /*********************/
   s = 0.0;                                        /*  Loop 1.          */
   v = 0.0;                                        /*********************/
   w = one;
   for( i = 1 ; i <= n-1 ; i++ )
   {
     v = v + w;
     u = v * x;
     s = s + (D1+u*(D2+u*D3))/(w+u*(D1+u*(E2+u*E3)));
   }

   sa = (D1+D2+D3)/(one+D1+E2+E3);
   sb = D1;

   sa = x * ( sa + sb + two * s ) / two;           /* Module 1 Results. */
   sb = one / sa;
   n  = (long)( (double)( 40000 * (long)sb ) / scale );
   sc = sb - 25.2;
   printf("     1   %13.4lf  %10.4lf  %10.4lf\n",
          sc* /* stabilize output */  1e-30,
          0* /* stabilize output */  1e-30,
          0 * /* stabilize output */  1e-30);
   return 0;
}
