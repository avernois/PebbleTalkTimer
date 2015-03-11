#include <pebble.h>
#include "constants.h"

#include "countdown.h"

Window *_window;
TextLayer *_countDownLayer;
NumberWindow *_durationNumberWindow;
NumberWindow *_firstAlertNumberWindow;
NumberWindow *_secondAlertNumberWindow;

CountDown _countDown = {.current = DEFAULT_DURATION,
                        .firstAlert = DEFAULT_FIRST_ALERT,
                        .secondAlert = DEFAULT_SECOND_ALERT,
                        .vibePattern = {
                          .durations = (uint32_t []) {500, 200, 500, 200, 500, 200, 500},
                          .num_segments = 7
                        },
                        .currentText = "  "};

void manage_alert(CountDown *countDown) {
  if(countdown_is_first_alert_time(countDown)
     || countdown_is_second_alert_time(countDown)
     || countdown_is_time_over(countDown)) {
    vibes_enqueue_custom_pattern(countdown_get_vibe_pattern(countDown));
  }
}

void update_countdown(CountDown *countDown) {
  countdown_decrease(countDown);
  text_layer_set_text(_countDownLayer, countdown_get_current_as_text(countDown));
}

void handle_timer() {
  update_countdown(&_countDown);
  if (!countdown_is_time_over(&_countDown)) {
    app_timer_register(DELAY, &handle_timer, NULL);
  } else {
    text_layer_set_text(_countDownLayer, "Time's up !");
  }
  manage_alert(&_countDown);
}

void start_countdown() {
  app_timer_register(DELAY, &handle_timer, NULL);
}

void init_countdown_window(CountDown *countDown) {
  _window = window_create();
  window_stack_push(_window, true /* Animated */);
  window_set_fullscreen(_window, true);
  window_set_background_color(_window, GColorBlack);

  _countDownLayer = text_layer_create(GRect(0, 0, 144, 168));
  text_layer_set_text_color(_countDownLayer, GColorWhite);
  text_layer_set_background_color(_countDownLayer, GColorClear);
  text_layer_set_text_alignment(_countDownLayer, GTextAlignmentCenter);
  text_layer_set_font(_countDownLayer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));

  text_layer_set_text(_countDownLayer, countdown_get_current_as_text(countDown));
  layer_add_child(window_get_root_layer(_window), text_layer_get_layer(_countDownLayer));
}

void select_time(CountDown *countDown, char* windowName, NumberWindow *window, NumberWindowCallback callback, int defaultValue) {
  window = number_window_create(windowName, 
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
  window_stack_push(_window, true);
}

void handle_first_alert_selected(struct NumberWindow *numberWindow, void *context){
  CountDown *countDown = (CountDown *) context;
  countdown_set_first_alert(countDown, number_window_get_value(numberWindow));
  select_time(countDown, "Second Alert", _secondAlertNumberWindow, (NumberWindowCallback) handle_second_alert_selected, DEFAULT_SECOND_ALERT);
}

void handle_duration_selected(struct NumberWindow *number_window, void *context) {
  CountDown *countDown = (CountDown *) context;
  countdown_set(countDown, number_window_get_value(number_window));
  select_time(countDown, "First Alert", _firstAlertNumberWindow, (NumberWindowCallback) handle_first_alert_selected, DEFAULT_FIRST_ALERT);
}

void handle_init() {
  select_time(&_countDown, "Duration", _durationNumberWindow, handle_duration_selected, DEFAULT_DURATION);
}

void handle_deinit() {
  window_destroy(_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}