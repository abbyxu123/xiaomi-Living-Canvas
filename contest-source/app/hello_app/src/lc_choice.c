#include "lc_choice.h"

#include <stddef.h>

void lc_choice_init(lc_choice_state_t *state)
{
  if (state != NULL)
    {
      state->selected = LC_CHOICE_TAKEOUT;
      state->confirmed = false;
    }
}

void lc_choice_next(lc_choice_state_t *state)
{
  if (state != NULL)
    {
      state->selected = (lc_choice_id_t)((state->selected + 1) %
                                         LC_CHOICE_COUNT);
      state->confirmed = false;
    }
}

void lc_choice_previous(lc_choice_state_t *state)
{
  if (state != NULL)
    {
      state->selected = state->selected == LC_CHOICE_TAKEOUT
                          ? LC_CHOICE_HOME
                          : (lc_choice_id_t)(state->selected - 1);
      state->confirmed = false;
    }
}

lc_choice_id_t lc_choice_confirm(lc_choice_state_t *state)
{
  if (state == NULL)
    {
      return LC_CHOICE_TAKEOUT;
    }

  state->confirmed = true;
  return state->selected;
}
