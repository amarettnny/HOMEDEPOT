#include "tctest.h"
#include <stdio.h>
#include <stdlib.h>

#include "uint256.h"

typedef struct {
  UInt256 zero;    // the value equal to 0
  UInt256 one;     // the value equal to 1
  UInt256 max;     // the value equal to (2^256)-1
  UInt256 msb_set; // the value equal to 2^255
} TestObjs;

// Helper functions for implementing tests
void set_all(UInt256 *val, uint32_t wordval);

#define ASSERT_SAME(expected, actual)                                          \
  do {                                                                         \
    ASSERT(expected.data[0] == actual.data[0]);                                \
    ASSERT(expected.data[1] == actual.data[1]);                                \
    ASSERT(expected.data[2] == actual.data[2]);                                \
    ASSERT(expected.data[3] == actual.data[3]);                                \
    ASSERT(expected.data[4] == actual.data[4]);                                \
    ASSERT(expected.data[5] == actual.data[5]);                                \
    ASSERT(expected.data[6] == actual.data[6]);                                \
    ASSERT(expected.data[7] == actual.data[7]);                                \
  } while (0)

#define INIT_FROM_ARR(val, arr)                                                \
  do {                                                                         \
    for (int i = 0; i < 8; ++i)                                                \
      val.data[i] = arr[i];                                                    \
  } while (0)

// Functions to create and cleanup the test fixture object
TestObjs *setup(void);
void cleanup(TestObjs *objs);

// Declarations of test functions
void test_get_bits(TestObjs *objs);
void test_is_bit_set(TestObjs *objs);
void test_create_from_u32(TestObjs *objs);
void test_create(TestObjs *objs);
void test_create_from_hex(TestObjs *objs);
void test_format_as_hex(TestObjs *objs);
void test_add(TestObjs *objs);
void test_sub(TestObjs *objs);
void test_negate(TestObjs *objs);
void test_neg_overflow(TestObjs *objs);
void test_mul(TestObjs *objs);
void test_lshift(TestObjs *objs);

// Add new test
void test_add2();
void test_create_from_hex2();
void test_format_as_hex2();
void test_sub2();
void test_negate2();
void test_lshift2(TestObjs *objs);
void test_mul2(TestObjs *objs);

int main(int argc, char **argv) {
  if (argc > 1)
    tctest_testname_to_execute = argv[1];

  TEST_INIT();

  TEST(test_get_bits);
  TEST(test_is_bit_set);
  TEST(test_create_from_u32);
  TEST(test_create);
  TEST(test_create_from_hex);
  TEST(test_format_as_hex);
  TEST(test_add);
  TEST(test_sub);
  TEST(test_negate);
  TEST(test_neg_overflow);
  TEST(test_mul);
  TEST(test_lshift);

  TEST(test_create_from_hex2);
  TEST(test_format_as_hex2);
  TEST(test_add2);
  TEST(test_sub2);
  TEST(test_negate2);
  TEST(test_lshift2);
  TEST(test_mul2);
  TEST_FINI();
}

// Set all of the "words" of a UInt256 to a specific initial value
void set_all(UInt256 *val, uint32_t wordval) {
  for (unsigned i = 0; i < 8; ++i) {
    val->data[i] = wordval;
  }
}

TestObjs *setup(void) {
  TestObjs *objs = (TestObjs *)malloc(sizeof(TestObjs));

  // initialize several UInt256 values "manually"
  set_all(&objs->zero, 0);
  set_all(&objs->one, 0);
  objs->one.data[0] = 1U;
  set_all(&objs->max, 0xFFFFFFFFU);

  // create a value with only the most-significant bit set
  uint32_t msb_set_data[8] = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0x80000000U};
  INIT_FROM_ARR(objs->msb_set, msb_set_data);

  return objs;
}

void cleanup(TestObjs *objs) { free(objs); }

void test_get_bits(TestObjs *objs) {
  ASSERT(0U == uint256_get_bits(objs->zero, 0));
  ASSERT(0U == uint256_get_bits(objs->zero, 1));
  ASSERT(0U == uint256_get_bits(objs->zero, 2));
  ASSERT(0U == uint256_get_bits(objs->zero, 3));
  ASSERT(0U == uint256_get_bits(objs->zero, 4));
  ASSERT(0U == uint256_get_bits(objs->zero, 5));
  ASSERT(0U == uint256_get_bits(objs->zero, 6));
  ASSERT(0U == uint256_get_bits(objs->zero, 7));

  ASSERT(1U == uint256_get_bits(objs->one, 0));
  ASSERT(0U == uint256_get_bits(objs->one, 1));
  ASSERT(0U == uint256_get_bits(objs->one, 2));
  ASSERT(0U == uint256_get_bits(objs->one, 3));
  ASSERT(0U == uint256_get_bits(objs->one, 4));
  ASSERT(0U == uint256_get_bits(objs->one, 5));
  ASSERT(0U == uint256_get_bits(objs->one, 6));
  ASSERT(0U == uint256_get_bits(objs->one, 7));

  ASSERT(0xFFFFFFFFU == uint256_get_bits(objs->max, 0));
  ASSERT(0xFFFFFFFFU == uint256_get_bits(objs->max, 1));
  ASSERT(0xFFFFFFFFU == uint256_get_bits(objs->max, 2));
  ASSERT(0xFFFFFFFFU == uint256_get_bits(objs->max, 3));
  ASSERT(0xFFFFFFFFU == uint256_get_bits(objs->max, 4));
  ASSERT(0xFFFFFFFFU == uint256_get_bits(objs->max, 5));
  ASSERT(0xFFFFFFFFU == uint256_get_bits(objs->max, 6));
  ASSERT(0xFFFFFFFFU == uint256_get_bits(objs->max, 7));
}

void test_is_bit_set(TestObjs *objs) {
  // no bits should be set in the zero value
  for (int i = 0; i < 256; ++i)
    ASSERT(!uint256_is_bit_set(objs->zero, i));

  // only bit 0 should be set in the one value
  ASSERT(uint256_is_bit_set(objs->one, 0));
  for (int i = 1; i < 256; ++i)
    ASSERT(!uint256_is_bit_set(objs->one, i));

  // only bit 255 should be set in the msb_set value
  for (int i = 0; i < 255; ++i)
    ASSERT(!uint256_is_bit_set(objs->msb_set, i));
  ASSERT(uint256_is_bit_set(objs->msb_set, 255));

  // all bits should be set in the max value
  for (int i = 0; i < 256; ++i)
    ASSERT(uint256_is_bit_set(objs->max, i));
}

void test_create_from_u32(TestObjs *objs) {
  UInt256 zero = uint256_create_from_u32(0U);
  UInt256 one = uint256_create_from_u32(1U);

  ASSERT_SAME(objs->zero, zero);
  ASSERT_SAME(objs->one, one);
}

void test_create(TestObjs *objs) {
  (void)objs;

  uint32_t data1[8] = {1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U};
  UInt256 val1 = uint256_create(data1);
  ASSERT(1U == val1.data[0]);
  ASSERT(2U == val1.data[1]);
  ASSERT(3U == val1.data[2]);
  ASSERT(4U == val1.data[3]);
  ASSERT(5U == val1.data[4]);
  ASSERT(6U == val1.data[5]);
  ASSERT(7U == val1.data[6]);
  ASSERT(8U == val1.data[7]);
}

void test_create_from_hex(TestObjs *objs) {
  UInt256 zero = uint256_create_from_hex("0");
  ASSERT_SAME(objs->zero, zero);

  UInt256 one = uint256_create_from_hex("1");
  ASSERT_SAME(objs->one, one);

  UInt256 max = uint256_create_from_hex(
      "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
  ASSERT_SAME(objs->max, max);
}

void test_format_as_hex(TestObjs *objs) {
  char *s;

  s = uint256_format_as_hex(objs->zero);
  ASSERT(0 == strcmp("0", s));
  free(s);

  s = uint256_format_as_hex(objs->one);
  ASSERT(0 == strcmp("1", s));
  free(s);

  s = uint256_format_as_hex(objs->max);
  ASSERT(
      0 ==
      strcmp("ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
             s));
  free(s);
}

void test_add(TestObjs *objs) {
  UInt256 result;

  result = uint256_add(objs->zero, objs->zero);
  ASSERT_SAME(objs->zero, result);

  result = uint256_add(objs->zero, objs->one);
  ASSERT_SAME(objs->one, result);

  uint32_t two_data[8] = {2U};
  UInt256 two;
  INIT_FROM_ARR(two, two_data);
  result = uint256_add(objs->one, objs->one);
  ASSERT_SAME(two, result);

  result = uint256_add(objs->max, objs->one);
  ASSERT_SAME(objs->zero, result);
}

void test_sub(TestObjs *objs) {
  UInt256 result;

  result = uint256_sub(objs->zero, objs->zero);
  ASSERT_SAME(objs->zero, result);

  result = uint256_sub(objs->one, objs->one);
  ASSERT_SAME(objs->zero, result);

  result = uint256_sub(objs->zero, objs->one);
  ASSERT_SAME(objs->max, result);
}

void test_negate(TestObjs *objs) {
  UInt256 result;

  result = uint256_negate(objs->zero);
  ASSERT_SAME(objs->zero, result);

  result = uint256_negate(objs->one);
  ASSERT_SAME(objs->max, result);

  result = uint256_negate(objs->max);
  ASSERT_SAME(objs->one, result);
}

void test_neg_overflow(TestObjs *objs) {
  UInt256 result;

  uint32_t two_data[8] = {2U};
  UInt256 two;
  INIT_FROM_ARR(two, two_data);

  result = uint256_sub(objs->one, objs->max);
  ASSERT_SAME(two, result);
}

void test_mul(TestObjs *objs) {
  UInt256 left, right, result;

  // some very basic tests

  result = uint256_mul(objs->one, objs->one);
  ASSERT_SAME(objs->one, result);

  result = uint256_mul(objs->one, objs->zero);
  ASSERT_SAME(objs->zero, result);

  result = uint256_mul(objs->zero, objs->one);
  ASSERT_SAME(objs->zero, result);

  UInt256 two = {{2, 0, 0, 0, 0, 0, 0, 0}}, four = {{4, 0, 0, 0, 0, 0, 0, 0}};
  result = uint256_mul(two, two);
  ASSERT_SAME(four, result);

  // a more complicated test

  // fc42c691d6284761fb49dd54f3a13eb * 4cfd2c7d8790c50c280ff0ff77617a8e =
  // 4bdd4cc8b6067f7617c05917f828d17a26046ba5f436cb7df595f6c68c00a5a
  left.data[0] = 0x4f3a13ebU;
  left.data[1] = 0x1fb49dd5U;
  left.data[2] = 0x1d628476U;
  left.data[3] = 0xfc42c69U;
  left.data[4] = 0x0U;
  left.data[5] = 0x0U;
  left.data[6] = 0x0U;
  left.data[7] = 0x0U;
  right.data[0] = 0x77617a8eU;
  right.data[1] = 0x280ff0ffU;
  right.data[2] = 0x8790c50cU;
  right.data[3] = 0x4cfd2c7dU;
  right.data[4] = 0x0U;
  right.data[5] = 0x0U;
  right.data[6] = 0x0U;
  right.data[7] = 0x0U;
  result = uint256_mul(left, right);
  ASSERT(0x68c00a5aU == result.data[0]);
  ASSERT(0xdf595f6cU == result.data[1]);
  ASSERT(0x5f436cb7U == result.data[2]);
  ASSERT(0xa26046baU == result.data[3]);
  ASSERT(0x7f828d17U == result.data[4]);
  ASSERT(0x617c0591U == result.data[5]);
  ASSERT(0x8b6067f7U == result.data[6]);
  ASSERT(0x4bdd4ccU == result.data[7]);
}

void test_lshift(TestObjs *objs) {
  UInt256 result;

  UInt256 two = {{2, 0, 0, 0, 0, 0, 0, 0}}, four = {{4, 0, 0, 0, 0, 0, 0, 0}};

  // some very basic tests

  result = uint256_lshift(objs->one, 0);
  ASSERT_SAME(objs->one, result);

  result = uint256_lshift(objs->one, 1);
  ASSERT_SAME(two, result);

  result = uint256_lshift(objs->one, 2);
  ASSERT_SAME(four, result);

  // a more complicated test
  {
    // Test shifting
    // 727767d07ccff5fe25cd125b4523e8c7db1b8d1a2c8a2830284d72bb872c33a5 left by
    // 50 bit(s)
    uint32_t arr[8] = {0x872c33a5U, 0x284d72bbU, 0x2c8a2830U, 0xdb1b8d1aU,
                       0x4523e8c7U, 0x25cd125bU, 0x7ccff5feU, 0x727767d0U};
    UInt256 val;
    INIT_FROM_ARR(val, arr);
    uint32_t expected_arr[8] = {0x00000000U, 0xce940000U, 0xcaee1cb0U,
                                0xa0c0a135U, 0x3468b228U, 0xa31f6c6eU,
                                0x496d148fU, 0xd7f89734U};
    UInt256 expected;
    INIT_FROM_ARR(expected, expected_arr);
    result = uint256_lshift(val, 50U);
    ASSERT_SAME(expected, result);
  }
}

void test_add2() {
  UInt256 left, right, result;
  // aad9bd3a269e7215f3f3ab0b45d8b636e2541961f36a9380d12797ff6a2d5ba +
  // 883d36aadcc660b259e4007b3ab43b5aa470e9c902697f62d4a08afb6fd367d =
  // 13316f3e50364d2c84dd7ab86808cf19186c5032af5d412e3a5c822fada00c37
  left.data[0] = 0xf6a2d5baU;
  left.data[1] = 0x0d12797fU;
  left.data[2] = 0x1f36a938U;
  left.data[3] = 0x6e254196U;
  left.data[4] = 0xb45d8b63U;
  left.data[5] = 0x5f3f3ab0U;
  left.data[6] = 0xa269e721U;
  left.data[7] = 0xaad9bd3U;
  right.data[0] = 0xb6fd367dU;
  right.data[1] = 0x2d4a08afU;
  right.data[2] = 0x902697f6U;
  right.data[3] = 0xaa470e9cU;
  right.data[4] = 0xb3ab43b5U;
  right.data[5] = 0x259e4007U;
  right.data[6] = 0xadcc660bU;
  right.data[7] = 0x883d36aU;
  result = uint256_add(left, right);
  ASSERT(0xada00c37U == result.data[0]);
  ASSERT(0x3a5c822fU == result.data[1]);
  ASSERT(0xaf5d412eU == result.data[2]);
  ASSERT(0x186c5032U == result.data[3]);
  ASSERT(0x6808cf19U == result.data[4]);
  ASSERT(0x84dd7ab8U == result.data[5]);
  ASSERT(0x50364d2cU == result.data[6]);
  ASSERT(0x13316f3eU == result.data[7]);

  // left + 0 = left
  UInt256 result2;
  UInt256 zero = uint256_create_from_hex("0");
  result2 = uint256_add(left, zero);
  ASSERT_SAME(result2, left);

  // left + max = left - 1
  UInt256 result3;
  UInt256 max = uint256_create_from_hex(
      "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
  result3 = uint256_add(left, max);
  ASSERT(0xf6a2d5b9U == result3.data[0]);
  ASSERT(0x0d12797fU == result3.data[1]);
  ASSERT(0x1f36a938U == result3.data[2]);
  ASSERT(0x6e254196U == result3.data[3]);
  ASSERT(0xb45d8b63U == result3.data[4]);
  ASSERT(0x5f3f3ab0U == result3.data[5]);
  ASSERT(0xa269e721U == result3.data[6]);
  ASSERT(0xaad9bd3U == result3.data[7]);
}

void test_create_from_hex2() {
  // input hex string has length 9 (not a multiple of 8)
  UInt256 hex1 = uint256_create_from_hex("111111111");
  ASSERT(0x11111111U == hex1.data[0]);
  ASSERT(0x1U == hex1.data[1]);
  ASSERT(0x0U == hex1.data[2]);
  ASSERT(0x0U == hex1.data[3]);
  ASSERT(0x0U == hex1.data[4]);
  ASSERT(0x0U == hex1.data[5]);
  ASSERT(0x0U == hex1.data[6]);
  ASSERT(0x0U == hex1.data[7]);
  // input hex string has length 65
  UInt256 hex65 = uint256_create_from_hex(
      "10000000000000000000000000000000000000000000000000000000000000000");
  ASSERT(0x0U == hex65.data[0]);
  ASSERT(0x0U == hex65.data[1]);
  ASSERT(0x0U == hex65.data[2]);
  ASSERT(0x0U == hex65.data[3]);
  ASSERT(0x0U == hex65.data[4]);
  ASSERT(0x0U == hex65.data[5]);
  ASSERT(0x0U == hex65.data[6]);
  ASSERT(0x0U == hex65.data[7]);
}

void test_format_as_hex2() {
  // check there is no 0 in the front, and correct number of 0 in the 'middle'
  char *s;
  uint32_t first[8] = {1, 1, 0, 0, 0, 0, 0, 0};
  UInt256 one;
  INIT_FROM_ARR(one, first);
  s = uint256_format_as_hex(one);
  ASSERT(0 == strcmp("100000001", s));
  free(s);

  uint32_t second[8] = {15, 1, 0, 0, 0, 0, 0, 0};
  UInt256 two;
  INIT_FROM_ARR(two, second);
  s = uint256_format_as_hex(two);
  ASSERT(0 == strcmp("10000000f", s));
  free(s);

  uint32_t third[8] = {31, 0, 0, 0, 0, 0, 0, 0};
  UInt256 three;
  INIT_FROM_ARR(three, third);
  s = uint256_format_as_hex(three);
  ASSERT(0 == strcmp("1f", s));
  free(s);
}

void test_sub2() {
  UInt256 left, right, result;
  // 4c6f73ce12e4b57ea94818d6ea352ef8cce425b89dbf782c5d550292821c41f -
  // de2437573a034536735f24fa78bb55efc7f7867e1a7637464e37d77ac8b273 =
  // 3e8d30589f44812b4212268742a97999d064ad50bc1814b7f871851ad5911ac
  left.data[0] = 0x2821c41fU;
  left.data[1] = 0xc5d55029U;
  left.data[2] = 0x89dbf782U;
  left.data[3] = 0x8cce425bU;
  left.data[4] = 0x6ea352efU;
  left.data[5] = 0xea94818dU;
  left.data[6] = 0xe12e4b57U;
  left.data[7] = 0x4c6f73cU;
  right.data[0] = 0x7ac8b273U;
  right.data[1] = 0x464e37d7U;
  right.data[2] = 0x7e1a7637U;
  right.data[3] = 0xefc7f786U;
  right.data[4] = 0xfa78bb55U;
  right.data[5] = 0x36735f24U;
  right.data[6] = 0x573a0345U;
  right.data[7] = 0xde2437U;
  result = uint256_sub(left, right);
  ASSERT(0xad5911acU == result.data[0]);
  ASSERT(0x7f871851U == result.data[1]);
  ASSERT(0x0bc1814bU == result.data[2]);
  ASSERT(0x9d064ad5U == result.data[3]);
  ASSERT(0x742a9799U == result.data[4]);
  ASSERT(0xb4212268U == result.data[5]);
  ASSERT(0x89f44812U == result.data[6]);
  ASSERT(0x3e8d305U == result.data[7]);

  // left - 0 = left
  UInt256 result2;
  UInt256 zero = uint256_create_from_hex("0");
  result2 = uint256_sub(left, zero);
  ASSERT_SAME(result2, left);

  // left - max = left + 1
  UInt256 result3;
  UInt256 max = uint256_create_from_hex(
      "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
  result3 = uint256_sub(left, max);
  ASSERT(0x2821c420U == result3.data[0]);
  ASSERT(0xc5d55029U == result3.data[1]);
  ASSERT(0x89dbf782U == result3.data[2]);
  ASSERT(0x8cce425bU == result3.data[3]);
  ASSERT(0x6ea352efU == result3.data[4]);
  ASSERT(0xea94818dU == result3.data[5]);
  ASSERT(0xe12e4b57U == result3.data[6]);
  ASSERT(0x4c6f73cU == result3.data[7]);
}

void test_negate2() {
  UInt256 result;

  // minuts 2
  uint32_t minus_two_data[8] = {0xFFFFFFFEU, 0xFFFFFFFFU, 0xFFFFFFFFU,
                                0xFFFFFFFFU, 0xFFFFFFFFU, 0xFFFFFFFFU,
                                0xFFFFFFFFU, 0xFFFFFFFFU};
  UInt256 minus_two;
  INIT_FROM_ARR(minus_two, minus_two_data);

  // Expected result of negating -2
  uint32_t two_data[8] = {2U};
  UInt256 two;
  INIT_FROM_ARR(two, two_data);

  result = uint256_negate(minus_two);
  ASSERT_SAME(two, result);
}

void test_lshift2(TestObjs *objs) {
  UInt256 result;

  // test shift by 31
  UInt256 shift_31 = {{1 << 31, 0, 0, 0, 0, 0, 0, 0}};
  result = uint256_lshift(objs->one, 31);
  ASSERT_SAME(shift_31, result);

  // test shift by 32
  UInt256 shift_32 = {{0, 1, 0, 0, 0, 0, 0, 0}};
  result = uint256_lshift(objs->one, 32);
  ASSERT_SAME(shift_32, result);

  // test shift by 33
  UInt256 shift_33 = {{0, 2, 0, 0, 0, 0, 0, 0}};
  result = uint256_lshift(objs->one, 33);
  ASSERT_SAME(shift_33, result);

  // test shift max
  UInt256 one = uint256_create_from_hex("1");
  UInt256 shift_max = uint256_sub(objs->max, one);
  result = uint256_lshift(objs->max, 1);
  ASSERT_SAME(shift_max, result);
}

void test_mul2(TestObjs *objs) {
  UInt256 result;

  // test 0 * 0 = 0
  result = uint256_mul(objs->zero, objs->zero);
  ASSERT_SAME(objs->zero, result);

  // test shift with multiplication (2^32 * 2^64 = 2^96)
  UInt256 shift_32 = uint256_lshift(objs->one, 32);
  UInt256 shift_64 = uint256_lshift(objs->one, 64);
  UInt256 shift_96 = uint256_lshift(objs->one, 96);

  result = uint256_mul(shift_32, shift_64);
  ASSERT_SAME(shift_96, result);
}