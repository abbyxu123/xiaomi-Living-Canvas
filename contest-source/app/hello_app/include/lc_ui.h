#ifndef LIVING_CANVAS_LC_UI_H
#define LIVING_CANVAS_LC_UI_H

#include "lc_clock.h"
#include "lc_state.h"

#include <stdbool.h>

#define LC_UI_STATE_LABEL_MAX 24u
#define LC_UI_TIME_LABEL_MAX 6u

typedef struct
{
  unsigned int x;
  unsigned int y;
  unsigned int width;
  unsigned int height;
} lc_ui_rect_t;

typedef struct
{
  lc_ui_rect_t clock;
  lc_ui_rect_t weather;
  lc_ui_rect_t cat;
  lc_ui_rect_t dialog;
  lc_ui_rect_t qr;
} lc_ui_layout_t;

typedef struct
{
  bool cat_eyes_open;
  bool qr_visible;
  bool clock_visible;
  char state_label[LC_UI_STATE_LABEL_MAX];
  char time_label[LC_UI_TIME_LABEL_MAX];
  lc_clock_angles_t clock_angles;
} lc_ui_model_t;

typedef enum
{
  LC_UI_OK = 0,
  LC_UI_TIME_UNSYNCED,
  LC_UI_INVALID
} lc_ui_status_t;

lc_ui_status_t lc_ui_build_model(lc_state_t state,
                                 bool time_synced,
                                 unsigned int hour,
                                 unsigned int minute,
                                 lc_ui_model_t *model);
bool lc_ui_compute_layout(unsigned int width,
                          unsigned int height,
                          lc_ui_layout_t *layout);

#endif
