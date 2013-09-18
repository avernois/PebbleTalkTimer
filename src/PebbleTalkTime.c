#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#include "constants.h"

#define MY_UUID { 0xD3, 0x8B, 0x2F, 0x08, 0x04, 0xAB, 0x4A, 0x68, 0xB2, 0xF7, 0x1E, 0xB4, 0xD2, 0x64, 0x4C, 0xFE }
PBL_APP_INFO(MY_UUID,
             "Talk Timer", "Antoine Vernois // Crafting Labs",
             0, 1, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_STANDARD_APP);

Window _window;
TextLayer _countDownLayer;
NumberWindow _durationNumberWindow;
NumberWindow _firstAlertNumberWindow;

int _talkDuration;
int _firstAlert;

typedef struct {
  int current;
  char currentText[3];
} CountDown;

CountDown _countDown;

const VibePattern _customPattern = {
  .durations = (uint32_t []) {500, 200, 500, 200, 500, 200, 500},
  .num_segments = 7
};


void set_coundown(CountDown *countdown, int value) {
  countdown->current = value;
  snprintf(countdown->currentText, 3, "%d", countdown->current);
}

void decrease_countdown(CountDown *countdown) {
  set_coundown(countdown, countdown->current - 1);
}

void start_countdown(AppContextRef ctx, int talkDuration) {
  set_coundown(&_countDown, talkDuration);
  app_timer_send_event(ctx, DELAY, 1);
}


void first_alert_selected(struct NumberWindow *numberWindow, void *context){
  _firstAlert = number_window_get_value(numberWindow);
  
  start_countdown(context, _talkDuration);
  window_stack_push((Window*)&_window, true);
}

void select_first_alert(void *context) {
  number_window_init(&_firstAlertNumberWindow, "First alert", (NumberWindowCallbacks){
    .decremented = NULL,
    .incremented = NULL,
    .selected = (NumberWindowCallback) first_alert_selected}, context);

  number_window_set_max(&_firstAlertNumberWindow, _talkDuration);
  number_window_set_min(&_firstAlertNumberWindow, 0);
  number_window_set_value(&_firstAlertNumberWindow, DEFAULT_FIRST_ALERT);

  window_stack_push((Window*)&_firstAlertNumberWindow, true);
}

void duration_selected(struct NumberWindow *number_window, void *context) {
  _talkDuration = number_window_get_value(number_window);
  select_first_alert(context);
}


void handle_init(AppContextRef ctx) {
  window_init(&_window, "Window Name");
  window_stack_push(&_window, true /* Animated */);
  window_set_fullscreen(&_window, true);   
  window_set_background_color(&_window, GColorBlack);

  text_layer_init(&_countDownLayer, GRect(0, 0, 144, 168));
  text_layer_set_text_color(&_countDownLayer, GColorWhite);
  text_layer_set_background_color(&_countDownLayer, GColorClear);
  text_layer_set_text_alignment(&_countDownLayer, GTextAlignmentCenter);
  text_layer_set_font(&_countDownLayer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  layer_add_child(&_window.layer, &_countDownLayer.layer);

  number_window_init(&_durationNumberWindow, "Duration", (NumberWindowCallbacks){
    .decremented = NULL,
    .incremented = NULL,
    .selected = (NumberWindowCallback) duration_selected}, ctx);

  number_window_set_max(&_durationNumberWindow, MAX_DURATION);
  number_window_set_min(&_durationNumberWindow, MIN_DURATION);
  number_window_set_value(&_durationNumberWindow, DEFAULT_DURATION);

  window_stack_push((Window *)&_durationNumberWindow, true);

  text_layer_set_text(&_countDownLayer, _countDown.currentText);
}

void handle_timer(AppContextRef ctx, AppTimerHandle handle, uint32_t cookie) {
  if (_countDown.current > 0) {
    decrease_countdown(&_countDown);
    if (_countDown.current == _firstAlert) {
      vibes_enqueue_custom_pattern(_customPattern);
    }
    text_layer_set_text(&_countDownLayer, _countDown.currentText);
    app_timer_send_event(ctx, DELAY, 1);
  } else {
    text_layer_set_text(&_countDownLayer, "Time's up !");
  }
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .timer_handler = &handle_timer
  };
  app_event_loop(params, &handlers);
}