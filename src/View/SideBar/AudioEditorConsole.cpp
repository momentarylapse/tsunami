#include "AudioEditorConsole.h"
#include "../Mode/ViewModeEditAudio.h"
#include "../AudioView.h"
#include "../../Session.h"



AudioEditorConsole::AudioEditorConsole(Session *session) :
	SideBarConsole(_("Audio"), session)
{
	from_resource("audio-editor");
	
	event("edit-mode", [=]{ on_edit_mode(); });
	event("edit_track", [=]{ session->set_mode("default/track"); });
	event("edit_song", [=]{ session->set_mode("default/song"); });
}

void AudioEditorConsole::on_layer_delete() {
}
void AudioEditorConsole::on_view_cur_layer_change() {
}

void AudioEditorConsole::on_edit_mode() {
	int n = get_int("");
	if (n == 0)
		view->mode_edit_audio->set_edit_mode(ViewModeEditAudio::EditMode::SELECT);
	if (n == 1)
		view->mode_edit_audio->set_edit_mode(ViewModeEditAudio::EditMode::SMOOTHEN);
	if (n == 2)
		view->mode_edit_audio->set_edit_mode(ViewModeEditAudio::EditMode::CLONE);
	if (n == 3)
		view->mode_edit_audio->set_edit_mode(ViewModeEditAudio::EditMode::RUBBER);
}

void AudioEditorConsole::clear() {
}
void AudioEditorConsole::set_layer(TrackLayer *t) {
}

