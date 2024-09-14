/*
 * SettingsDialog.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "SettingsDialog.h"
#include "../helper/CpuDisplay.h"
#include "../helper/Slider.h"
#include "../audioview/AudioView.h"
#include "../mainview/MainView.h"
#include "../TsunamiWindow.h"
#include "../ColorScheme.h"
#include "../../device/DeviceManager.h"
#include "../../module/stream/AudioOutput.h"
#include "../../module/SignalChain.h"
#include "../../lib/base/iter.h"
#include "../../lib/hui/config.h"
#include "../../lib/hui/language.h"
#include "../../lib/hui/common_dlg.h"
#include "../../storage/Storage.h"
#include "../../Session.h"
#include "../../Playback.h"
#include "../../Tsunami.h"


namespace tsunami {

SettingsDialog::SettingsDialog(AudioView *_view, hui::Window *_parent) :
		hui::Dialog("settings_dialog", _parent) {
	view = _view;
	event("language", [this] { on_language(); });
	event("color_scheme", [this] { on_color_scheme(); });
	event("ogg_bitrate", [this] { on_ogg_bitrate(); });
	event("default_artist", [this] { on_default_artist(); });
	event("controls:toolbar", [this] { on_controls(false); });
	event("controls:headerbar", [this] { on_controls(true); });
	event("scroll_speed", [this] { on_scroll_speed(); });
	event("cpu_meter", [this] { on_cpu_meter(); });
	event("antialiasing", [this] { on_antialiasing(); });
	event("high_details", [this] { on_high_details(); });
	event("audio_api", [this] { on_audio_api(); });
	event("midi_api", [this] { on_midi_api(); });
	event("prebuffer_size", [this] { on_prebuffer(); });
	event("suck_size", [this] { on_suck_buffer(); });
	event("quick_export_dir_find", [this] { on_qed_find(); });
	event("hui:close", [this] { request_destroy(); });
	event("close", [this] { request_destroy(); });

	set_options("default_artist", "placeholder=" + AppName);

	set_options("quick_export_dir", "placeholder=" + hui::Application::directory.str());

	ogg_quality = {{0.0f, 64}, {0.1f, 80}, {0.2f, 96}, {0.3f, 112}, {0.4f, 128}, {0.5f, 160}, {0.6f, 192}, {0.7f, 224}, {0.8f, 256}, {0.9f, 320}, {1.0f, 500}};


	//device_console = new DeviceConsole(view->session, this);
	//embed(device_console, "device-console", 0, 0);

	load_data();

}

void SettingsDialog::load_data() {
	// language
	Array<string> lang = hui::get_languages();
	for (const auto& [i, l]: enumerate(lang)) {
		add_string("language", l);
		if (l == hui::get_cur_language())
			set_int("language", i);
	}

	// color scheme
	for (const auto& [i, b]: enumerate(view->win->main_view->themes)) {
		add_string("color_scheme", b.name);
		if (b.name == theme.name)
			set_int("color_scheme", i);
	}

	check("controls:toolbar", !hui::config.get_bool("Window.HeaderBar", false));
	check("controls:headerbar", hui::config.get_bool("Window.HeaderBar", false));

	// ogg quality
	for (const auto& [i, q]: enumerate(ogg_quality))
		if (Storage::default_ogg_quality > q.quality - 0.05f)
			set_int("ogg_bitrate", i);
	set_decimals(1);

	set_string("default_artist", hui::config.get_str("DefaultArtist", ""));

	set_string("quick_export_dir", str(Storage::quick_export_directory));

	check("cpu_meter", hui::config.get_bool("CpuDisplay", false));
	check("antialiasing", view->antialiasing);
	check("high_details", view->high_details);
	set_float("scroll_speed", hui::config.get_float("hui.scroll-factor", 1.0f));


	int n_audio = 0, n_midi = 0;
	for (const auto& a: DeviceManager::api_descriptions) {
		if (!a.available)
			continue;
		if (a.mode & 1) {
			add_string("audio_api", a.name);
			if (a.type == Session::GLOBAL->device_manager->audio_api)
				set_int("audio_api", n_audio);
			n_audio++;
		}
		if (a.mode & 2) {
			add_string("midi_api", a.name);
			if (a.type == Session::GLOBAL->device_manager->midi_api)
				set_int("midi_api", n_midi);
			n_midi++;
		}
	}
	set_int("prebuffer_size", hui::config.get_int("Output.BufferSize", AudioOutputStream::DEFAULT_PREBUFFER_SIZE));
	set_int("suck_size", hui::config.get_int("SignalChain.BufferSize", SignalChain::DEFAULT_BUFFER_SIZE));
}

void SettingsDialog::applyData() {
}

void SettingsDialog::on_language() {
	Array<string> lang = hui::get_languages();
	int l = get_int("");
	hui::set_language(lang[l]);
	hui::config.set_str("Language", lang[l]);
}

void SettingsDialog::on_color_scheme() {
	int i = get_int("");
	if ((i >= 0) and (i < view->win->main_view->themes.num))
		view->win->main_view->set_theme(view->win->main_view->themes[i].name);
}

void SettingsDialog::on_ogg_bitrate() {
	Storage::default_ogg_quality = ogg_quality[get_int("")].quality;
}

void SettingsDialog::on_default_artist() {
	hui::config.set_str("DefaultArtist", get_string(""));
}

void SettingsDialog::on_scroll_speed() {
	hui::config.set_float("hui.scroll-factor", get_float(""));
}

void SettingsDialog::on_controls(bool header) {
	hui::config.set_bool("Window.HeaderBar", header);
}

void SettingsDialog::on_audio_api() {
	int n = get_int("");
	int n_audio = 0;
	for (auto& a: DeviceManager::api_descriptions) {
		if (!a.available)
			continue;
		if (a.mode & 1) {
			if (n_audio == n)
				hui::config.set_str("AudioApi", a.name);
			n_audio++;
		}
	}
}

void SettingsDialog::on_midi_api() {
	int n = get_int("");
	int n_midi = 0;
	for (auto& a: DeviceManager::api_descriptions) {
		if (!a.available)
			continue;
		if (a.mode & 2) {
			if (n_midi == n)
				hui::config.set_str("MidiApi", a.name);
			n_midi++;
		}
	}
}

void SettingsDialog::on_prebuffer() {
	int n = get_int("");
	hui::config.set_int("Output.BufferSize", n);
	view->session->playback->output_stream->set_prebuffer_size(n);
}

void SettingsDialog::on_suck_buffer() {
	int n = get_int("");
	hui::config.set_int("SignalChain.BufferSize", n);
	view->session->playback->signal_chain->set_buffer_size(n);
}

void SettingsDialog::on_cpu_meter() {
	bool show = is_checked("");
	hui::config.set_bool("CpuDisplay", show);
	view->win->main_view->cpu_display->enable(show);
}

void SettingsDialog::on_antialiasing() {
	view->set_antialiasing(is_checked(""));
}

void SettingsDialog::on_high_details() {
	view->set_high_details(is_checked(""));
}

void SettingsDialog::on_qed_find() {
	hui::file_dialog_dir(this, _("Quick export directory"), "", {}).then([this] (const Path &dir) {
		Storage::quick_export_directory = dir;
		set_string("quick_export_dir", dir.str());
	});
}

}
