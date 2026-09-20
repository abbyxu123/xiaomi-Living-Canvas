#include "lc_memory.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static void test_only_confirmed_permanent_preferences_are_saved(void)
{
  lc_memory_t memory;
  lc_memory_update_t update = {
    .confirmed = false,
    .remember_budget = true,
    .budget_cents = 3200u,
    .remember_allergens = true,
    .allergen_flags = LC_ALLERGEN_DAIRY,
    .transient_no_spicy = true,
  };

  lc_memory_init(&memory);
  assert(!lc_memory_apply_confirmation(&memory, &update));
  assert(!memory.has_budget);
  assert(memory.allergen_flags == LC_ALLERGEN_NONE);

  update.confirmed = true;
  assert(lc_memory_apply_confirmation(&memory, &update));
  assert(memory.has_budget);
  assert(memory.budget_cents == 3200u);
  assert(memory.allergen_flags == LC_ALLERGEN_DAIRY);
  assert(memory.version == LC_MEMORY_VERSION);
}

static void test_transient_no_spicy_is_not_permanent_memory(void)
{
  lc_memory_t memory;
  lc_memory_update_t update = {
    .confirmed = true,
    .transient_no_spicy = true,
  };

  lc_memory_init(&memory);
  assert(!lc_memory_apply_confirmation(&memory, &update));
  assert(!memory.has_budget);
  assert(memory.allergen_flags == LC_ALLERGEN_NONE);
}

static void test_unknown_allergen_bits_do_not_report_a_change(void)
{
  lc_memory_t memory;
  lc_memory_update_t update = {
    .confirmed = true,
    .remember_allergens = true,
    .allergen_flags = 1u << 31,
  };

  lc_memory_init(&memory);
  assert(!lc_memory_apply_confirmation(&memory, &update));
  assert(memory.allergen_flags == LC_ALLERGEN_NONE);
}

static void test_clear_one_and_clear_all(void)
{
  lc_memory_t memory = {
    .version = LC_MEMORY_VERSION,
    .has_budget = true,
    .budget_cents = 2800u,
    .allergen_flags = LC_ALLERGEN_EGG,
  };

  lc_memory_clear_budget(&memory);
  assert(!memory.has_budget);
  assert(memory.allergen_flags == LC_ALLERGEN_EGG);

  lc_memory_clear_all(&memory);
  assert(memory.version == LC_MEMORY_VERSION);
  assert(!memory.has_budget);
  assert(memory.allergen_flags == LC_ALLERGEN_NONE);
}

static void test_atomic_file_round_trip_and_version_check(void)
{
  char path[160];
  lc_memory_t memory = {
    .version = LC_MEMORY_VERSION,
    .has_budget = true,
    .budget_cents = 4500u,
    .allergen_flags = LC_ALLERGEN_PEANUT | LC_ALLERGEN_SHELLFISH,
  };
  lc_memory_t loaded;
  FILE *file;

  snprintf(path, sizeof(path), "/tmp/lc-memory-test-%ld.txt", (long)getpid());
  unlink(path);
  assert(lc_memory_save(path, &memory) == LC_MEMORY_OK);
  assert(lc_memory_load(path, &loaded) == LC_MEMORY_OK);
  assert(memcmp(&memory, &loaded, sizeof(memory)) == 0);

  file = fopen(path, "w");
  assert(file != NULL);
  fputs("LCM99\nbudget_known=0\nbudget=0\nallergens=0\n", file);
  assert(fclose(file) == 0);
  assert(lc_memory_load(path, &loaded) == LC_MEMORY_UNSUPPORTED_VERSION);

  file = fopen(path, "w");
  assert(file != NULL);
  fputs("LCM1\nbudget_known=0\nbudget=0\nallergens=0\nunexpected\n",
        file);
  assert(fclose(file) == 0);
  assert(lc_memory_load(path, &loaded) == LC_MEMORY_MALFORMED);
  unlink(path);
}

int main(void)
{
  test_only_confirmed_permanent_preferences_are_saved();
  test_transient_no_spicy_is_not_permanent_memory();
  test_unknown_allergen_bits_do_not_report_a_change();
  test_clear_one_and_clear_all();
  test_atomic_file_round_trip_and_version_check();
  puts("PASS: lc_memory");
  return 0;
}
