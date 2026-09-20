#include "lc_clock.h"

#include <assert.h>
#include <stdio.h>

static void expect_angles(unsigned int hour,
                          unsigned int minute,
                          unsigned int hour_tenths,
                          unsigned int minute_tenths)
{
  lc_clock_angles_t angles = {9999u, 9999u};

  assert(lc_clock_compute(true, hour, minute, &angles) == LC_TIME_OK);
  assert(angles.hour_tenths == hour_tenths);
  assert(angles.minute_tenths == minute_tenths);
}

static void test_reference_times(void)
{
  expect_angles(0u, 0u, 0u, 0u);
  expect_angles(3u, 0u, 900u, 0u);
  expect_angles(6u, 30u, 1950u, 1800u);
  expect_angles(12u, 0u, 0u, 0u);
  expect_angles(23u, 59u, 3595u, 3540u);
}

static void test_unsynced_time_does_not_fabricate_angles(void)
{
  lc_clock_angles_t angles = {111u, 222u};

  assert(lc_clock_compute(false, 12u, 34u, &angles) == LC_TIME_UNSYNCED);
  assert(angles.hour_tenths == 111u);
  assert(angles.minute_tenths == 222u);
}

static void test_invalid_time_is_rejected(void)
{
  lc_clock_angles_t angles = {0u, 0u};

  assert(lc_clock_compute(true, 24u, 0u, &angles) == LC_TIME_INVALID);
  assert(lc_clock_compute(true, 12u, 60u, &angles) == LC_TIME_INVALID);
  assert(lc_clock_compute(true, 12u, 0u, NULL) == LC_TIME_INVALID);
}

int main(void)
{
  test_reference_times();
  test_unsynced_time_does_not_fabricate_angles();
  test_invalid_time_is_rejected();
  puts("PASS: lc_clock");
  return 0;
}
