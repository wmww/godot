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
#include "drivers/dummy/rasterizer_dummy.h"
#include "drivers/gles2/rasterizer_gles2.h"
#include "drivers/gles3/rasterizer_gles3.h"
#include "servers/visual/visual_server_raster.h"
//#include <wayland-client-protocol.h>
//#include <wayland-client.h>
// #include <wayland-egl.h> // Wayland EGL MUST be included before EGL headers
//#include <wayland-server.h>
#define DISPLAY_WL (Display_wayland *)Display_wayland::get_singleton()
void Display_wayland::global_registry_handler(void *data, struct wl_registry *registry, uint32_t id, const char *interface, uint32_t version) {
	Display_wayland *d_wl = DISPLAY_WL;

	printf("Got a registry event for %s id %d\n", interface, id);

	if (strcmp(interface, "wl_compositor") == 0) {

		d_wl->compositor = (wl_compositor *)wl_registry_bind(registry, id, &wl_compositor_interface, 1);
	}

	// else if (strcmp(interface, "wl_shell") == 0) {
	//
	// 	d_wl->shell = (wl_shell *)wl_registry_bind(registry, id, &wl_shell_interface, 1);
	// }

	else if (strcmp(interface, "zxdg_shell_v6") == 0) {

		d_wl->xdg_shell = (zxdg_shell_v6 *)wl_registry_bind(registry, id, &zxdg_shell_v6_interface, 1);
	}
}

void Display_wayland::global_registry_remover(void *data, struct wl_registry *wl_registry, uint32_t name) {
}

void Display_wayland::xdg_toplevel_configure_handler(void *data, struct zxdg_toplevel_v6 *xdg_toplevel, int32_t width, int32_t height, struct wl_array *states) {

	printf("configure: %dx%d\n", width, height);
}

void Display_wayland::xdg_toplevel_close_handler(void *data, struct zxdg_toplevel_v6 *xdg_toplevel) {

	printf("close\n");
}

void Display_wayland::xdg_surface_configure_handler(void *data, struct zxdg_surface_v6 *xdg_surface, uint32_t serial) {
	printf("configure surface: %d", serial);
	zxdg_surface_v6_ack_configure(xdg_surface, serial);
}
void Display_wayland::xdg_shell_ping_handler(void *data, struct zxdg_shell_v6 *xdg_shell, uint32_t serial) {
	zxdg_shell_v6_pong(xdg_shell, serial);
	printf("ping-pong\n");
}
Error Display_wayland::initialize_display(const VideoMode &p_desired, int p_video_driver) {

	// server stuff getten
	display = NULL;
	display = wl_display_connect(NULL);
	if (display == NULL) {
		print_line("Can't connect to wayland display !?\n");
		exit(1);
	}
	print_line("Got a display !");
	struct wl_registry *wl_registry = wl_display_get_registry(display);
	wl_registry_add_listener(wl_registry, &registry_listener, NULL);

	// This call the attached listener global_registry_handler
	wl_display_dispatch(display);
	wl_display_roundtrip(display);

	// If at this point, global_registry_handler didn't set the
	// compositor, nor the shell, bailout !
	if (compositor == NULL || xdg_shell == NULL) {
		print_verbose("No compositor !? No Shell !! There's NOTHING in here !\n");
		exit(1);
	} else {
		print_verbose("Okay, we got a compositor and a shell... That's something !\n");
		// ESContext.native_display = display;
	}
	// create window
	surface = wl_compositor_create_surface(compositor);
	if (surface == NULL) {
		print_verbose("No Compositor surface ! Yay....\n");
		exit(1);
	} else
		print_verbose("Got a compositor surface !\n");

	xdg_surface = zxdg_shell_v6_get_xdg_surface(xdg_shell, surface);
	zxdg_surface_v6_add_listener(xdg_surface, &xdg_surface_listener, NULL);
	// shell_surface = wl_shell_get_shell_surface(shell, surface);

	xdg_toplevel = zxdg_surface_v6_get_toplevel(xdg_surface);
	zxdg_toplevel_v6_add_listener(xdg_toplevel, &xdg_toplevel_listener, NULL);

	wl_surface_commit(surface);

	// wait for the "initial" set of globals to appear

	zxdg_shell_v6_add_listener(xdg_shell, &xdg_shell_listener, NULL);
	wl_display_roundtrip(display);
	//make opaque
	// region = wl_compositor_create_region(compositor);
	// wl_region_add(region, 0, 0, p_desired.width, p_desired.height);
	// wl_surface_set_opaque_region(surface, region);

	//wl_display_dispatch(display);
	struct wl_egl_window *egl_window = wl_egl_window_create(surface, p_desired.width, p_desired.height);

	if (egl_window == EGL_NO_SURFACE) {
		print_verbose("No window !?\n");
		exit(1);
	} else
		print_verbose("Window created !\n");

	context_gl_egl = NULL;
	ContextGL_EGL::Driver context_type = ContextGL_EGL::Driver::GLES_3_0; //TODO: check for possible context types

	bool gl_initialization_error = false;
	while (!context_gl_egl) {
		EGLNativeDisplayType n_disp = (EGLNativeDisplayType)display;
		EGLNativeWindowType n_wind = (EGLNativeWindowType)egl_window;
		context_gl_egl = memnew(ContextGL_EGL(n_disp, n_wind, p_desired, context_type));
		if (context_gl_egl->initialize() != OK) {
			memdelete(context_gl_egl);
			context_gl_egl = NULL;
			if (GLOBAL_GET("rendering/quality/driver/driver_fallback") == "Best") {
				if (p_video_driver == VIDEO_DRIVER_GLES2) {
					gl_initialization_error = true;
					break;
				}

				p_video_driver = VIDEO_DRIVER_GLES2;
				context_type = ContextGL_EGL::GLES_2_0;
			} else {
				gl_initialization_error = true;
				break;
			}
		}
	}

	eglBindAPI(EGL_OPENGL_API);

	while (true) {
		if (context_type == ContextGL_EGL::GLES_3_0) {
			if (RasterizerGLES3::is_viable() == OK) {
				RasterizerGLES3::register_config();
				RasterizerGLES3::make_current();
				break;
			} else {
				if (GLOBAL_GET("rendering/quality/driver/driver_fallback") == "Best") {
					p_video_driver = VIDEO_DRIVER_GLES2;
					context_type = ContextGL_EGL::GLES_2_0;
					continue;
				} else {
					gl_initialization_error = true;
					break;
				}
			}
		}

		if (context_type == ContextGL_EGL::GLES_2_0) {
			if (RasterizerGLES2::is_viable() == OK) {
				RasterizerGLES2::register_config();
				RasterizerGLES2::make_current();
				break;
			} else {
				gl_initialization_error = true;
				break;
			}
		}
	}

	if (gl_initialization_error) {
		OS::get_singleton()->alert("Your video card driver does not support any of the supported OpenGL versions.\n"
								   "Please update your drivers or if you have a very old or integrated GPU upgrade it.",
				"Unable to initialize Video driver");
		return ERR_UNAVAILABLE;
	}

	// video_driver_index = p_video_driver;

	// context_gl->set_use_vsync(current_videomode.use_vsync);

	//#endif

	//VISUAL SERVER
	visual_server = memnew(VisualServerRaster);

	visual_server->init();

	input = memnew(InputDefault);

	// if (get_render_thread_mode() != RENDER_THREAD_UNSAFE) {

	// 	visual_server = memnew(VisualServerWrapMT(visual_server, get_render_thread_mode() == RENDER_SEPARATE_THREAD));
	// }
	// ESContext.window_width = width;
	// ESContext.window_height = height;
	// ESContext.native_window = egl_window;

	//INPUT

	return Error::OK;
}
void Display_wayland::finalize_display() {
	print_line("not implemented (Display_wayland): finalize_display");
}
void Display_wayland::set_main_loop(MainLoop *p_main_loop) {
	main_loop = p_main_loop;
	print_line("not implemented (Display_wayland): set_main_loop");
}
void Display_wayland::delete_main_loop() {
	print_line("not implemented (Display_wayland): delete_main_loop");
}

MainLoop *Display_wayland::get_main_loop() const {
	return main_loop;
}

Point2 Display_wayland::get_mouse_position() const {
	//print_line("not implemented (Display_wayland): get_mouse_position");
	return Point2(0, 0);
}
int Display_wayland::get_mouse_button_state() const {
	print_line("not implemented (Display_wayland): get_mouse_button_state");
	return 0;
}
void Display_wayland::set_window_title(const String &p_title) {
	zxdg_toplevel_v6_set_title(xdg_toplevel, (char *)p_title.c_str());
	print_line("not implemented (Display_wayland): set_window_title" + p_title);
}
void Display_wayland::set_video_mode(const VideoMode &p_video_mode, int p_screen) {
	print_line("not implemented (Display_wayland): set_video_mode");
}
DisplayDriver::VideoMode Display_wayland::get_video_mode(int p_screen) const {
	print_line("not implemented (Display_wayland): get_video_mode");
	return VideoMode();
}
void Display_wayland::get_fullscreen_mode_list(List<VideoMode> *p_list, int p_screen) const {
	print_line("not implemented (Display_wayland): get_fullscreen_mode_list");
}
Size2 Display_wayland::get_window_size() const {
	//print_line("not implemented (Display_wayland): get_mouse_position");
	return Size2(0, 0);
}
bool Display_wayland::get_window_per_pixel_transparency_enabled() const {
	print_line("not implemented (Display_wayland): get_window_per_pixel_transparency_enabled");
	return false;
}
void Display_wayland::set_window_per_pixel_transparency_enabled(bool p_enabled) {
	print_line("not implemented (Display_wayland): set_window_per_pixel_transparency_enabled");
}
int Display_wayland::get_video_driver_count() const {
	print_line("not implemented (Display_wayland): get_video_driver_count");
	return 0;
}
const char *Display_wayland::get_video_driver_name(int p_driver) const {
	print_line("not implemented (Display_wayland): get_video_driver_name");
	return "";
}
int Display_wayland::get_current_video_driver() const {
	print_line("not implemented (Display_wayland): get_current_video_driver");
	return 0;
}
String Display_wayland::get_name() {
	print_line("not implemented (Display_wayland): get_name");
	return String("");
}
bool Display_wayland::can_draw() const {
	wl_display_dispatch_pending(display);
	return true;
}
void Display_wayland::set_cursor_shape(CursorShape p_shape) {
	print_line("not implemented (Display_wayland): set_cursor_shape");
}
void Display_wayland::set_custom_mouse_cursor(const RES &p_cursor, CursorShape p_shape, const Vector2 &p_hotspot) {
	print_line("not implemented (Display_wayland): set_custom_mouse_cursor");
}