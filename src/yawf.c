#include "pebble.h"
#include "pebble_fonts.h"

Window* window;
TextLayer* text_day_layer;
TextLayer* text_date_layer;
TextLayer* text_time_layer;
TextLayer* text_day_of_the_week_layer;
Layer* line_layer;

/**
 * A mapping for the day suffix.
 */
char* day_suffix_map[] = {
  "th", // 0
  "st", // 1
  "nd", // 2
  "rd", // 3
  "th", // 4
  "th", // 5
  "th", // 6
  "th", // 7
  "th", // 8
  "th"  // 9
};

void line_layer_update_callback(Layer* layer, GContext* ctx) {
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

/**
 * Adds a suffix to the {@code day} value.
 * <p>
 * For example: 20 => 20th
 *              21 => 21st
 *              ...
 *              29 => 29th
 * day: the day value to format.
 * returns the formatted day value.
 */
void format_day(char* day) {
  int day_num = atoi(day);
  strcat(day, day_suffix_map[day_num % 10]);
}

void handle_tick(struct tm* tick_time, TimeUnits units_changed) {
  // Need to be static because they're used by the system later.
  static char time_text[] = "00:00";
  static char date_text[16] = "";
  static char month[] = "Xxx";
  static char day[] = "00xx";
  static char year[] = "0000";
  static char day_of_the_week[] = "Xxxxxxxxx";
  
  strftime(day_of_the_week, sizeof(day_of_the_week), "%A", tick_time);
  text_layer_set_text(text_day_of_the_week_layer, day_of_the_week);

  strftime(month, sizeof(month), "%b", tick_time);
  strftime(day, sizeof(day), "%e", tick_time);
  format_day(day);
  //strftime(year, sizeof(year), "%Y", tick_time);
  snprintf(date_text, sizeof(date_text), "%s %s", month, day);
  text_layer_set_text(text_date_layer, date_text);

  if (clock_is_24h_style()) {
    strftime(time_text, sizeof(time_text), "%H:%M", tick_time);
  } else {
    strftime(time_text, sizeof(time_text), "%I:%M", tick_time);
  }

  // Kludge to handle lack of non-padded hour format string
  // for twelve hour clock.
  //if (!clock_is_24h_style() && (time_text[0] == '0')) {
  //  memmove(time_text, &time_text[1], sizeof(time_text) - 1);
  //}

  text_layer_set_text(text_time_layer, time_text);
}

void handle_deinit(void) {
  tick_timer_service_unsubscribe();
}

void handle_init(void) {
  window = window_create();
  window_stack_push(window, true /* Animated */);
  window_set_background_color(window, GColorBlack);

  Layer* window_layer = window_get_root_layer(window);
  
  text_day_of_the_week_layer = text_layer_create(GRect(8, 20, 127, 27));
  text_layer_set_text_color(text_day_of_the_week_layer, GColorWhite);
  text_layer_set_background_color(text_day_of_the_week_layer, GColorClear);
  text_layer_set_font(text_day_of_the_week_layer, 
                      fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_UBUNTU_MONO_REGULAR_21)));
  layer_add_child(window_layer, text_layer_get_layer(text_day_of_the_week_layer));

  text_date_layer = text_layer_create(GRect(8, 48, 144-8, 168-68));
  text_layer_set_text_color(text_date_layer, GColorWhite);
  text_layer_set_background_color(text_date_layer, GColorClear);
  text_layer_set_font(text_date_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_UBUNTU_MONO_REGULAR_21)));
  layer_add_child(window_layer, text_layer_get_layer(text_date_layer));
  
  GRect line_frame = GRect(8, 85, 130, 2);
  line_layer = layer_create(line_frame);
  layer_set_update_proc(line_layer, line_layer_update_callback);
  layer_add_child(window_layer, line_layer);

  text_time_layer = text_layer_create(GRect(5, 92, 137, 168-92));
  text_layer_set_text_color(text_time_layer, GColorWhite);
  text_layer_set_background_color(text_time_layer, GColorClear);
  text_layer_set_font(text_time_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_LCD_NARROW_35)));
  layer_add_child(window_layer, text_layer_get_layer(text_time_layer));

  tick_timer_service_subscribe(SECOND_UNIT, handle_tick);
  // TODO: Update display here to avoid blank display on launch?
}


int main(void) {
  handle_init();

  app_event_loop();
  
  handle_deinit();
}
