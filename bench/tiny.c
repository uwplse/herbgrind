#include "herbgrind.h"

int main(int argc, char** argv){
  int x;
  HERBGRIND_BEGIN();
  x = 1 + 2;
  HERBGRIND_END();
  return x;
}
