#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

typedef int (*gcf_func)(int, int);
typedef float (*square_func)(float, float);

int main() {
  void *gcf_lib = NULL;
  void *square_lib = NULL;

  gcf_func gcf = NULL;
  square_func square = NULL;

  int use_euclid = 1;
  int use_rectangle = 1;

  char command[256];

  printf("\n=================================\n\n");
  printf("Commands:\n");
  printf("0: Switch implementations\n");
  printf("1 <int> <int>: Calculate GCF\n");
  printf("2 <float> <float>: Calculate Square\n");
  printf("3: Show current implementations\n");
  printf("4: Exit\n\n");
  printf("=================================\n\n");

  while (1) {
    printf("Enter command: ");
    fgets(command, sizeof(command), stdin);

    if (command[0] == '0') {
      use_euclid = !use_euclid;
      use_rectangle = !use_rectangle;
      printf("Switched implementations.\n\n");

    } else if (command[0] == '1') {
      int a, b;
      if (sscanf(command, "1 %d %d", &a, &b) != 2) {
        printf("Invalid input. Format: 1 <int> <int>\n\n");
        continue;
      }

      if (use_euclid) {
        if (!gcf_lib) {
          gcf_lib = dlopen("./libgcfeuclid.so", RTLD_LAZY);
        }
        if (!gcf) {
          gcf = dlsym(gcf_lib, "gcf_euclid");
        }
      } else {
        if (!gcf_lib) {
          gcf_lib = dlopen("./libgcfnaive.so", RTLD_LAZY);
        }
        if (!gcf) {
          gcf = dlsym(gcf_lib, "gcf_naive");
        }
      }

      if (!gcf_lib) {
        fprintf(stderr, "Error loading GCF library: %s\n", dlerror());
        return 1;
      }

      if (!gcf) {
        fprintf(stderr, "Error loading GCF symbol: %s\n", dlerror());
        return 1;
      }

      printf("GCF: %d\n\n", gcf(a, b));

    } else if (command[0] == '2') {
      float fa, fb;
      if (sscanf(command, "2 %f %f", &fa, &fb) != 2) {
        printf("Invalid input. Format: 2 <float> <float>\n\n");
        continue;
      }

      if (use_rectangle) {
        if (!square_lib) {
          square_lib = dlopen("./libsquarerectangle.so", RTLD_LAZY);
        }
        if (!square) {
          square = dlsym(square_lib, "square_rectangle");
        }
      } else {
        if (!square_lib) {
          square_lib = dlopen("./libsquaretriangle.so", RTLD_LAZY);
        }
        if (!square) {
          square = dlsym(square_lib, "square_triangle");
        }
      }

      if (!square_lib) {
        fprintf(stderr, "Error loading Square library: %s\n", dlerror());
        return 1;
      }

      if (!square) {
        fprintf(stderr, "Error loading Square symbol: %s\n", dlerror());
        return 1;
      }

      printf("Square: %f\n\n", square(fa, fb));

    } else if (command[0] == '3') {
      printf("Current implementations:\n");
      printf("GCF: %s\n",
             use_euclid ? "Euclid's algorithm" : "Naive algorithm");
      printf("Square: %s\n\n", use_rectangle ? "Rectangle" : "Triangle");

    } else if (command[0] == '4') {
      break;

    } else {
      printf("Unknown command. Valid commands:\n");
      printf("0: Switch implementations\n");
      printf("1 <int> <int>: Calculate GCF\n");
      printf("2 <float> <float>: Calculate Square\n");
      printf("3: Show current implementations\n");
      printf("4: Exit\n\n");
    }
  }

  if (gcf_lib) {
    dlclose(gcf_lib);
  }
  if (square_lib) {
    dlclose(square_lib);
  }

  return 0;
}
