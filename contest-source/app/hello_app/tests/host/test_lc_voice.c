#include "lc_voice.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
  lc_voice_session_t session;

  assert(!lc_voice_init(NULL, 3000u));
  assert(!lc_voice_init(&session, 0u));
  assert(lc_voice_init(&session, 3000u));
  assert(!lc_voice_is_active(&session));

  assert(lc_voice_start(&session, 1000u) == LC_VOICE_OK);
  assert(lc_voice_is_active(&session));
  assert(lc_voice_start(&session, 1001u) == LC_VOICE_BUSY);
  assert(lc_voice_poll(&session, 3999u) == LC_VOICE_ACTIVE);
  assert(lc_voice_poll(&session, 4000u) == LC_VOICE_TIMEOUT);
  assert(!lc_voice_is_active(&session));
  assert(lc_voice_stop(&session) == LC_VOICE_NOT_ACTIVE);

  assert(lc_voice_start(&session, 5000u) == LC_VOICE_OK);
  assert(lc_voice_stop(&session) == LC_VOICE_OK);
  assert(!lc_voice_is_active(&session));

  assert(lc_voice_start(&session, 7000u) == LC_VOICE_OK);
  assert(lc_voice_poll(&session, 6999u) == LC_VOICE_CLOCK_ERROR);
  assert(!lc_voice_is_active(&session));

  puts("PASS: lc_voice");
  return 0;
}
