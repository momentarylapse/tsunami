/*
 * CaptureDialog.cpp
 *
 *  Created on: 27.03.2012
 *      Author: michi
 */

#include "../../Tsunami.h"
#include "../../TsunamiWindow.h"
#include "../../Device/InputStreamAudio.h"
#include "../../Device/InputStreamMidi.h"
#include "../../Device/OutputStream.h"
#include "../../Audio/Synth/Synthesizer.h"
#include "../../Audio/AudioSucker.h"
#include "../AudioView.h"
#include "../Mode/ViewModeCapture.h"
#include "../../Stuff/Log.h"
#include "../../Plugins/PluginManager.h"

#include "../../Action/Track/Buffer/ActionTrackEditBuffer.h"
#include "CaptureConsole.h"

#include "../../Audio/Source/SongRenderer.h"
#include "../../Device/DeviceManager.h"
#include "../../Device/Device.h"


extern AudioSucker *export_view_sucker;

class CaptureConsoleMode
{
public:
	CaptureConsoleMode(CaptureConsole *_cc)
{
		cc = _cc;
		song = cc->song;
		view = cc->view;
	}
	virtual ~CaptureConsoleMode(){};
	virtual void start() = 0;
	virtual void stop() = 0;
	virtual void enter() = 0;
	virtual void leave() = 0;
	virtual void enterParent(){};
	virtual void leaveParent(){};
	virtual bool insert() = 0;
	virtual void pause() = 0;
	virtual void dump() = 0;
	virtual int getSampleCount() = 0;
	virtual bool isCapturing() = 0;

	CaptureConsole *cc;
	Song *song;
	AudioView *view;
};

class CaptureConsoleModeAudio : public CaptureConsoleMode
{
	InputStreamAudio *input;
	AudioSucker *sucker;
	Array<Device*> sources;
	Device *chosen_device;
	Track *target;

public:
	CaptureConsoleModeAudio(CaptureConsole *_cc) :
		CaptureConsoleMode(_cc)
	{
		chosen_device = cc->device_manager->chooseDevice(Device::TYPE_AUDIO_INPUT);
		input = NULL;
		target = NULL;
		sucker = NULL;

		cc->event("capture_audio_source", std::bind(&CaptureConsoleModeAudio::onSource, this));
		cc->event("capture_audio_target", std::bind(&CaptureConsoleModeAudio::onTarget, this));
	}

	void onSource()
	{
		int n = cc->getInt("");
		if ((n >= 0) and (n < sources.num)){
			chosen_device = sources[n];
			input->setDevice(chosen_device);
		}
	}

	void onTarget()
	{
		int index = cc->getInt("capture_audio_target");
		if (index >= 0)
			setTarget(cc->song->tracks[index]);
	}

	void setTarget(Track *t)
	{
		target = t;
		view->setCurTrack(target);
		view->mode_capture->capturing_track = target;
		cc->setInt("capture_audio_target", target->get_index());

		bool ok = (target->type == Track::TYPE_AUDIO);
		cc->setString("capture_audio_message", "");
		if (!ok)
			cc->setString("capture_audio_message", format(_("Please select a track of type %s."), track_type(Track::TYPE_AUDIO).c_str()));
		cc->enable("capture_start", ok);
	}

	virtual void enterParent()
	{
		target = view->cur_track;
	}

	virtual void enter()
	{
		sources = cc->device_manager->getGoodDeviceList(Device::TYPE_AUDIO_INPUT);

		// add all
		cc->reset("capture_audio_source");
		for (Device *d: sources)
			cc->setString("capture_audio_source", d->get_name());

		// select current
		foreachi(Device *d, sources, i)
			if (d == chosen_device)
				cc->setInt("capture_audio_source", i);


		// target list
		cc->reset("capture_audio_target");
		for (Track *t: song->tracks)
			cc->addString("capture_audio_target", t->getNiceName() + "     (" + track_type(t->type) + ")");
		//cc->addString("capture_audio_target", _("  - create new track -"));

		setTarget(target);


		input = new InputStreamAudio(song->sample_rate);
		input->setBackupMode(BACKUP_MODE_TEMP);
		input->setChunkSize(4096);
		input->setUpdateDt(0.03f);
		view->mode_capture->setInputAudio(input);
		cc->peak_meter->setSource(input);

		input->setDevice(chosen_device);

		//enable("capture_audio_source", false);

		if (!input->start()){
			/*HuiErrorBox(MainWin, _("Error"), _("Could not open recording device"));
			CapturingByDialog = false;
			msg_db_l(1);
			return;*/
		}

		sucker = new AudioSucker(input->source);
		sucker->start();
		export_view_sucker = sucker;
	}

	virtual void leave()
	{
		delete sucker;
		cc->peak_meter->setSource(NULL);
		view->mode_capture->setInputAudio(NULL);
		delete(input);
		input = NULL;
	}

	virtual void pause()
	{
		sucker->accumulate(false);
	}

	virtual void start()
	{
		input->resetSync();
		sucker->accumulate(true);
		cc->enable("capture_audio_source", false);
		cc->enable("capture_audio_target", false);
	}

	virtual void stop()
	{
		input->stop();
	}

	virtual void dump()
	{
		sucker->accumulate(false);
		sucker->resetAccumulation();
		cc->enable("capture_audio_source", true);
		cc->enable("capture_audio_target", true);
	}

	virtual bool insert()
	{
		int s_start = view->sel.range.start();

		// insert recorded data with some delay
		int dpos = input->getDelay();

		// overwrite
		int i0 = s_start + dpos;

		if (target->type != Track::TYPE_AUDIO){
			tsunami->log->error(format(_("Can't insert recorded data (%s) into target (%s)."), track_type(Track::TYPE_AUDIO).c_str(), track_type(target->type).c_str()));
			return false;
		}

		// insert data
		Range r = Range(i0, sucker->buf.length);
		cc->song->action_manager->beginActionGroup();
		AudioBuffer tbuf = target->getBuffers(view->cur_layer, r);
		ActionTrackEditBuffer *a = new ActionTrackEditBuffer(target, view->cur_layer, r);

		if (hui::Config.getInt("Input.Mode", 0) == 1)
			tbuf.add(sucker->buf, 0, 1.0f, 0);
		else
			tbuf.set(sucker->buf, 0, 1.0f);
		song->execute(a);
		song->action_manager->endActionGroup();

		sucker->resetAccumulation();
		return true;
	}

	virtual int getSampleCount()
	{
		return sucker->buf.length;
	}

	virtual bool isCapturing()
	{
		return input->isCapturing();
	}
};


class CaptureConsoleModeMidi : public CaptureConsoleMode
{
	InputStreamMidi *input;
	Array<Device*> sources;
	Device *chosen_device;
	Track *target;
	Synthesizer *preview_synth;
	OutputStream *preview_stream;



public:
	CaptureConsoleModeMidi(CaptureConsole *_cc) :
		CaptureConsoleMode(_cc)
	{
		chosen_device = cc->device_manager->chooseDevice(Device::TYPE_MIDI_INPUT);
		input = NULL;
		target = NULL;

		preview_synth = NULL;
		preview_stream = NULL;

		cc->event("capture_midi_source", std::bind(&CaptureConsoleModeMidi::onSource, this));
		cc->event("capture_midi_target", std::bind(&CaptureConsoleModeMidi::onTarget, this));
	}

	void onSource()
	{
		int n = cc->getInt("");
		if ((n >= 0) and (n < sources.num)){
			chosen_device = sources[n];
			input->setDevice(chosen_device);
		}
	}

	void onTarget()
	{
		setTarget(cc->song->tracks[cc->getInt("capture_midi_target")]);
	}


	void setTarget(Track *t)
	{
		if (preview_stream)
			delete preview_stream;
		if (preview_synth)
			delete preview_synth;

		target = t;
		preview_synth = (Synthesizer*)t->synth->copy();
		preview_synth->out->setSource(input->out);
		preview_stream = new OutputStream(preview_synth->out);
		preview_stream->setBufferSize(512);
		preview_stream->_play();
		view->setCurTrack(target);
		view->mode_capture->capturing_track = target;
		cc->setInt("capture_midi_target", target->get_index());


		bool ok = (target->type == Track::TYPE_MIDI);
		cc->setString("capture_midi_message", "");
		if (!ok)
			cc->setString("capture_midi_message", format(_("Please select a track of type %s."), track_type(Track::TYPE_MIDI).c_str()));
		cc->enable("capture_start", ok);
	}

	virtual void enterParent()
	{
		target = view->cur_track;
	}

	virtual void enter()
	{
		sources = cc->device_manager->getGoodDeviceList(Device::TYPE_MIDI_INPUT);

		// add all
		cc->reset("capture_midi_source");
		for (Device *d: sources)
			cc->setString("capture_midi_source", d->get_name());

		// select current
		foreachi(Device *d, sources, i)
			if (d == chosen_device)
				cc->setInt("capture_midi_source", i);


		// target list
		cc->reset("capture_midi_target");
		for (Track *t: song->tracks)
			cc->addString("capture_midi_target", t->getNiceName() + "     (" + track_type(t->type) + ")");
		//cc->addString("capture_midi_target", _("  - create new track -"));

		input = new InputStreamMidi(song->sample_rate);
		input->setBackupMode(BACKUP_MODE_TEMP);
		input->setChunkSize(512);
		input->setUpdateDt(0.005f);
		view->mode_capture->setInputMidi(input);
		cc->peak_meter->setSource(input);

		input->setDevice(chosen_device);

		setTarget(target);

		if (!input->start()){
			/*HuiErrorBox(MainWin, _("Error"), _("Could not open recording device"));
			CapturingByDialog = false;
			msg_db_l(1);
			return;*/
		}
	}

	virtual void leave()
	{
		cc->peak_meter->setSource(NULL);
		view->mode_capture->setInputMidi(NULL);
		delete(input);
		input = NULL;
	}

	virtual void pause()
	{
		input->accumulate(false);
	}

	virtual void start()
	{
		input->resetSync();
		input->accumulate(true);
		cc->enable("capture_midi_source", false);
		cc->enable("capture_midi_target", false);
	}

	virtual void stop()
	{
		preview_stream->_stop();
		input->stop();
	}

	virtual void dump()
	{
		input->accumulate(false);
		input->resetAccumulation();
		cc->enable("capture_midi_source", true);
		cc->enable("capture_midi_target", true);
	}

	virtual bool insert()
	{
		int s_start = view->sel.range.start();

		// insert recorded data with some delay
		int dpos = input->getDelay();

		int i0 = s_start + dpos;

		if (target->type != Track::TYPE_MIDI){
			tsunami->log->error(format(_("Can't insert recorded data (%s) into target (%s)."), track_type(Track::TYPE_MIDI).c_str(), track_type(target->type).c_str()));
			return false;
		}

		// insert data
		target->insertMidiData(i0, midi_events_to_notes(input->midi));

		input->resetAccumulation();
		return true;
	}

	virtual int getSampleCount()
	{
		return input->getSampleCount();
	}

	virtual bool isCapturing()
	{
		return input->isCapturing();
	}
};




class CaptureConsoleModeMulti: public CaptureConsoleMode
{
	Array<Device*> sources_audio;
	Array<Device*> sources_midi;

	int size;


public:
	CaptureConsoleModeMulti(CaptureConsole *_cc) :
		CaptureConsoleMode(_cc)
	{
		size = 0;
	}

	virtual ~CaptureConsoleModeMulti()
	{
	}

	virtual void enterParent()
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

	virtual void leaveParent()
	{
		for (int i=0; i<size; i++){
			cc->removeControl("capture-multi-target-" + i2s(i));
			cc->removeControl("capture-multi-type-" + i2s(i));
			cc->removeControl("capture-multi-source-" + i2s(i));
		}
		size = 0;
	}

	virtual void enter()
	{
	}

	virtual void leave()
	{
	}

	virtual void pause()
	{
	}

	virtual void start()
	{
	}

	virtual void stop()
	{
	}

	virtual void dump()
	{
	}

	virtual bool insert()
	{
		return false;
	}

	virtual int getSampleCount()
	{
		return 0;
	}

	virtual bool isCapturing()
	{
		return false;
	}
};

CaptureConsole::CaptureConsole(Song *s, AudioView *v):
	SideBarConsole(_("Recording"))
{
	song = s;
	view = v;
	mode = NULL;


	// dialog
	setBorderWidth(5);
	embedDialog("record_dialog", 0, 0);

	device_manager = tsunami->device_manager;


	// dialog
	peak_meter = new PeakMeter(this, "capture_level", NULL, view);



	event("cancel", std::bind(&CaptureConsole::onCancel, this));
	//event("hui:close", std::bind(&CaptureConsole::onClose, this));
	event("ok", std::bind(&CaptureConsole::onOk, this));
	event("capture_type", std::bind(&CaptureConsole::onType, this));
	event("capture_start", std::bind(&CaptureConsole::onStart, this));
	event("capture_delete", std::bind(&CaptureConsole::onDelete, this));
	event("capture_pause", std::bind(&CaptureConsole::onPause, this));

	mode_audio = new CaptureConsoleModeAudio(this);
	mode_midi = new CaptureConsoleModeMidi(this);
	mode_multi = new CaptureConsoleModeMulti(this);
}

CaptureConsole::~CaptureConsole()
{
	delete(mode_audio);
	delete(mode_midi);
	delete(mode_multi);
	delete(peak_meter);
}

inline int dev_type(int type)
{
	if (type == Track::TYPE_AUDIO)
		return Device::TYPE_AUDIO_INPUT;
	return Device::TYPE_MIDI_INPUT;
}

void CaptureConsole::onEnter()
{
	view->renderer->prepare(view->getPlaybackSelection(), false);


	if (view->cur_track->type == Track::TYPE_AUDIO){
		mode = mode_audio;
		setInt("capture_type", 0);
	}else if (view->cur_track->type == Track::TYPE_MIDI){
		mode = mode_midi;
		setInt("capture_type", 1);
	}else{ // TYPE_TIME
		mode = mode_multi;
		setInt("capture_type", 2);
	}

	mode_audio->enterParent();
	mode_midi->enterParent();
	mode_multi->enterParent();

	view->setMode(view->mode_capture);

	mode->enter();

	// automatically start
	onStart();
}

void CaptureConsole::onLeave()
{
	if (mode->isCapturing())
		mode->insert();

	view->stop();

	mode->leave();

	mode_audio->leaveParent();
	mode_midi->leaveParent();
	mode_multi->leaveParent();

	view->setMode(view->mode_default);
}


void CaptureConsole::onType()
{
	mode->leave();

	int n = getInt("capture_type");
	if (n == 0)
		mode = mode_audio;
	if (n == 1)
		mode = mode_midi;
	if (n == 2)
		mode = mode_multi;
	mode->enter();
}


void CaptureConsole::onStart()
{
	if (view->isPlaying())
		view->pause(false);
	else
		view->play();
	view->stream->subscribe(this, std::bind(&CaptureConsole::onUpdate, this));

	mode->start();
	enable("capture_start", false);
	enable("capture_pause", true);
	enable("capture_delete", true);
	enable("ok", true);
	enable("capture_type", false);
}

void CaptureConsole::onDelete()
{
	view->stream->unsubscribe(this);
	if (view->isPlaying())
		view->stop();
	view->renderer->prepare(view->getPlaybackSelection(), false);
	mode->dump();
	enable("capture_start", true);
	enable("capture_pause", false);
	enable("capture_delete", false);
	enable("capture_type", true);
	enable("ok", false);
	updateTime();
}

void CaptureConsole::onPause()
{
	// TODO...
	view->stream->unsubscribe(this);
	view->pause(true);
	mode->pause();
	enable("capture_start", true);
	enable("capture_pause", false);
}


void CaptureConsole::onOk()
{
	mode->stop();
	if (mode->insert())
		bar()->_hide();
}

void CaptureConsole::onCancel()
{
	mode->stop();
	bar()->_hide();
}

void CaptureConsole::onClose()
{
	bar()->_hide();
}

void CaptureConsole::updateTime()
{
	setString("capture_time", song->get_time_str_long(mode->getSampleCount()));
}

void CaptureConsole::onUpdate()
{
	if (view->sel.range.length > 0)
		if (mode->getSampleCount() >= view->getPlaybackSelection().length)
			onPause();
	//if (mode->input->cur_message() == MESSAGE_DELETE)
	//	return;
	// FIXME
	updateTime();
}

bool CaptureConsole::isCapturing()
{
	return mode->isCapturing();
}
