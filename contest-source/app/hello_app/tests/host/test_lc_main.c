#define LC_MAIN_HOST_TEST 1
#define main living_canvas_main
#include "../../hello_app_main.c"
#undef main

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

int lc_display_run_preview(void)
{
  return 31;
}

int lc_display_run_image_preview(void)
{
  return 32;
}

int lc_display_run_choice_preview(void)
{
  return 33;
}

int lc_display_run_competition_demo(const char *qr_url)
{
  assert(qr_url != NULL);
  assert(strcmp(qr_url, "http://192.168.1.8:8090/c/demo") == 0);
  return 34;
}

int main(void)
{
  char *argv[] = {"living_canvas", NULL};
  char *image_argv[] = {"living_canvas", "--image-preview", NULL};
  char *choice_argv[] = {"living_canvas", "--choice-preview", NULL};
  char *competition_argv[] = {"living_canvas", "--competition-demo",
                              "http://192.168.1.8:8090/c/demo", NULL};
  char *missing_url_argv[] = {"living_canvas", "--competition-demo", NULL};
  char *diagnostic_argv[] = {"living_canvas", "--ui-preview", NULL};
  char *invalid_argv[] = {"living_canvas", "--unknown", NULL};

  assert(living_canvas_main(1, argv) == 0);
  assert(living_canvas_main(2, image_argv) == 32);
  assert(living_canvas_main(2, choice_argv) == 33);
  assert(living_canvas_main(3, competition_argv) == 34);
  assert(living_canvas_main(2, missing_url_argv) == 2);
  assert(living_canvas_main(2, diagnostic_argv) == 31);
  assert(living_canvas_main(2, invalid_argv) == 2);
  puts("PASS: lc_main");
  return 0;
}
