#ifndef LIVING_CANVAS_LC_MEMORY_H
#define LIVING_CANVAS_LC_MEMORY_H

#include "lc_dinner.h"

#include <stdbool.h>

#define LC_MEMORY_VERSION 1u

typedef struct
{
  unsigned int version;
  bool has_budget;
  unsigned int budget_cents;
  unsigned int allergen_flags;
} lc_memory_t;

typedef struct
{
  bool confirmed;
  bool remember_budget;
  unsigned int budget_cents;
  bool remember_allergens;
  unsigned int allergen_flags;
  bool transient_no_spicy;
} lc_memory_update_t;

typedef enum
{
  LC_MEMORY_OK = 0,
  LC_MEMORY_IO_ERROR,
  LC_MEMORY_MALFORMED,
  LC_MEMORY_UNSUPPORTED_VERSION
} lc_memory_status_t;

void lc_memory_init(lc_memory_t *memory);
bool lc_memory_apply_confirmation(lc_memory_t *memory,
                                  const lc_memory_update_t *update);
void lc_memory_clear_budget(lc_memory_t *memory);
void lc_memory_clear_all(lc_memory_t *memory);
lc_memory_status_t lc_memory_save(const char *path, const lc_memory_t *memory);
lc_memory_status_t lc_memory_load(const char *path, lc_memory_t *memory);

#endif
