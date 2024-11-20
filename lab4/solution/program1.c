#include "libgcf.h"
#include "libsquare.h"
#include <stdio.h>

int main() {
  int a, b;
  float fa, fb;

  printf("Enter two integers for GCF: ");
  scanf("%d %d", &a, &b);
  printf("GCF (Euclid): %d\n", gcf_euclid(a, b));

  printf("Enter two floats for Square: ");
  scanf("%f %f", &fa, &fb);
  printf("Square (Rectangle): %f\n", square_rectangle(fa, fb));

  return 0;
}
