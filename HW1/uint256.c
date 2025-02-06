#include "uint256.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Create a UInt256 value from a single uint32_t value.
// Only the least-significant 32 bits are initialized directly,
// all other bits are set to 0.
UInt256 uint256_create_from_u32(uint32_t val) {
  UInt256 result;
  result.data[0] = val; // set the leftmost to val

  // set the rest to zeros
  for (int i = 1; i < 8; i++) {
    result.data[i] = 0;
  }
  return result;
}

// Create a UInt256 value from an array of NWORDS uint32_t values.
// The element at index 0 is the least significant, and the element
// at index 7 is the most significant.
UInt256 uint256_create(const uint32_t data[8]) {
  UInt256 result;

  for (int i = 0; i < 8; i++) {
    result.data[i] = data[i];
  }
  return result;
}

// Create a UInt256 value from a string of hexadecimal digits.
UInt256 uint256_create_from_hex(const char *hex) {
  UInt256 result;
  int len = strlen(hex);
  const char *start;
  int count;
  int remain;
  // if length is greater than 64, use the rightmost 64
  if (len > 64) {
    start = hex + len - 64;
    count = 8;
    remain = 0;
  } else {
    start = hex;
    count = len / 8;
    remain = len % 8;
  }
  // start from the rightmost 8 char, initailize uint256 value
  int idx = 0;
  while (idx < count) {
    char hex_str[9];
    strncpy(hex_str, start + remain + (count - idx - 1) * 8, 8);
    hex_str[8] = '\0';
    result.data[idx] = strtoul(hex_str, NULL, 16);
    idx += 1;
  }
  if (remain != 0 && idx < 8) {
    char hex_str[remain + 1];
    strncpy(hex_str, start, remain);
    hex_str[remain] = '\0';
    result.data[idx] = strtoul(hex_str, NULL, 16);
    idx += 1;
  }
  // fill 0 for the uninitailized part
  while (idx < 8) {
    result.data[idx] = 0;
    idx += 1;
  }
  return result;
}

// Return a dynamically-allocated string of hex digits representing the
// given UInt256 value.
char *uint256_format_as_hex(UInt256 val) {
  char *hex = NULL;
  // find the part of uint256 to start
  int idx = 7;
  while (idx >= 0 && val.data[idx] == 0) {
    idx--;
  }
  // if uint256 value is 0, output "0"
  if (idx == -1) {
    hex = malloc(2);
    strcpy(hex, "0");
    return hex;
  }
  // get the hex string for each part
  char *strings[idx + 1];
  int sum_len = 0;
  for (int i = 0; i <= idx; i++) {
    strings[i] = malloc(9);
    uint32_t value = val.data[idx - i];
    if (i == 0) {
      sprintf(strings[i], "%x", value);
      sum_len += strlen(strings[i]);
    } else {
      sprintf(strings[i], "%08x", value);
      sum_len += 8;
    }
  }
  // combine the hex strings together as a output
  hex = malloc(sum_len + 1);
  strcpy(hex, strings[0]);
  for (int i = 1; i <= idx; i++) {
    strcat(hex, strings[i]);
  }
  for (int i = 0; i <= idx; i++) {
    free(strings[i]);
  }
  return hex;
}

// Get 32 bits of data from a UInt256 value.
// Index 0 is the least significant 32 bits, index 7 is the most
// significant 32 bits.
uint32_t uint256_get_bits(UInt256 val, unsigned index) {
  uint32_t bits;
  bits = val.data[index];
  return bits;
}

// Return 1 if bit at given index is set, 0 otherwise.
int uint256_is_bit_set(UInt256 val, unsigned index) {
  int bit_idx = index / 32;
  int idx = index % 32;
  int bit = (int)((val.data[bit_idx] << (31 - idx)) >> 31);
  return bit;
}

// Compute the sum of two UInt256 values.
UInt256 uint256_add(UInt256 left, UInt256 right) {
  UInt256 sum;
  int overflow = 0;
  for (int i = 0; i < 8; i++) {
    uint32_t leftpart = left.data[i];
    uint32_t rightpart = right.data[i];
    uint32_t total = leftpart + rightpart + overflow;
    sum.data[i] = total;
    // check whether there is an overflow
    if ((overflow == 0 && total < leftpart) ||
        (overflow == 1 && total <= leftpart)) {
      overflow = 1;
    } else {
      overflow = 0;
    }
  }
  return sum;
}

// Compute the difference of two UInt256 values.
UInt256 uint256_sub(UInt256 left, UInt256 right) {
  UInt256 result;
  // TODO: implement
  UInt256 nright = uint256_negate(right);
  result = uint256_add(left, nright);
  return result;
}

// Return the two's-complement negation of the given UInt256 value.
UInt256 uint256_negate(UInt256 val) {
  UInt256 result;

  // Inverting all bits
  for (int i = 0; i < 8; i++) {
    result.data[i] = ~val.data[i]; // negating all bits for each uint32
  }

  // Add 1 for 2's complement's negation
  UInt256 one = uint256_create_from_u32(1);
  result = uint256_add(result, one);
  return result;
}

// Compute the product of two UInt256 values.
UInt256 uint256_mul(UInt256 left, UInt256 right) {
  // Initialize the product with 0
  UInt256 product = {0};
  
  // Iterate through all bits that has been set
  for (int i = 0; i < 256; i ++){
    if (uint256_is_bit_set(left, i)){
      // left shifting 'right' by the bit position of the bit in 'left'
      UInt256 temp = uint256_lshift(right, i);
      product = uint256_add(product, temp);
    }
  }

  return product;
}

UInt256 uint256_lshift(UInt256 val, unsigned shift) {
  assert(shift < 256); // undefined behavior

  // Initializing the result with 0
  UInt256 result = {0};

  int element_shift = shift / 32; // 32-bits element need to shift
  int bits_shift = shift % 32;    // individual bits need to shift

  if (bits_shift > 0) {
    for (int i = 7; i >= element_shift; i--) {
      // shifting elements and bits
      result.data[i] = val.data[i - element_shift] << bits_shift;

      // handle overflow from the previous element
      if ((i - element_shift) > 0) {
        uint32_t overflow = val.data[i - element_shift - 1] >> (32 - bits_shift);
        result.data[i] = result.data[i] | overflow; //bitwise OR
      }
    }
  } else { // only shift elements (shift is divisible by 32)
    for (int i = 7; i >= element_shift; i--) {
      result.data[i] = val.data[i - element_shift];
    }
  }
  return result;
}
