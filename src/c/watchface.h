#include <pebble.h>
#pragma once

////////////////////
// font variables //
////////////////////
#define WORD_FONT RESOURCE_ID_WORD_FONT_40
#define STATUS_FONT RESOURCE_ID_STATUS_FONT_13

// persistent storage keys
#define SETTINGS_KEY 1

#define OFF 0

///////////////////
// Clay settings //
///////////////////
typedef struct ClaySettings {
  GColor BackgroundColor;
  GColor ForegroundColor;
  bool InvertColors;
  bool VibrateOnBTLost;
  uint8_t RefreshInterval;
  uint8_t BatteryWarning;
  uint16_t  VibrateInterval;
} ClaySettings; // Define our settings struct

//////////////////
// status flags
/////////////////
typedef struct Status {
    bool changed;
    bool is_charging;
    bool is_battery_discharged;
    bool is_bt_connected;
    bool is_quiet_time;
} Status;

static void config_clear();
static void config_load();
static void config_save();
static void status_clear();
static void setColors();
static void o_hide();
static void o_show();
static void draw_circle (GContext *ctx, GColor color, GPoint center, uint8_t radius, uint8_t width );
static void update_proc_main(Layer *layer, GContext *ctx);
static void update_proc_second(Layer *layer, GContext *ctx);
static void update_proc_o(Layer *layer, GContext *ctx);
static void handler_tick(struct tm *tick_time, TimeUnits units_changed);
static void handler_battery(BatteryChargeState charge_state);
static void callback_bluetooth(bool connected);
static void inbox_received_callback(DictionaryIterator *iterator, void *context);
static void inbox_dropped_callback(AppMessageResult reason, void *context);
static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context);
static void outbox_sent_callback(DictionaryIterator *iterator, void *context);
static void main_window_load(Window *window);
static void main_window_unload(Window *window);
static void init();
static void deinit();
