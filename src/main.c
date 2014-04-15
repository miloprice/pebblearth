/* 	Earth Daylight Time, a Pebble watchface by Milo Price
	some portions adapted from the "bitmap image" template on CloudPebble
	and the Pebble SDK Tutorial found at http://ninedof.wordpress.com/pebble-sdk-tutorial/
*/
#include "pebble.h"

static Window *window;

static BitmapLayer *image_layer;
static BitmapLayer *h_layer;

static TextLayer *text_layer;
static TextLayer *text_layer_shadow1;
static TextLayer *text_layer_shadow2;
static TextLayer *text_layer_shadow3;
static TextLayer *text_layer_shadow4;

static TextLayer *text_layer_weekday;
static TextLayer *text_layer_monthday;
static TextLayer *text_layer_monthname;

//static TextLayer *text_layer_test;

static GBitmap *image;
static GBitmap *himage;

enum SettingsKey{
	SETTING_DATE_KEY = 0x0,		// TUPLE_CSTRING
	SETTING_GMT_KEY = 0x1,		// TUPLE_CSTRING
};

static AppSync async;
static uint8_t sync_buffer[32];

const char *showdate = "Y";
const char *gmtmod = "000";

InverterLayer *inv_layer;	//Inverter layer

char buffer[] = "00:00";

char weekday[] = "Sun";
char monthday[] = "14";
char monthname[] = "SEPTEMBER";

int hasanim;
int lastx1 = 144;
int lastx2 = 144;
int nighthr = 0;
int lasthr = 22;

//static uint8_t syncbuffer[256];

int adj_hour(int claimedhour){
	int gmod = 0;
	if (strcmp(gmtmod,"-12")==0){
		gmod = 4;
	} else if (strcmp(gmtmod,"-11")==0){
		gmod = 3;
	} else if (strcmp(gmtmod,"-10")==0){
		gmod = 2;
		
	  //text_layer_set_text(text_layer_test, "yesfound");
	} else if (strcmp(gmtmod,"-09")==0){
		gmod = 1;
	} else if (strcmp(gmtmod,"-08")==0){
		gmod = 0;
	} else if (strcmp(gmtmod,"-07")==0){
		gmod = -1;
	} else if (strcmp(gmtmod,"-06")==0){
		gmod = -2;
	} else if (strcmp(gmtmod,"-05")==0){
		gmod = -3;
	} else if (strcmp(gmtmod,"-04")==0){
		gmod = -4;
	} else if (strcmp(gmtmod,"-03")==0){
		gmod = -5;
	} else if (strcmp(gmtmod,"-02")==0){
		gmod = -6;
	} else if (strcmp(gmtmod,"-01")==0){
		gmod = -7;
	} else if (strcmp(gmtmod,"000")==0){
		gmod = -8;
	} else if (strcmp(gmtmod,"001")==0){
		gmod = -9;
	} else if (strcmp(gmtmod,"002")==0){
		gmod = -10;
	} else if (strcmp(gmtmod,"003")==0){
		gmod = -11;
	} else if (strcmp(gmtmod,"004")==0){
		gmod = -12;
	} else if (strcmp(gmtmod,"005")==0){
		gmod = -13;
	} else if (strcmp(gmtmod,"006")==0){
		gmod = -14;
	} else if (strcmp(gmtmod,"007")==0){
		gmod = -15;
	} else if (strcmp(gmtmod,"008")==0){
		gmod = -16;
	} else if (strcmp(gmtmod,"009")==0){
		gmod = -17;
	} else if (strcmp(gmtmod,"010")==0){
		gmod = -18;
	} else if (strcmp(gmtmod,"011")==0){
		gmod = -19;
	} else if (strcmp(gmtmod,"012")==0){
		gmod = -20;
		
	  //text_layer_set_text(text_layer_test, "notfound");
	} else {
		gmod = 0;
	}
	int adjhour = claimedhour + 24 + gmod;
	return adjhour % 24;
}

void on_animation_stopped(Animation *anim, bool finished, void *context)
{   // Added for step 4
	// Free memory used by Animation
	property_animation_destroy((PropertyAnimation*) anim);
}

void animate_layer(Layer *layer, GRect *start, GRect *finish, int duration, int delay)
{
    //Declare animation
    PropertyAnimation *anim = property_animation_create_layer_frame(layer, start, finish);
 
    //Set characteristics
    animation_set_duration((Animation*) anim, duration);
    animation_set_delay((Animation*) anim, delay);
 
    //Set stopped handler to free memory
    AnimationHandlers handlers = {
        //The reference to the stopped handler is the only one in the array
        .stopped = (AnimationStoppedHandler) on_animation_stopped
    };
    animation_set_handlers((Animation*) anim, handlers, NULL);
 
    //Start animation!
    animation_schedule((Animation*) anim);
}

void tick_handler(struct tm *tick_time, TimeUnits units_changed){
	if (clock_is_24h_style()){
		strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
	} else {
		strftime(buffer, sizeof("00:00"), "%l:%M", tick_time);
	}
	
	strftime(weekday, sizeof("Sun"), "%a", tick_time);
	strftime(monthday, sizeof("14"), "%e", tick_time);
	strftime(monthname, sizeof("September"), "%B", tick_time);
	
	text_layer_set_text(text_layer_shadow1, buffer);
	text_layer_set_text(text_layer_shadow2, buffer);
	text_layer_set_text(text_layer_shadow3, buffer);
	text_layer_set_text(text_layer_shadow4, buffer);
	text_layer_set_text(text_layer, buffer);
	
	text_layer_set_text(text_layer_weekday, weekday);
	text_layer_set_text(text_layer_monthday, monthday);
	text_layer_set_text(text_layer_monthname, monthname);
	
	int hourraw = tick_time->tm_hour;
	int hours = adj_hour(hourraw);
	int mins = tick_time->tm_min;
	int secs = tick_time->tm_sec;
	int hour1 = (secs / 2) % 24;
	if ((hours > 11 && hours < 23) && ((mins == 0 && secs == 0) || hasanim == 0)){
		// 12 <= hours <= 22
		int updateamt = hours-11;
		// 1 <= updateamt <= 11
		GRect start = GRect(lastx1, 0, 144, 168);
		GRect finish = GRect(144-(updateamt * 12), 0, 144, 168);
		lastx1 = 144-(updateamt * 12);
		animate_layer(inverter_layer_get_layer(inv_layer), &start, &finish, 300, 0);
	} else if (hours < 11 && ((mins == 0 && secs == 0) || hasanim == 0)){    // hour = 0 to 11
		// 0 <= hours <= 10
		int updateamt = hours + 1;
		// 1 <= updateamt <= 11
		GRect start = GRect(0, 0, lastx2, 168);
		GRect finish = GRect(0, 0, 144-(updateamt * 12), 168);
		lastx2 = 144 - (updateamt * 12);
		animate_layer(inverter_layer_get_layer(inv_layer), &start, &finish, 300, 0);
	} else if (hours == 11 && ((mins == 0 && secs == 0) || hasanim == 0)){
		GRect start = GRect(0, 0, 12, 168);
		GRect finish = GRect(0, 0, 0, 168);
		animate_layer(inverter_layer_get_layer(inv_layer), &start, &finish, 300, 0);
		lastx1 = 144;
		lastx2 = 144;
	} else if (hours == 23 && ((mins == 0 && secs == 0) || hasanim == 0)){
		GRect start = GRect(12, 0, 144, 168);
		GRect finish = GRect(0, 0, 144, 168);
		animate_layer(inverter_layer_get_layer(inv_layer), &start, &finish, 300, 0);
		lastx1 = 144;
		lastx2 = 144;
	}
	if (strcmp(showdate,"N") == 0){
		text_layer_destroy(text_layer_weekday);
		text_layer_destroy(text_layer_monthday);
		text_layer_destroy(text_layer_monthname);
	}
    hasanim = 1;
}

static void app_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void* context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "app error %d", app_message_error);
}

static void settings_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  //int value = new_tuple->value->uint8;
  switch (key) {
    case SETTING_DATE_KEY:
      showdate = new_tuple->value->cstring;
	  //int numval = new_tuple->value->uint8;
	  //int numval2 = app_sync_get(&async,0)->value->uint8;
	  //char str[1] = "u";
	  //sprintf(str, "%d", value);
	  
	  //app_log(APP_LOG_LEVEL_DEBUG, "date value", numval2, "precedes");

      break;
    case SETTING_GMT_KEY:
	  //text_layer_destroy(text_layer_monthname);

      gmtmod = new_tuple->value->cstring;
	  //text_layer_set_text(text_layer_test, gmtmod);
	  hasanim = 0;
	  app_log(APP_LOG_LEVEL_DEBUG, "gmt value", 100, new_tuple->value->cstring);
      break;
  }
}

void window_load(Window *window)
{	
	hasanim = 0;
	
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_frame(window_layer);
	
	window_set_background_color	(window, GColorBlack);
	
	// This needs to be deinited on app exit which is when the event loop ends
	image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_GLOBE);
	himage = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BIG_H);
	
	// The bitmap layer holds the image for display
	image_layer = bitmap_layer_create(bounds);
	bitmap_layer_set_bitmap(image_layer, image);
	bitmap_layer_set_alignment(image_layer, GAlignCenter);
	layer_add_child(window_layer, bitmap_layer_get_layer(image_layer));
	

	
	//Load font
	ResHandle font_handle = resource_get_handle(RESOURCE_ID_FONT_NUNITO_48);
	ResHandle date_font = resource_get_handle(RESOURCE_ID_FONT_NUNITO_BOLD_20);

	
	//Time layer
	text_layer = text_layer_create(GRect(1, 50, 144, 168));
	text_layer_set_background_color(text_layer, GColorClear);
	text_layer_set_text_color(text_layer, GColorBlack);
	text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
	text_layer_set_font(text_layer, fonts_load_custom_font(font_handle));
	
	//layer_add_child(window_get_root_layer(window), (Layer*) text_layer);
	
	text_layer_shadow1 = text_layer_create(GRect(2, 50, 144, 168));
	text_layer_set_background_color(text_layer_shadow1, GColorClear);
	text_layer_set_text_color(text_layer_shadow1, GColorWhite);
	text_layer_set_text_alignment(text_layer_shadow1, GTextAlignmentCenter);
	text_layer_set_font(text_layer_shadow1, fonts_load_custom_font(font_handle));
	
	layer_add_child(window_get_root_layer(window), (Layer*) text_layer_shadow1);
	
	text_layer_shadow2 = text_layer_create(GRect(0, 50, 144, 168));
	text_layer_set_background_color(text_layer_shadow2, GColorClear);
	text_layer_set_text_color(text_layer_shadow2, GColorWhite);
	text_layer_set_text_alignment(text_layer_shadow2, GTextAlignmentCenter);
	text_layer_set_font(text_layer_shadow2, fonts_load_custom_font(font_handle));
	
	layer_add_child(window_get_root_layer(window), (Layer*) text_layer_shadow2);
	
	text_layer_shadow3 = text_layer_create(GRect(1, 49, 144, 168));
	text_layer_set_background_color(text_layer_shadow3, GColorClear);
	text_layer_set_text_color(text_layer_shadow3, GColorWhite);
	text_layer_set_text_alignment(text_layer_shadow3, GTextAlignmentCenter);
	text_layer_set_font(text_layer_shadow3, fonts_load_custom_font(font_handle));
	
	layer_add_child(window_get_root_layer(window), (Layer*) text_layer_shadow3);
	
	text_layer_shadow4 = text_layer_create(GRect(1, 51, 144, 168));
	text_layer_set_background_color(text_layer_shadow4, GColorClear);
	text_layer_set_text_color(text_layer_shadow4, GColorWhite);
	text_layer_set_text_alignment(text_layer_shadow4, GTextAlignmentCenter);
	text_layer_set_font(text_layer_shadow4, fonts_load_custom_font(font_handle));
	
	layer_add_child(window_get_root_layer(window), (Layer*) text_layer_shadow4);
	
	layer_add_child(window_get_root_layer(window), (Layer*) text_layer);
	
	//Inverter layer
	inv_layer = inverter_layer_create(GRect(80,0,144,162));
	layer_add_child(window_get_root_layer(window), (Layer*) inv_layer);
	
	
		// Star mask
	h_layer = bitmap_layer_create(bounds);
	bitmap_layer_set_bitmap(h_layer, himage);
	bitmap_layer_set_alignment(h_layer, GAlignCenter);
	layer_add_child(window_layer, bitmap_layer_get_layer(h_layer));
	bitmap_layer_set_compositing_mode(h_layer, GCompOpAnd);
	
		
	//Test layer
	/*text_layer_test = text_layer_create(GRect(1,140,140,168));
	text_layer_set_background_color(text_layer_test,GColorWhite);
	text_layer_set_text_color(text_layer_test,GColorBlack);
	text_layer_set_text_alignment(text_layer_test, GTextAlignmentLeft);
	text_layer_set_font(text_layer_test, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	layer_add_child(window_get_root_layer(window), (Layer*) text_layer_test);
	text_layer_set_text(text_layer_test, "Inits");*/
	
	
		// Day of week
	text_layer_weekday = text_layer_create(GRect(0, 28, 30, 50));
	text_layer_set_background_color(text_layer_weekday, GColorClear);
	text_layer_set_text_color(text_layer_weekday, GColorWhite);
	text_layer_set_text_alignment(text_layer_weekday, GTextAlignmentLeft);
	text_layer_set_font(text_layer_weekday, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	
	layer_add_child(window_get_root_layer(window), (Layer*) text_layer_weekday);
	
		// Name of month
	text_layer_monthname = text_layer_create(GRect(0, 0, 50, 20));
	text_layer_set_background_color(text_layer_monthname, GColorClear);
	text_layer_set_text_color(text_layer_monthname, GColorWhite);
	text_layer_set_text_alignment(text_layer_monthname, GTextAlignmentLeft);
	text_layer_set_font(text_layer_monthname, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	
	layer_add_child(window_get_root_layer(window), (Layer*) text_layer_monthname);
	
	
		// Day of month
	text_layer_monthday = text_layer_create(GRect(0, 11, 30, 30));
	text_layer_set_background_color(text_layer_monthday, GColorClear);
	text_layer_set_text_color(text_layer_monthday, GColorWhite);
	text_layer_set_text_alignment(text_layer_monthday, GTextAlignmentLeft);
	text_layer_set_font(text_layer_monthday, fonts_load_custom_font(date_font));
	
	layer_add_child(window_get_root_layer(window), (Layer*) text_layer_monthday);
		
}

void window_unload(Window *window)
{
	text_layer_destroy(text_layer);
	text_layer_destroy(text_layer_shadow1);
	text_layer_destroy(text_layer_shadow2);
	text_layer_destroy(text_layer_shadow3);
	text_layer_destroy(text_layer_shadow4);
	text_layer_destroy(text_layer_weekday);
	text_layer_destroy(text_layer_monthday);
	text_layer_destroy(text_layer_monthname);
	//text_layer_destroy(text_layer_test);
	
	inverter_layer_destroy(inv_layer);
}

void handle_init(void) {
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	
	window_stack_push(window, true /* Animated */);
	
	
	
	tick_timer_service_subscribe(SECOND_UNIT, (TickHandler) tick_handler);

}

static void s_date(int shoulddate) {
		text_layer_destroy(text_layer_weekday);
		text_layer_destroy(text_layer_monthday);
		text_layer_destroy(text_layer_monthname);
}

/*static void s_gmt(int offset){
	gmtmod = offset;
}*/

//static void in_received_handler(DictionaryIterator *iter, void *context) {
  /*Tuple *set_gmt = dict_find(iter, SETTING_GMT);
  Tuple *set_date = dict_find(iter, SETTING_DATE);

  if (set_gmt) {
    s_gmt((int)set_gmt);
  }

  if (set_date) {
    s_date((int)set_gmt);
  }*/
//}

static void app_message_init(void) {
  // Reduce the sniff interval for more responsive messaging at the expense of
  // increased energy consumption by the Bluetooth module
  // The sniff interval will be restored by the system after the app has been
  // unloaded
  app_comm_set_sniff_interval(SNIFF_INTERVAL_REDUCED);
  	// Init buffers
	const int inbound_size = 64;
	const int outbound_size = 64;
	app_message_open(inbound_size, outbound_size);
	
	Tuplet settings_init[] = {
		TupletCString(SETTING_DATE_KEY, "Y"),
		TupletCString(SETTING_GMT_KEY, "None"),
	};
	/*app_message_open(160,160);*/
	app_sync_init(&async, sync_buffer, sizeof(sync_buffer), settings_init, ARRAY_LENGTH(settings_init),
				 settings_changed_callback, app_error_callback, NULL);
	
	// Register message handlers
	//app_message_register_inbox_received(in_received_handler);
}

int main(void) {
	
	app_message_init();
	handle_init();


  	app_event_loop();

	tick_timer_service_unsubscribe();
	
  	gbitmap_destroy(image);
  	gbitmap_destroy(himage);
	
	app_sync_deinit(&async);
	
	bitmap_layer_destroy(image_layer);
	bitmap_layer_destroy(h_layer);
	window_destroy(window);
}
