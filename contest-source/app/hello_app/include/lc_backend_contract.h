#ifndef LIVING_CANVAS_LC_BACKEND_CONTRACT_H
#define LIVING_CANVAS_LC_BACKEND_CONTRACT_H

#include "lc_choice.h"

#include <stdbool.h>
#include <stddef.h>

#define LC_BACKEND_SESSION_ENDPOINT "/v1/session"
#define LC_BACKEND_INPUT_ENDPOINT "/v1/input"
#define LC_BACKEND_CONFIRM_ENDPOINT "/v1/confirm"

typedef struct
{
  const char *input_text;
  const char *channel;
  const char *novelty;
  bool requires_confirmation;
} lc_backend_contract_t;

bool lc_backend_contract_for_choice(lc_choice_id_t choice,
                                    lc_backend_contract_t *contract);
bool lc_backend_build_session_json(const char *device_id,
                                   char *output,
                                   size_t output_capacity);
bool lc_backend_build_input_json(const lc_backend_contract_t *contract,
                                 const char *session_id,
                                 char *output,
                                 size_t output_capacity);
bool lc_backend_build_confirm_json(const char *session_id,
                                   char *output,
                                   size_t output_capacity);

#endif
