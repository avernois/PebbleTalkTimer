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
NumberWindow _secondAlertNumberWindow;

AppContextRef _appContextRef;

CountDown _countDown = {.current = DEFAULT_DURATION,
                        .firstAlert = DEFAULT_FIRST_ALERT,
                        .secondAlert = DEFAULT_SECOND_ALERT,
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

void select_time(CountDown *countDown, char* windowName, NumberWindow *window, NumberWindowCallback callback, int defaultValue) {
  number_window_init(window, windowName,
                     (NumberWindowCallbacks){
                      .decremented = NULL,
                      .incremented = NULL,
                      .selected = (NumberWindowCallback) callback
                      },
                      countDown);
  number_window_set_min(window, 0);
  number_window_set_value(window, defaultValue);
  number_window_set_max(window, MAX_DURATION);
  number_window_set_min(window, MIN_DURATION);

  window_stack_push((Window*)window, true);
}

void handle_second_alert_selected(struct NumberWindow *numberWindow, void *context){
  CountDown *countDown = (CountDown *) context;
  countdown_set_second_alert(countDown, number_window_get_value(numberWindow));
  
  init_countdown_window(countDown);
  start_countdown();
  window_stack_push(&_window, true);
}

void handle_first_alert_selected(struct NumberWindow *numberWindow, void *context){
  CountDown *countDown = (CountDown *) context;
  countdown_set_first_alert(countDown, number_window_get_value(numberWindow));
  select_time(countDown, "Second Alert", &_secondAlertNumberWindow, (NumberWindowCallback) handle_second_alert_selected, DEFAULT_SECOND_ALERT);
}

void handle_duration_selected(struct NumberWindow *number_window, void *context) {
  CountDown *countDown = (CountDown *) context;
  countdown_set(countDown, number_window_get_value(number_window));
  select_time(countDown, "First Alert", &_firstAlertNumberWindow, (NumberWindowCallback) handle_first_alert_selected, DEFAULT_FIRST_ALERT);
}

void manage_alert(CountDown *countDown) {
  if(countdown_is_first_alert_time(countDown)
     || countdown_is_second_alert_time(countDown)
     || countdown_is_time_over(countDown)) {
    vibes_enqueue_custom_pattern(countdown_get_vibe_pattern(countDown));
  }
}

void update_countdown(CountDown *countDown) {
  countdown_decrease(countDown);
  text_layer_set_text(&_countDownLayer, countdown_get_current_as_text(countDown));
}

void handle_timer(AppContextRef ctx, AppTimerHandle handle, uint32_t cookie) {
  update_countdown(&_countDown);
  if (!countdown_is_time_over(&_countDown)) {
    app_timer_send_event(ctx, DELAY, 1);
  } else {
    text_layer_set_text(&_countDownLayer, "Time's up !");
  }
  manage_alert(&_countDown);
}

void handle_init(AppContextRef ctx) {
  _appContextRef = ctx;
  select_time(&_countDown, "Duration", &_durationNumberWindow, handle_duration_selected, DEFAULT_DURATION);
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .timer_handler = &handle_timer
  };
  app_event_loop(params, &handlers);
}