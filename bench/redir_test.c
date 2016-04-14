#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "herbgrind.h"

int main(int argc, char** argv){
  HERBGRIND_BEGIN();
  for (int i = 0; i < 2; ++i)
    sqrt(i);
  HERBGRIND_END();
}
