#ifndef LIVING_CANVAS_LC_AGENT_H
#define LIVING_CANVAS_LC_AGENT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define LC_AGENT_INPUT_MAX 256u
#define LC_AGENT_REPLY_MAX 512u
#define LC_AGENT_TIMEOUT_MAX_MS 30000u

typedef enum
{
  LC_AGENT_OK = 0,
  LC_AGENT_IDLE,
  LC_AGENT_WAITING,
  LC_AGENT_READY,
  LC_AGENT_ERROR,
  LC_AGENT_LOCKED,
  LC_AGENT_INVALID,
  LC_AGENT_BUSY,
  LC_AGENT_STALE,
  LC_AGENT_REMOTE_ERROR,
  LC_AGENT_CANCELLED,
  LC_AGENT_TIMEOUT,
  LC_AGENT_CLOCK_ERROR
} lc_agent_status_t;

typedef struct
{
  unsigned int id;
  uint32_t timeout_ms;
  char text[LC_AGENT_INPUT_MAX];
} lc_agent_request_t;

typedef struct
{
  lc_agent_status_t status;
  unsigned int next_id;
  unsigned int active_id;
  uint32_t started_ms;
  uint32_t timeout_ms;
  char reply[LC_AGENT_REPLY_MAX];
} lc_agent_session_t;

bool lc_agent_init(lc_agent_session_t *session);
lc_agent_status_t lc_agent_begin(lc_agent_session_t *session,
                                 const char *text,
                                 uint32_t now_ms,
                                 uint32_t timeout_ms,
                                 lc_agent_request_t *request);
lc_agent_status_t lc_agent_accept_reply(lc_agent_session_t *session,
                                        unsigned int request_id,
                                        int remote_status,
                                        const char *reply);
lc_agent_status_t lc_agent_poll(lc_agent_session_t *session,
                                uint32_t now_ms);
lc_agent_status_t lc_agent_cancel(lc_agent_session_t *session);
lc_agent_status_t lc_agent_transport_reset(lc_agent_session_t *session);

#endif
