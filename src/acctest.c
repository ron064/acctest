/*
 * This registers and AccelerometerService handler and increments a counter each time
 * the handler is called.  Pressing the down button displays the current count.
 * After a fairly short time (typically < 3 minutes) it stops updating.
 * Update(By Ron): 	Added tick timer service to update screen every sec.
 * 					Added restart or accelerometer service and fail count.
 */

#include <pebble.h>

static Window *window;
static TextLayer *text_layer;
static TextLayer *text_layer_fail;
static uint16_t acc_count = 0;
static uint16_t fail_count = 0;
AccelSamplingRate sample_rate = ACCEL_SAMPLING_100HZ;
	
static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(text_layer, "Select");
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {

	static char percentText[11];
	BatteryChargeState state = battery_state_service_peek();
	snprintf(percentText,sizeof(percentText), "Bat: %hd%%",state.charge_percent);

	text_layer_set_text(text_layer, percentText);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
	static char msg[30];
	snprintf(msg, sizeof(msg), "Acc count %u", acc_count);
	text_layer_set_text(text_layer, msg);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  text_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(text_layer, "Press Down for Count");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));

  text_layer_fail = text_layer_create((GRect) { .origin = { 0, 100 }, .size = { bounds.size.w, 20 } });
  //text_layer_set_text(text_layer_fail, "Press Down for Count");
  text_layer_set_text_alignment(text_layer_fail, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer_fail));
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
  text_layer_destroy(text_layer_fail);

}

static void accelHandle(AccelData *data, uint32_t num_samples) {
	acc_count++;
}

void handle_second_tick(struct tm *tick_time, TimeUnits units_changed)
{
	static uint16_t last_count = 0;
	static char msg[30];
	static char fail_msg[30];
	
	if (acc_count>last_count)
	{
		snprintf(msg, sizeof(msg), "Acc count %u", acc_count);
		text_layer_set_text(text_layer, msg);
		last_count = acc_count;
	}
	else
	if (last_count!=0)
	{
		accel_data_service_unsubscribe();
		fail_count++;
		snprintf(fail_msg, sizeof(fail_msg), "Fails:%u  last at: %u", fail_count, acc_count);
		text_layer_set_text(text_layer_fail, fail_msg);
		acc_count = 0; last_count=0;
		accel_service_set_sampling_rate(sample_rate);
		accel_data_service_subscribe(25, accelHandle);
	}		
}

static void acc_start()
{
	accel_service_set_sampling_rate(sample_rate);
	accel_data_service_subscribe(25, accelHandle);
}

static void acc_stop()
{
	accel_data_service_unsubscribe();
}

static void init(void) {
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
   tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);
	

  acc_start();

  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
  acc_stop();
}

int main(void) {
  init();

  app_event_loop();
  deinit();
}
