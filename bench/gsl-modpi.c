#include <math.h>
#include <stdio.h>

int main() {
  double theta = 7e20;

  const double P1 = 4 * 7.85398125648498535156e-01;
  const double P2 = 4 * 3.77489470793079817668e-08;
  const double P3 = 4 * 2.69515142907905952645e-15;
  const double TwoPi = 2*(P1 + P2 + P3);

  double y = 2*floor(theta/TwoPi);
  double r = ((theta - y*P1) - y*P2) - y*P3;

  printf("%e\n", r);
}
