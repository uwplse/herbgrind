#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <stdio.h>


double add_numbers(double a, double b, double c) {
    double d = a+b+c;
    return d;
}



int main() {
    double a = 1e16;
    double b = 1;
    double c = -1e16;
    double d = add_numbers(a, b, c);
    // std::cout << d << "\n";
    printf("%e \n", d);
}

