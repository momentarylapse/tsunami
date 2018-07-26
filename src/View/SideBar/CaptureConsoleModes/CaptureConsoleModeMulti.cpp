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
#include "../../../Data/base.h"
#include "../../../Data/Track.h"
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
	cc->hideControl("multi_grid", false);
	sources_audio = cc->device_manager->getGoodDeviceList(DeviceType::AUDIO_INPUT);
	sources_midi = cc->device_manager->getGoodDeviceList(DeviceType::MIDI_INPUT);

	// target list multi
	for (Track *t: song->tracks){
		if ((t->type != SignalType::AUDIO) and (t->type != SignalType::MIDI))
			continue;
		if (!view->sel.has(t))
			continue;
		CaptureItem c;
		int i = items.num;
		c.track = t;
		c.device = nullptr;
		c.id_target = "target-" + i2s(i);
		c.id_type = "type-" + i2s(i);
		c.id_source = "source-" + i2s(i);
		cc->setTarget("multi_grid");
		cc->addLabel(t->getNiceName(), 0, i+1, c.id_target);
		cc->addLabel(signal_type_name(t->type), 1, i+1, c.id_type);
		if (t->type == SignalType::AUDIO){
			cc->addComboBox(_("        - none -"), 2, i+1, c.id_source);
			for (Device *d: sources_audio)
				cc->addString(c.id_source, d->get_name());
		}else if (t->type == SignalType::MIDI){
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
