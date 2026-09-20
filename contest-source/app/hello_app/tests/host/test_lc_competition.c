#include "lc_competition.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_takeout_happy_path(void)
{
  lc_competition_t flow;

  lc_competition_init(&flow);
  assert(flow.state == LC_COMPETITION_CHOICE);
  assert(flow.choice == LC_CHOICE_TAKEOUT);

  assert(lc_competition_start(&flow, LC_CHOICE_TAKEOUT));
  assert(flow.state == LC_COMPETITION_REQUESTING);

  assert(lc_competition_set_recommendation(
    &flow, "番茄牛腩饭", "符合暖胃和预算要求", "约 32 元", false));
  assert(flow.state == LC_COMPETITION_RECOMMENDATION);
  assert(strcmp(flow.dish, "番茄牛腩饭") == 0);
  assert(strcmp(flow.reason, "符合暖胃和预算要求") == 0);
  assert(strcmp(flow.price, "约 32 元") == 0);
  assert(!flow.rules_fallback);

  assert(lc_competition_confirm(&flow));
  assert(flow.state == LC_COMPETITION_CONFIRMING);

  assert(lc_competition_set_handoff(
    &flow, "http://192.168.1.8:8090/c/12ab34cd56ef"));
  assert(flow.state == LC_COMPETITION_QR);
  assert(strcmp(flow.qr_url,
                "http://192.168.1.8:8090/c/12ab34cd56ef") == 0);
}

static void test_rules_fallback_is_visible(void)
{
  lc_competition_t flow;

  lc_competition_init(&flow);
  assert(lc_competition_start(&flow, LC_CHOICE_TAKEOUT));
  assert(lc_competition_set_recommendation(
    &flow, "鸡肉饭", "网络不可用，使用安全规则", "约 28 元", true));
  assert(flow.rules_fallback);
}

static void test_cancel_returns_to_safe_choice(void)
{
  lc_competition_t flow;

  lc_competition_init(&flow);
  assert(lc_competition_start(&flow, LC_CHOICE_MYSTERY));
  lc_competition_cancel(&flow);
  assert(flow.state == LC_COMPETITION_CHOICE);
  assert(flow.choice == LC_CHOICE_TAKEOUT);
  assert(flow.qr_url[0] == '\0');
}

static void test_gateway_error_is_recoverable(void)
{
  lc_competition_t flow;

  lc_competition_init(&flow);
  assert(lc_competition_start(&flow, LC_CHOICE_TAKEOUT));
  lc_competition_fail(&flow, "gateway timeout");
  assert(flow.state == LC_COMPETITION_ERROR);
  assert(strcmp(flow.error, "gateway timeout") == 0);
  lc_competition_cancel(&flow);
  assert(flow.state == LC_COMPETITION_CHOICE);
}

static void test_invalid_results_are_rejected(void)
{
  lc_competition_t flow;
  char oversized[LC_COMPETITION_DISH_MAX + 2u];

  memset(oversized, 'x', sizeof(oversized));
  oversized[sizeof(oversized) - 1u] = '\0';

  lc_competition_init(&flow);
  assert(!lc_competition_set_recommendation(
    &flow, "dish", "reason", "price", false));
  assert(lc_competition_start(&flow, LC_CHOICE_TAKEOUT));
  assert(!lc_competition_set_recommendation(
    &flow, "", "reason", "price", false));
  assert(!lc_competition_set_recommendation(
    &flow, oversized, "reason", "price", false));
  assert(!lc_competition_set_handoff(&flow, "https://example.com"));
}

int main(void)
{
  test_takeout_happy_path();
  test_rules_fallback_is_visible();
  test_cancel_returns_to_safe_choice();
  test_gateway_error_is_recoverable();
  test_invalid_results_are_rejected();
  puts("PASS: lc_competition");
  return 0;
}
