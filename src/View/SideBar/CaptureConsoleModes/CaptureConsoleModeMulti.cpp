/*
 * CaptureConsoleModeMulti.cpp
 *
 *  Created on: 24.09.2017
 *      Author: michi
 */

#include "CaptureConsoleModeMulti.h"
#include "../CaptureConsole.h"
#include "../../../Device/DeviceManager.h"
#include "../../../Device/Device.h"
#include "../../../Data/Song.h"
#include "../../AudioView.h"
#include "../../Mode/ViewModeCapture.h"

CaptureConsoleModeMulti::CaptureConsoleModeMulti(CaptureConsole *_cc) :
	CaptureConsoleMode(_cc)
{
	size = 0;
}

CaptureConsoleModeMulti::~CaptureConsoleModeMulti()
{
}

void CaptureConsoleModeMulti::enterParent()
{
	sources_audio = cc->device_manager->getGoodDeviceList(Device::TYPE_AUDIO_INPUT);
	sources_midi = cc->device_manager->getGoodDeviceList(Device::TYPE_MIDI_INPUT);

	// target list multi
	//reset("capture_multi_list");
	//for (Track *t: song->tracks)
	//	addString("capture_multi_list", t->getNiceName() + "\\" + track_type(t->type) + "\\ - none -");
	foreachi (Track *t, song->tracks, i){
		cc->setTarget("capture_multi_grid", -1);
		cc->addLabel(t->getNiceName(), 0, i+1, 0, 0, "capture-multi-target-" + i2s(i));
		cc->addLabel(track_type(t->type), 1, i+1, 0, 0, "capture-multi-type-" + i2s(i));
		if (t->type == Track::TYPE_AUDIO){
			cc->addComboBox(_("        - none -"), 2, i+1, 0, 0, "capture-multi-source-" + i2s(i));
			for (Device *d: sources_audio)
				cc->addString("capture-multi-source-" + i2s(i), d->get_name());
		}else if (t->type == Track::TYPE_MIDI){
			cc->addComboBox(_("        - none -"), 2, i+1, 0, 0, "capture-multi-source-" + i2s(i));
			for (Device *d: sources_midi)
				cc->addString("capture-multi-source-" + i2s(i), d->get_name());
		}else{
			cc->addLabel(_("        - none -"), 2, i+1, 0, 0, "capture-multi-source-" + i2s(i));
		}
	}
	size = song->tracks.num;
}

void CaptureConsoleModeMulti::leaveParent()
{
	for (int i=0; i<size; i++){
		cc->removeControl("capture-multi-target-" + i2s(i));
		cc->removeControl("capture-multi-type-" + i2s(i));
		cc->removeControl("capture-multi-source-" + i2s(i));
	}
	size = 0;
}
