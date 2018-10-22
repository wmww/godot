/*************************************************************************/
/*  os_x11.cpp							   */
/*************************************************************************/
/*		       This file is part of:			   */
/*			   GODOT ENGINE				*/
/*		      https://godotengine.org			  */
/*************************************************************************/
/* Copyright (c) 2007-2018 Juan Linietsky, Ariel Manzur.		 */
/* Copyright (c) 2014-2018 Godot Engine contributors (cf. AUTHORS.md)    */
/*								       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:					     */
/*								       */
/* The above copyright notice and this permission notice shall be	*/
/* included in all copies or substantial portions of the Software.       */
/*								       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.		*/
/*************************************************************************/

#include "display_wayland.h"

Error Display_wayland::initialize_display(const VideoMode &p_desired, int p_video_driver) {

	// server stuff getten
	struct wl_display *display = wl_display_connect(NULL);
	if (display == NULL) {
		LOG("Can't connect to wayland display !?\n");
		exit(1);
	}
	LOG("Got a display !");

	struct wl_registry *wl_registry = wl_display_get_registry(display);
	wl_registry_add_listener(wl_registry, &listener, NULL);

    //TODO: add the listener struct and callback

	// This call the attached listener global_registry_handler
	wl_display_dispatch(display);
	wl_display_roundtrip(display);

	// If at this point, global_registry_handler didn't set the
	// compositor, nor the shell, bailout !
	if (compositor == NULL || shell == NULL) {
		LOG("No compositor !? No Shell !! There's NOTHING in here !\n");
		exit(1);
	} else {
		LOG("Okay, we got a compositor and a shell... That's something !\n");
		// ESContext.native_display = display;
	}
	// create window
	surface = wl_compositor_create_surface(compositor);
	if (surface == NULL) {
		print_verbose("No Compositor surface ! Yay....\n");
		exit(1);
	} else
		print_verbose("Got a compositor surface !\n");

	shell_surface = wl_shell_get_shell_surface(shell, surface);
	wl_shell_surface_set_toplevel(shell_surface);

	region = wl_compositor_create_region(compositor);

	wl_region_add(region, 0, 0, width, height);
	wl_surface_set_opaque_region(surface, region);

	struct wl_egl_window *egl_window = wl_egl_window_create(surface, width, height);

	if (egl_window == EGL_NO_SURFACE) {
		LOG("No window !?\n");
		exit(1);
	} else
		LOG("Window created !\n");
    context_gl_egl = memnew(ContextGL_EGL(display, egl_window, p_desired, ContextType p_context_type));
    context_gl_egl->initialize();
	// ESContext.window_width = width;
	// ESContext.window_height = height;
	// ESContext.native_window = egl_window;
	return Error::OK;
}
void Display_wayland::finalize_display() {
	print_verbose("not implemented (Display_wayland): get_mouse_position");
}
void Display_wayland::set_main_loop(MainLoop *p_main_loop) {
	print_verbose("not implemented (Display_wayland): set_main_loop");
}
void Display_wayland::delete_main_loop() {
	print_verbose("not implemented (Display_wayland): delete_main_loop");
}

MainLoop *Display_wayland::get_main_loop() const {
	return main_loop;
}

Point2 Display_wayland::get_mouse_position() const {
	print_verbose("not implemented (Display_wayland): get_mouse_position");
	return Point2(0, 0);
}
int Display_wayland::get_mouse_button_state() const {
	print_verbose("not implemented (Display_wayland): get_mouse_button_state");
	return 0;
}
void Display_wayland::set_window_title(const String &p_title) {
	print_verbose("not implemented (Display_wayland): set_window_title");
}
void Display_wayland::set_video_mode(const VideoMode &p_video_mode, int p_screen) {
	print_verbose("not implemented (Display_wayland): set_video_mode");
}
DisplayDriver::VideoMode Display_wayland::get_video_mode(int p_screen) const {
	print_verbose("not implemented (Display_wayland): get_video_mode");
	return VideoMode();
}
void Display_wayland::get_fullscreen_mode_list(List<VideoMode> *p_list, int p_screen) const {
	print_verbose("not implemented (Display_wayland): get_fullscreen_mode_list");
}
Size2 Display_wayland::get_window_size() const {
	print_verbose("not implemented (Display_wayland): get_mouse_position");
	return Size2(0, 0);
}
bool Display_wayland::get_window_per_pixel_transparency_enabled() const {
	print_verbose("not implemented (Display_wayland): get_window_per_pixel_transparency_enabled");
	return false;
}
void Display_wayland::set_window_per_pixel_transparency_enabled(bool p_enabled) {
	print_verbose("not implemented (Display_wayland): set_window_per_pixel_transparency_enabled");
}
int Display_wayland::get_video_driver_count() const {
	print_verbose("not implemented (Display_wayland): get_video_driver_count");
	return 0;
}
const char *Display_wayland::get_video_driver_name(int p_driver) const {
	print_verbose("not implemented (Display_wayland): get_video_driver_name");
	return "";
}
int Display_wayland::get_current_video_driver() const {
	print_verbose("not implemented (Display_wayland): get_current_video_driver");
	return 0;
}
String Display_wayland::get_name() {
	print_verbose("not implemented (Display_wayland): get_name");
	return String("");
}
bool Display_wayland::can_draw() const {
	print_verbose("not implemented (Display_wayland): v");
	return false;
}
void Display_wayland::set_cursor_shape(CursorShape p_shape) {
	print_verbose("not implemented (Display_wayland): set_cursor_shape");
}
void Display_wayland::set_custom_mouse_cursor(const RES &p_cursor, CursorShape p_shape, const Vector2 &p_hotspot) {
	print_verbose("not implemented (Display_wayland): set_custom_mouse_cursor");
}