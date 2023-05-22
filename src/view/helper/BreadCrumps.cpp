/*
 * BreadCrumps.cpp
 *
 *  Created on: 22 May 2023
 *      Author: michi
 */

#include "BreadCrumps.h"
#include "../../lib/hui/hui.h"
#include "../../EditModes.h"
#include "../../Session.h"

namespace BreadCrumps {

void add(hui::Panel* panel, Session* session) {
	panel->event("edit-song", [session] {
		session->set_mode(EditMode::DefaultSong);
	});
	panel->event("edit-samples", [session] {
		session->set_mode(EditMode::DefaultSamples);
	});
	panel->event("edit-track", [session] {
		session->set_mode(EditMode::DefaultTrack);
	});
	panel->event("edit-track-fx", [session] {
		session->set_mode(EditMode::DefaultTrackFx);
	});
	panel->event("edit-track-curves", [session] {
		session->set_mode(EditMode::Curves);
	});
	panel->event("edit-track-data", [session] {
		session->set_mode(EditMode::EditTrack);
	});
	/*event("edit_samples", [this, session] {
		bar()->sample_manager->set_selection({sample->origin.get()});
		session->set_mode(EditMode::DefaultSamples);
	});*/
}

}
