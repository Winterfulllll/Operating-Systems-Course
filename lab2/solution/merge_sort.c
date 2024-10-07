#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define ARR_SIZE 100
int max_threads = 0;

typedef struct {
  int *array;
  int left;
  int right;
} merge_sort_args;

void merge(int *array, int left, int mid, int right) {
  int n1 = mid - left + 1;
  int n2 = right - mid;

  int *L = malloc(n1 * sizeof(int));
  int *R = malloc(n2 * sizeof(int));
  if (L == NULL || R == NULL) {
    perror("Failed to allocate memory in merge");
    exit(1);
  }

  for (int i = 0; i < n1; i++) {
    L[i] = array[left + i];
  }
  for (int j = 0; j < n2; j++) {
    R[j] = array[mid + 1 + j];
  }

  int i = 0, j = 0, k = left;
  while (i < n1 && j < n2) {
    if (L[i] <= R[j]) {
      array[k] = L[i];
      i++;
    } else {
      array[k] = R[j];
      j++;
    }
    k++;
  }

  while (i < n1) {
    array[k] = L[i];
    i++;
    k++;
  }

  while (j < n2) {
    array[k] = R[j];
    j++;
    k++;
  }

  free(L);
  free(R);
}

void merge_sort_sequential(int *array, int left, int right) {
  if (left < right) {
    int mid = left + (right - left) / 2;
    merge_sort_sequential(array, left, mid);
    merge_sort_sequential(array, mid + 1, right);
    merge(array, left, mid, right);
  }
}

void *merge_sort_thread(void *arg) {
  merge_sort_args *args = (merge_sort_args *)arg;
  merge_sort_sequential(args->array, args->left, args->right);
  return NULL;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s threads=<max_threads>\n", argv[0]);
    return 1;
  }

  char *arg = argv[1];
  if (strncmp(arg, "threads=", 8) != 0) {
    printf("Usage: %s threads=<max_threads>\n", argv[0]);
    return 1;
  }

  char *num_str = arg + 8;
  char *endptr;
  max_threads = strtol(num_str, &endptr, 10);

  if (*endptr != '\0' || max_threads <= 0) {
    printf("Invalid thread count: %s\n", num_str);
    return 1;
  }

  int *arr = (int *)malloc(ARR_SIZE * sizeof(int));
  srand(time(NULL));
  for (int i = 0; i < ARR_SIZE; i++) {
    arr[i] = rand() % 1000;
  }

  struct timeval start, end;
  gettimeofday(&start, NULL);

  pthread_t threads[max_threads];
  merge_sort_args args[max_threads];
  int chunk_size = ARR_SIZE / max_threads;

  for (int i = 0; i < max_threads; i++) {
    args[i].array = arr;
    args[i].left = i * chunk_size;
    args[i].right =
        (i == max_threads - 1) ? ARR_SIZE - 1 : (i + 1) * chunk_size - 1;
    pthread_create(&threads[i], NULL, merge_sort_thread, &args[i]);
  }

  for (int i = 0; i < max_threads; i++) {
    pthread_join(threads[i], NULL);
  }

  for (int i = 1; i < max_threads; i++) {
    merge(arr, 0, (i * chunk_size) - 1, (i + 1) * chunk_size - 1);
  }

  gettimeofday(&end, NULL);
  double elapsed_time =
      (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

  // printf("Sorted array:\n");
  // for (int i = 0; i < ARR_SIZE; i++) {
  //   printf("%d ", arr[i]);
  // }
  // printf("\n");

  printf("Elapsed time: %.6f seconds\n", elapsed_time);

  free(arr);
  return 0;
}
