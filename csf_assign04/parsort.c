#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>

int compare( const void *left, const void *right );
void swap( int64_t *arr, unsigned long i, unsigned long j );
unsigned long partition( int64_t *arr, unsigned long start, unsigned long end );
int quicksort( int64_t *arr, unsigned long start, unsigned long end, unsigned long par_threshold );
pid_t quicksort_subproc(int64_t *arr,
                        unsigned long start,
                        unsigned long end,
                        unsigned long par_threshold);
int quicksort_wait(pid_t pid);

// TODO: declare additional helper functions if needed

int main( int argc, char **argv ) {
  unsigned long par_threshold;
  if ( argc != 3 || sscanf( argv[2], "%lu", &par_threshold ) != 1 ) {
    fprintf( stderr, "Usage: parsort <file> <par threshold>\n" );
    exit( 1 );
  }

  int fd;

  // open the named file
  // TODO: open the named file
  char* filename = argv[1];
  fd = open(filename, O_RDWR);
  if (fd < 0){
    fprintf(stderr, "File couldn't be opend\n");
    exit(1);
  }
  // determine file size and number of elements
  unsigned long file_size, num_elements;
  // TODO: determine the file size and number of elements
  struct stat statbuf;
  int rc = fstat(fd, &statbuf);
  if (rc != 0){
    fprintf(stderr, "Fstat error occurs\n");
    close(fd);
    exit(1);
  }
  file_size = statbuf.st_size;
  num_elements = file_size/(sizeof(int64_t));
  // mmap the file data
  int64_t *arr;
  // TODO: mmap the file data
  arr = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );
  close(fd);
  if (arr == MAP_FAILED){
    fprintf(stderr, "Mmap error occurs\n");
    exit(1);
  }
  // Sort the data!
  int success;
  success = quicksort( arr, 0, num_elements, par_threshold );
  if ( !success ) {
    fprintf( stderr, "Error: sorting failed\n" );
    exit( 1 );
  }

  // Unmap the file data
  // TODO: unmap the file data
  munmap(arr, file_size);
  return 0;
}

// Compare elements.
// This function can be used as a comparator for a call to qsort.
//
// Parameters:
//   left - pointer to left element
//   right - pointer to right element
//
// Return:
//   negative if *left < *right,
//   positive if *left > *right,
//   0 if *left == *right
int compare( const void *left, const void *right ) {
  int64_t left_elt = *(const int64_t *)left, right_elt = *(const int64_t *)right;
  if ( left_elt < right_elt )
    return -1;
  else if ( left_elt > right_elt )
    return 1;
  else
    return 0;
}

// Swap array elements.
//
// Parameters:
//   arr - pointer to first element of array
//   i - index of element to swap
//   j - index of other element to swap
void swap( int64_t *arr, unsigned long i, unsigned long j ) {
  int64_t tmp = arr[i];
  arr[i] = arr[j];
  arr[j] = tmp;
}

// Partition a region of given array from start (inclusive)
// to end (exclusive).
//
// Parameters:
//   arr - pointer to first element of array
//   start - inclusive lower bound index
//   end - exclusive upper bound index
//
// Return:
//   index of the pivot element, which is globally in the correct place;
//   all elements to the left of the pivot will have values less than
//   the pivot element, and all elements to the right of the pivot will
//   have values greater than or equal to the pivot
unsigned long partition( int64_t *arr, unsigned long start, unsigned long end ) {
  assert( end > start );

  // choose the middle element as the pivot
  unsigned long len = end - start;
  assert( len >= 2 );
  unsigned long pivot_index = start + (len / 2);
  int64_t pivot_val = arr[pivot_index];

  // stash the pivot at the end of the sequence
  swap( arr, pivot_index, end - 1 );

  // partition all of the elements based on how they compare
  // to the pivot element: elements less than the pivot element
  // should be in the left partition, elements greater than or
  // equal to the pivot should go in the right partition
  unsigned long left_index = start,
                right_index = start + ( len - 2 );

  while ( left_index <= right_index ) {
    // extend the left partition?
    if ( arr[left_index] < pivot_val ) {
      ++left_index;
      continue;
    }

    // extend the right partition?
    if ( arr[right_index] >= pivot_val ) {
      --right_index;
      continue;
    }

    // left_index refers to an element that should be in the right
    // partition, and right_index refers to an element that should
    // be in the left partition, so swap them
    swap( arr, left_index, right_index );
  }

  // at this point, left_index points to the first element
  // in the right partition, so place the pivot element there
  // and return the left index, since that's where the pivot
  // element is now
  swap( arr, left_index, end - 1 );
  return left_index;
}

// Sort specified region of array.
// Note that the only reason that sorting should fail is
// if a child process can't be created or if there is any
// other system call failure.
//
// Parameters:
//   arr - pointer to first element of array
//   start - inclusive lower bound index
//   end - exclusive upper bound index
//   par_threshold - if the number of elements in the region is less
//                   than or equal to this threshold, sort sequentially,
//                   otherwise sort in parallel using child processes
//
// Return:
//   1 if the sort was successful, 0 otherwise
int quicksort( int64_t *arr, unsigned long start, unsigned long end, unsigned long par_threshold ) {
  assert( end >= start );
  unsigned long len = end - start;

  // Base case: if there are fewer than 2 elements to sort,
  // do nothing
  if ( len < 2 )
    return 1;

  // Base case: if number of elements is less than or equal to
  // the threshold, sort sequentially using qsort
  if ( len <= par_threshold ) {
    qsort( arr + start, len, sizeof(int64_t), compare );
    return 1;
  }

  // Partition
  unsigned long mid = partition( arr, start, end );

  // Recursively sort the left and right partitions
  int left_success, right_success;
  pid_t left_pid, right_pid;

  // Fork a left child process
  left_pid = quicksort_subproc(arr, start, mid, par_threshold);
  if (left_pid < 0) {
    return 0; // fork failed
  }

  right_pid = quicksort_subproc(arr, mid + 1, end, par_threshold);
  if (right_pid < 0) {
    // If the second fork fails, we should wait for the first child to avoid zombie process
    quicksort_wait(left_pid);
    return 0;
  }

  // Wait for both child processes
  left_success  = quicksort_wait(left_pid);
  right_success = quicksort_wait(right_pid);

  return left_success && right_success;
}

// Fork a child process to do quicksort on [start, end).
// Returns the PID of the child process or 0 if fork failed.
// 
// Parameters:
//   arr - pointer to first element of array
//   start - inclusive lower bound index
//   end - exclusive upper bound index
//   par_threshold - if the number of elements in the region is less
//                   than or equal to this threshold, sort sequentially,
//                   otherwise sort in parallel using child processes
//
// Return:
//   0 if fork failed, the PID of the child process otherwise
pid_t quicksort_subproc(int64_t *arr,
                        unsigned long start,
                        unsigned long end,
                        unsigned long par_threshold) {
  pid_t pid = fork();
  if (pid < 0) {
    return 0; // fork failed
  }
  else if (pid == 0) { //child process
    int ret = quicksort(arr, start, end, par_threshold);

    if (ret == 1) { // Sorting succeed
      exit(EXIT_SUCCESS);
    } else{
      exit(EXIT_FAILURE);
    }
  }
  // Returns child's PID
  return pid;
}

// Wait for the child process to finish.
// 
// Parameters:
//   pid - process id of the process to wait for. 
// 
// Return:
//   1 if successful, 0 otherwise.
int quicksort_wait(pid_t pid) {
  if (pid < 0) {
    return 0; // fork failed
  }
  int wstatus;
  pid_t rc = waitpid(pid, &wstatus, 0);
  if (rc < 0) {
    perror("waitpid");
    return 0;
  }
  // Check if child exited normally --> status 0
  if (!WIFEXITED(wstatus) || (WEXITSTATUS(wstatus) != 0)) {
    return 0; // Child either exit abnormally or with nonzero exit code
  }
  return 1;
}
