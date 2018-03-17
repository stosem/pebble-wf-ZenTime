// Zen Time
// pebble watchface by stosem (github.com/stosem)
//
//

#include <pebble.h>
#include "watchface.h"
//#define DEBUG 1
//#define DEMO 1

static Window *s_main_window, *s_sec_window;
static Layer *s_main_layer, *s_o_layer;
static TextLayer *s_status_layer, *s_nw_layer;
static GFont s_word_font, s_status_font;
static ClaySettings settings; // An instance of the struct
static Status status;
static bool SecLoaded = false;

// MAIN

///////////////////////////////
// set default Clay settings //
///////////////////////////////
static void config_clear() {
  settings.BackgroundColor = GColorBlack;
  settings.ForegroundColor = GColorWhite;
  settings.InvertColors = false;
  settings.RefreshInterval = 1;
  settings.BatteryWarning = 30;
  settings.VibrateInterval = OFF;
  settings.VibrateOnBTLost = true;
};


/////////////////////////////////////
// load default settings from Clay //
/////////////////////////////////////
static void config_load() {
	persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));	// Read settings from persistent storage, if they exist
#ifdef DEBUG 
  APP_LOG(APP_LOG_LEVEL_DEBUG, "config_load");
#endif
};


/////////////////////////
// saves user settings //
/////////////////////////
static void config_save() {
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
#ifdef DEBUG 
  APP_LOG(APP_LOG_LEVEL_DEBUG, "config_save");
#endif
};


//////////////////////////////
// set defaults status
// //////////////////////////
static void status_clear() {
    status.changed=true;
    status.is_charging=false;
    status.is_battery_discharged=0;
    status.is_bt_connected=false;
    status.is_quiet_time=false;
};


///////////////////////
// sets watch colors //
///////////////////////
static void setColors() {
#ifdef DEBUG 
  APP_LOG(APP_LOG_LEVEL_DEBUG, "setColors");
#endif
  if(settings.InvertColors==1) {
    settings.BackgroundColor = GColorWhite;
    settings.ForegroundColor = GColorBlack;
  } else {
    settings.BackgroundColor = GColorBlack;
    settings.ForegroundColor = GColorWhite;
  }
  // set background color
  window_set_background_color(s_main_window, settings.BackgroundColor);
  // set text color for TextLayers
  text_layer_set_text_color(s_nw_layer, settings.ForegroundColor);
  text_layer_set_text_color(s_status_layer, settings.ForegroundColor);
};


////////////////
// hide o window
// ////////////
static void o_hide () {
  if (SecLoaded) {
      window_stack_pop(false);
      SecLoaded=false;
  }
};

////////////
// show o window
// ////////
static void o_show() {
  if (!SecLoaded){
    window_stack_push(s_sec_window, false);
    SecLoaded=true;  
    //optionally move frequently updated layer to frequently updated window.
  } 
};


///////////////////////
// sub function for draw circles
// ///////////////////
static void draw_circle (GContext *ctx, GColor color, GPoint center, uint8_t radius, uint8_t width ) {
  graphics_context_set_stroke_color(ctx, color);
  graphics_context_set_fill_color(ctx, color);
  graphics_context_set_stroke_width(ctx, width);
  graphics_draw_circle(ctx, center, radius);
};


/////////////////////////
// draw main window //
/////////////////////////
static void update_proc_main(Layer *layer, GContext *ctx) {
#ifdef DEBUG
 APP_LOG(APP_LOG_LEVEL_DEBUG,"Redraw: main window");
#endif

  o_hide() ;

  text_layer_set_text(s_nw_layer, "NOW"); //
  // write date to buffer
  static char str_buffer[128];
  strcpy( str_buffer, "" );

  status.is_quiet_time = quiet_time_is_active();  
#ifdef DEMO
  status.is_quiet_time = true; 
#endif
  if ( status.is_quiet_time ) { strcat( str_buffer, " silence" ); };
  if ( status.is_battery_discharged ) { strcat( str_buffer, " void"); }; 
  if ( !status.is_bt_connected ) { strcat( str_buffer, " freedom"); };
  if ( status.is_charging ) { strcat( str_buffer, " power"); };
  
  status.changed = false;
  // display this time on the text layer
  text_layer_set_text(s_status_layer, str_buffer); //

#ifdef DEBUG
 APP_LOG(APP_LOG_LEVEL_DEBUG,"call sec_window");
#endif

  if ( settings.RefreshInterval != 0 ) o_show() ;
};


///////////////
//  draw second window
//////////////
static void update_proc_second(Layer *layer, GContext *ctx) {
#ifdef DEBUG
 APP_LOG(APP_LOG_LEVEL_DEBUG,"Redraw: second windows");
#endif
    graphics_context_set_fill_color(ctx, settings.BackgroundColor);
    graphics_fill_rect(ctx, GRect(PBL_IF_ROUND_ELSE(65,49), PBL_IF_ROUND_ELSE(70,60), 38, 42), 0, 0 );
};


/////////////////////////////////
// draw O  
/////////////////////////////////
static void update_proc_o(Layer *layer, GContext *ctx) {
#ifdef DEBUG
 APP_LOG(APP_LOG_LEVEL_DEBUG,"Redraw: circle layer");
#endif
  if ( settings.RefreshInterval == 0 ) return ; // no draw on no refresh 

  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds); 
  //center.x -= 4;
  //center.y -= 4;
    
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
 
//  graphics_draw_rotated_bitmap !! bug in diore

  int radius = 15;
  // O
  draw_circle( ctx, settings.ForegroundColor, center, radius, 4 );
  // other
  float angle;
  GPoint t_center;
  // black  hour
  angle = TRIG_MAX_ANGLE * ((((t->tm_hour % 12) * 6) + ((t->tm_min+10) / 10))) / (12 * 6);
  t_center.x = (int)(sin_lookup(angle) * (int)(radius-2) / TRIG_MAX_RATIO) + center.x,
  t_center.y = (int)(-cos_lookup(angle) * (int)(radius-2) / TRIG_MAX_RATIO) + center.y,
  draw_circle( ctx, settings.BackgroundColor, t_center, 2, 6 );
  // white after hour 
  angle = TRIG_MAX_ANGLE * ((((t->tm_hour % 12) * 6) + ((t->tm_min+30) / 10))) / (12 * 6);
  t_center.x = (int)(sin_lookup(angle) * (int)(radius+1) / TRIG_MAX_RATIO) + center.x,
  t_center.y = (int)(-cos_lookup(angle) * (int)(radius+1) / TRIG_MAX_RATIO) + center.y,
  draw_circle( ctx, settings.ForegroundColor, t_center, 2, 5 );
  // minute
  if ( settings.RefreshInterval < 60 ) { // display minutes
    angle = TRIG_MAX_ANGLE * (t->tm_min) / 60;  
    t_center.x = (int)(sin_lookup(angle) * (int)(radius+3) / TRIG_MAX_RATIO) + center.x,
    t_center.y = (int)(-cos_lookup(angle) * (int)(radius+3) / TRIG_MAX_RATIO) + center.y,
    draw_circle( ctx, settings.BackgroundColor, t_center, 1, 3 );
  };
};



//////////////////
// handle ticks //
//////////////////
static void handler_tick(struct tm *tick_time, TimeUnits units_changed) {
#ifdef DEBUG
  APP_LOG(APP_LOG_LEVEL_DEBUG,"Run: handler tick");
#endif
  // update quiet time status
  bool qt =  quiet_time_is_active();
  if ( status.is_quiet_time != qt ) {
      status.changed = true;
  }
  status.is_quiet_time = qt;
#ifdef DEMO
  status.is_quiet_time = true; 
#endif

 // vibrate 
  if (settings.VibrateInterval > 0) {
     if ( (tick_time->tm_min % settings.VibrateInterval == 0) || (
          (tick_time->tm_min == 0 ) && ( settings.VibrateInterval == 60) ) )  {
        if (!status.is_quiet_time) { vibes_short_pulse(); };
     };
  };

  // refresh status
  if (status.changed) {
      o_hide();
      layer_mark_dirty( s_main_layer );
  };

  if ( settings.RefreshInterval == 0 ) { return; }; // no refresh time
  if ( tick_time->tm_min % settings.RefreshInterval == 0 ) {
  	// refresh interval 
  	layer_mark_dirty( s_o_layer );
  };
};


/////////////////////////////////////
// registers battery update events //
/////////////////////////////////////
static void handler_battery(BatteryChargeState charge_state) {
#ifdef DEBUG
  APP_LOG(APP_LOG_LEVEL_DEBUG,"Run: handler battery");
#endif
  bool bd = ( charge_state.charge_percent <= settings.BatteryWarning );
  if ( status.is_battery_discharged != bd ) {
      status.is_battery_discharged = bd;
      status.changed = true;
  };
  bool ch=(charge_state.is_charging || charge_state.is_plugged );
  if ( status.is_charging != ch ) {
      status.is_charging = ch;
      status.changed = true;
      o_hide(); // force update main layer now (fast status)
      layer_mark_dirty( s_main_layer );
  };
#ifdef DEMO
  status.is_charging = true; 
  status.is_battery_discharged = true;
#endif
};


/////////////////////////////
// manage bluetooth status //
/////////////////////////////
static void callback_bluetooth(bool connected) {
#ifdef DEBUG
  APP_LOG(APP_LOG_LEVEL_DEBUG,"Run: bluetooth_callback");
#endif
#ifdef DEMO
  connected=false;
#endif
 // vibrate on disconnect not on quiet time 
  if(status.is_bt_connected && !connected && !status.is_quiet_time && settings.VibrateOnBTLost ) {
    vibes_double_pulse();
  }
 // check if status changed
  if(status.is_bt_connected != connected) {
      status.is_bt_connected=connected;
      status.changed = true;
      o_hide();
      layer_mark_dirty( s_main_layer ); // force update on bluetooth
  }
#ifdef DEBUG
   APP_LOG(APP_LOG_LEVEL_DEBUG,"Redraw: bluetooth, %d", status.is_bt_connected);
#endif
};


////////////////////////////
// Clay calls //
////////////////////////////
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
#ifdef DEBUG
  APP_LOG(APP_LOG_LEVEL_INFO, "inbox_received_callback");
#endif
  // Read tuples for data
 
  // inverted colors
  Tuple *invert_colors_t = dict_find(iterator, MESSAGE_KEY_KEY_INVERT_COLORS);
  if(invert_colors_t) { settings.InvertColors = invert_colors_t->value->int32 == 1; }

  // vibrate on bt lost
  Tuple *vibrate_on_bt_lost_t = dict_find(iterator, MESSAGE_KEY_KEY_VIBRATE_ON_BT_LOST);
  if(vibrate_on_bt_lost_t) { settings.VibrateOnBTLost = vibrate_on_bt_lost_t->value->int32 == 1; }

  // vibrate interval
  Tuple *vibrate_interval_t = dict_find(iterator, MESSAGE_KEY_KEY_VIBRATE_INTERVAL);
  if(vibrate_interval_t) { 
      settings.VibrateInterval = atoi(vibrate_interval_t->value->cstring);
#ifdef DEBUG
    APP_LOG(APP_LOG_LEVEL_INFO, "set Vibrate interval=%d", settings.VibrateInterval );
#endif
  }

  // power save mode
  Tuple *refresh_interval_t = dict_find(iterator, MESSAGE_KEY_KEY_REFRESH_INTERVAL);
  if(refresh_interval_t) { settings.RefreshInterval = atoi(refresh_interval_t->value->cstring); }

  // battery warinig level
  Tuple *battery_warning_t = dict_find(iterator, MESSAGE_KEY_KEY_BATTERY_WARNING);
  if(battery_warning_t) { settings.BatteryWarning = atoi(battery_warning_t->value->cstring); }
 
	setColors();	
    status.changed = true;
    o_hide();
    layer_mark_dirty( s_main_layer );

	config_save();
};


static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
};


static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
};


static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
};


//////////////////////
// load main window //
//////////////////////
static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  // fonts
  s_word_font = fonts_load_custom_font(resource_get_handle(WORD_FONT));
  s_status_font = fonts_load_custom_font(resource_get_handle(STATUS_FONT));

  // setup layers

  // create main canvas layer 
  s_main_layer = layer_create(bounds);
  layer_set_update_proc(s_main_layer, update_proc_main);
  layer_add_child(window_layer, s_main_layer);  
  
 // create status layer text
  s_status_layer = text_layer_create(GRect(PBL_IF_ROUND_ELSE(26, 10), PBL_IF_ROUND_ELSE(130, 140), 128, 32));
  text_layer_set_background_color(s_status_layer, GColorClear); // xxx
  text_layer_set_text_alignment(s_status_layer, GTextAlignmentCenter);
  text_layer_set_font(s_status_layer, s_status_font);
  layer_add_child(s_main_layer, text_layer_get_layer(s_status_layer));  
  
 // create N_W layer text
  s_nw_layer = text_layer_create(GRect(PBL_IF_ROUND_ELSE(28, 12), PBL_IF_ROUND_ELSE(70, 60), 126, 46));
  text_layer_set_background_color(s_nw_layer, GColorClear); // xxx
  text_layer_set_text_alignment(s_nw_layer, GTextAlignmentCenter);
  text_layer_set_font(s_nw_layer, s_word_font);
  layer_add_child(s_main_layer, text_layer_get_layer(s_nw_layer));  

  // detect quiet
  status.is_quiet_time = quiet_time_is_active(); 

  status_clear();
  setColors();	
  config_save(); 
#ifdef DEBUG 
  APP_LOG(APP_LOG_LEVEL_DEBUG, "main_window_load");
#endif
};


///////////////////
// unload window //
///////////////////
static void main_window_unload(Window *window) {
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  
  text_layer_destroy(s_status_layer);
  text_layer_destroy(s_nw_layer);

  layer_destroy(s_main_layer); 
  layer_destroy(s_o_layer); 

  fonts_unload_custom_font(s_word_font);
  fonts_unload_custom_font(s_status_font);

#ifdef DEBUG
    APP_LOG(APP_LOG_LEVEL_INFO, "end main window unload");
#endif
};


////////////////////
// initialize app //
////////////////////
static void init() {
  config_clear();
  config_load();
  
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  // show window on the watch with animated=true
  window_stack_push(s_main_window, false);

  // creating the smaller window for the frequent updates.
  s_sec_window = window_create();
  window_set_background_color(s_sec_window, GColorClear); 
  Layer *window_sec_layer = window_get_root_layer(s_sec_window);
  layer_set_update_proc(window_sec_layer, update_proc_second);
  s_o_layer = layer_create(GRect(PBL_IF_ROUND_ELSE(65,49), PBL_IF_ROUND_ELSE(71,61), 38, 38));
  layer_set_update_proc(s_o_layer, update_proc_o);
  layer_add_child(window_sec_layer, s_o_layer);
  
  // subscribe to time events
  tick_timer_service_subscribe(MINUTE_UNIT, handler_tick); /

  // register with Battery State Service
  battery_state_service_subscribe(handler_battery);
  // force initial update
  handler_battery(battery_state_service_peek());      
  
  // register with bluetooth state service
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = callback_bluetooth
  });
  // force bluetooth
  callback_bluetooth(connection_service_peek_pebble_app_connection());  
    
  // Register weather callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);  
  app_message_open(128, 128);  

#ifdef DEBUG 
  APP_LOG(APP_LOG_LEVEL_DEBUG, "init");  
#endif
};


///////////////////////
// de-initialize app //
///////////////////////
static void deinit() {
  window_destroy(s_sec_window);
  window_destroy(s_main_window);
};


/////////////
// run app //
/////////////
int main(void) {
  init();
  app_event_loop();
  deinit();
};
