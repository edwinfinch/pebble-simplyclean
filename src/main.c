/*
Simply Clean
Edwin Finch
Designed by Philipp Weder

Under MIT license
*/

#include <pebble.h>
#include "elements.h"
#include "appkeys.h"
#include "battbar.h"
void glance_this(const char *glancetext, bool vibrate, int vibrateNum, int animationLength, int fullNotify);
void tick_handler(struct tm *t, TimeUnits units_changed);
void bt_handler(bool connected);
bool refresh_theme(bool connected);
void refresh_battbar();
#define WHITE_THEME 1
#define DARK_THEME 0

static TextLayer* text_layer_init(GRect location, GColor background, GTextAlignment alignment, int font)
{
	TextLayer *layer = text_layer_create(location);
	text_layer_set_text_color(layer, GColorBlack);
	text_layer_set_background_color(layer, background);
	text_layer_set_text_alignment(layer, alignment);
	if(font == 1){
		text_layer_set_font(layer, h_n);
	}
	else if(font == 2){
		text_layer_set_font(layer, h_n_small);
	}
	else if(font == 3){
		text_layer_set_font(layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
	}
	return layer;
}

void stopped(Animation *anim, bool finished, void *context)
{
    property_animation_destroy((PropertyAnimation*) anim);
}
 
void animate_layer(Layer *layer, GRect *start, GRect *finish, int duration, int delay)
{
    PropertyAnimation *anim = property_animation_create_layer_frame(layer, start, finish);
     
    animation_set_duration((Animation*) anim, duration);
    animation_set_delay((Animation*) anim, delay);
     
    AnimationHandlers handlers = {
        .stopped = (AnimationStoppedHandler) stopped
    };
    animation_set_handlers((Animation*) anim, handlers, NULL);
     
    animation_schedule((Animation*) anim);
}


void glance_this(const char *glancetext, bool vibrate, int vibrateNum, int animationLength, int fullNotify){
	if(currentlyGlancing == 1){
		return;
	}
	else if(currentlyGlancing == 0){
			if(vibrate == true){
				if(vibrateNum == 1){
					vibes_short_pulse();
		        }
				else if(vibrateNum == 2){
					vibes_double_pulse();
		        }
				else if(vibrateNum == 3){
					vibes_long_pulse();
		        }
			}
			snprintf(glance_buffer, sizeof(glance_buffer), "%s", glancetext);
			text_layer_set_text(update_at_a_glance, glance_buffer);
			currentlyGlancing = 1;
				GRect start01 = GRect(0, 300, 144, 168);
				GRect finish02 = GRect(0, 300, 144, 168);
				if(fullNotify == 2){
					finish01 = GRect(0, 73, 144, 168);
					start02 = GRect(0, 73, 144, 168);
				}
				else if(fullNotify == 1){
					finish01 = GRect(0, 0, 144, 168);
					start02 = GRect(0, 0, 144, 168);
		        }
				else if(fullNotify == 0){
					finish01 = GRect(0, 145, 144, 168);
					start02 = GRect(0, 145, 144, 168);
		        }
				animate_layer(text_layer_get_layer(update_at_a_glance), &start01, &finish01, 1000, 0);
				animate_layer(text_layer_get_layer(update_at_a_glance), &start02, &finish02, 1000, animationLength);
	}
}


void reload_settings(){
	//Update language
	text_layer_set_text(tis_layer, lang_itis[settings.language]);
	
	//Hide layers accordingly
	layer_set_hidden(text_layer_get_layer(date_layer), !settings.showdate);
	layer_set_hidden(text_layer_get_layer(battery_layer), !settings.showbattext);
	
	//Refresh tick_handler to update date text with proper language
	struct tm *t;
  	time_t temp;        
  	temp = time(NULL);        
  	t = localtime(&temp);
	
	tick_handler(t, MINUTE_UNIT);
	
	//Refresh bluetooth themes
	bool ignore_me = bluetooth_connection_service_peek();
	refresh_theme(ignore_me);
	
	refresh_battbar();
}

void process_tuple(Tuple *t)
{
	int key = t->key;
	int value = t->value->int32;
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Loaded key num: %d with value %d", key, value);
  switch (key) {
	  case DATEFORMAT_KEY:
	  	settings.dateformat = value;
	  	break;
	  case BTDISALERT_KEY:
	  	settings.btdisalert = value;
	  	break;
	  case BTREALERT_KEY:
	  	settings.btrealert = value;
	  	break;
	  case SHOWDESTEXT_KEY:
	  	settings.showdestext = value;
	  	break;
	  case LANGUAGE_KEY:
	 	settings.language = value;
	  	break;
	  case BTDISTHEME_KEY:
	  	settings.btdistheme = value;
	  	break;
	  case BTCONTHEME_KEY:
	  	settings.btcontheme = value;
	  	break;
	  case SHOWBATTEXT_KEY:
	  	settings.showbattext = value;
	  	break;
	  case SHOWDATE_KEY:
	  	settings.showdate = value;
	  	reload_settings();
	    glance_this("Settings updated.", 1, 2, 5000, 0);
	  	break;
	  case WFUPDATE_KEY:
	  	if(versionChecked == 0){
				if(value > currentAppVer){
					APP_LOG(APP_LOG_LEVEL_WARNING, "Watchapp version outdated");
					glance_this("Watchface version out of date! Unload watchface and load again from the appstore or MyPebbleFaces to update. The new version contains new features and bug fixes.", 1, 3, 15000, 1);
				}
				else if(value == currentAppVer){
					APP_LOG(APP_LOG_LEVEL_INFO, "Watchapp version the same as API");
				}
				else if(value < currentAppVer){
					APP_LOG(APP_LOG_LEVEL_INFO, "Watchapp version ahead of API! You must be an eleet 1337 hax0r.");
					glance_this("Hello beta tester :)", 0, 0, 4000, 0);
				}
			versionChecked = 1;
		  }
	  	break;
  }
}

static void in_received_handler(DictionaryIterator *iter, void *context) 
{
	(void) context;

	Tuple *t = dict_read_first(iter);
	if(t)
	{
		process_tuple(t);
	}
	while(t != NULL)
	{
		t = dict_read_next(iter);
		if(t)
		{
			process_tuple(t);
		}
	}
}

void format_date_buffer(struct tm *t){
	if(settings.dateformat == 0){
		if(settings.showdestext){
			switch(settings.language){
				case 0:
					strftime(date_buffer, sizeof(date_buffer), "on %d %b. %Y", t);
					break;
				case 1:
					strftime(date_buffer, sizeof(date_buffer), "am %d %b. %Y", t);
					break;
				case 2:
					strftime(date_buffer, sizeof(date_buffer), "en %d %b. %Y", t);
					break;
				case 3:
					strftime(date_buffer, sizeof(date_buffer), "op %d %b. %Y", t);
					break;
			}
		}
		else{
			switch(settings.language){
				case 0:
					strftime(date_buffer, sizeof(date_buffer), "%d %b. %Y", t);
					break;
				case 1:
					strftime(date_buffer, sizeof(date_buffer), "%d %b. %Y", t);
					break;
				case 2:
					strftime(date_buffer, sizeof(date_buffer), "%d %b. %Y", t);
					break;
				case 3:
					strftime(date_buffer, sizeof(date_buffer), "%d %b. %Y", t);
					break;
			}
		}
	}
	else if(settings.dateformat == 1){
		strftime(date_buffer, sizeof(date_buffer), "%d/%m/%y", t);
	}
	else if(settings.dateformat == 2){
		strftime(date_buffer, sizeof(date_buffer), "%D", t);
	}
	else{
		snprintf(date_buffer, sizeof(date_buffer), "Err. 01, date format missing");
	}
}

void tick_handler(struct tm *t, TimeUnits units_changed){
	if(clock_is_24h_style()){
		strftime(timeBuffer, sizeof(timeBuffer), "%H:%M", t);
	}
	else{
		strftime(timeBuffer,sizeof(timeBuffer),"%I:%M", t);
	}
	text_layer_set_text(time_layer, timeBuffer);
	
	format_date_buffer(t);
	
	text_layer_set_text(date_layer, date_buffer);
}

void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100%";
  if (charge_state.is_charging) {
	  snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
    } else {
        snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
      }
	text_layer_set_text(battery_layer, battery_text);
	DrawBattBar(); /* Initial display of the bar */
}

void refresh_battbar(){
	Layer *window_layer = window_get_root_layer(window);
	
	BBOptions options;
	options.position = BATTBAR_POSITION_BOTTOM;
	options.direction = BATTBAR_DIRECTION_DOWN;
	if(refresh_theme(bluetooth_connection_service_peek())){
		APP_LOG(APP_LOG_LEVEL_INFO, "Theme is black");
		options.color = BATTBAR_COLOR_BLACK;
	}
	else{
		options.color = BATTBAR_COLOR_WHITE;
		APP_LOG(APP_LOG_LEVEL_INFO, "Theme is white");
	}
	options.isWatchApp = false;
	SetupBattBar(options, window_layer);
	DrawBattBar();
}

bool refresh_theme(bool connected){
	if(connected){
		if(settings.btcontheme == WHITE_THEME){
			layer_set_hidden(inverter_layer_get_layer(theme), WHITE_THEME);
		}
		else{
			layer_set_hidden(inverter_layer_get_layer(theme), DARK_THEME);
		}
	}
	else{
		if(settings.btdistheme == WHITE_THEME){
			layer_set_hidden(inverter_layer_get_layer(theme), WHITE_THEME);
		}
		else{
			layer_set_hidden(inverter_layer_get_layer(theme), DARK_THEME);
		}
	}
	bool return_data = layer_get_hidden(inverter_layer_get_layer(theme));
	return return_data;
}

void bt_handler(bool connected){
	if(booted == 1){
		if(connected){
			if(settings.btrealert){
				snprintf(glance_buffer, sizeof(glance_buffer), "%s", lang_con[settings.language]);
				glance_this(glance_buffer, 1, 1, 5000, 0);
			}
		}
		else{
			if(settings.btdisalert){
				snprintf(glance_buffer, sizeof(glance_buffer), "%s", lang_dis[settings.language]);
				glance_this(glance_buffer, 1, 2, 5000, 0);
			}
		}
		bool themeenabled = refresh_theme(connected);
		APP_LOG(APP_LOG_LEVEL_INFO, "Theme set to %d (1 == white, 0 == black)", (int)themeenabled);
		
		refresh_battbar();
	}
	else{
		bool themeenabled = refresh_theme(connected);
		APP_LOG(APP_LOG_LEVEL_INFO, "Theme set to %d (1 == white, 0 == black)", (int)themeenabled);
		booted = 1;
	}
}

void window_load(Window *w){
	Layer *window_layer = window_get_root_layer(w);
	time_layer = text_layer_init(GRect(0, 40, 144, 168), GColorClear, GTextAlignmentCenter, 1);
	date_layer = text_layer_init(GRect(0, 100, 144, 168), GColorClear, GTextAlignmentCenter, 2);
	battery_layer = text_layer_init(GRect(0, 140, 144, 168), GColorClear, GTextAlignmentCenter, 2);
	tis_layer = text_layer_init(GRect(-45, 20, 144, 168), GColorClear, GTextAlignmentCenter, 2);
	update_at_a_glance = text_layer_init(GRect(0, 300, 144, 168), GColorWhite, GTextAlignmentCenter, 3);
	theme = inverter_layer_create(GRect(0, 0, 144, 168));
	
	text_layer_set_text(tis_layer, "It is");
	layer_add_child(window_layer, text_layer_get_layer(time_layer));
	layer_add_child(window_layer, text_layer_get_layer(date_layer));
	layer_add_child(window_layer, text_layer_get_layer(tis_layer));
	layer_add_child(window_layer, text_layer_get_layer(battery_layer));
	layer_add_child(window_layer, text_layer_get_layer(update_at_a_glance));
	refresh_battbar(); //If you don't call it here it crashes
	layer_add_child(window_layer, inverter_layer_get_layer(theme));
	
	BatteryChargeState charge = battery_state_service_peek();
	handle_battery(charge);
	
	struct tm *t;
  	time_t temp;        
  	temp = time(NULL);        
  	t = localtime(&temp);
	
	tick_handler(t, MINUTE_UNIT);
	reload_settings();
	
	bt_handler(bluetooth_connection_service_peek());
}

void window_unload(Window *w){
	text_layer_destroy(time_layer);
	text_layer_destroy(date_layer);
	text_layer_destroy(battery_layer);
	text_layer_destroy(tis_layer);
	text_layer_destroy(update_at_a_glance);
	inverter_layer_destroy(theme);
}
	
void init(){
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers){
		.load = window_load,
		.unload = window_unload,
	});
	h_n = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HELVETICA_NEUE_50));
	h_n_small = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HELVETICA_NEUE_18));
	battery_state_service_subscribe(&handle_battery);
	app_message_register_inbox_received(in_received_handler);
	app_message_open(1028, 512);
	bluetooth_connection_service_subscribe(&bt_handler);
	tick_timer_service_subscribe(MINUTE_UNIT, &tick_handler);
	
	persistvalue = persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
	APP_LOG(APP_LOG_LEVEL_INFO, "Simply Clean initialied. %d bytes loaded.", persistvalue);
	
	window_stack_push(window, true);
}

void deinit(){
	fonts_unload_custom_font(h_n);
	fonts_unload_custom_font(h_n_small);
	window_destroy(window);
	tick_timer_service_unsubscribe();
	persistvalue = persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
	APP_LOG(APP_LOG_LEVEL_INFO, "Simply Clean deinitialied. %d bytes written.", persistvalue);
}

int main(){
	init();
	app_event_loop();
	deinit();
}