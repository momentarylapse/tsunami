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
}

CaptureConsoleModeMulti::~CaptureConsoleModeMulti()
{
}

void CaptureConsoleModeMulti::enter()
{
	cc->hideControl("capture_multi_grid", false);
	sources_audio = cc->device_manager->getGoodDeviceList(Device::Type::AUDIO_INPUT);
	sources_midi = cc->device_manager->getGoodDeviceList(Device::Type::MIDI_INPUT);

	// target list multi
	for (Track *t: song->tracks){
		if ((t->type != t->Type::AUDIO) and (t->type != t->Type::MIDI))
			continue;
		if (!view->sel.has(t))
			continue;
		CaptureItem c;
		int i = items.num;
		c.track = t;
		c.device = NULL;
		c.id_target = "capture-multi-target-" + i2s(i);
		c.id_type = "capture-multi-type-" + i2s(i);
		c.id_source = "capture-multi-source-" + i2s(i);
		cc->setTarget("capture_multi_grid");
		cc->addLabel(t->getNiceName(), 0, i+1, c.id_target);
		cc->addLabel(track_type(t->type), 1, i+1, c.id_type);
		if (t->type == Track::Type::AUDIO){
			cc->addComboBox(_("        - none -"), 2, i+1, c.id_source);
			for (Device *d: sources_audio)
				cc->addString(c.id_source, d->get_name());
		}else if (t->type == Track::Type::MIDI){
			cc->addComboBox(_("        - none -"), 2, i+1, c.id_source);
			for (Device *d: sources_midi)
				cc->addString(c.id_source, d->get_name());
		}else{
			cc->addLabel(_("        - none -"), 2, i+1, c.id_source);
		}
		items.add(c);
	}
}

void CaptureConsoleModeMulti::leave()
{
	for (auto c: items){
		cc->removeControl(c.id_target);
		cc->removeControl(c.id_type);
		cc->removeControl(c.id_source);
	}
	items.clear();
}
