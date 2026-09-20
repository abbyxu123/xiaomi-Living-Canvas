#include "lc_display.h"
#include "lc_choice.h"
#include "lc_competition.h"
#include "generated/lc_choice_assets.h"

#ifdef __NuttX__

#  include <nuttx/config.h>

#  include <stdbool.h>
#  include <stdint.h>
#  include <stdio.h>
#  include <string.h>
#  include <sys/boardctl.h>
#  include <unistd.h>

#  include <lvgl/lvgl.h>

#  undef NEED_BOARDINIT

#  if defined(CONFIG_BOARDCTL) && !defined(CONFIG_NSH_ARCHINIT)
#    define NEED_BOARDINIT 1
#  endif

static void configure_label(lv_obj_t *label, uint32_t color)
{
  lv_obj_set_width(label, LV_PCT(88));
  lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
  lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
}

static void create_preview(void)
{
  lv_obj_t *screen = lv_screen_active();
  lv_obj_t *panel;
  lv_obj_t *label;

  lv_obj_set_style_bg_color(screen, lv_color_hex(0x160f2f), 0);
  lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

  label = lv_label_create(screen);
  configure_label(label, 0xffffff);
  lv_label_set_text(label, "LIVING CANVAS");
  lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 12);

  label = lv_label_create(screen);
  configure_label(label, 0xc8b9ff);
  lv_label_set_text(label, "SAFE OFFLINE PREVIEW");
  lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 36);

  panel = lv_obj_create(screen);
  lv_obj_remove_style_all(panel);
  lv_obj_set_size(panel, LV_PCT(90), LV_PCT(54));
  lv_obj_align(panel, LV_ALIGN_CENTER, 0, 14);
  lv_obj_set_style_bg_color(panel, lv_color_hex(0x2b2150), 0);
  lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
  lv_obj_set_style_border_color(panel, lv_color_hex(0x8c73ff), 0);
  lv_obj_set_style_border_width(panel, 2, 0);
  lv_obj_set_style_radius(panel, 16, 0);

  label = lv_label_create(panel);
  configure_label(label, 0xffc6df);
  lv_label_set_text(label, "( =^.^= )");
  lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 16);

  label = lv_label_create(panel);
  configure_label(label, 0xffffff);
  lv_label_set_text(label, "Dinner, gently decided.");
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 5);

  label = lv_label_create(panel);
  configure_label(label, 0xc8b9ff);
  lv_label_set_text(label, "Local safety rules are ready.\nAgent bridge is built in.");
  lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -14);

  label = lv_label_create(screen);
  configure_label(label, 0x8ee6c4);
  lv_label_set_text(label, "DISPLAY OK  |  NO DEVICE ACTIONS");
  lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, -12);
}

static lv_obj_t *g_choice_cards[LC_CHOICE_COUNT];
static bool g_pulse_bright;
static lc_competition_t g_competition;
static lv_obj_t *g_competition_panel;
static lv_obj_t *g_competition_title;
static lv_obj_t *g_competition_detail;
static lv_obj_t *g_competition_badge;
#  if LV_USE_QRCODE
static lv_obj_t *g_competition_qr;
#  endif
static lv_timer_t *g_competition_timer;
static unsigned int g_competition_step;
static char g_competition_handoff[LC_COMPETITION_QR_URL_MAX];
static char g_competition_detail_text[LC_COMPETITION_REASON_MAX +
                                      LC_COMPETITION_PRICE_MAX + 4u];

static void init_image_descriptor(lv_image_dsc_t *descriptor,
                                  const uint8_t *data,
                                  uint32_t data_size,
                                  uint32_t width,
                                  uint32_t height)
{
  memset(descriptor, 0, sizeof(*descriptor));
  descriptor->header.magic = LV_IMAGE_HEADER_MAGIC;
  descriptor->header.cf = LV_COLOR_FORMAT_RGB565;
  descriptor->header.w = width;
  descriptor->header.h = height;
  descriptor->header.stride = width * 2u;
  descriptor->data_size = data_size;
  descriptor->data = data;
}

static bool create_background(lv_display_t *display, lv_obj_t *screen)
{
  static lv_image_dsc_t background;
  int32_t width = lv_display_get_horizontal_resolution(display);
  int32_t height = lv_display_get_vertical_resolution(display);
  lv_obj_t *image;

  if (width == (int32_t)LC_BG_PORTRAIT_WIDTH &&
      height == (int32_t)LC_BG_PORTRAIT_HEIGHT)
    {
      init_image_descriptor(&background, lc_bg_portrait,
                            LC_BG_PORTRAIT_BYTES,
                            LC_BG_PORTRAIT_WIDTH,
                            LC_BG_PORTRAIT_HEIGHT);
    }
  else if (width == (int32_t)LC_BG_LANDSCAPE_WIDTH &&
           height == (int32_t)LC_BG_LANDSCAPE_HEIGHT)
    {
      init_image_descriptor(&background, lc_bg_landscape,
                            LC_BG_LANDSCAPE_BYTES,
                            LC_BG_LANDSCAPE_WIDTH,
                            LC_BG_LANDSCAPE_HEIGHT);
    }
  else
    {
      fprintf(stderr, "Living Canvas unsupported display: %ldx%ld\n",
              (long)width, (long)height);
      return false;
    }

  lv_obj_remove_style_all(screen);
  lv_obj_set_style_bg_color(screen, lv_color_hex(0x1b1024), 0);
  lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

  image = lv_image_create(screen);
  lv_image_set_src(image, &background);
  lv_obj_center(image);
  return true;
}

static void style_choice_card(lv_obj_t *card, bool selected)
{
  lv_obj_set_style_border_width(card, selected ? 4 : 2, 0);
  lv_obj_set_style_border_color(
    card, lv_color_hex(selected ? 0xffd66b : 0x7d604e), 0);
  lv_obj_set_style_border_opa(card,
                              selected ? LV_OPA_COVER : LV_OPA_70, 0);
}

static lv_obj_t *create_choice_card(lv_obj_t *screen,
                                    const lv_image_dsc_t *source,
                                    int32_t x_offset,
                                    bool selected)
{
  lv_obj_t *card = lv_obj_create(screen);
  lv_obj_t *image;

  lv_obj_remove_style_all(card);
  lv_obj_set_size(card, 70, 70);
  lv_obj_align(card, LV_ALIGN_BOTTOM_MID, x_offset, -5);
  lv_obj_set_style_bg_color(card, lv_color_hex(0xf6e1c2), 0);
  lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
  lv_obj_set_style_radius(card, 10, 0);
  style_choice_card(card, selected);

  image = lv_image_create(card);
  lv_image_set_src(image, source);
  lv_obj_center(image);
  return card;
}

static void pulse_selected_card(lv_timer_t *timer)
{
  (void)timer;
  g_pulse_bright = !g_pulse_bright;
  lv_obj_set_style_border_opa(g_choice_cards[LC_CHOICE_TAKEOUT],
                              g_pulse_bright ? LV_OPA_COVER : LV_OPA_70,
                              0);
}

static void create_choice_overlay(lv_obj_t *screen)
{
  static lv_image_dsc_t card_images[LC_CHOICE_COUNT];
  lc_choice_state_t choice;

  lc_choice_init(&choice);
  init_image_descriptor(&card_images[LC_CHOICE_TAKEOUT], lc_card_takeout,
                        LC_CARD_BYTES, LC_CARD_WIDTH, LC_CARD_HEIGHT);
  init_image_descriptor(&card_images[LC_CHOICE_MYSTERY], lc_card_mystery,
                        LC_CARD_BYTES, LC_CARD_WIDTH, LC_CARD_HEIGHT);
  init_image_descriptor(&card_images[LC_CHOICE_HOME], lc_card_home,
                        LC_CARD_BYTES, LC_CARD_WIDTH, LC_CARD_HEIGHT);

  g_choice_cards[LC_CHOICE_TAKEOUT] =
    create_choice_card(screen, &card_images[LC_CHOICE_TAKEOUT], -76,
                       choice.selected == LC_CHOICE_TAKEOUT);
  g_choice_cards[LC_CHOICE_MYSTERY] =
    create_choice_card(screen, &card_images[LC_CHOICE_MYSTERY], 0,
                       choice.selected == LC_CHOICE_MYSTERY);
  g_choice_cards[LC_CHOICE_HOME] =
    create_choice_card(screen, &card_images[LC_CHOICE_HOME], 76,
                       choice.selected == LC_CHOICE_HOME);
  g_pulse_bright = true;
  (void)lv_timer_create(pulse_selected_card, 450u, NULL);
}

static void set_competition_text(const char *title,
                                 const char *detail,
                                 const char *badge)
{
  lv_label_set_text(g_competition_title, title);
  lv_label_set_text(g_competition_detail, detail);
  lv_label_set_text(g_competition_badge, badge);
}

static void render_competition_state(void)
{
  unsigned int index;

#  if LV_USE_QRCODE
  if (g_competition_qr != NULL)
    {
      lv_obj_add_flag(g_competition_qr, LV_OBJ_FLAG_HIDDEN);
    }
#  endif

  for (index = 0u; index < LC_CHOICE_COUNT; index++)
    {
      if (g_choice_cards[index] != NULL)
        {
          lv_obj_remove_flag(g_choice_cards[index], LV_OBJ_FLAG_HIDDEN);
        }
    }

  switch (g_competition.state)
    {
      case LC_COMPETITION_CHOICE:
        set_competition_text("WHAT SHALL WE EAT?",
                             "Takeout is ready to start",
                             "BAG = HELP ME ORDER");
        break;

      case LC_COMPETITION_REQUESTING:
        set_competition_text("BEAGLE IS COORDINATING",
                             "Finding one warm, reliable dinner...",
                             "AI + SAFE RULES");
        break;

      case LC_COMPETITION_RECOMMENDATION:
        (void)snprintf(g_competition_detail_text,
                       sizeof(g_competition_detail_text), "%s | %s",
                       g_competition.reason, g_competition.price);
        set_competition_text(g_competition.dish,
                             g_competition_detail_text,
                             g_competition.rules_fallback
                               ? "SAFE RULES FALLBACK"
                               : "CAT MEMORY: GOOD MATCH");
        break;

      case LC_COMPETITION_CONFIRMING:
        set_competition_text("CONFIRMED ON FRAME",
                             "Preparing a phone handoff...",
                             "PAYMENT STAYS ON PHONE");
        break;

      case LC_COMPETITION_QR:
        for (index = 0u; index < LC_CHOICE_COUNT; index++)
          {
            lv_obj_add_flag(g_choice_cards[index], LV_OBJ_FLAG_HIDDEN);
          }
#  if LV_USE_QRCODE
        if (lv_qrcode_update(g_competition_qr, g_competition.qr_url,
                             (uint32_t)strlen(g_competition.qr_url)) !=
            LV_RESULT_OK)
          {
            lc_competition_fail(&g_competition, "QR generation failed");
            set_competition_text("HANDOFF UNAVAILABLE",
                                 g_competition.error, "SAFE STOP");
            break;
          }

        lv_obj_remove_flag(g_competition_qr, LV_OBJ_FLAG_HIDDEN);
        set_competition_text("SCAN ON PHONE", "Review the order there",
                             "PAYMENT STAYS ON PHONE");
#  else
        lc_competition_fail(&g_competition, "QR module is disabled");
        set_competition_text("HANDOFF UNAVAILABLE", g_competition.error,
                             "SAFE STOP");
#  endif
        break;

      case LC_COMPETITION_ERROR:
        set_competition_text("HANDOFF UNAVAILABLE",
                             g_competition.error[0] == '\0'
                               ? "Check the Living Canvas backend and retry"
                               : g_competition.error,
                             "SAFE STOP");
        break;
    }
}

static void advance_competition_demo(lv_timer_t *timer)
{
  g_competition_step++;

  if (g_competition_step == 1u)
    {
      if (!lc_competition_start(&g_competition, LC_CHOICE_TAKEOUT))
        {
          lc_competition_fail(&g_competition, "could not start");
        }

    }
  else if (g_competition_step == 2u)
    {
      if (!lc_competition_set_recommendation(
            &g_competition, "Tomato beef rice",
            "Warm, balanced and within budget", "About CNY 32", false))
        {
          lc_competition_fail(&g_competition, "invalid recommendation");
        }

    }
  else if (g_competition_step == 3u)
    {
      if (!lc_competition_confirm(&g_competition))
        {
          lc_competition_fail(&g_competition, "could not confirm");
        }

    }
  else if (g_competition_step == 4u)
    {
      if (!lc_competition_set_handoff(&g_competition,
                                      g_competition_handoff))
        {
          lc_competition_fail(&g_competition, "invalid handoff URL");
        }

      lv_timer_pause(timer);
    }

  render_competition_state();
}

static void create_competition_overlay(lv_obj_t *screen,
                                       const char *handoff_url)
{
  size_t length = strlen(handoff_url);

  lc_competition_init(&g_competition);
  g_competition_step = 0u;
  memcpy(g_competition_handoff, handoff_url, length + 1u);

  g_competition_panel = lv_obj_create(screen);
  lv_obj_remove_style_all(g_competition_panel);
  lv_obj_set_size(g_competition_panel, LV_PCT(92), 112);
  lv_obj_align(g_competition_panel, LV_ALIGN_TOP_MID, 0, 8);
  lv_obj_set_style_bg_color(g_competition_panel, lv_color_hex(0x24150f), 0);
  lv_obj_set_style_bg_opa(g_competition_panel, LV_OPA_80, 0);
  lv_obj_set_style_border_color(g_competition_panel,
                                lv_color_hex(0xffd07b), 0);
  lv_obj_set_style_border_width(g_competition_panel, 2, 0);
  lv_obj_set_style_radius(g_competition_panel, 14, 0);

  g_competition_title = lv_label_create(g_competition_panel);
  configure_label(g_competition_title, 0xffffff);
  lv_obj_align(g_competition_title, LV_ALIGN_TOP_MID, 0, 12);

  g_competition_detail = lv_label_create(g_competition_panel);
  configure_label(g_competition_detail, 0xffe9c8);
  lv_obj_align(g_competition_detail, LV_ALIGN_CENTER, 0, 4);

  g_competition_badge = lv_label_create(g_competition_panel);
  configure_label(g_competition_badge, 0xffd26a);
  lv_obj_align(g_competition_badge, LV_ALIGN_BOTTOM_MID, 0, -10);

#  if LV_USE_QRCODE
  g_competition_qr = lv_qrcode_create(screen);
  lv_qrcode_set_size(g_competition_qr, 124);
  lv_qrcode_set_dark_color(g_competition_qr, lv_color_hex(0x1d130f));
  lv_qrcode_set_light_color(g_competition_qr, lv_color_hex(0xfff7e8));
  lv_obj_align(g_competition_qr, LV_ALIGN_CENTER, 0, 25);
  lv_obj_add_flag(g_competition_qr, LV_OBJ_FLAG_HIDDEN);
#  endif

  render_competition_state();
  g_competition_timer = lv_timer_create(advance_competition_demo, 1800u, NULL);
}

static int run_artwork_preview(bool with_choices)
{
  lv_nuttx_dsc_t info;
  lv_nuttx_result_t result;
  lv_obj_t *screen;

  if (lv_is_initialized())
    {
      fprintf(stderr, "Living Canvas display is already in use\n");
      return 2;
    }

#  ifdef NEED_BOARDINIT
  boardctl(BOARDIOC_INIT, 0);
#  endif

  lv_init();
  lv_nuttx_dsc_init(&info);

#  ifdef CONFIG_LV_USE_NUTTX_LCD
  info.fb_path = "/dev/lcd0";
#  endif

  lv_nuttx_init(&info, &result);
  if (result.disp == NULL)
    {
      fprintf(stderr, "Living Canvas could not open /dev/lcd0\n");
      lv_nuttx_deinit(&result);
      lv_deinit();
      return 2;
    }

  screen = lv_screen_active();
  if (!create_background(result.disp, screen))
    {
      lv_nuttx_deinit(&result);
      lv_deinit();
      return 2;
    }

  if (with_choices)
    {
      create_choice_overlay(screen);
    }

  for (;;)
    {
      uint32_t idle = lv_timer_handler();

      if (idle == 0u)
        {
          idle = 1u;
        }
      else if (idle > 50u)
        {
          idle = 50u;
        }

      usleep(idle * 1000u);
    }
}

int lc_display_run_preview(void)
{
  lv_nuttx_dsc_t info;
  lv_nuttx_result_t result;

  if (lv_is_initialized())
    {
      fprintf(stderr, "Living Canvas display is already in use\n");
      return 2;
    }

#  ifdef NEED_BOARDINIT
  boardctl(BOARDIOC_INIT, 0);
#  endif

  lv_init();
  lv_nuttx_dsc_init(&info);

#  ifdef CONFIG_LV_USE_NUTTX_LCD
  info.fb_path = "/dev/lcd0";
#  endif

  lv_nuttx_init(&info, &result);
  if (result.disp == NULL)
    {
      fprintf(stderr, "Living Canvas could not open /dev/lcd0\n");
      lv_nuttx_deinit(&result);
      lv_deinit();
      return 2;
    }

  create_preview();

  for (;;)
    {
      uint32_t idle = lv_timer_handler();

      if (idle == 0u)
        {
          idle = 1u;
        }
      else if (idle > 50u)
        {
          idle = 50u;
        }

      usleep(idle * 1000u);
    }
}

int lc_display_run_image_preview(void)
{
  return run_artwork_preview(false);
}

int lc_display_run_choice_preview(void)
{
  return run_artwork_preview(true);
}

int lc_display_run_competition_demo(const char *qr_url)
{
  lv_nuttx_dsc_t info;
  lv_nuttx_result_t result;
  lv_obj_t *screen;
  size_t length;

  if (qr_url == NULL ||
      (strncmp(qr_url, "http://", 7u) != 0 &&
       strncmp(qr_url, "https://", 8u) != 0))
    {
      fprintf(stderr, "Living Canvas requires an HTTP handoff URL\n");
      return 2;
    }

  length = strlen(qr_url);
  if (length == 0u || length >= sizeof(g_competition_handoff))
    {
      fprintf(stderr, "Living Canvas handoff URL is too long\n");
      return 2;
    }

  if (lv_is_initialized())
    {
      fprintf(stderr, "Living Canvas display is already in use\n");
      return 2;
    }

#  ifdef NEED_BOARDINIT
  boardctl(BOARDIOC_INIT, 0);
#  endif

  lv_init();
  lv_nuttx_dsc_init(&info);
#  ifdef CONFIG_LV_USE_NUTTX_LCD
  info.fb_path = "/dev/lcd0";
#  endif
  lv_nuttx_init(&info, &result);
  if (result.disp == NULL)
    {
      fprintf(stderr, "Living Canvas could not open /dev/lcd0\n");
      lv_nuttx_deinit(&result);
      lv_deinit();
      return 2;
    }

  screen = lv_screen_active();
  if (!create_background(result.disp, screen))
    {
      lv_nuttx_deinit(&result);
      lv_deinit();
      return 2;
    }

  create_choice_overlay(screen);
  create_competition_overlay(screen, qr_url);

  for (;;)
    {
      uint32_t idle = lv_timer_handler();

      if (idle == 0u)
        {
          idle = 1u;
        }
      else if (idle > 50u)
        {
          idle = 50u;
        }

      usleep(idle * 1000u);
    }
}

#else

int lc_display_run_preview(void)
{
  return 2;
}

int lc_display_run_image_preview(void)
{
  return 2;
}

int lc_display_run_choice_preview(void)
{
  return 2;
}

int lc_display_run_competition_demo(const char *qr_url)
{
  (void)qr_url;
  return 2;
}

#endif
