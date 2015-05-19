/*
 * Tsunami.cpp
 *
 *  Created on: 13.08.2014
 *      Author: michi
 */

#include "Tsunami.h"
#include "TsunamiWindow.h"
#include "Storage/Storage.h"
#include "Stuff/Log.h"
#include "Stuff/Clipboard.h"
#include "Audio/AudioOutput.h"
#include "Audio/AudioInput.h"
#include "View/Helper/Progress.h"
#include "Plugins/PluginManager.h"


#include <pulse/pulseaudio.h>
#include <math.h>


string AppName = "Tsunami";
string AppVersion = "0.6.12.0";

Tsunami *tsunami = NULL;

Tsunami::Tsunami() :
	HuiApplication("tsunami", "Deutsch", HUI_FLAG_LOAD_RESOURCE)
{
	HuiSetProperty("name", AppName);
	HuiSetProperty("version", AppVersion);
	HuiSetProperty("comment", _("Editor f&ur Audio Dateien"));
	HuiSetProperty("website", "http://michi.is-a-geek.org/software");
	HuiSetProperty("copyright", "© 2007-2015 by Michael Ankele");
	HuiSetProperty("author", "Michael Ankele <michi@lupina.de>");
}

Tsunami::~Tsunami()
{
	delete(storage);
	delete(input);
	delete(output);
	delete(audio);
	delete(plugin_manager);
}

void _pa_sink_info_cb(pa_context *c, const pa_sink_info *i, int eol, void *userdata)
{
	if (eol > 0)
		return;
	msg_write(string("sink info: ") + i->name);
}


// This callback gets called when our context changes state.  We really only
// care about when it's ready or if it has failed
void pa_state_cb(pa_context *c, void *userdata)
{
	msg_write("  state");
	int *pa_ready = (int*)userdata;
	pa_context_state_t state = pa_context_get_state(c);
	switch(state){
		case PA_CONTEXT_UNCONNECTED:
		case PA_CONTEXT_CONNECTING:
		case PA_CONTEXT_AUTHORIZING:
		case PA_CONTEXT_SETTING_NAME:
		default:
			break;
		case PA_CONTEXT_FAILED:
		case PA_CONTEXT_TERMINATED:
			msg_write("  pa fail");
			*pa_ready = 2;
			break;
		case PA_CONTEXT_READY:
			msg_write("  pa ready");
			*pa_ready = 1;
			break;
	}
}

void _pa_stream_request_cb(pa_stream *p, size_t nbytes, void *userdata)
{
	msg_write("read " + i2s(nbytes));
	const void *data;
	int r = pa_stream_peek(p, &data, &nbytes);
	msg_write(r);

	/*if (data){
		r = pa_stream_write((pa_stream*)userdata, data, nbytes, NULL, 0, (pa_seek_mode_t)PA_SEEK_RELATIVE);
		msg_write(r);
	}*/
}

void _pa_stream_success_cb(pa_stream *s, int success, void *userdata)
{
	msg_write("--success");
}

bool Tsunami::onStartup(const Array<string> &arg)
{
	tsunami = this;
	win = NULL;
	_win = NULL;
	_view = NULL;

	progress = new Progress;
	log = new Log;

	clipboard = new Clipboard;

	output = new AudioOutput;
	input = new AudioInput;

	audio = new AudioFile;
	audio->newWithOneTrack(Track::TYPE_AUDIO, DEFAULT_SAMPLE_RATE);

	storage = new Storage;


	pa_mainloop* m = pa_mainloop_new();
	pa_mainloop_api *mainloop_api = pa_mainloop_get_api(m);
	pa_context *context = pa_context_new(mainloop_api, "tsunami");
	if (!context){
		msg_error("pa_context_new() failed.\n");
	}

	if (pa_context_connect(context, NULL, (pa_context_flags_t)0, NULL) < 0) {
		msg_error(string("pa_context_connect() failed: ") + pa_strerror(pa_context_errno(context)));
	}

	int pa_ready = 0;
    pa_context_set_state_callback(context, pa_state_cb, &pa_ready);

	pa_operation *pa_op;

    while (pa_ready == 0)
        pa_mainloop_iterate(m, 1, NULL);


	pa_op = pa_context_get_sink_info_list(context, _pa_sink_info_cb, NULL);
	while (true){
        pa_mainloop_iterate(m, 1, NULL);
		if (pa_operation_get_state(pa_op) == PA_OPERATION_DONE) {
			pa_operation_unref(pa_op);
			break;
		}
	}
	pa_sample_spec ss;
	ss.rate = DEFAULT_SAMPLE_RATE;
	ss.channels = 2;
	ss.format = PA_SAMPLE_FLOAT32LE;//PA_SAMPLE_S16LE;
	pa_stream *stream_in = pa_stream_new(context, "stream-in", &ss, NULL);
	pa_stream *stream_out = pa_stream_new(context, "stream-out", &ss, NULL);
	msg_write(p2s(stream_in));
	msg_write(p2s(stream_out));


	pa_stream_set_read_callback(stream_in, &_pa_stream_request_cb, stream_out);

	pa_buffer_attr attr_out;
	attr_out.fragsize = 512;
	attr_out.maxlength = -1;
	attr_out.minreq = -1;
	attr_out.tlength = -1;
	attr_out.prebuf = -1;
	int r = pa_stream_connect_playback(stream_out, NULL, &attr_out, (pa_stream_flags)0, NULL, NULL);
	msg_write(r);

	pa_buffer_attr attr_in;
	attr_in.fragsize = 512;
	attr_in.maxlength = -1;
	attr_in.minreq = -1;
	attr_in.tlength = -1;
	attr_in.prebuf = -1;
	//r = pa_stream_connect_record(stream_in, NULL, &attr_in, (pa_stream_flags_t)0);
	//msg_write(r);


	/*pa_ready = 0;
    while (pa_ready == 0)
        pa_mainloop_iterate(m, 1, NULL);
    msg_write("trigger");
	pa_op = pa_stream_trigger(stream_in, NULL, NULL);
	while (true){
        pa_mainloop_iterate(m, 1, NULL);
		if (pa_operation_get_state(pa_op) == PA_OPERATION_DONE) {
		    msg_write(" ok");
			pa_operation_unref(pa_op);
			break;
		}
	}*/


	while (true){
		int sstate = pa_stream_get_state(stream_out);
		msg_write("sstate: " + i2s(sstate));
		if (sstate == PA_STREAM_READY)
			break;
		pa_mainloop_iterate(m, 1, NULL);
	}
	msg_write("ok");

	/*msg_write("wait");
	pa_ready = 0;
    while (pa_ready == 0)
        pa_mainloop_iterate(m, 1, NULL);*/

	/*pa_mainloop_iterate(m, 1, NULL);
	pa_mainloop_iterate(m, 1, NULL);*/
    msg_write("trigger");
	pa_op = pa_stream_trigger(stream_out, &_pa_stream_success_cb, NULL);
	msg_write(p2s(pa_op));
	while (true){
        pa_mainloop_iterate(m, 1, NULL);
		if (pa_operation_get_state(pa_op) == PA_OPERATION_DONE) {
		    msg_write(" ok");
			pa_operation_unref(pa_op);
			break;
		}
	}

	float offset = 0;
	float *data = new float[1024];
	float vol = 0;
	while (true){

		for (int i=0; i<512; i++){
			data[i*2  ] = 0.2f * sin(offset) * vol;
			data[i*2+1] = 0.2f * sin(offset) * vol;
			offset += 0.05f;
			if (offset > 2 * 3.14159265f)
				offset -= 2 * 3.14159265f;
			if (vol < 1)
				vol += 0.00001f;
		}

		int r = pa_stream_write(stream_out, data, 4096, NULL, 0, (pa_seek_mode_t)PA_SEEK_RELATIVE);
		msg_write("write " + i2s(r));
        pa_mainloop_iterate(m, 1, NULL);
	}


	// create (link) PluginManager after all other components are ready
	plugin_manager = new PluginManager;

	if (!HandleArguments(arg))
		return false;

	CreateWindow();
	_arg = arg;
	HuiRunLaterM(0.01f, this, &Tsunami::_HandleArguments);
	return true;
}

void Tsunami::_HandleArguments()
{
	if (_arg.num >= 2)
		storage->load(audio, _arg[1]);
	audio->notify(audio->MESSAGE_NEW);
}

bool Tsunami::HandleArguments(const Array<string> &arg)
{
	if (arg.num < 2)
		return true;
	if (arg[1] == "--info"){
		if (arg.num < 3){
			log->error(_("Aufruf: tsunami --info <Datei>"));
		}else if (storage->load(audio, arg[2])){
			msg_write(format("sample-rate: %d", audio->sample_rate));
			msg_write(format("samples: %d", audio->getRange().num));
			msg_write("length: " + audio->get_time_str(audio->getRange().num));
			msg_write(format("tracks: %d", audio->tracks.num));
			foreach(Tag &t, audio->tags)
				msg_write("tag: " + t.key + " = " + t.value);
		}
		return false;
	}else if (arg[1] == "--export"){
		if (arg.num < 4){
			log->error(_("Aufruf: tsunami --export <Datei> <Exportdatei>"));
		}else if (storage->load(audio, arg[2])){
			storage->_export(audio, audio->getRange(), arg[3]);
		}
		return false;
	}
	return true;
}

void Tsunami::CreateWindow()
{
	log->info(AppName + " " + AppVersion);
	log->info(_("  ...keine Sorge, das wird schon!"));

	win = new TsunamiWindow;
	_win = dynamic_cast<HuiWindow*>(win);
	_view = win->view;
	plugin_manager->AddPluginsToMenu(win);

	win->show();
	//HuiRunLaterM(0.01f, win, &TsunamiWindow::OnViewOptimal);
}

void Tsunami::LoadKeyCodes()
{
}

bool Tsunami::AllowTermination()
{
	if (win)
		return win->allowTermination();
	return true;
}

HuiExecute(Tsunami);
