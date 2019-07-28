#include "AudioEditorConsole.h"
#include "../Mode/ViewModeEditAudio.h"
#include "../AudioView.h"



AudioEditorConsole::AudioEditorConsole(Session *session) :
	SideBarConsole(_("Audio"), session)
{
	from_resource("audio-editor");
	
	event("edit-mode", [=]{ on_edit_mode(); });
	event("edit_track", [=]{ on_edit_track(); });
	event("edit_song", [=]{ on_edit_song(); });
}

AudioEditorConsole::~AudioEditorConsole()
{
}


void AudioEditorConsole::on_enter() {
	
}
void AudioEditorConsole::on_leave() {
}

void AudioEditorConsole::on_layer_delete() {
}
void AudioEditorConsole::on_view_cur_layer_change() {
}

void AudioEditorConsole::on_edit_mode() {
	int n = get_int("");
	msg_write(n);
	if (n == 0)
		view->mode_edit_audio->set_edit_mode(ViewModeEditAudio::EditMode::SELECT);
	if (n == 1)
		view->mode_edit_audio->set_edit_mode(ViewModeEditAudio::EditMode::SMOOTHEN);
	if (n == 2)
		view->mode_edit_audio->set_edit_mode(ViewModeEditAudio::EditMode::CLONE);
}

void AudioEditorConsole::on_edit_track() {
}
void AudioEditorConsole::on_edit_song() {
}

void AudioEditorConsole::clear() {
}
void AudioEditorConsole::set_layer(TrackLayer *t) {
}

