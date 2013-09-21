#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "constants.h"

#include "countdown.h"

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

AppContextRef _appContextRef;

CountDown _countDown = {.current = DEFAULT_DURATION,
                        .firstAlert = DEFAULT_FIRST_ALERT,
                        .vibePattern = {
                          .durations = (uint32_t []) {500, 200, 500, 200, 500, 200, 500},
                          .num_segments = 7
                        },
                        .currentText = "  "};



void start_countdown() {
  app_timer_send_event(_appContextRef, DELAY, 1);
}

void init_countdown_window(CountDown *countDown) {
  window_init(&_window, "Window Name");
  window_stack_push(&_window, true /* Animated */);
  window_set_fullscreen(&_window, true);   
  window_set_background_color(&_window, GColorBlack);

  text_layer_init(&_countDownLayer, GRect(0, 0, 144, 168));
  text_layer_set_text_color(&_countDownLayer, GColorWhite);
  text_layer_set_background_color(&_countDownLayer, GColorClear);
  text_layer_set_text_alignment(&_countDownLayer, GTextAlignmentCenter);
  text_layer_set_font(&_countDownLayer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));

  text_layer_set_text(&_countDownLayer, countdown_get_current_as_text(countDown));
  layer_add_child(&_window.layer, &_countDownLayer.layer);
}

void handle_first_alert_selected(struct NumberWindow *numberWindow, void *context){
  CountDown *countDown = (CountDown *) context;
  countdown_set_first_alert(countDown, number_window_get_value(numberWindow));
  
  init_countdown_window(countDown);
  start_countdown();
  window_stack_push((Window*)&_window, true);
}

void select_first_alert(CountDown *countDown) {
  number_window_init(&_firstAlertNumberWindow, "First alert", 
                     (NumberWindowCallbacks){
                      .decremented = NULL,
                      .incremented = NULL,
                      .selected = (NumberWindowCallback) handle_first_alert_selected
                      }, 
                      countDown);
  number_window_set_min(&_firstAlertNumberWindow, 0);
  number_window_set_value(&_firstAlertNumberWindow, DEFAULT_FIRST_ALERT);

  window_stack_push((Window*)&_firstAlertNumberWindow, true);
}

void handle_duration_selected(struct NumberWindow *number_window, void *context) {
  CountDown *countDown = (CountDown *) context;
  countdown_set(countDown, number_window_get_value(number_window));
  select_first_alert(countDown);
}

void select_talk_duration(CountDown *countDown) {
  number_window_init(&_durationNumberWindow, "Duration",
                     (NumberWindowCallbacks){
                      .decremented = NULL,
                      .incremented = NULL,
                      .selected = (NumberWindowCallback) handle_duration_selected
                     },
                     countDown);

  number_window_set_max(&_durationNumberWindow, MAX_DURATION);
  number_window_set_min(&_durationNumberWindow, MIN_DURATION);
  number_window_set_value(&_durationNumberWindow, DEFAULT_DURATION);

  window_stack_push((Window *)&_durationNumberWindow, true);
}

void manage_alert(CountDown *countDown) {
  if(countdown_is_first_alert_time(countDown)) {
    vibes_enqueue_custom_pattern(countdown_get_vibe_pattern(countDown));
  }
}

void update_countdown(CountDown *countDown) {
  countdown_decrease(countDown);
  manage_alert(countDown);
  text_layer_set_text(&_countDownLayer, countdown_get_current_as_text(countDown));
}

void handle_timer(AppContextRef ctx, AppTimerHandle handle, uint32_t cookie) {
  if (!countdown_is_time_over(&_countDown)) {
    update_countdown(&_countDown);
    app_timer_send_event(ctx, DELAY, 1);
  } else {
    text_layer_set_text(&_countDownLayer, "Time's up !");
  }
}

void handle_init(AppContextRef ctx) {
  _appContextRef = ctx;
  select_talk_duration(&_countDown);
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .timer_handler = &handle_timer
  };
  app_event_loop(params, &handlers);
}