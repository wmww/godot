/*************************************************************************/
/*  os_x11.h                                                             */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2018 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2018 Godot Engine contributors (cf. AUTHORS.md)    */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/
/**
	@author Juan Linietsky <reduzio@gmail.com>
*/
#include "core/os/displaydriver.h"
#include "core/os/input.h"
#include "joypad_linux.h"
#include "main/input_default.h"
#include "power_x11.h"
#include "servers/visual/rasterizer.h"
#include "servers/visual_server.h"

#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <wayland-egl.h> // Wayland EGL MUST be included before EGL headers
#include <wayland-server.h>

#include "context_egl_wayland.h"
#include "wayland_protocol/xdg-shell.h"
#undef CursorShape

class Display_wayland : public DisplayDriver {
private:
	MainLoop *main_loop;
	VisualServer *visual_server;
	VideoMode current_videomode;
	List<String> args;
	ContextGL_EGL *context_gl_egl;
	InputDefault *input;
	struct wl_compositor *compositor = NULL;

	struct wl_display *display = NULL;
	struct wl_surface *surface;
	struct wl_egl_window *egl_window;
	struct wl_region *region;
	// struct wl_shell *shell;
	struct zxdg_shell_v6 *xdg_shell;
	struct zxdg_surface_v6 *xdg_surface;
	struct zxdg_toplevel_v6 *xdg_toplevel;
	//wayland listeners
	const struct wl_registry_listener registry_listener = {
		&global_registry_handler,
		&global_registry_remover
	};
	static void global_registry_handler(void *data, struct wl_registry *registry, uint32_t id, const char *interface, uint32_t version);
	static void global_registry_remover(void *data, struct wl_registry *wl_registry, uint32_t name);

	const struct zxdg_toplevel_v6_listener xdg_toplevel_listener = {
		.configure = &xdg_toplevel_configure_handler,
		.close = &xdg_toplevel_close_handler
	};
	static void xdg_toplevel_configure_handler(void *data, struct zxdg_toplevel_v6 *xdg_toplevel, int32_t width, int32_t height, struct wl_array *states);
	static void xdg_toplevel_close_handler(void *data, struct zxdg_toplevel_v6 *xdg_toplevel);

	const struct zxdg_surface_v6_listener xdg_surface_listener = {
		.configure = &xdg_surface_configure_handler
	};
	static void xdg_surface_configure_handler(void *data, struct zxdg_surface_v6 *xdg_surface, uint32_t serial);

	const struct zxdg_shell_v6_listener xdg_shell_listener = {
		.ping = &xdg_shell_ping_handler
	};
	static void xdg_shell_ping_handler(void *data, struct zxdg_shell_v6 *xdg_shell, uint32_t serial);

protected:
	Error initialize_display(const VideoMode &p_desired, int p_video_driver);
	void finalize_display();

	void set_main_loop(MainLoop *p_main_loop);
	void delete_main_loop();

public:
	MainLoop *get_main_loop() const;

	// virtual void set_mouse_mode(MouseMode p_mode);
	// virtual MouseMode get_mouse_mode() const;

	// virtual void warp_mouse_position(const Point2 &p_to) {}
	Point2 get_mouse_position() const;
	int get_mouse_button_state() const;
	void set_window_title(const String &p_title);

	// virtual void set_clipboard(const String &p_text);
	// virtual String get_clipboard() const;

	void set_video_mode(const VideoMode &p_video_mode, int p_screen = 0);
	VideoMode get_video_mode(int p_screen = 0) const;
	void get_fullscreen_mode_list(List<VideoMode> *p_list, int p_screen = 0) const;

	// virtual int get_screen_count() const { return 1; }
	// virtual int get_current_screen() const { return 0; }
	// virtual void set_current_screen(int p_screen) {}
	// virtual Point2 get_screen_position(int p_screen = -1) const { return Point2(); }
	// virtual Size2 get_screen_size(int p_screen = -1) const { return get_window_size(); }
	// virtual int get_screen_dpi(int p_screen = -1) const { return 72; }
	// virtual Point2 get_window_position() const { return Vector2(); }
	// virtual void set_window_position(const Point2 &p_position) {}
	Size2 get_window_size() const;
	// virtual Size2 get_real_window_size() const { return get_window_size(); }
	// virtual void set_window_size(const Size2 p_size) {}
	// virtual void set_window_fullscreen(bool p_enabled) {}
	// virtual bool is_window_fullscreen() const { return true; }
	// virtual void set_window_resizable(bool p_enabled) {}
	// virtual bool is_window_resizable() const { return false; }
	// virtual void set_window_minimized(bool p_enabled) {}
	// virtual bool is_window_minimized() const { return false; }
	// virtual void set_window_maximized(bool p_enabled) {}
	// virtual bool is_window_maximized() const { return true; }
	// virtual void set_window_always_on_top(bool p_enabled) {}
	// virtual bool is_window_always_on_top() const { return false; }
	// virtual void request_attention() {}
	// virtual void center_window();
	// virtual Rect2 get_window_safe_area() const;

	// virtual void set_borderless_window(bool p_borderless) {}
	// virtual bool get_borderless_window() { return 0; }

	bool get_window_per_pixel_transparency_enabled() const;
	void set_window_per_pixel_transparency_enabled(bool p_enabled);

	int get_video_driver_count() const;
	const char *get_video_driver_name(int p_driver) const;
	int get_current_video_driver() const;

	// virtual void set_ime_active(const bool p_active) {}
	// virtual void set_ime_position(const Point2 &p_pos) {}
	// virtual void set_ime_intermediate_text_callback(ImeCallback p_callback, void *p_inp) {}

	// virtual void set_keep_screen_on(bool p_enabled);
	// virtual bool is_keep_screen_on() const;

	String get_name();
	bool can_draw() const;

	// virtual bool has_virtual_keyboard() const;
	// virtual void show_virtual_keyboard(const String &p_existing_text, const Rect2 &p_screen_rect = Rect2());
	// virtual void hide_virtual_keyboard();

	// returns height of the currently shown virtual keyboard (0 if keyboard is hidden)
	// virtual int get_virtual_keyboard_height() const;

	void set_cursor_shape(CursorShape p_shape);
	void set_custom_mouse_cursor(const RES &p_cursor, CursorShape p_shape, const Vector2 &p_hotspot);

	// RenderThreadMode get_render_thread_mode() const { return _render_thread_mode; }

	// virtual void set_no_window_mode(bool p_enable);
	// virtual bool is_no_window_mode_enabled() const;

	// virtual bool has_touchscreen_ui_hint() const;

	// virtual void set_screen_orientation(ScreenOrientation p_orientation);
	// ScreenOrientation get_screen_orientation() const;

	// virtual void enable_for_stealing_focus(OS::ProcessID pid) {}
	// virtual void move_window_to_foreground() {}

	// virtual void release_rendering_thread();
	// virtual void make_rendering_thread();
	virtual void swap_buffers();

	// virtual void set_icon(const Ref<Image> &p_icon);

	// virtual Error native_video_play(String p_path, float p_volume, String p_audio_track, String p_subtitle_track);
	// virtual bool native_video_is_playing() const;
	// virtual void native_video_pause();
	// virtual void native_video_unpause();
	// virtual void native_video_stop();

	// virtual Error dialog_show(String p_title, String p_description, Vector<String> p_buttons, Object *p_obj, String p_callback);
	// virtual Error dialog_input_text(String p_title, String p_description, String p_partial, Object *p_obj, String p_callback);

	// virtual LatinKeyboardVariant get_latin_keyboard_variant() const;

	// virtual bool is_joy_known(int p_device);
	// virtual String get_joy_guid(int p_device) const;

	//amazing hack because OpenGL needs this to be set on a separate thread..
	//also core can't access servers, so a callback must be used
	// typedef void (*SwitchVSyncCallbackInThread)(bool);

	// static SwitchVSyncCallbackInThread switch_vsync_function;
	// void set_use_vsync(bool p_enable);
	// bool is_vsync_enabled() const;

	//real, actual overridable function to switch vsync, which needs to be called from graphics thread if needed
	// virtual void _set_use_vsync(bool p_enable) {}

	// virtual void force_process_input(){};
	// bool has_feature(const String &p_feature);

	// bool is_hidpi_allowed() const { return _allow_hidpi; }
	// bool is_layered_allowed() const { return _allow_layered; }

	// virtual void set_context(int p_context);
	// virtual void process_events();

	// DisplayDriver();
	// virtual ~DisplayDriver();
};