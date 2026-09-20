#ifndef LIVING_CANVAS_LC_VOICE_H
#define LIVING_CANVAS_LC_VOICE_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
  LC_VOICE_OK = 0,
  LC_VOICE_ACTIVE,
  LC_VOICE_BUSY,
  LC_VOICE_NOT_ACTIVE,
  LC_VOICE_TIMEOUT,
  LC_VOICE_CLOCK_ERROR,
  LC_VOICE_INVALID
} lc_voice_status_t;

typedef struct
{
  uint64_t started_ms;
  uint32_t max_duration_ms;
  bool active;
} lc_voice_session_t;

bool lc_voice_init(lc_voice_session_t *session, uint32_t max_duration_ms);
bool lc_voice_is_active(const lc_voice_session_t *session);
lc_voice_status_t lc_voice_start(lc_voice_session_t *session,
                                 uint64_t now_ms);
lc_voice_status_t lc_voice_poll(lc_voice_session_t *session,
                                uint64_t now_ms);
lc_voice_status_t lc_voice_stop(lc_voice_session_t *session);

#endif
