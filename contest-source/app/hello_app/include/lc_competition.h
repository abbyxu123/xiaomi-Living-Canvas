#ifndef LIVING_CANVAS_LC_COMPETITION_H
#define LIVING_CANVAS_LC_COMPETITION_H

#include "lc_choice.h"

#include <stdbool.h>

#define LC_COMPETITION_DISH_MAX 48u
#define LC_COMPETITION_REASON_MAX 96u
#define LC_COMPETITION_PRICE_MAX 32u
#define LC_COMPETITION_QR_URL_MAX 160u
#define LC_COMPETITION_ERROR_MAX 64u

typedef enum
{
  LC_COMPETITION_CHOICE = 0,
  LC_COMPETITION_REQUESTING,
  LC_COMPETITION_RECOMMENDATION,
  LC_COMPETITION_CONFIRMING,
  LC_COMPETITION_QR,
  LC_COMPETITION_ERROR
} lc_competition_state_t;

typedef struct
{
  lc_competition_state_t state;
  lc_choice_id_t choice;
  bool rules_fallback;
  char dish[LC_COMPETITION_DISH_MAX];
  char reason[LC_COMPETITION_REASON_MAX];
  char price[LC_COMPETITION_PRICE_MAX];
  char qr_url[LC_COMPETITION_QR_URL_MAX];
  char error[LC_COMPETITION_ERROR_MAX];
} lc_competition_t;

void lc_competition_init(lc_competition_t *flow);
bool lc_competition_start(lc_competition_t *flow, lc_choice_id_t choice);
bool lc_competition_set_recommendation(lc_competition_t *flow,
                                       const char *dish,
                                       const char *reason,
                                       const char *price,
                                       bool rules_fallback);
bool lc_competition_confirm(lc_competition_t *flow);
bool lc_competition_set_handoff(lc_competition_t *flow,
                                const char *qr_url);
void lc_competition_fail(lc_competition_t *flow, const char *message);
void lc_competition_cancel(lc_competition_t *flow);

#endif
