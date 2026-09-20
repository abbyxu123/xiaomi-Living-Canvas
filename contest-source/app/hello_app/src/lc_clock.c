#include "lc_clock.h"

#include <stddef.h>

lc_time_status_t lc_clock_compute(bool time_synced,
                                  unsigned int hour,
                                  unsigned int minute,
                                  lc_clock_angles_t *angles)
{
  if (!time_synced)
    {
      return LC_TIME_UNSYNCED;
    }

  if (angles == NULL || hour >= 24u || minute >= 60u)
    {
      return LC_TIME_INVALID;
    }

  angles->minute_tenths = minute * 60u;
  angles->hour_tenths = (hour % 12u) * 300u + minute * 5u;
  return LC_TIME_OK;
}
