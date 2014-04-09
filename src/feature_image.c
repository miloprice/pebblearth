/*

   Demonstrate how to display a two color, black and white bitmap
   image with no transparency.

   The original source image is from:

      <http://openclipart.org/detail/26728/aiga-litter-disposal-by-anonymous>

   The source image was converted from an SVG into a RGB bitmap using
   Inkscape. It has no transparency and uses only black and white as
   colors.

 */

#include "pebble.h"

static Window *window;

static BitmapLayer *image_layer;

static TextLayer *text_layer;
static TextLayer *text_layer_shadow;

static GBitmap *image;

char buffer[] = "00:00";

void tick_handler(struct tm *tick_time, TimeUnits units_changed){
	strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
	
	text_layer_set_text(text_layer_shadow, buffer);
	text_layer_set_text(text_layer, buffer);
}

void window_load(Window *window)
{
	
	
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_frame(window_layer);
	
	window_set_background_color	(window, GColorBlack);
	
	// This needs to be deinited on app exit which is when the event loop ends
	image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_STOCK_EARTH);
	
	// The bitmap layer holds the image for display
	image_layer = bitmap_layer_create(bounds);
	bitmap_layer_set_bitmap(image_layer, image);
	bitmap_layer_set_alignment(image_layer, GAlignCenter);
	layer_add_child(window_layer, bitmap_layer_get_layer(image_layer));
	
	//Load font
	ResHandle font_handle = resource_get_handle(RESOURCE_ID_FONT_SHARE_TECH_48);
	ResHandle font_handle2 = resource_get_handle(RESOURCE_ID_FONT_SHARE_TECH_52);

	//Time layer
	text_layer = text_layer_create(GRect(0, 50, 144, 168));
	text_layer_set_background_color(text_layer, GColorClear);
	text_layer_set_text_color(text_layer, GColorWhite);
	text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
	text_layer_set_font(text_layer, fonts_load_custom_font(font_handle));
	
	layer_add_child(window_get_root_layer(window), (Layer*) text_layer);
	
	text_layer_shadow = text_layer_create(GRect(0, 50, 144, 168));
	text_layer_set_background_color(text_layer_shadow, GColorClear);
	text_layer_set_text_color(text_layer_shadow, GColorBlack);
	text_layer_set_text_alignment(text_layer_shadow, GTextAlignmentCenter);
	text_layer_set_font(text_layer_shadow, fonts_load_custom_font(font_handle2));
	
	layer_add_child(window_get_root_layer(window), (Layer*) text_layer_shadow);
	
	layer_add_child(window_get_root_layer(window), (Layer*) text_layer);
	
	//Arbitrary text:
	//text_layer_set_text(text_layer, "Radscorpion sighted");	
}

void window_unload(Window *window)
{
	text_layer_destroy(text_layer);
	text_layer_destroy(text_layer_shadow);

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

  bitmap_layer_destroy(image_layer);
  window_destroy(window);
}
