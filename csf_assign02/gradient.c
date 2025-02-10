#include <stdint.h>
#include <stdio.h>
#include <assert.h>

int64_t gradient( int64_t x ) {
  assert( x >= 0L );
  assert( x <= 2000L );

  x -= 1000L;

  return 1000000L - x*x;
}

int main( void ) {
  for ( int64_t i = 0L; i <= 2000L; i += 10L ) {
    printf( "gradient(%lu)=%lu\n", i, gradient( i ) );
  }
  return 0;
}
