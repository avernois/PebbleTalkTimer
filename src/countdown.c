#include "pebble_os.h"

#include "countdown.h"

void countdown_set(CountDown *countdown, int value) {
  countdown->current = value;
  snprintf(countdown->currentText, 3, "%d", countdown->current);
}

void countdown_set_first_alert(CountDown *countdown, int firstAlert) {
  countdown->firstAlert = firstAlert;
}

void countdown_decrease(CountDown *countDown) {
  countdown_set(countDown, countDown->current - 1);
}

bool countdown_is_first_alert_time(CountDown *countDown) {
  return (countDown->current == countDown->firstAlert);
}

bool countdown_is_time_over(CountDown *countDown) {
  return countDown->current <= 0;
}

char* countdown_get_current_as_text(CountDown *countDown) {
  return countDown->currentText;
}

VibePattern countdown_get_vibe_pattern(CountDown *countDown) {
  return countDown->vibePattern;
}