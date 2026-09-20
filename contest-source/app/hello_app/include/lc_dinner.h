#ifndef LIVING_CANVAS_LC_DINNER_H
#define LIVING_CANVAS_LC_DINNER_H

#include "lc_state.h"

#include <stdbool.h>
#include <stddef.h>

#define LC_DINNER_MAX_CANDIDATES 3u
#define LC_DINNER_NAME_MAX 48u
#define LC_DINNER_REASON_MAX 96u
#define LC_DINNER_REPLY_MAX 160u
#define LC_DINNER_QR_TARGET_MAX 128u

typedef enum
{
  LC_ALLERGEN_NONE = 0u,
  LC_ALLERGEN_PEANUT = 1u << 0,
  LC_ALLERGEN_DAIRY = 1u << 1,
  LC_ALLERGEN_EGG = 1u << 2,
  LC_ALLERGEN_SHELLFISH = 1u << 3
} lc_allergen_t;

typedef struct
{
  char name[LC_DINNER_NAME_MAX];
  char reason[LC_DINNER_REASON_MAX];
  char qr_target[LC_DINNER_QR_TARGET_MAX];
  unsigned int price_cents;
  unsigned int allergen_flags;
  bool spicy;
  bool requires_cooking;
} lc_dinner_candidate_t;

typedef struct
{
  bool budget_known;
  unsigned int max_budget_cents;
  unsigned int excluded_allergen_flags;
  bool allow_spicy;
  bool allow_cooking;
} lc_dinner_constraints_t;

typedef struct
{
  char reply[LC_DINNER_REPLY_MAX];
  lc_dinner_candidate_t candidates[LC_DINNER_MAX_CANDIDATES];
  size_t candidate_count;
  lc_state_t next_state;
  lc_actions_t actions;
} lc_dinner_result_t;

typedef enum
{
  LC_DINNER_OK = 0,
  LC_DINNER_NEEDS_BUDGET,
  LC_DINNER_NO_MATCH,
  LC_DINNER_INVALID
} lc_dinner_status_t;

lc_dinner_status_t lc_dinner_select(const lc_dinner_candidate_t *catalog,
                                    size_t catalog_count,
                                    const lc_dinner_constraints_t *constraints,
                                    lc_dinner_result_t *result);
bool lc_dinner_validate_result(const lc_dinner_result_t *result);

#endif
