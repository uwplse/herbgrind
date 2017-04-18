#define RANGE   {{0.0, -3.0}, {.25, 3.0}}

// #define SQRT    sqrt

#include "EasyBMP/EasyBMP.h"
#include <complex>
#include <cstdio>
#include <cstdlib>

typedef std::complex<double> cc;
const double pi   = 3.141592653590;
using std::tan; using std::abs; using std::sqrt;

/* Constants */
const cc i(0, 1);

// Bad and good square root implementation
extern inline cc badsqrt(cc z) {
    double r = sqrt(real(z)*real(z) + imag(z)*imag(z));
    double sqrt2 = sqrt(2.0);
    if (imag(z) >= 0) {
      return cc(sqrt(r + real(z)) / sqrt2, sqrt(r - real(z)) / sqrt2);
    } else {
      return cc(sqrt(r + real(z)) / sqrt2, -sqrt(r - real(z)) / sqrt2);
    }
}

extern inline cc goodsqrt(cc z) {
    double r = sqrt(real(z)*real(z) + imag(z)*imag(z));
    double sqrt2 = sqrt(2.0);
    double re, im;
    if (real(z) >= 0) {
        re = sqrt(r + real(z)) / sqrt2;
    } else {
        re = abs(imag(z)) / sqrt(r - real(z)) / sqrt2;
    }
    if (real(z) <= 0) {
        im = sqrt(r - real(z)) / sqrt2;
    } else {
        im = abs(imag(z)) / sqrt(r + real(z)) / sqrt2;
    }
    if (imag(z) >= 0) {
        return cc(re, im);
    } else {
        return cc(re, -im);
    }
}

// Function to plot

extern inline cc f(cc z) {
    cc orig = z;
    z = real(orig) + i * exp(-20.0 + orig);
    z = 1.0 / (sqrt(real(orig)) - SQRT(z));
    return z;
}

// Plotting code

#define OUTPACK(r, g, b) (int(256*r) << 16) + (int(256*g) << 8) + (int(256*b));

inline int hsi2rgb(double H, double I) {
     double r, g, b;

     if(H==1) H = 0;
     double z = floor(H*6); int i = int(z);
     double f = H*6 - z;
     double p = I*(1-I);
     double q = I*(1-I*f);

     if (I > .999) I = .999;
     if (! (i & 1)) {
          q = I + p - q;
     }
 
     switch(i){
     case 0: r=I, g=q, b=p; break;
     case 1: r=q, g=I, b=p; break;
     case 2: r=p, g=I, b=q; break;
     case 3: r=p, g=q, b=I; break;
     case 4: r=q, g=p, b=I; break;
     case 5: r=I, g=p, b=q; break;
     }

     return OUTPACK(r, g, b);
}

inline double decc(cc a) {
     if (imag(a) == 0) {
          return real(a);
     } else {
          return abs(a);
     }
}

BMP image;
double win[2][2] = RANGE;
double xstep;
double ystep;
int size;

int run(int c, int r) {
     cc z = cc(win[0][0] + xstep * c, win[1][1] - ystep * r);
     cc w = log(-f(z));

     double in = atan(real(w)) / pi + .5;
     double hue = imag(w) / (2 * pi) + .5;
     return hsi2rgb(hue, in);
}
	
int main(int argc, char **argv) {
     if (argc < 3) {
         printf("USAGE: plotter <size> <outname.bmp>");
         exit(1);
     }

     size = atoi(argv[1]);
     xstep = (win[1][0] - win[0][0]) / size;
     ystep = (win[1][1] - win[0][1]) / size;

     image.SetSize(size, size);
     cc z = cc(win[0][0], win[1][1]);
     for(int c=0; c < size; c++) {
          for(int r=0; r < size; r++) {
            
               int color = run(c, r);
               RGBApixel* p = image(c, r);
               p->Red = (color >> 16) & 0xff;
               p->Green = (color >> 8) & 0xff;
               p->Blue = color & 0xff;
          }
     }
	
     image.WriteToFile(argv[2]);
     return 0;
}
