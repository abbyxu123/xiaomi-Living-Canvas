#include "lc_voice.h"

#include <stddef.h>

bool lc_voice_init(lc_voice_session_t *session, uint32_t max_duration_ms)
{
  if (session == NULL || max_duration_ms == 0u)
    {
      return false;
    }

  session->started_ms = 0u;
  session->max_duration_ms = max_duration_ms;
  session->active = false;
  return true;
}

bool lc_voice_is_active(const lc_voice_session_t *session)
{
  return session != NULL && session->active;
}

lc_voice_status_t lc_voice_start(lc_voice_session_t *session,
                                 uint64_t now_ms)
{
  if (session == NULL || session->max_duration_ms == 0u)
    {
      return LC_VOICE_INVALID;
    }

  if (session->active)
    {
      return LC_VOICE_BUSY;
    }

  session->started_ms = now_ms;
  session->active = true;
  return LC_VOICE_OK;
}

lc_voice_status_t lc_voice_poll(lc_voice_session_t *session,
                                uint64_t now_ms)
{
  if (session == NULL)
    {
      return LC_VOICE_INVALID;
    }

  if (!session->active)
    {
      return LC_VOICE_NOT_ACTIVE;
    }

  if (now_ms < session->started_ms)
    {
      session->active = false;
      return LC_VOICE_CLOCK_ERROR;
    }

  if (now_ms - session->started_ms >= session->max_duration_ms)
    {
      session->active = false;
      return LC_VOICE_TIMEOUT;
    }

  return LC_VOICE_ACTIVE;
}

lc_voice_status_t lc_voice_stop(lc_voice_session_t *session)
{
  if (session == NULL)
    {
      return LC_VOICE_INVALID;
    }

  if (!session->active)
    {
      return LC_VOICE_NOT_ACTIVE;
    }

  session->active = false;
  return LC_VOICE_OK;
}
