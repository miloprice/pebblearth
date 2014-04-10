/* 	Global Shadow, a Pebble watchface by Milo Price
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

static GBitmap *image;
static GBitmap *himage;

InverterLayer *inv_layer;	//Inverter layer

char buffer[] = "00:00";

int hasanim;
int lastx1 = 144;
int lastx2 = 144;
int nighthr = 0;
int lasthr = 22;

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
	
	text_layer_set_text(text_layer_shadow1, buffer);
	text_layer_set_text(text_layer_shadow2, buffer);
	text_layer_set_text(text_layer_shadow3, buffer);
	text_layer_set_text(text_layer_shadow4, buffer);
	text_layer_set_text(text_layer, buffer);
	
	int hours = tick_time->tm_hour;
	int mins = tick_time->tm_min;
	int secs = tick_time->tm_sec;
	int hour1 = (secs / 2) % 24;
	if ((hours > 11 && hours < 23) && (mins == 0 || hasanim == 0)){
		// 12 <= hours <= 22
		int updateamt = hours-11;
		// 1 <= updateamt <= 11
		GRect start = GRect(lastx1, 0, 144, 168);
		GRect finish = GRect(144-(updateamt * 12), 0, 144, 168);
		lastx1 = 144-(updateamt * 12);
		animate_layer(inverter_layer_get_layer(inv_layer), &start, &finish, 300, 0);
	} else if (hours < 11 && (mins == 0 || hasanim == 0)){    // hour = 0 to 11
		// 0 <= hours <= 10
		int updateamt = hours + 1;
		// 1 <= updateamt <= 11
		GRect start = GRect(0, 0, lastx2, 168);
		GRect finish = GRect(0, 0, 144-(updateamt * 12), 168);
		lastx2 = 144 - (updateamt * 12);
		animate_layer(inverter_layer_get_layer(inv_layer), &start, &finish, 300, 0);
	} else if (hours == 11 && (mins == 0 || hasanim == 0)){
		GRect start = GRect(0, 0, 12, 168);
		GRect finish = GRect(0, 0, 0, 168);
		animate_layer(inverter_layer_get_layer(inv_layer), &start, &finish, 300, 0);
		lastx1 = 144;
		lastx2 = 144;
	} else if (hours == 23 && (mins == 0 || hasanim == 0)){
		GRect start = GRect(12, 0, 144, 168);
		GRect finish = GRect(0, 0, 144, 168);
		animate_layer(inverter_layer_get_layer(inv_layer), &start, &finish, 300, 0);
		lastx1 = 144;
		lastx2 = 144;
	}
    hasanim = 1;
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
	ResHandle font_handle = resource_get_handle(RESOURCE_ID_FONT_SHARE_TECH_48);

	//Time layer
	text_layer = text_layer_create(GRect(1, 50, 144, 168));
	text_layer_set_background_color(text_layer, GColorClear);
	text_layer_set_text_color(text_layer, GColorBlack);
	text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
	text_layer_set_font(text_layer, fonts_load_custom_font(font_handle));
	
	layer_add_child(window_get_root_layer(window), (Layer*) text_layer);
	
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
	
	//Arbitrary text:
	//text_layer_set_text(text_layer, "Radscorpion sighted");	
	
	//Inverter layer
	inv_layer = inverter_layer_create(GRect(80,0,144,162));
	layer_add_child(window_get_root_layer(window), (Layer*) inv_layer);
	
	
			// Test H layer
	h_layer = bitmap_layer_create(bounds);
	bitmap_layer_set_bitmap(h_layer, himage);
	bitmap_layer_set_alignment(h_layer, GAlignCenter);
	layer_add_child(window_layer, bitmap_layer_get_layer(h_layer));
	bitmap_layer_set_compositing_mode(h_layer, GCompOpAnd);
	
}

void window_unload(Window *window)
{
	text_layer_destroy(text_layer);
	text_layer_destroy(text_layer_shadow1);
	text_layer_destroy(text_layer_shadow2);
	text_layer_destroy(text_layer_shadow3);
	text_layer_destroy(text_layer_shadow4);

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

int main(void) {
	handle_init();


  app_event_loop();

	tick_timer_service_unsubscribe();
	
  gbitmap_destroy(image);
  gbitmap_destroy(himage);

  bitmap_layer_destroy(image_layer);
  bitmap_layer_destroy(h_layer);
  window_destroy(window);
}
