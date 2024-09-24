#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main() {
  char str[BUFFER_SIZE];
  size_t str_sz;
  FILE *output;
  size_t error_count = 0;
  memset(str, 0, BUFFER_SIZE);

  if (read(STDIN_FILENO, &str_sz, sizeof(size_t)) < 0 ||
      read(STDIN_FILENO, str, sizeof(char) * str_sz) < 0) {
    perror("Reading failed");
    exit(-1);
  }

  output = fopen(str, "w");

  if (output == NULL) {
    perror("The file could not be opened");
    exit(-1);
  }

  while (read(STDIN_FILENO, &str_sz, sizeof(size_t)) > 0) {
    memset(str, 0, BUFFER_SIZE);

    if (read(STDIN_FILENO, str, sizeof(char) * str_sz) < 0) {
      perror("Reading failed");
      exit(-1);
    }

    if (str[str_sz - 1] == '.' || str[str_sz - 1] == ';') {
      fputs(str, output);
      fputs("\n", output);
    } else {
      fprintf(stderr, "Error: line '%s' is invalid\n", str);
    }
  }

  fclose(output);

  if (write(STDOUT_FILENO, &error_count, sizeof(size_t)) < 0) {
    perror("Writing error count to parent failed");
    exit(-1);
  }

  return 0;
}