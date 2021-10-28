#include "AudioEditorConsole.h"
#include "../Mode/ViewModeEditAudio.h"
#include "../Graph/AudioViewTrack.h"
#include "../AudioView.h"
#include "../../Session.h"
#include "../../EditModes.h"



AudioEditorConsole::AudioEditorConsole(Session *session) :
	SideBarConsole(_("Audio editor"), session)
{
	from_resource("audio-editor");

	event("mode-peaks", [=]{ view->cur_vtrack()->set_audio_mode(AudioViewMode::PEAKS); update(); });
	event("mode-spectrum", [=]{ view->cur_vtrack()->set_audio_mode(AudioViewMode::SPECTRUM); update(); });

	event("mode-select", [=]{ on_edit_mode((int)ViewModeEditAudio::EditMode::SELECT); });
	event("mode-smoothen", [=]{ on_edit_mode((int)ViewModeEditAudio::EditMode::SMOOTHEN); });
	event("mode-clone", [=]{ on_edit_mode((int)ViewModeEditAudio::EditMode::CLONE); });
	event("mode-rubber", [=]{ on_edit_mode((int)ViewModeEditAudio::EditMode::RUBBER); });
	event("edit_track", [=]{ session->set_mode(EditMode::DefaultTrack); });
	event("edit_song", [=]{ session->set_mode(EditMode::DefaultSong); });

	view->mode_edit_audio->subscribe(this, [=] { update(); }, view->mode_edit_audio->MESSAGE_ANY);
	update();

	view->subscribe(this, [=]{ update(); }, view->MESSAGE_CUR_TRACK_CHANGE);
}

AudioEditorConsole::~AudioEditorConsole() {
	view->mode_edit_audio->unsubscribe(this);
}

void AudioEditorConsole::on_layer_delete() {
}
void AudioEditorConsole::on_view_cur_layer_change() {
}

void AudioEditorConsole::on_edit_mode(int m) {
	view->mode_edit_audio->set_edit_mode(ViewModeEditAudio::EditMode(m));
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

