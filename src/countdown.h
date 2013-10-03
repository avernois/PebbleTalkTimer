#ifndef COUNTDOWN_H
#define COUNTDOWN_H

typedef struct {
  int current;
  int firstAlert;
  int secondAlert;
  VibePattern vibePattern;
  char currentText[3];
} CountDown;


void countdown_set(CountDown *countdown, int value);

void countdown_set_first_alert(CountDown *countdown, int firstAlert);
void countdown_set_second_alert(CountDown *countdown, int secondAlert);

void countdown_decrease(CountDown *countDown);

bool countdown_is_first_alert_time(CountDown *countDown);
bool countdown_is_second_alert_time(CountDown *countDown);

bool countdown_is_time_over(CountDown *countDown);

char* countdown_get_current_as_text(CountDown *countDown);
int countdown_get_current(CountDown *countDown);

VibePattern countdown_get_vibe_pattern(CountDown *countDown);

#endif