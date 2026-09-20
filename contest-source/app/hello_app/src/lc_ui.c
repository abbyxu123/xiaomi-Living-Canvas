#include "lc_ui.h"

#include <stdio.h>
#include <string.h>

#define LC_UI_MIN_DIMENSION 64u

static unsigned int min_uint(unsigned int first, unsigned int second)
{
  return first < second ? first : second;
}

static const char *state_label(lc_state_t state)
{
  switch (state)
    {
      case LC_STATE_IDLE:
        return "IDLE";
      case LC_STATE_GREETING:
        return "GREETING";
      case LC_STATE_LISTENING:
        return "LISTENING";
      case LC_STATE_THINKING:
        return "THINKING";
      case LC_STATE_RECOMMENDATION:
        return "RECOMMENDATION";
      case LC_STATE_CONFIRMED:
        return "CONFIRMED";
      case LC_STATE_ERROR:
        return "ERROR";
      default:
        return NULL;
    }
}

lc_ui_status_t lc_ui_build_model(lc_state_t state,
                                 bool time_synced,
                                 unsigned int hour,
                                 unsigned int minute,
                                 lc_ui_model_t *model)
{
  const char *label;
  lc_time_status_t time_status;

  if (model == NULL || (label = state_label(state)) == NULL)
    {
      return LC_UI_INVALID;
    }

  memset(model, 0, sizeof(*model));
  snprintf(model->state_label, sizeof(model->state_label), "%s", label);
  model->cat_eyes_open = state != LC_STATE_IDLE;
  model->qr_visible = state == LC_STATE_RECOMMENDATION;

  time_status = lc_clock_compute(time_synced, hour, minute,
                                 &model->clock_angles);
  if (time_status == LC_TIME_UNSYNCED)
    {
      snprintf(model->time_label, sizeof(model->time_label), "--:--");
      model->clock_visible = false;
      return LC_UI_TIME_UNSYNCED;
    }

  if (time_status != LC_TIME_OK)
    {
      return LC_UI_INVALID;
    }

  snprintf(model->time_label, sizeof(model->time_label), "%02u:%02u",
           hour, minute);
  model->clock_visible = true;
  return LC_UI_OK;
}

bool lc_ui_compute_layout(unsigned int width,
                          unsigned int height,
                          lc_ui_layout_t *layout)
{
  unsigned int short_side;
  unsigned int margin;
  unsigned int clock_size;
  unsigned int dialog_height;
  unsigned int qr_size;

  if (layout == NULL || width < LC_UI_MIN_DIMENSION ||
      height < LC_UI_MIN_DIMENSION)
    {
      return false;
    }

  memset(layout, 0, sizeof(*layout));
  short_side = min_uint(width, height);
  margin = short_side / 30u;
  if (margin < 2u)
    {
      margin = 2u;
    }

  clock_size = min_uint(width / 5u, height / 5u);
  dialog_height = height / 4u;

  layout->clock.x = margin;
  layout->clock.y = margin;
  layout->clock.width = clock_size;
  layout->clock.height = clock_size;

  layout->weather.x = margin + clock_size + margin;
  layout->weather.y = margin;
  layout->weather.width = width - layout->weather.x - margin;
  layout->weather.height = clock_size;

  layout->dialog.x = margin;
  layout->dialog.y = height - margin - dialog_height;
  layout->dialog.width = width - 2u * margin;
  layout->dialog.height = dialog_height;

  layout->cat.x = margin;
  layout->cat.y = margin + clock_size + margin;
  layout->cat.width = width - 2u * margin;
  layout->cat.height = layout->dialog.y - margin - layout->cat.y;

  qr_size = min_uint(dialog_height - 2u * margin, width / 5u);
  layout->qr.x = layout->dialog.x + layout->dialog.width - margin - qr_size;
  layout->qr.y = layout->dialog.y + margin;
  layout->qr.width = qr_size;
  layout->qr.height = qr_size;
  return true;
}
