#include "lc_competition.h"

#include <stddef.h>
#include <string.h>

static bool copy_bounded(char *destination,
                         size_t capacity,
                         const char *source)
{
  size_t length;

  if (destination == NULL || capacity == 0u || source == NULL ||
      source[0] == '\0')
    {
      return false;
    }

  length = strlen(source);
  if (length >= capacity)
    {
      return false;
    }

  memcpy(destination, source, length + 1u);
  return true;
}

void lc_competition_init(lc_competition_t *flow)
{
  if (flow == NULL)
    {
      return;
    }

  memset(flow, 0, sizeof(*flow));
  flow->state = LC_COMPETITION_CHOICE;
  flow->choice = LC_CHOICE_TAKEOUT;
}

bool lc_competition_start(lc_competition_t *flow, lc_choice_id_t choice)
{
  if (flow == NULL || flow->state != LC_COMPETITION_CHOICE ||
      choice < LC_CHOICE_TAKEOUT || choice >= LC_CHOICE_COUNT)
    {
      return false;
    }

  flow->choice = choice;
  flow->state = LC_COMPETITION_REQUESTING;
  return true;
}

bool lc_competition_set_recommendation(lc_competition_t *flow,
                                       const char *dish,
                                       const char *reason,
                                       const char *price,
                                       bool rules_fallback)
{
  char safe_dish[LC_COMPETITION_DISH_MAX];
  char safe_reason[LC_COMPETITION_REASON_MAX];
  char safe_price[LC_COMPETITION_PRICE_MAX];

  if (flow == NULL || flow->state != LC_COMPETITION_REQUESTING ||
      !copy_bounded(safe_dish, sizeof(safe_dish), dish) ||
      !copy_bounded(safe_reason, sizeof(safe_reason), reason) ||
      !copy_bounded(safe_price, sizeof(safe_price), price))
    {
      return false;
    }

  memcpy(flow->dish, safe_dish, sizeof(safe_dish));
  memcpy(flow->reason, safe_reason, sizeof(safe_reason));
  memcpy(flow->price, safe_price, sizeof(safe_price));
  flow->rules_fallback = rules_fallback;
  flow->state = LC_COMPETITION_RECOMMENDATION;
  return true;
}

bool lc_competition_confirm(lc_competition_t *flow)
{
  if (flow == NULL || flow->state != LC_COMPETITION_RECOMMENDATION)
    {
      return false;
    }

  flow->state = LC_COMPETITION_CONFIRMING;
  return true;
}

bool lc_competition_set_handoff(lc_competition_t *flow,
                                const char *qr_url)
{
  char safe_url[LC_COMPETITION_QR_URL_MAX];

  if (flow == NULL || flow->state != LC_COMPETITION_CONFIRMING ||
      qr_url == NULL ||
      (strncmp(qr_url, "http://", 7u) != 0 &&
       strncmp(qr_url, "https://", 8u) != 0) ||
      !copy_bounded(safe_url, sizeof(safe_url), qr_url))
    {
      return false;
    }

  memcpy(flow->qr_url, safe_url, sizeof(safe_url));
  flow->state = LC_COMPETITION_QR;
  return true;
}

void lc_competition_fail(lc_competition_t *flow, const char *message)
{
  if (flow == NULL)
    {
      return;
    }

  flow->error[0] = '\0';
  if (message != NULL && message[0] != '\0')
    {
      size_t length = strlen(message);

      if (length >= sizeof(flow->error))
        {
          length = sizeof(flow->error) - 1u;
        }

      memcpy(flow->error, message, length);
      flow->error[length] = '\0';
    }

  flow->state = LC_COMPETITION_ERROR;
}

void lc_competition_cancel(lc_competition_t *flow)
{
  lc_competition_init(flow);
}
