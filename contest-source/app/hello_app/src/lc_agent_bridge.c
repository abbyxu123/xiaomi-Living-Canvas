#include "lc_agent_bridge.h"

#include <velaclaw/client.h>

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#define LC_AGENT_CLIENT_NAME_MAX 32u

struct lc_agent_bridge_s
{
  pthread_mutex_t mutex;
  lc_agent_session_t *session;
  velaclaw_client_t *client;
  unsigned int active_id;
  bool closing;
};

static bool valid_client_name(const char *name)
{
  size_t index;

  if (name == NULL || name[0] == '\0')
    {
      return false;
    }

  for (index = 0u; index < LC_AGENT_CLIENT_NAME_MAX; index++)
    {
      if (name[index] == '\0')
        {
          return true;
        }
    }

  return false;
}

static void bridge_reply_callback(int status, const char *reply,
                                  void *cookie)
{
  lc_agent_bridge_t *bridge = cookie;
  lc_agent_status_t result;

  if (bridge == NULL)
    {
      return;
    }

  pthread_mutex_lock(&bridge->mutex);
  if (!bridge->closing)
    {
      result = lc_agent_accept_reply(bridge->session, bridge->active_id,
                                     status, reply);
      if (result == LC_AGENT_OK || result == LC_AGENT_REMOTE_ERROR)
        {
          bridge->active_id = 0u;
        }
    }
  pthread_mutex_unlock(&bridge->mutex);
}

lc_agent_bridge_t *lc_agent_bridge_open(lc_agent_session_t *session,
                                        const char *client_name)
{
  lc_agent_bridge_t *bridge;

  if (session == NULL || !valid_client_name(client_name))
    {
      return NULL;
    }

  bridge = calloc(1u, sizeof(*bridge));
  if (bridge == NULL)
    {
      return NULL;
    }

  if (pthread_mutex_init(&bridge->mutex, NULL) != 0)
    {
      free(bridge);
      return NULL;
    }

  bridge->session = session;
  bridge->client = velaclaw_client_open(client_name);
  if (bridge->client == NULL)
    {
      pthread_mutex_destroy(&bridge->mutex);
      free(bridge);
      return NULL;
    }

  return bridge;
}

bool lc_agent_bridge_close(lc_agent_bridge_t *bridge)
{
  velaclaw_client_t *client;

  if (bridge == NULL)
    {
      return false;
    }

  pthread_mutex_lock(&bridge->mutex);
  if (bridge->closing || bridge->session->status == LC_AGENT_WAITING ||
      bridge->session->status == LC_AGENT_LOCKED)
    {
      pthread_mutex_unlock(&bridge->mutex);
      return false;
    }

  bridge->closing = true;
  client = bridge->client;
  bridge->client = NULL;
  pthread_mutex_unlock(&bridge->mutex);

  velaclaw_client_close(client);
  pthread_mutex_destroy(&bridge->mutex);
  free(bridge);
  return true;
}

lc_agent_status_t lc_agent_bridge_ask(lc_agent_bridge_t *bridge,
                                      const char *text,
                                      uint32_t now_ms,
                                      uint32_t timeout_ms)
{
  lc_agent_request_t request;
  velaclaw_ask_req_t transport_request;
  lc_agent_status_t result;
  int transport_result;

  if (bridge == NULL)
    {
      return LC_AGENT_INVALID;
    }

  pthread_mutex_lock(&bridge->mutex);
  if (bridge->closing)
    {
      pthread_mutex_unlock(&bridge->mutex);
      return LC_AGENT_LOCKED;
    }

  result = lc_agent_begin(bridge->session, text, now_ms, timeout_ms,
                          &request);
  if (result != LC_AGENT_OK)
    {
      pthread_mutex_unlock(&bridge->mutex);
      return result;
    }

  bridge->active_id = request.id;
  pthread_mutex_unlock(&bridge->mutex);

  memset(&transport_request, 0, sizeof(transport_request));
  transport_request.text = request.text;
  transport_request.timeout_ms = (int)request.timeout_ms;
  transport_result = velaclaw_ask(bridge->client, &transport_request,
                                  bridge_reply_callback, bridge);
  if (transport_result < 0)
    {
      pthread_mutex_lock(&bridge->mutex);
      result = lc_agent_accept_reply(bridge->session, bridge->active_id,
                                     transport_result, NULL);
      bridge->active_id = 0u;
      pthread_mutex_unlock(&bridge->mutex);
      return result;
    }

  return LC_AGENT_OK;
}

lc_agent_status_t lc_agent_bridge_poll(lc_agent_bridge_t *bridge,
                                       uint32_t now_ms)
{
  lc_agent_status_t result;

  if (bridge == NULL)
    {
      return LC_AGENT_INVALID;
    }

  pthread_mutex_lock(&bridge->mutex);
  result = lc_agent_poll(bridge->session, now_ms);
  if (result == LC_AGENT_TIMEOUT || result == LC_AGENT_CLOCK_ERROR)
    {
      bridge->active_id = 0u;
    }
  pthread_mutex_unlock(&bridge->mutex);
  return result;
}

lc_agent_status_t lc_agent_bridge_copy_reply(lc_agent_bridge_t *bridge,
                                             char *reply,
                                             size_t reply_capacity)
{
  size_t length;

  if (bridge == NULL || reply == NULL || reply_capacity == 0u)
    {
      return LC_AGENT_INVALID;
    }

  pthread_mutex_lock(&bridge->mutex);
  if (bridge->session->status != LC_AGENT_READY)
    {
      lc_agent_status_t status = bridge->session->status;
      pthread_mutex_unlock(&bridge->mutex);
      return status;
    }

  length = strlen(bridge->session->reply);
  if (length + 1u > reply_capacity)
    {
      pthread_mutex_unlock(&bridge->mutex);
      return LC_AGENT_INVALID;
    }

  memcpy(reply, bridge->session->reply, length + 1u);
  pthread_mutex_unlock(&bridge->mutex);
  return LC_AGENT_OK;
}
