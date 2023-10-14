#include "AudioEditorConsole.h"
#include "../audioview/AudioView.h"
#include "../audioview/graph/AudioViewTrack.h"
#include "../mode/ViewModeEditAudio.h"
#include "../dialog/VolumeDialog.h"
#include "../dialog/ModuleSelectorDialog.h"
#include "../TsunamiWindow.h"
#include "../../command/Unsorted.h"
#include "../../module/Module.h"
#include "../../plugins/PluginManager.h"
#include "../../Session.h"
#include "../../EditModes.h"



AudioEditorConsole::AudioEditorConsole(Session *session, SideBar *bar) :
	SideBarConsole(_("Audio editor"), "audio-editor", session, bar)
{
	from_resource("audio-editor");

	event("mode-peaks", [this] {
		view->cur_vtrack()->set_audio_mode(AudioViewMode::PEAKS);
		update();
	});
	event("mode-spectrum", [this] {
		view->cur_vtrack()->set_audio_mode(AudioViewMode::SPECTRUM);
		update();
	});

	event("mode-select", [this] {
		on_edit_mode((int)ViewModeEditAudio::EditMode::SELECT);
	});
	event("mode-smoothen", [this] {
		on_edit_mode((int)ViewModeEditAudio::EditMode::SMOOTHEN);
	});
	event("mode-clone", [this] {
		on_edit_mode((int)ViewModeEditAudio::EditMode::CLONE);
	});
	event("mode-rubber", [this] {
		on_edit_mode((int)ViewModeEditAudio::EditMode::RUBBER);
	});
	event("stretch-apply", [this] {
		view->mode_edit_audio->apply_stretch();
	});
	event("compensate-pitch", [this] {
		view->mode_edit_audio->flag_pitch_compensate = is_checked("");
	});
	event("action-source", [this] {
		on_action_source();
	});
	event("action-effect", [this] {
		on_action_effect();
	});
	event("action-volume", [this] {
		on_action_volume();
	});
}

void AudioEditorConsole::on_enter() {
	view->mode_edit_audio->out_changed >> create_sink([this] {
		update();
	});
	view->out_cur_track_changed >> create_sink([this] {
		update();
	});
	update();
}

void AudioEditorConsole::on_leave() {
	view->mode_edit_audio->unsubscribe(this);
	view->unsubscribe(this);
}

void AudioEditorConsole::on_layer_delete() {
}

void AudioEditorConsole::on_view_cur_layer_change() {
}

void AudioEditorConsole::on_edit_mode(int m) {
	auto mode = (ViewModeEditAudio::EditMode)m;
	expand("revealer-clone", mode == ViewModeEditAudio::EditMode::CLONE);
	expand("revealer-stretch", mode == ViewModeEditAudio::EditMode::RUBBER);
	view->mode_edit_audio->set_edit_mode(mode);
}

void AudioEditorConsole::on_action_source() {
	ModuleSelectorDialog::choose(win, session, ModuleCategory::AUDIO_SOURCE).then([this] (const string &name) {
		session->win->on_menu_execute_audio_source(name);
	});
}

void AudioEditorConsole::on_action_effect() {
	ModuleSelectorDialog::choose(win, session, ModuleCategory::AUDIO_EFFECT).then([this] (const string &name) {
		session->win->on_menu_execute_audio_effect(name);
	});
}

void AudioEditorConsole::on_action_volume() {
	VolumeDialog::ask(win, 1, 0, 8).then([this] (float f) {
		song_apply_volume(session->song.get(), f, VolumeDialog::maximize, session->view->sel, win);
	});

}

void AudioEditorConsole::clear() {
}
void AudioEditorConsole::set_layer(TrackLayer *t) {
}

void AudioEditorConsole::update() {
	if (view->cur_vtrack()) {
		check("mode-peaks", view->cur_vtrack()->audio_mode == AudioViewMode::PEAKS);
		check("mode-spectrum", view->cur_vtrack()->audio_mode == AudioViewMode::SPECTRUM);
	}

	check("mode-select", view->mode_edit_audio->edit_mode == ViewModeEditAudio::EditMode::SELECT);
	check("mode-smoothen", view->mode_edit_audio->edit_mode == ViewModeEditAudio::EditMode::SMOOTHEN);
	check("mode-clone", view->mode_edit_audio->edit_mode == ViewModeEditAudio::EditMode::CLONE);
	check("mode-rubber", view->mode_edit_audio->edit_mode == ViewModeEditAudio::EditMode::RUBBER);
}

