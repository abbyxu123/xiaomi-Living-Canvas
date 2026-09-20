#include "lc_choice.h"
#include "lc_backend_contract.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_fixed_endpoints(void)
{
  assert(strcmp(LC_BACKEND_SESSION_ENDPOINT, "/v1/session") == 0);
  assert(strcmp(LC_BACKEND_INPUT_ENDPOINT, "/v1/input") == 0);
  assert(strcmp(LC_BACKEND_CONFIRM_ENDPOINT, "/v1/confirm") == 0);
}

static void test_takeout_contract(void)
{
  lc_backend_contract_t contract;
  char json[480];

  assert(lc_backend_contract_for_choice(LC_CHOICE_TAKEOUT, &contract));
  assert(strcmp(contract.channel, "delivery") == 0);
  assert(strcmp(contract.novelty, "balanced") == 0);
  assert(contract.requires_confirmation);
  assert(lc_backend_build_input_json(&contract, "meal_123abc456def", json,
                                 sizeof(json)));
  assert(strstr(json, "\"session_id\":\"meal_123abc456def\"") != NULL);
  assert(strstr(json, "\"channel\":\"delivery\"") != NULL);
  assert(strstr(json, "\"max_total_price_cny\":50") != NULL);
  assert(strstr(json, "\"max_delivery_minutes\":30") != NULL);
}

static void test_session_and_confirmation_payloads(void)
{
  char json[96];

  assert(lc_backend_build_session_json("gemini-s1", json, sizeof(json)));
  assert(strcmp(json, "{\"device_id\":\"gemini-s1\"}") == 0);

  assert(lc_backend_build_confirm_json("meal_12ab34cd56ef", json,
                                   sizeof(json)));
  assert(strcmp(json,
                "{\"session_id\":\"meal_12ab34cd56ef\",\"platform\":\"eleme\"}") == 0);
}

static void test_mystery_and_home_mapping(void)
{
  lc_backend_contract_t mystery;
  lc_backend_contract_t home;

  assert(lc_backend_contract_for_choice(LC_CHOICE_MYSTERY, &mystery));
  assert(strcmp(mystery.channel, "delivery") == 0);
  assert(strcmp(mystery.novelty, "exploratory") == 0);
  assert(strstr(mystery.input_text, "随机") != NULL);

  assert(lc_backend_contract_for_choice(LC_CHOICE_HOME, &home));
  assert(strcmp(home.channel, "pickup") == 0);
  assert(strcmp(home.novelty, "familiar") == 0);
  assert(strstr(home.input_text, "冰箱") != NULL);
}

static void test_rejects_unsafe_or_truncated_session_ids(void)
{
  lc_backend_contract_t contract;
  char json[48];

  assert(lc_backend_contract_for_choice(LC_CHOICE_TAKEOUT, &contract));
  assert(!lc_backend_build_input_json(&contract, "bad\"id", json,
                                  sizeof(json)));
  assert(!lc_backend_build_input_json(&contract, "meal_123abc456def", json,
                                  sizeof(json)));
  assert(!lc_backend_build_session_json("bad\"device", json, sizeof(json)));
  assert(!lc_backend_build_confirm_json("bad/id", json, sizeof(json)));
  assert(!lc_backend_build_confirm_json("meal_123abc456def", json, 8u));
}

int main(void)
{
  test_fixed_endpoints();
  test_takeout_contract();
  test_session_and_confirmation_payloads();
  test_mystery_and_home_mapping();
  test_rejects_unsafe_or_truncated_session_ids();
  puts("PASS: lc_backend_contract");
  return 0;
}
