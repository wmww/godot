/*************************************************************************/
/*  context_egl_wayland.cpp                                              */
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

#ifdef X11_ENABLED
#if defined(OPENGL_ENABLED)
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <wayland-egl.h> // Wayland EGL MUST be included before EGL headers
#include <wayland-server.h>

#include "context_egl_wayland.h"

void ContextGL_X11::release_current() {

	eglMakeCurrent(egl_display, None ,None, NULL);
}

void ContextGL_X11::make_current() {

    eglMakeCurrent(egl_display, egl_surface,egl_surface, p->context)
}

void ContextGL_X11::swap_buffers() {
    
    eglSwapBuffers(egl_display, egl_surface);
}

// static bool ctxErrorOccurred = false;
// static int ctxErrorHandler(Display *dpy, XErrorEvent *ev) {
// 	ctxErrorOccurred = true;
// 	return 0;
// }

static void set_class_hint(Display *p_display, Window p_window) {
	XClassHint *classHint;

	/* set the name and class hints for the window manager to use */
	classHint = XAllocClassHint();
	if (classHint) {
		classHint->res_name = (char *)"Godot_Engine";
		classHint->res_class = (char *)"Godot";
	}
	XSetClassHint(p_display, p_window, classHint);
	XFree(classHint);
}

Error ContextGL_EGL::initialize() {

	EGLint numConfigs;
	EGLint majorVersion;
	EGLint minorVersion;
	EGLConfig config;
	EGLint fbAttribs[] =
			{
				EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
				EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
				EGL_RED_SIZE, 8,
				EGL_GREEN_SIZE, 8,
				EGL_BLUE_SIZE, 8,
				EGL_NONE
			};
	EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE };
	
    EGLDisplay display = eglGetDisplay(native_display);
	EGLSurface surface;
	EGLContext context;

	if (display == EGL_NO_DISPLAY) {
		LOG("No EGL Display...\n");
		return EGL_FALSE;
	}

	// Initialize EGL
	if (!eglInitialize(display, &majorVersion, &minorVersion)) {
		LOG("No Initialisation...\n");
		return EGL_FALSE;
	}

	// Get configs
	if ((eglGetConfigs(display, NULL, 0, &numConfigs) != EGL_TRUE) || (numConfigs == 0)) {
		LOG("No configuration...\n");
		return EGL_FALSE;
	}

	// Choose config
	if ((eglChooseConfig(display, fbAttribs, &config, 1, &numConfigs) != EGL_TRUE) || (numConfigs != 1)) {
		LOG("No configuration...\n");
		return EGL_FALSE;
	}

	// Create a surface
	surface = eglCreateWindowSurface(display, config, native_window, NULL);
	if (surface == EGL_NO_SURFACE) {
		LOG("No surface...\n");
		return EGL_FALSE;
	}

	// Create a GL context
	context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
	if (context == EGL_NO_CONTEXT) {
		LOG("No context...\n");
		return EGL_FALSE;
	}

	// Make the context current
	if (!eglMakeCurrent(display, surface, surface, context)) {
		LOG("Could not make the current window current !\n");
		return EGL_FALSE;
	}

	egl_display = display;
	egl_surface = surface;
	p->egl_context = context;
	// return EGL_TRUE;
    return OK;
	// switch (context_type) {
	// 	case OLDSTYLE: {

	// 		p->glx_context = glXCreateContext(x11_display, vi, 0, GL_TRUE);
	// 		ERR_FAIL_COND_V(!p->glx_context, ERR_UNCONFIGURED);
	// 	} break;
	// 	case GLES_2_0_COMPATIBLE: {

	// 		p->glx_context = glXCreateNewContext(x11_display, fbconfig, GLX_RGBA_TYPE, 0, true);
	// 		ERR_FAIL_COND_V(!p->glx_context, ERR_UNCONFIGURED);
	// 	} break;
	// 	case GLES_3_0_COMPATIBLE: {

	// 		static int context_attribs[] = {
	// 			GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
	// 			GLX_CONTEXT_MINOR_VERSION_ARB, 3,
	// 			GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
	// 			GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB /*|GLX_CONTEXT_DEBUG_BIT_ARB*/,
	// 			None
	// 		};

	// 		p->glx_context = glXCreateContextAttribsARB(x11_display, fbconfig, NULL, true, context_attribs);
	// 		ERR_FAIL_COND_V(ctxErrorOccurred || !p->glx_context, ERR_UNCONFIGURED);
	// 	} break;
	// }

	// x11_window = XCreateWindow(x11_display, RootWindow(x11_display, vi->screen), 0, 0, DisplayDriver::get_singleton()->get_video_mode().width, DisplayDriver::get_singleton()->get_video_mode().height, 0, vi->depth, InputOutput, vi->visual, valuemask, &swa);

	// ERR_FAIL_COND_V(!x11_window, ERR_UNCONFIGURED);
	// set_class_hint(x11_display, x11_window);
	// XMapWindow(x11_display, x11_window);

	// XSync(x11_display, False);
	// XSetErrorHandler(oldHandler);

	// make_current(x11_display, x11_window, p->glx_context);

	// XFree(vi);

	// return OK;
}

int ContextGL_X11::get_window_width() {

	XWindowAttributes xwa;
	XGetWindowAttributes(x11_display, x11_window, &xwa);

	return xwa.width;
}

int ContextGL_X11::get_window_height() {
	XWindowAttributes xwa;
	XGetWindowAttributes(x11_display, x11_window, &xwa);

	return xwa.height;
}

void ContextGL_X11::set_use_vsync(bool p_use) {
	static bool setup = false;
	static PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT = NULL;
	static PFNGLXSWAPINTERVALSGIPROC glXSwapIntervalMESA = NULL;
	static PFNGLXSWAPINTERVALSGIPROC glXSwapIntervalSGI = NULL;

	if (!setup) {
		setup = true;
		String extensions = glXQueryExtensionsString(x11_display, DefaultScreen(x11_display));
		if (extensions.find("GLX_EXT_swap_control") != -1)
			glXSwapIntervalEXT = (PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddressARB((const GLubyte *)"glXSwapIntervalEXT");
		if (extensions.find("GLX_MESA_swap_control") != -1)
			glXSwapIntervalMESA = (PFNGLXSWAPINTERVALSGIPROC)glXGetProcAddressARB((const GLubyte *)"glXSwapIntervalMESA");
		if (extensions.find("GLX_SGI_swap_control") != -1)
			glXSwapIntervalSGI = (PFNGLXSWAPINTERVALSGIPROC)glXGetProcAddressARB((const GLubyte *)"glXSwapIntervalSGI");
	}
	int val = p_use ? 1 : 0;
	if (glXSwapIntervalMESA) {
		glXSwapIntervalMESA(val);
	} else if (glXSwapIntervalSGI) {
		glXSwapIntervalSGI(val);
	} else if (glXSwapIntervalEXT) {
		GLXDrawable drawable = glXGetCurrentDrawable();
		glXSwapIntervalEXT(x11_display, drawable, val);
	} else
		return;
	use_vsync = p_use;
}
bool ContextGL_X11::is_using_vsync() const {

	return use_vsync;
}

ContextGL_EGL::ContextGL_EGL(EGLNativeDisplayType *p_egl_display, EGLNativeWindowType &p_egl_window, const DisplayDriver::VideoMode &p_default_video_mode, ContextType p_context_type) {

	default_video_mode = p_default_video_mode;

	context_type = p_context_type;

	double_buffer = false;
	direct_render = false;
	egl_minor = egl_major = 0;
    native_display = p_egl_display;
    native_window = p_egl_window;
	p = memnew(ContextGL_EGL_Private);
	p->egl_context = 0;
	use_vsync = false;
}

ContextGL_EGL::~ContextGL_EGL() {
	release_current();
	eglDestroySurface(ESContext.display, ESContext.surface);
	// wl_egl_window_destroy(ESContext.native_window);
	// wl_shell_surface_destroy(shell_surface);
	// wl_surface_destroy(surface);
	eglDestroyContext(p->display, p->context);
	memdelete(p);
}

#endif
#endif
