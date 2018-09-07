/*
 * CaptureConsoleModeMulti.cpp
 *
 *  Created on: 24.09.2017
 *      Author: michi
 */

#include "CaptureConsoleModeMulti.h"
#include "../CaptureConsole.h"
#include "../../../Device/DeviceManager.h"
#include "../../../Device/InputStreamAudio.h"
#include "../../../Device/InputStreamMidi.h"
#include "../../../Device/Device.h"
#include "../../../Data/base.h"
#include "../../../Data/Track.h"
#include "../../../Data/Song.h"
#include "../../../Session.h"
#include "../../AudioView.h"
#include "../../Mode/ViewModeCapture.h"
#include "../../../Stuff/BackupManager.h"
#include "../../../Module/Audio/AudioSucker.h"
#include "../../../Module/Audio/PeakMeter.h"

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
	sources_audio = session->device_manager->getGoodDeviceList(DeviceType::AUDIO_INPUT);
	sources_midi = session->device_manager->getGoodDeviceList(DeviceType::MIDI_INPUT);

	// target list multi
	for (Track *t: song->tracks){
		if ((t->type != SignalType::AUDIO) and (t->type != SignalType::MIDI))
			continue;
		if (!view->sel.has(t))
			continue;
		CaptureItem c;
		int i = items.num;
		c.sucker = nullptr;
		c.input_audio = nullptr;
		c.input_midi = nullptr;
		c.track = t;
		c.device = nullptr;
		c.id_target = "target-" + i2s(i);
		c.id_type = "type-" + i2s(i);
		c.id_source = "source-" + i2s(i);
		c.id_peaks = "peaks-" + i2s(i);
		cc->setTarget("multi_grid");
		cc->addLabel(t->getNiceName(), 0, i*2+1, c.id_target);
		cc->addLabel(signal_type_name(t->type), 1, i*2+1, c.id_type);
		if (t->type == SignalType::AUDIO){
			c.input_audio = new InputStreamAudio(session);
			c.input_audio->set_backup_mode(BACKUP_MODE_TEMP);
			cc->addComboBox(_("        - none -"), 2, i*2+1, c.id_source);
			for (Device *d: sources_audio)
				cc->addString(c.id_source, d->get_name());
		}else if (t->type == SignalType::MIDI){
			c.input_midi = new InputStreamMidi(session);
			cc->addComboBox(_("        - none -"), 2, i*2+1, c.id_source);
			for (Device *d: sources_midi)
				cc->addString(c.id_source, d->get_name());
		/*}else{
			cc->addLabel(_("        - none -"), 2, i*2+1, c.id_source);*/
		}
		cc->addDrawingArea("!height=30,noexpandy", 2, i*2+2, c.id_peaks);
		c.peak_meter = (PeakMeter*)CreateAudioVisualizer(session, "PeakMeter");
		c.peak_meter_display = new PeakMeterDisplay(cc, c.id_peaks, c.peak_meter);

		if (t->type == SignalType::AUDIO){
			c.peak_meter->set_source(c.input_audio->out);
			c.sucker = CreateAudioSucker(session);
			c.sucker->set_source(c.peak_meter->out);
		}

		items.add(c);
		cc->event(c.id_source, [&]{ on_source(); });
	}
}

bool CaptureConsoleModeMulti::is_capturing()
{
	for (auto &c: items){
		if (c.track->type == SignalType::AUDIO){
			return c.input_audio->is_capturing();
		}else if (c.track->type == SignalType::MIDI){
				return c.input_midi->is_capturing();
		}
	}
	return false;
}

void CaptureConsoleModeMulti::pause()
{
	for (auto &c: items){
		if (c.track->type == SignalType::AUDIO){
			c.sucker->accumulate(false);
		}else if (c.track->type == SignalType::MIDI){
			c.input_midi->accumulate(false);
		}
	}
}

bool CaptureConsoleModeMulti::insert()
{
	bool ok = true;

	song->beginActionGroup();

	for (auto &c: items){
		if (c.track->type == SignalType::AUDIO){
			int dpos = c.input_audio->get_delay();
			ok &= cc->insert_audio(c.track, c.sucker->buf, dpos);
			c.sucker->reset_accumulation();
		}else if (c.track->type == SignalType::MIDI){
			int dpos = c.input_midi->get_delay();
			ok &= cc->insert_midi(c.track, c.input_midi->midi, dpos);
			c.input_midi->reset_accumulation();
		}
	}
	song->endActionGroup();
	return ok;
}

void CaptureConsoleModeMulti::on_source()
{
	int index = hui::GetEvent()->id.substr(7, -1)._int();
	if (index < 0 or index >= items.num)
		return;
	int n = cc->getInt("");
	auto &c = items[index];
	if (c.track->type == SignalType::AUDIO){
		if (n > 0){
			c.input_audio->set_device(sources_audio[n - 1]);
			c.input_audio->start();
			c.sucker->start();
		}
	}else if (c.track->type == SignalType::MIDI){
		if (n > 0){
			c.input_midi->set_device(sources_midi[n - 1]);
			c.input_midi->start();
		}
	}
	/*if ((n >= 0) and (n < sources.num)){
		chosen_device = sources[n];
		input->set_device(chosen_device);
	}*/
}

void CaptureConsoleModeMulti::leave()
{
	view->mode_capture->set_data({});
	for (auto c: items){
		c.peak_meter_display->set_source(nullptr);
		c.peak_meter->set_source(nullptr);

		if (c.track->type == SignalType::AUDIO){
			delete c.sucker;
			delete c.input_audio;
		}else if (c.track->type == SignalType::MIDI){
			c.peak_meter->set_source(nullptr);
			delete c.input_midi;
		}

		delete c.peak_meter_display;
		delete c.peak_meter;
		cc->removeControl(c.id_target);
		cc->removeControl(c.id_type);
		cc->removeControl(c.id_source);
		cc->removeControl(c.id_peaks);
	}
	items.clear();
}


void CaptureConsoleModeMulti::start()
{
	Array<CaptureTrackData> data;
	for (auto &c: items){
		if (c.track->type == SignalType::AUDIO){
			c.input_audio->reset_sync();
			c.sucker->accumulate(true);
			data.add({c.track, c.input_audio, c.sucker});
		}else if (c.track->type == SignalType::MIDI){
			c.input_midi->accumulate(true);
			data.add({c.track, c.input_midi, nullptr});
		}
		cc->enable(c.id_source, false);
	}
	view->mode_capture->set_data(data);
}

void CaptureConsoleModeMulti::stop()
{
	for (auto &c: items){
		if (c.track->type == SignalType::AUDIO){
			c.sucker->stop();
			c.input_audio->stop();
		}else if (c.track->type == SignalType::MIDI){
			c.input_midi->stop();
		}
	}
}

void CaptureConsoleModeMulti::dump()
{
	for (auto &c: items){
		if (c.track->type == SignalType::AUDIO){
			c.sucker->accumulate(false);
			c.sucker->reset_accumulation();
		}else if (c.track->type == SignalType::MIDI){
			c.input_midi->accumulate(true);
		}
		cc->enable(c.id_source, true);
	}
}

int CaptureConsoleModeMulti::get_sample_count()
{
	for (auto &c: items){
		if (c.track->type == SignalType::AUDIO){
			return c.sucker->buf.length;
		}else if (c.track->type == SignalType::MIDI){
			return c.input_midi->get_sample_count();
		}
	}
	return 0;
}
