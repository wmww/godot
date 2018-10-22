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
    print_verbose("not implemented (Display_wayland): get_mouse_position");
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
    return Point2(0,0);
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
    return Size2(0,0);
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