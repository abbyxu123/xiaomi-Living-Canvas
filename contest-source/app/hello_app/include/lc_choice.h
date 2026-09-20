#ifndef LIVING_CANVAS_LC_CHOICE_H
#define LIVING_CANVAS_LC_CHOICE_H

#include <stdbool.h>

typedef enum
{
  LC_CHOICE_TAKEOUT = 0,
  LC_CHOICE_MYSTERY,
  LC_CHOICE_HOME,
  LC_CHOICE_COUNT
} lc_choice_id_t;

typedef struct
{
  lc_choice_id_t selected;
  bool confirmed;
} lc_choice_state_t;

void lc_choice_init(lc_choice_state_t *state);
void lc_choice_next(lc_choice_state_t *state);
void lc_choice_previous(lc_choice_state_t *state);
lc_choice_id_t lc_choice_confirm(lc_choice_state_t *state);

#endif
