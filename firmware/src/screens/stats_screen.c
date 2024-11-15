#include "screens/stats_screen.h"
#include "esp_log.h"
#include "remote/display.h"
#include <core/lv_event.h>
#include <remote/stats.h>

static const char *TAG = "PUBREMOTE-STATS_SCREEN";

StatsScreenDisplayOptions stat_display_options = {
    .primary_stat = STAT_DISPLAY_SPEED,
    .secondary_stat = STAT_DISPLAY_DUTY,
    .battery_display = BATTERY_DISPLAY_PERCENT,
};

static void change_stat_display(int direction) {
  if (direction > 0) {
    stat_display_options.primary_stat = (stat_display_options.primary_stat + 1) % 4;
  }
  else {
    stat_display_options.primary_stat = (stat_display_options.primary_stat + 3) % 4;
  }
}

bool is_stats_screen_active() {
  lv_obj_t *active_screen = lv_scr_act();
  return active_screen == ui_StatsScreen;
}

static float converted_speed = 0.0f; // Use a single float to store the converted speed

// Convert speed from km/h to mph
static void convert_speed() {
  if (remoteStats.speedUnit == SPEED_UNIT_KMH) {    // Correct check for km/h
    converted_speed = remoteStats.speed * 0.621371; // Convert km/h to mph
  }
}

static void update_speed_display() {
  ESP_LOGI(TAG, "Updating speed display");
  char *formattedString;
  if (converted_speed >= 10) {
    asprintf(&formattedString, "%.0f", converted_speed);
  }
  else {
    asprintf(&formattedString, "%.1f", converted_speed);
  }
  lv_arc_set_range(ui_PrimaryDial, 0, 40);
  lv_arc_set_value(ui_PrimaryDial, converted_speed);
  lv_label_set_text(ui_PrimaryStat, formattedString);
  lv_label_set_text(ui_PrimaryStatUnit, "mph");
  free(formattedString);
}

static void update_duty_display() {
  ESP_LOGI(TAG, "Updating duty display");
  lv_arc_set_range(ui_PrimaryDial, 0, 100);
  lv_arc_set_value(ui_PrimaryDial, remoteStats.dutyCycle);
  lv_label_set_text_fmt(ui_PrimaryStat, "%d%%", remoteStats.dutyCycle);
  lv_label_set_text(ui_PrimaryStatUnit, "duty");
}

static void update_battery_display() {
  ESP_LOGI(TAG, "Updating battery display");
  char *formattedString;
  asprintf(&formattedString, "%.1fv", remoteStats.batteryVoltage);
  lv_arc_set_value(ui_PrimaryDial, remoteStats.batteryVoltage);
  lv_label_set_text(ui_PrimaryStat, formattedString);
  lv_label_set_text(ui_PrimaryStatUnit, "batt");
  free(formattedString);
}

typedef void (*StatUpdateFunction)();
static StatUpdateFunction stat_update_functions[] = {update_speed_display, update_duty_display, update_battery_display};

static void update_secondary_dial_display() {
  lv_arc_set_value(ui_SecondaryDial, converted_speed); // Set the dial value to the converted speed
}

void update_primary_display() {
  if (stat_display_options.primary_stat < 0 ||
      stat_display_options.primary_stat >= sizeof(stat_update_functions) / sizeof(StatUpdateFunction)) {
    ESP_LOGE(TAG, "Primary stat out of bounds: %d", stat_display_options.primary_stat);
    // update_unknown_display();
    return;
  }

  // Log the function being called
  ESP_LOGI(TAG, "Calling update function for primary stat: %d", stat_display_options.primary_stat);

  // Call the appropriate update function
  stat_update_functions[stat_display_options.primary_stat]();
}

static void update_secondary_stat_display() {
  char *formattedString;
  float converted_val = converted_speed; // TODO - apply based on primary stat display option

  if (converted_val >= 10) {
    asprintf(&formattedString, "%.0f mph", converted_speed);
  }
  else {
    asprintf(&formattedString, "%.1f mph", converted_speed);
  }

  lv_label_set_text(ui_SecondaryStat, formattedString);
  free(formattedString);
}

static void update_footpad_display() {
  switch (remoteStats.switchState) {
  case SWITCH_STATE_OFF:
    lv_arc_set_value(ui_LeftSensor, 0);
    lv_arc_set_value(ui_RightSensor, 0);
    break;
  case SWITCH_STATE_LEFT:
    lv_arc_set_value(ui_LeftSensor, 1);
    lv_arc_set_value(ui_RightSensor, 0);
    break;
  case SWITCH_STATE_RIGHT:
    lv_arc_set_value(ui_LeftSensor, 0);
    lv_arc_set_value(ui_RightSensor, 1);
    break;
  case SWITCH_STATE_BOTH:
    lv_arc_set_value(ui_LeftSensor, 1);
    lv_arc_set_value(ui_RightSensor, 1);
    break;
  default:
    break;
  }
}

void update_stats_screen_display() {
  LVGL_lock(-1);
  convert_speed();                 // Ensure speed is converted
  update_primary_display();        // Centralized update
  update_secondary_dial_display(); // Other dynamic updates
  update_footpad_display();
  // update_battery_display();
  LVGL_unlock();
}

// Event handlers
void stats_screen_loaded(lv_event_t *e) {
  ESP_LOGI(TAG, "Stats screen loaded");
}

void stats_screen_unloaded(lv_event_t *e) {
  ESP_LOGI(TAG, "Stats screen unloaded");
}

void stat_long_press(lv_event_t *e) {
  change_stat_display(1);   // Increment stat
  update_primary_display(); // Refresh display
  ESP_LOGI(TAG, "Primary Stat: %d", stat_display_options.primary_stat);
}

void stat_swipe_left(lv_event_t *e) {
  change_stat_display(1);
}

void stat_swipe_right(lv_event_t *e) {
  change_stat_display(-1);
}

void stats_footer_long_press(lv_event_t *e) {
  // Your code here
}