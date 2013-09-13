#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"


#define MY_UUID { 0xD3, 0x8B, 0x2F, 0x08, 0x04, 0xAB, 0x4A, 0x68, 0xB2, 0xF7, 0x1E, 0xB4, 0xD2, 0x64, 0x4C, 0xFE }
PBL_APP_INFO(MY_UUID,
             "Talk Timer", "Antoine Vernois // Crafting Labs",
             0, 1, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_STANDARD_APP);

#define DELAY 1000 * 60
#define COUNTDOWN_START_VALUE 45

Window window;
TextLayer countDownLayer;

typedef struct {
  int current;
  char currentText[3];
} CountDown;

CountDown countDown;

void set_coundown(CountDown *countdown, int value) {
  countdown->current = value;
  snprintf(countdown->currentText, 3, "%d", countdown->current);
}

void decrease_countdown(CountDown *countdown) {
  set_coundown(countdown, countdown->current - 1);
}

void start_countdown(AppContextRef ctx) {
  set_coundown(&countDown, COUNTDOWN_START_VALUE);
  app_timer_send_event(ctx, DELAY, 1);
}

void handle_init(AppContextRef ctx) {
  window_init(&window, "Window Name");
  window_stack_push(&window, true /* Animated */);
  window_set_fullscreen(&window, true);   
  window_set_background_color(&window, GColorBlack);

  text_layer_init(&countDownLayer, GRect(0, 0, 144, 168));
  text_layer_set_text_color(&countDownLayer, GColorWhite);
  text_layer_set_background_color(&countDownLayer, GColorClear);
  text_layer_set_text_alignment(&countDownLayer, GTextAlignmentCenter);
  text_layer_set_font(&countDownLayer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  layer_add_child(&window.layer, &countDownLayer.layer);

  start_countdown(ctx);
  text_layer_set_text(&countDownLayer, countDown.currentText);
}

void handle_timer(AppContextRef ctx, AppTimerHandle handle, uint32_t cookie) {
  if (countDown.current > 0) {
    decrease_countdown(&countDown);
    text_layer_set_text(&countDownLayer, countDown.currentText);
    app_timer_send_event(ctx, DELAY, 1);
  } else {
    text_layer_set_text(&countDownLayer, "Time's up !");
  }
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .timer_handler = &handle_timer
  };
  app_event_loop(params, &handlers);
}