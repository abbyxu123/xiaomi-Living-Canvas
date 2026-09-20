#ifndef LIVING_CANVAS_LC_CLOCK_H
#define LIVING_CANVAS_LC_CLOCK_H

#include <stdbool.h>

typedef enum
{
  LC_TIME_OK = 0,
  LC_TIME_UNSYNCED,
  LC_TIME_INVALID
} lc_time_status_t;

typedef struct
{
  unsigned int hour_tenths;
  unsigned int minute_tenths;
} lc_clock_angles_t;

lc_time_status_t lc_clock_compute(bool time_synced,
                                  unsigned int hour,
                                  unsigned int minute,
                                  lc_clock_angles_t *angles);

#endif
