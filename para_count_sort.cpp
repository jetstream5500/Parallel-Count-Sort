// Author: Michael Carter Orwin
// Date: Monday, February 18th, 2019
// Course: COMP481: Parallel Algorithms

// Note: The `()` in `new int[n]()` sets all values to 0
// Additionally `delete[] a` is needed when using `new`

#include <cstdlib> /*rand*/
#include <fstream> /*fout*/
#include <omp.h> /*omp*/

static int MAX = 300000;

void print_array(int a[], int size, const char * tag = "") {
  for (int i = 0; i<size; i++) {
    if (i == 0) {
      printf("%s{%d, ", tag, a[i]);
    } else if (i < size-1) {
      printf("%d, ", a[i]);
    } else {
      printf("%d}\n", a[i]);
    }
  }
}

void copy_array(int from[], int to[], int size) {
  for (int i = 0; i<size; i++) {
      to[i] = from[i];
    }
}

void generate_random_nums(int a[], int size, int max) {
  for (int i = 0; i<size; i++) {
    a[i] = rand()%(max+1);
  }
}

void seq_count_sort(int a[], int size, int max) {
  int * temp = new int[max+1]();

  for (int i = 0; i<size; i++) {
    temp[a[i]]++;
  }

  for (int i = 1; i<=max; i++) {
    temp[i]+=temp[i-1];
  }

  int prev = 0;
  for (int j = 0; j<max+1; j++) {
    for (int k = 0; k<temp[j]-prev; k++) {
      a[temp[j]-k-1] = j;
    }
    prev = temp[j];
  }

  delete[] temp;
}

void par_count_sort(int a[], int size, int max) {
  int * temp = new int[max+1]();

  #pragma omp parallel for reduction(+:temp[:max+1])
  for (int i = 0; i<size; i++) {
    temp[a[i]]++;
  }

  for (int i = 1; i<=max; i++) {
    temp[i]+=temp[i-1];
  }

  #pragma omp parallel for
  for (int i = 0; i<omp_get_num_threads(); i++) {
    int start;
    int thread_id = omp_get_thread_num();
    int length = (max+1)/omp_get_num_threads();
    int num_extra = (max+1)%omp_get_num_threads();
    if (thread_id < num_extra) {
      length++;
      start = thread_id*length;
    } else {
      start = num_extra*(length+1)+(thread_id-num_extra)*length;
    }

    int prev = 0;
    if (start > 0) {
      prev = temp[start-1];
    }
    for (int j = start; j<start+length; j++) {
      for (int k = 0; k<temp[j]-prev; k++) {
        a[temp[j]-k-1] = j;
      }
      prev = temp[j];
    }
  }

  delete[] temp;
}

int main(int argc, char **argv) {
  double start, end;

  for (int size = 2; size>0; size*=2) {
    printf("-------------------------------\n");
    printf("Size: %d\n", size);
    int * a = new int[size];
    int * b = new int[size];
    generate_random_nums(a, size, MAX);
    copy_array(a, b, size);
    //print_array(a, size, "Original List: ");

    start = omp_get_wtime();
    seq_count_sort(a, size, MAX);
    //print_array(a, size, "Sorted List (Sequential): ");
    end = omp_get_wtime();
    printf("Elapsed Time: %f secs\n", end-start);

    start = omp_get_wtime();
    par_count_sort(b, size, MAX);
    //print_array(b, size, "Sorted List (Parallel): ");
    end = omp_get_wtime();
    printf("Elapsed Time: %f secs\n", end-start);

    delete[] a;
    delete[] b;
  }
  return 0;
}
