#include "lc_choice.h"

#include <assert.h>
#include <stdio.h>

static void test_default_is_takeout(void)
{
  lc_choice_state_t state;

  lc_choice_init(&state);
  assert(state.selected == LC_CHOICE_TAKEOUT);
  assert(!state.confirmed);
}

static void test_navigation_wraps(void)
{
  lc_choice_state_t state;

  lc_choice_init(&state);
  lc_choice_next(&state);
  assert(state.selected == LC_CHOICE_MYSTERY);
  lc_choice_next(&state);
  assert(state.selected == LC_CHOICE_HOME);
  lc_choice_next(&state);
  assert(state.selected == LC_CHOICE_TAKEOUT);

  lc_choice_previous(&state);
  assert(state.selected == LC_CHOICE_HOME);
  lc_choice_previous(&state);
  assert(state.selected == LC_CHOICE_MYSTERY);
}

static void test_confirm_preserves_selection(void)
{
  lc_choice_state_t state;

  lc_choice_init(&state);
  lc_choice_next(&state);
  assert(lc_choice_confirm(&state) == LC_CHOICE_MYSTERY);
  assert(state.confirmed);
  assert(state.selected == LC_CHOICE_MYSTERY);
}

int main(void)
{
  test_default_is_takeout();
  test_navigation_wraps();
  test_confirm_preserves_selection();
  puts("PASS: lc_choice");
  return 0;
}
