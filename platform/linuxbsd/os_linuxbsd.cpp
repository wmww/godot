/*************************************************************************/
/*  os_linuxbsd.cpp                                                      */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2019 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2019 Godot Engine contributors (cf. AUTHORS.md)    */
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

#include "os_linuxbsd.h"
#include "core/os/display_driver.h"

#include "core/print_string.h"
#include "errno.h"
#include "key_mapping_x11.h"

#ifdef HAVE_MNTENT
#include <mntent.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main/main.h"

#include <dlfcn.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

//stupid linux.h
#ifdef KEY_TAB
#undef KEY_TAB
#endif

int OS_LinuxBSD::get_audio_driver_count() const {
	return AudioDriverManager::get_driver_count();
}

const char *OS_LinuxBSD::get_audio_driver_name(int p_driver) const {

	AudioDriver *driver = AudioDriverManager::get_driver(p_driver);
	ERR_FAIL_COND_V(!driver, "");
	return AudioDriverManager::get_driver(p_driver)->get_name();
}

void OS_LinuxBSD::initialize_core() {

	crash_handler.initialize();

	OS_Unix::initialize_core();
}

Error OS_LinuxBSD::initialize(int p_audio_driver) {
	args = OS::get_singleton()->get_cmdline_args();
	last_timestamp = 0;

	AudioDriverManager::initialize(p_audio_driver);

	_ensure_user_data_dir();

	power_manager = memnew(PowerLinuxBSD);

	return OK;
}

String OS_LinuxBSD::get_unique_id() const {

	static String machine_id;
	if (machine_id.empty()) {
		if (FileAccess *f = FileAccess::open("/etc/machine-id", FileAccess::READ)) {
			while (machine_id.empty() && !f->eof_reached()) {
				machine_id = f->get_line().strip_edges();
			}
		}
	}

	return machine_id;
}

void OS_LinuxBSD::finalize() {

	/*
	if (debugger_connection_console) {
		memdelete(debugger_connection_console);
	}
	*/
#ifdef ALSAMIDI_ENABLED
	driver_alsamidi.close();
#endif

	memdelete(power_manager);

	args.clear();
}

String OS_LinuxBSD::get_name() {

	return "LinuxBSD";
}

Error OS_LinuxBSD::shell_open(String p_uri) {

	Error ok;
	List<String> args;
	args.push_back(p_uri);
	ok = execute("xdg-open", args, false);
	if (ok == OK)
		return OK;
	ok = execute("gnome-open", args, false);
	if (ok == OK)
		return OK;
	ok = execute("kde-open", args, false);
	return ok;
}

bool OS_LinuxBSD::_check_internal_feature_support(const String &p_feature) {

	return p_feature == "pc";
}

String OS_LinuxBSD::get_config_path() const {

	if (has_environment("XDG_CONFIG_HOME")) {
		return get_environment("XDG_CONFIG_HOME");
	} else if (has_environment("HOME")) {
		return get_environment("HOME").plus_file(".config");
	} else {
		return ".";
	}
}

String OS_LinuxBSD::get_data_path() const {

	if (has_environment("XDG_DATA_HOME")) {
		return get_environment("XDG_DATA_HOME");
	} else if (has_environment("HOME")) {
		return get_environment("HOME").plus_file(".local/share");
	} else {
		return get_config_path();
	}
}

String OS_LinuxBSD::get_cache_path() const {

	if (has_environment("XDG_CACHE_HOME")) {
		return get_environment("XDG_CACHE_HOME");
	} else if (has_environment("HOME")) {
		return get_environment("HOME").plus_file(".cache");
	} else {
		return get_config_path();
	}
}

String OS_LinuxBSD::get_system_dir(SystemDir p_dir) const {

	String xdgparam;

	switch (p_dir) {
		case SYSTEM_DIR_DESKTOP: {

			xdgparam = "DESKTOP";
		} break;
		case SYSTEM_DIR_DCIM: {

			xdgparam = "PICTURES";

		} break;
		case SYSTEM_DIR_DOCUMENTS: {

			xdgparam = "DOCUMENTS";

		} break;
		case SYSTEM_DIR_DOWNLOADS: {

			xdgparam = "DOWNLOAD";

		} break;
		case SYSTEM_DIR_MOVIES: {

			xdgparam = "VIDEOS";

		} break;
		case SYSTEM_DIR_MUSIC: {

			xdgparam = "MUSIC";

		} break;
		case SYSTEM_DIR_PICTURES: {

			xdgparam = "PICTURES";

		} break;
		case SYSTEM_DIR_RINGTONES: {

			xdgparam = "MUSIC";

		} break;
	}

	String pipe;
	List<String> arg;
	arg.push_back(xdgparam);
	Error err = const_cast<OS_LinuxBSD *>(this)->execute("xdg-user-dir", arg, true, NULL, &pipe);
	if (err != OK)
		return ".";
	return pipe.strip_edges();
}

void OS_LinuxBSD::alert(const String &p_alert, const String &p_title) {
	const char *message_programs[] = { "zenity", "kdialog", "Xdialog", "xmessage" };

	String path = get_environment("PATH");
	Vector<String> path_elems = path.split(":", false);
	String program;

	for (int i = 0; i < path_elems.size(); i++) {
		for (unsigned int k = 0; k < sizeof(message_programs) / sizeof(char *); k++) {
			String tested_path = path_elems[i] + "/" + message_programs[k];

			if (FileAccess::exists(tested_path)) {
				program = tested_path;
				break;
			}
		}

		if (program.length())
			break;
	}

	List<String> args;

	if (program.ends_with("zenity")) {
		args.push_back("--error");
		args.push_back("--width");
		args.push_back("500");
		args.push_back("--title");
		args.push_back(p_title);
		args.push_back("--text");
		args.push_back(p_alert);
	}

	if (program.ends_with("kdialog")) {
		args.push_back("--error");
		args.push_back(p_alert);
		args.push_back("--title");
		args.push_back(p_title);
	}

	if (program.ends_with("Xdialog")) {
		args.push_back("--title");
		args.push_back(p_title);
		args.push_back("--msgbox");
		args.push_back(p_alert);
		args.push_back("0");
		args.push_back("0");
	}

	if (program.ends_with("xmessage")) {
		args.push_back("-center");
		args.push_back("-title");
		args.push_back(p_title);
		args.push_back(p_alert);
	}

	if (program.length()) {
		execute(program, args, true);
	} else {
		print_line(p_alert);
	}

	return;
}

void OS_LinuxBSD::run() {

	force_quit = false;

	MainLoop *main_loop = DisplayDriver::get_singleton()->get_main_loop();

	if (!main_loop)
		return;

	main_loop->init();

	//uint64_t last_ticks=get_ticks_usec();

	//int frames=0;
	//uint64_t frame=0;

	while (!force_quit) {

		DisplayDriver::get_singleton()->force_process_input();
		if (Main::iteration())
			break;
	};

	main_loop->finish();
}

OS::PowerState OS_LinuxBSD::get_power_state() {
	return power_manager->get_power_state();
}

int OS_LinuxBSD::get_power_seconds_left() {
	return power_manager->get_power_seconds_left();
}

int OS_LinuxBSD::get_power_percent_left() {
	return power_manager->get_power_percent_left();
}

void OS_LinuxBSD::disable_crash_handler() {
	crash_handler.disable();
}

bool OS_LinuxBSD::is_disable_crash_handler() const {
	return crash_handler.is_disabled();
}

static String get_mountpoint(const String &p_path) {
	struct stat s;
	if (stat(p_path.utf8().get_data(), &s)) {
		return "";
	}

#ifdef HAVE_MNTENT
	dev_t dev = s.st_dev;
	FILE *fd = setmntent("/proc/mounts", "r");
	if (!fd) {
		return "";
	}

	struct mntent mnt;
	char buf[1024];
	size_t buflen = 1024;
	while (getmntent_r(fd, &mnt, buf, buflen)) {
		if (!stat(mnt.mnt_dir, &s) && s.st_dev == dev) {
			endmntent(fd);
			return String(mnt.mnt_dir);
		}
	}

	endmntent(fd);
#endif
	return "";
}

Error OS_LinuxBSD::move_to_trash(const String &p_path) {
	String trash_can = "";
	String mnt = get_mountpoint(p_path);

	// If there is a directory "[Mountpoint]/.Trash-[UID]/files", use it as the trash can.
	if (mnt != "") {
		String path(mnt + "/.Trash-" + itos(getuid()) + "/files");
		struct stat s;
		if (!stat(path.utf8().get_data(), &s)) {
			trash_can = path;
		}
	}

	// Otherwise, if ${XDG_DATA_HOME} is defined, use "${XDG_DATA_HOME}/Trash/files" as the trash can.
	if (trash_can == "") {
		char *dhome = getenv("XDG_DATA_HOME");
		if (dhome) {
			trash_can = String(dhome) + "/Trash/files";
		}
	}

	// Otherwise, if ${HOME} is defined, use "${HOME}/.local/share/Trash/files" as the trash can.
	if (trash_can == "") {
		char *home = getenv("HOME");
		if (home) {
			trash_can = String(home) + "/.local/share/Trash/files";
		}
	}

	// Issue an error if none of the previous locations is appropriate for the trash can.
	if (trash_can == "") {
		ERR_PRINTS("move_to_trash: Could not determine the trash can location");
		return FAILED;
	}

	// Create needed directories for decided trash can location.
	DirAccess *dir_access = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
	Error err = dir_access->make_dir_recursive(trash_can);
	memdelete(dir_access);

	// Issue an error if trash can is not created proprely.
	if (err != OK) {
		ERR_PRINTS("move_to_trash: Could not create the trash can \"" + trash_can + "\"");
		return err;
	}

	// The trash can is successfully created, now move the given resource to it.
	// Do not use DirAccess:rename() because it can't move files across multiple mountpoints.
	List<String> mv_args;
	mv_args.push_back(p_path);
	mv_args.push_back(trash_can);
	int retval;
	err = execute("mv", mv_args, true, NULL, NULL, &retval);

	// Issue an error if "mv" failed to move the given resource to the trash can.
	if (err != OK || retval != 0) {
		ERR_PRINTS("move_to_trash: Could not move the resource \"" + p_path + "\" to the trash can \"" + trash_can + "\"");
		return FAILED;
	}

	return OK;
}
OS_LinuxBSD::OS_LinuxBSD() {

#ifdef PULSEAUDIO_ENABLED
	AudioDriverManager::add_driver(&driver_pulseaudio);
#endif

#ifdef ALSA_ENABLED
	AudioDriverManager::add_driver(&driver_alsa);
#endif
}
