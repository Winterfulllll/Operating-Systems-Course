#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define _GREEN_ "\x1b[32m"
#define _RED_ "\x1b[31m"
#define _RESET_ "\x1b[0m"

int CreateProcess();

int main() {
  int pipe1[2], pipe2[2];

  if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
    perror("Pipe creation failed");
    exit(-1);
  }

  // freopen("input.txt", "r", stdin); // for auto-testing

  pid_t pid = CreateProcess();

  if (pid == 0) {
    // CHILD PROCESS
    close(pipe1[1]);
    close(pipe2[0]);

    dup2(pipe1[0], STDIN_FILENO);
    dup2(pipe2[1], STDOUT_FILENO);

    execl("./child", "child", NULL);

    perror("Execl failed");
    exit(1);
  } else {
    // PARENT PROCESS
    close(pipe1[0]);
    close(pipe2[1]);

    char str[BUFFER_SIZE];
    size_t str_sz;

    // Input filename
    printf("Enter the filename for the output: ");
    fgets(str, BUFFER_SIZE, stdin);
    str[strlen(str) - 1] = '\0';
    str_sz = strlen(str);

    if (write(pipe1[1], &str_sz, sizeof(size_t)) < 0 ||
        write(pipe1[1], str, sizeof(char) * str_sz) < 0) {
      perror("Writing failed");
      exit(-1);
    }

    // Input text

    printf("Enter text (press Ctrl+D to send EOF):\n");
    while (fgets(str, BUFFER_SIZE, stdin) != NULL) {
      if (strlen(str) == 1 && str[0] == '\n') {
        break;
      }

      str[strlen(str) - 1] = '\0';
      str_sz = strlen(str);

      if (write(pipe1[1], &str_sz, sizeof(size_t)) < 0 ||
          write(pipe1[1], str, sizeof(char) * str_sz) < 0) {
        perror("Writing failed");
        exit(-1);
      }
    }

    close(pipe1[1]);

    // Wait for child to finish
    wait(NULL);

    // Receive error count from child
    size_t error_count = 0;
    if (read(pipe2[0], &error_count, sizeof(size_t)) < 0) {
      perror("Reading error count failed");
      exit(-1);
    }

    if (error_count > 0) {
      printf(_RED_ "\nThere were %ld invalid lines that did not end with '.' "
                   "or ';'\n" _RESET_,
             error_count);
    } else {
      printf(_GREEN_ "All lines were correct.\n" _RESET_);
    }

    close(pipe2[0]);
  }

  return 0;
}

int CreateProcess() {
  pid_t pid = fork();
  if (pid == -1) {
    perror("Fork failed");
    exit(-1);
  }
  return pid;
}
