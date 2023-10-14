/*
 * PluginManager.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "PluginManager.h"
#include "Plugin.h"
#include "ExtendedAudioBuffer.h"
#include "PresetManager.h"
#include "TsunamiPlugin.h"
#include "../Tsunami.h"
#include "../Session.h"
#include "../Playback.h"
#include "../view/helper/Slider.h"
#include "../view/helper/Drawing.h"
#include "../data/base.h"
#include "../data/Song.h"
#include "../data/SongSelection.h"
#include "../data/Track.h"
#include "../data/midi/Instrument.h"
#include "../data/TrackLayer.h"
#include "../data/TrackMarker.h"
#include "../data/Sample.h"
#include "../data/SampleRef.h"
#include "../data/audio/AudioBuffer.h"
#include "../data/rhythm/Bar.h"
#include "../processing/audio/BufferInterpolator.h"
#include "../module/ModuleFactory.h"
#include "../module/SignalChain.h"
#include "../module/ModuleConfiguration.h"
#include "../module/synthesizer/Synthesizer.h"
#include "../module/synthesizer/DummySynthesizer.h"
#include "../module/port/Port.h"
#include "../module/audio/AudioSource.h"
#include "../module/audio/SongRenderer.h"
#include "../module/audio/AudioVisualizer.h"
#include "../module/audio/PitchDetector.h"
#include "../module/midi/MidiSource.h"
#include "../module/audio/AudioEffect.h"
#include "../module/beats/BeatSource.h"
#include "../module/beats/BeatMidifier.h"
#include "../module/midi/MidiEffect.h"
#include "../view/module/ConfigPanel.h"
#include "../view/helper/Progress.h"
#include "../storage/Storage.h"
#include "../device/DeviceManager.h"
#include "../device/stream/AudioInput.h"
#include "../device/stream/AudioOutput.h"
#include "../device/stream/MidiInput.h"
#include "../module/audio/AudioAccumulator.h"
#include "../module/midi/MidiAccumulator.h"
#include "../stuff/Clipboard.h"
#include "../view/audioview/AudioView.h"
#include "../view/dialog/ModuleSelectorDialog.h"
#include "../view/dialog/SampleSelectionDialog.h"
#include "../view/audioview/graph/AudioViewLayer.h"
#include "../view/sidebar/SampleManagerConsole.h"
#include "../view/mode/ViewModeCapture.h"
#include "../view/helper/graph/Node.h"
#include "../view/painter/GridPainter.h"
#include "../view/painter/MidiPainter.h"
#include "../view/painter/MultiLinePainter.h"
#include "../view/TsunamiWindow.h"
#include "../lib/base/callable.h"
#include "../lib/os/filesystem.h"
#include "../lib/kaba/dynamic/exception.h"


namespace hui {
	void get_style_colors(Panel *p, const string &id, base::map<string,color> &colors);
}

#define _offsetof(CLASS, ELEMENT) (int)( (char*)&((CLASS*)1)->ELEMENT - (char*)((CLASS*)1) )

PluginManager::PluginManager() {
	presets = new PresetManager;

	find_plugins();

	package = kaba::default_context->create_empty_module("<tsunami-internal>");
	package->used_by_default = false;
	kaba::default_context->packages.add(package);
	kaba::default_context->public_modules.add(package.get());

	auto *type_dev = package->tree->create_new_class("Device", kaba::Class::Type::REGULAR, 0, 0, nullptr, {}, package->tree->base_class, -1);
	package->tree->get_pointer(type_dev);
	//package->tree->make_class("Device*", kaba::Class::Type::POINTER, sizeof(void*), 0, nullptr, {type_dev}, package->tree->base_class, -1);
}

PluginManager::~PluginManager() {
	//plugins.clear();
	//kaba::clean_up();
}


xfer<Module> _CreateBeatMidifier(Session *s) {
	return new BeatMidifier();
}

#pragma GCC push_options
#pragma GCC optimize("no-omit-frame-pointer")
#pragma GCC optimize("no-inline")
#pragma GCC optimize("0")

class XSignalChain : public SignalChain {
public:
	shared<Module> x_add_existing(shared<Module> m) {
		return _add(m);
	}
	shared<Module> x_add_basic(ModuleCategory type, const string& sub_type) {
		return add(type, sub_type);
	}
	shared<Module> x_add(const kaba::Class *type) {
		return _add(ModuleFactory::create_by_class(session, type));
	}
	void x_connect(Module *source, int source_port, Module *target, int target_port) {
		KABA_EXCEPTION_WRAPPER(connect(source,  source_port,  target, target_port));
	}
};

#pragma GCC pop_options

template<class T>
class ObservableKabaWrapper : public T {
public:
	obs::sink& create_sink_kaba(Callable<void()> *f) {
		return this->create_sink([f] {
			(*f)();
		});
	}
#if 0
	void _cdecl subscribe_kaba(hui::EventHandler *handler, Callable<void(VirtualBase*)> *f, const string &message) {
		/*T::subscribe(handler, [f, handler] {
			(*f)(handler);
		}, message);*/
		msg_error("X.subscribe() TODO in plugins");
	}
#endif
};


void wrapper_choose_module(hui::Panel *parent, Session *session, ModuleCategory type, Callable<void(const base::optional<string>&)> &cb, const string &old_name) {
	ModuleSelectorDialog::choose(parent, session, type, old_name).then([&cb] (const base::optional<string> &name) {
		cb(name);
	}).on_fail([&cb] {
		cb(base::None);
	});
}

void wrapper_select_sample(Session *session, hui::Panel *parent, Sample *old, Callable<void(Sample*)> &cb) {
	SampleSelectionDialog::select(session, parent, old).then([&cb] (Sample *s) {
		cb(s);
	}).on_fail([&cb, old] {
		cb(old);
	});
}

template<class T>
void generic__init__(T *me) {
	new(me) T;
}


void PluginManager::link_app_data() {
	kaba::config.directory = Path::EMPTY;

	auto ext = kaba::default_context->external.get();

	// api definition
	ext->link("device_manager", &tsunami->device_manager);
	ext->link("colors", &theme);
	ext->link("clipboard", &tsunami->clipboard);
	//ext->link("view_input", &export_view_input);
	ext->link("db2amp", (void*)&db2amplitude);
	ext->link("amp2db", (void*)&amplitude2db);
	/*ext->link("fft_c2c", (void*)&FastFourierTransform::fft_c2c);
	ext->link("fft_r2c", (void*)&FastFourierTransform::fft_r2c);
	ext->link("fft_c2r_inv", (void*)&FastFourierTransform::fft_c2r_inv);*/
	ext->link("CreateModuleBasic", (void*)&ModuleFactory::create);
	ext->link("CreateModuleX", (void*)&ModuleFactory::create_by_class);
	ext->link("CreateSynthesizer", (void*)&CreateSynthesizer);
	ext->link("CreateAudioEffect", (void*)&CreateAudioEffect);
	ext->link("CreateAudioSource", (void*)&CreateAudioSource);
	ext->link("CreateMidiEffect", (void*)&CreateMidiEffect);
	ext->link("CreateMidiSource", (void*)&CreateMidiSource);
	ext->link("CreateBeatSource", (void*)&CreateBeatSource);
	ext->link("CreateBeatMidifier", (void*)&_CreateBeatMidifier);
	ext->link("SelectSample", (void*)&wrapper_select_sample);
	ext->link("ChooseModule", (void*)&wrapper_choose_module);
	ext->link("draw_boxed_str", (void*)&draw_boxed_str);
	ext->link("draw_arrow", (void*)&draw_arrow);
	ext->link("interpolate_buffer", (void*)&BufferInterpolator::interpolate);
	ext->link("get_style_colors", (void*)&hui::get_style_colors);

	ext->link_class_func("obs.source.__rshift__", &obs::source::subscribe);

	ext->declare_class_size("Clipboard", sizeof(Clipboard));
	ext->declare_class_element("Clipboard.temp", &Clipboard::temp);
	ext->link_class_func("Clipboard.has_data", &Clipboard::has_data);
	ext->link_class_func("Clipboard.prepare_layer_map", &Clipboard::prepare_layer_map);

	ext->declare_class_size("Range", sizeof(Range));
	ext->declare_class_element("Range.offset", &Range::offset);
	ext->declare_class_element("Range.length", &Range::length);
	ext->link_class_func("Range.__and__", &Range::intersect);
	ext->link_class_func("Range.__str__", &Range::str);
	ext->link_class_func("Range.start", &Range::start);
	ext->link_class_func("Range.end", &Range::end);
	ext->link_class_func("Range.covers", &Range::covers);
	ext->link_class_func("Range.overlaps", &Range::overlaps);
	ext->link_class_func("Range.is_inside", &Range::is_inside);

	ext->declare_class_size("Bar", sizeof(Bar));
	ext->declare_class_element("Bar.beats", &Bar::beats);
	ext->declare_class_element("Bar.divisor", &Bar::divisor);
	ext->declare_class_element("Bar.total_sub_beats", &Bar::total_sub_beats);
	ext->declare_class_element("Bar.length", &Bar::length);
	ext->declare_class_element("Bar.index", &Bar::index);
	ext->declare_class_element("Bar.index_text", &Bar::index_text);
	ext->declare_class_element("Bar.offset", &Bar::offset);
	ext->declare_class_element("Bar." + kaba::Identifier::SHARED_COUNT, &Bar::_pointer_ref_counter);
	ext->link_class_func("Bar." + kaba::Identifier::Func::INIT, &Bar::__init__);
	ext->link_class_func("Bar.range", &Bar::range);
	ext->link_class_func("Bar.bpm", &Bar::bpm);

	ext->declare_class_size("BarCollection", sizeof(BarCollection));
	ext->link_class_func("BarCollection.get_bars", &BarCollection::get_bars);
	ext->link_class_func("BarCollection.get_beats", &BarCollection::get_beats);
	ext->link_class_func("BarCollection.get_next_beat", &BarCollection::get_next_beat);
	ext->link_class_func("BarCollection.get_prev_beat", &BarCollection::get_prev_beat);


	ext->declare_class_size("Instrument", sizeof(Instrument));
	ext->declare_class_element("Instrument.type", &Instrument::type);
	ext->declare_class_element("Instrument.string_pitch", &Instrument::string_pitch);
	ext->link_class_func("Instrument.name", &Instrument::name);
	ext->link_class_func("Instrument.enumerate", &Instrument::enumerate);

	ext->declare_class_size("Session", sizeof(Session));
	ext->declare_class_element("Session.id", &Session::id);
	ext->declare_class_element("Session.storage", &Session::storage);
	ext->declare_class_element("Session.win", &Session::_kaba_win);
	ext->declare_class_element("Session.view", &Session::view);
	ext->declare_class_element("Session.song", &Session::song);
	ext->declare_class_element("Session.playback", &Session::playback);
	ext->declare_class_element("Session." + kaba::Identifier::SHARED_COUNT, &Session::_pointer_ref_counter);
	ext->link_class_func("Session.sample_rate", &Session::sample_rate);
	ext->link_class_func("Session.i", &Session::i);
	ext->link_class_func("Session.w", &Session::w);
	ext->link_class_func("Session.e", &Session::e);
	ext->link_class_func("Session.add_signal_chain", &Session::add_signal_chain);
	ext->link_class_func("Session.create_signal_chain", &Session::create_signal_chain);
	ext->link_class_func("Session.load_signal_chain", &Session::load_signal_chain);
	ext->link_class_func("Session.remove_signal_chain", &Session::remove_signal_chain);
	ext->link_class_func("Session.create_child", &Session::create_child);


	{
		Module module(ModuleCategory::AUDIO_EFFECT, "");
		ext->declare_class_size("Module", sizeof(Module));
		ext->declare_class_element("Module.name", &Module::module_class);
		ext->declare_class_element("Module.session", &Module::session);
		ext->declare_class_element("Module.port_out", &Module::port_out);
		ext->declare_class_element("Module.out_changed", &Module::out_changed);
		ext->declare_class_element("Module.out_state_changed", &Module::out_state_changed);
		ext->declare_class_element("Module.out_read_end_of_stream", &Module::out_read_end_of_stream);
		ext->declare_class_element("Module.out_play_end_of_stream", &Module::out_play_end_of_stream);
		ext->declare_class_element("Module.out_tick", &Module::out_tick);
		ext->declare_class_element("Module." + kaba::Identifier::SHARED_COUNT, &Module::_pointer_ref_counter);
		ext->link_class_func("Module.__init__", &Module::__init__);
		ext->link_virtual("Module.__delete__", &Module::__delete__, &module);
		ext->link_virtual("Module.create_panel", &Module::create_panel, &module);
		ext->link_class_func("Module.reset_config", &Module::reset_config);
		ext->link_virtual("Module.reset_state", &Module::reset_state, &module);
		ext->link_class_func("Module.changed", &Module::changed);
		ext->link_virtual("Module.get_config", &Module::get_config, &module);
		ext->link_class_func("Module.config_to_string", &Module::config_to_string);
		ext->link_class_func("Module.config_from_string", &Module::config_from_string);
		ext->link_class_func("Module.config_to_any", &Module::config_to_any);
		ext->link_class_func("Module.config_from_any", &Module::config_from_any);
		ext->link_virtual("Module.on_config", &Module::on_config, &module);
		ext->link_virtual("Module.command", &Module::command, &module);
		ext->link_class_func("Module.create_sink", &ObservableKabaWrapper<Module>::create_sink_kaba);
		ext->link_class_func("Module.unsubscribe", &Module::unsubscribe);
		ext->link_class_func("Module.copy", &Module::copy);
		ext->link_class_func("Module.plug_in", &Module::_plug_in);
		ext->link_class_func("Module.unplug_in", &Module::_unplug_in);
	}


	{
		ModuleConfiguration plugin_data;
		ext->declare_class_size("Module.Config", sizeof(ModuleConfiguration));
		ext->link_class_func("Module.Config.__init__", &ModuleConfiguration::__init__);
		ext->link_virtual("Module.Config.__delete__", &ModuleConfiguration::__delete__, &plugin_data);
		ext->link_virtual("Module.Config.reset", &ModuleConfiguration::reset, &plugin_data);
		ext->link_class_func("Module.Config.from_string", &ModuleConfiguration::from_string);
		ext->link_class_func("Module.Config.to_string", &ModuleConfiguration::to_string);
		ext->link_virtual("Module.Config.auto_conf", &ModuleConfiguration::auto_conf, &plugin_data);
	}


	{
		ConfigPanel::_hidden_parent_check_ = false;
		ConfigPanel config_panel(nullptr);
		ext->declare_class_size("ConfigPanel", sizeof(ConfigPanel));
		ext->link_class_func("ConfigPanel.__init__", &ConfigPanel::__init__);
		ext->link_virtual("ConfigPanel.__delete__", &ConfigPanel::__delete__, &config_panel);
		ext->link_virtual("ConfigPanel.update", &ConfigPanel::update, &config_panel);
		ext->link_virtual("ConfigPanel.set_large", &ConfigPanel::set_large, &config_panel);
		ext->link_class_func("ConfigPanel.changed", &ConfigPanel::changed);
		ext->link_class_func("ConfigPanel.create_sink", &ObservableKabaWrapper<ConfigPanel>::create_sink_kaba);
		ext->declare_class_element("ConfigPanel.c", &ConfigPanel::c);
		ConfigPanel::_hidden_parent_check_ = true;
	}

	{
		AudioSource asource;
		ext->declare_class_size("AudioSource", sizeof(AudioSource));
		ext->link_class_func("AudioSource.__init__", &AudioSource::__init__);
		ext->link_virtual("AudioSource.__delete__", &AudioSource::__delete__, &asource);
		ext->link_virtual("AudioSource.read", &AudioSource::read, &asource);
	}

	{
		AudioEffect aeffect;
		ext->declare_class_size("AudioEffect", sizeof(AudioEffect));
		ext->declare_class_element("AudioEffect.sample_rate", &AudioEffect::sample_rate);
		ext->declare_class_element("AudioEffect.apply_to_whole_buffer", &AudioEffect::apply_to_whole_buffer);
		ext->declare_class_element("AudioEffect.wetness", &AudioEffect::wetness);
		ext->declare_class_element("AudioEffect.source", &AudioEffect::source);
		ext->link_class_func("AudioEffect.__init__", &AudioEffect::__init__);
		ext->link_virtual("AudioEffect.__delete__", &AudioEffect::__delete__, &aeffect);
		ext->link_virtual("AudioEffect.process", &AudioEffect::process, &aeffect);
		ext->link_virtual("AudioEffect.read", &AudioEffect::read, &aeffect);
	}

	{
		MidiEffect meffect;
		ext->declare_class_size("MidiEffect", sizeof(MidiEffect));
		ext->link_class_func("MidiEffect.__init__", &MidiEffect::__init__);
		ext->link_virtual("MidiEffect.__delete__", &MidiEffect::__delete__, &meffect);
		ext->link_virtual("MidiEffect.process", &MidiEffect::process, &meffect);
	}

	{
		AudioVisualizer avis;
		ext->declare_class_size("AudioVisualizer", sizeof(AudioVisualizer));
		ext->declare_class_element("AudioVisualizer.chunk_size", &AudioVisualizer::chunk_size);
		ext->declare_class_element("AudioVisualizer.ring_buffer", &AudioVisualizer::buffer);
		ext->declare_class_element("AudioVisualizer.next_writing", &AudioVisualizer::next_writing);
		ext->declare_class_element("AudioVisualizer.current_reading", &AudioVisualizer::current_reading);
		ext->link_class_func("AudioVisualizer.__init__", &AudioVisualizer::__init__);
		ext->link_virtual("AudioVisualizer.__delete__", &AudioVisualizer::__delete__, &avis);
		ext->link_virtual("AudioVisualizer.process", &AudioVisualizer::process, &avis);
		ext->link_class_func("AudioVisualizer.set_chunk_size", &AudioVisualizer::set_chunk_size);
		ext->link_class_func("AudioVisualizer.lock", &AudioVisualizer::lock);
		ext->link_class_func("AudioVisualizer.unlock", &AudioVisualizer::unlock);
		ext->link_class_func("AudioVisualizer.flip", &AudioVisualizer::flip);
	}

	{
		DummyPitchDetector pd;
		ext->declare_class_size("PitchDetector", sizeof(PitchDetector));
		ext->declare_class_element("PitchDetector.frequency", &PitchDetector::frequency);
		ext->declare_class_element("PitchDetector.pitch", &PitchDetector::pitch);
		ext->declare_class_element("PitchDetector.volume", &PitchDetector::volume);
		ext->declare_class_element("PitchDetector.loud_enough", &PitchDetector::loud_enough);
		ext->link_class_func("PitchDetector.__init__", &DummyPitchDetector::__init__);
		ext->link_virtual("PitchDetector.read", &PitchDetector::read, &pd);
		ext->link_virtual("PitchDetector.process", &DummyPitchDetector::process, &pd);
	}

	ext->declare_class_size("AudioBuffer", sizeof(AudioBuffer));
	ext->declare_class_element("AudioBuffer.offset", &AudioBuffer::offset);
	ext->declare_class_element("AudioBuffer.length", &AudioBuffer::length);
	ext->declare_class_element("AudioBuffer.channels", &AudioBuffer::channels);
	ext->declare_class_element("AudioBuffer.c", &AudioBuffer::c);
	//kaba::_declare_class_element("AudioBuffer.l", _offsetof(AudioBuffer, c[0]));
	//kaba::_declare_class_element("AudioBuffer.r", _offsetof(AudioBuffer, c[1]));
	ext->declare_class_element("AudioBuffer.version", &AudioBuffer::version);
	ext->link_class_func("AudioBuffer.__init__", &AudioBuffer::__init__);
	ext->link_class_func("AudioBuffer.__delete__", &AudioBuffer::__delete__);
	ext->link_class_func("AudioBuffer.clear", &AudioBuffer::clear);
	ext->link_class_func("AudioBuffer.__assign__", &AudioBuffer::__assign__);
	ext->link_class_func("AudioBuffer.range", &AudioBuffer::range);
	ext->link_class_func("AudioBuffer.resize", &AudioBuffer::resize);
	ext->link_class_func("AudioBuffer.set_channels", &AudioBuffer::set_channels);
	ext->link_class_func("AudioBuffer.add", &AudioBuffer::add);
	ext->link_class_func("AudioBuffer.set", &AudioBuffer::set);
	ext->link_class_func("AudioBuffer.set_as_ref", &AudioBuffer::set_as_ref);
	ext->link_class_func("AudioBuffer." + kaba::Identifier::Func::SUBARRAY, &AudioBuffer::ref);
	ext->link_class_func("AudioBuffer.get_spectrum", &ExtendedAudioBuffer::get_spectrum);


	ext->declare_class_size("RingBuffer", sizeof(RingBuffer));
	ext->link_class_func("RingBuffer.__init__", &RingBuffer::__init__);
	ext->link_class_func("RingBuffer.__delete__", &RingBuffer::__delete__);
	ext->link_class_func("RingBuffer.available", &RingBuffer::available);
	ext->link_class_func("RingBuffer.read", &RingBuffer::read);
	ext->link_class_func("RingBuffer.write", &RingBuffer::write);
	ext->link_class_func("RingBuffer.read_ref", &RingBuffer::read_ref);
	ext->link_class_func("RingBuffer.read_ref_done", &RingBuffer::read_ref_done);
	ext->link_class_func("RingBuffer.peek", &RingBuffer::peek);
	ext->link_class_func("RingBuffer.write_ref", &RingBuffer::write_ref);
	ext->link_class_func("RingBuffer.write_ref_done", &RingBuffer::write_ref_done);
	ext->link_class_func("RingBuffer.clear", &RingBuffer::clear);

	ext->declare_class_size("Sample", sizeof(Sample));
	ext->declare_class_element("Sample.name", &Sample::name);
	ext->declare_class_element("Sample.type", &Sample::type);
	ext->declare_class_element("Sample.buf", &Sample::buf);
	ext->declare_class_element("Sample.midi", &Sample::midi);
	ext->declare_class_element("Sample.volume", &Sample::volume);
	ext->declare_class_element("Sample.uid", &Sample::uid);
	ext->declare_class_element("Sample.ref_count", &Sample::ref_count);
	ext->declare_class_element("Sample.tags", &Sample::tags);
	ext->declare_class_element("Sample." + kaba::Identifier::SHARED_COUNT, &Sample::_pointer_ref_counter);
	ext->link_class_func("Sample.__init__", &Sample::__init__);
	ext->link_class_func("Sample.create_ref", &Sample::create_ref);
	ext->link_class_func("Sample.get_value", &Sample::get_value);
	ext->link_class_func("Sample.set_value", &Sample::set_value);

	{
		shared<Sample> sample = new Sample(SignalType::AUDIO);
		//sample.owner = tsunami->song;
		SampleRef sampleref(sample.get());
		ext->declare_class_size("SampleRef", sizeof(SampleRef));
		ext->link_class_func("SampleRef.buf", &SampleRef::buf);
		ext->link_class_func("SampleRef.midi", &SampleRef::midi);
		ext->declare_class_element("SampleRef.origin", &SampleRef::origin);
		ext->declare_class_element("SampleRef." + kaba::Identifier::SHARED_COUNT, &SampleRef::_pointer_ref_counter);
		ext->link_class_func("SampleRef.__init__", &SampleRef::__init__);
		ext->link_virtual("SampleRef.__delete__", &SampleRef::__delete__, &sampleref);
	}



	{
		Port port(SignalType::AUDIO, "");
		ext->declare_class_size("Module.Port", sizeof(Port));
		ext->link_class_func("Module.Port.__init__", &Port::__init__);
		ext->link_virtual("Module.Port.__delete__", &Port::__delete__, &port);
		ext->link_virtual("Module.Port.read_audio", &Port::read_audio, &port);
		ext->link_virtual("Module.Port.read_midi", &Port::read_midi, &port);
		ext->link_virtual("Module.Port.read_beats", &Port::read_beats, &port);
	}

	{
		MidiSource msource;
		ext->declare_class_size("MidiSource", sizeof(MidiSource));
		ext->declare_class_element("MidiSource.bh_midi", &MidiSource::bh_midi);
		ext->link_class_func("MidiSource.__init__", &MidiSource::__init__);
		ext->link_virtual("MidiSource.__delete__", &MidiSource::__delete__, &msource);
		ext->link_virtual("MidiSource.read", &MidiSource::read, &msource);
		ext->link_virtual("MidiSource.reset", &MidiSource::reset, &msource);
		ext->link_virtual("MidiSource.on_produce", &MidiSource::on_produce, &msource);
		ext->link_class_func("MidiSource.note", &MidiSource::note);
		ext->link_class_func("MidiSource.skip", &MidiSource::skip);
		ext->link_class_func("MidiSource.note_x", &MidiSource::note_x);
		ext->link_class_func("MidiSource.skip_x", &MidiSource::skip_x);
	}


	{
		BeatMidifier bmidifier;
		ext->declare_class_size("BeatMidifier", sizeof(BeatMidifier));
		ext->link_class_func("BeatMidifier.__init__", &BeatMidifier::__init__);
		//ext->link_virtual("BeatMidifier.__delete__", &MidiSource::__delete__, &bmidifier);
		ext->link_virtual("BeatMidifier.read", &BeatMidifier::read, &bmidifier);
		ext->link_virtual("BeatMidifier.reset", &BeatMidifier::reset, &bmidifier);
		ext->declare_class_element("BeatMidifier.volume", &BeatMidifier::volume);
	}

	//{
	Synthesizer synth;
	ext->declare_class_size("Synthesizer", sizeof(Synthesizer));
	ext->declare_class_element("Synthesizer.sample_rate", &Synthesizer::sample_rate);
	ext->declare_class_element("Synthesizer.events", &Synthesizer::events);
	ext->declare_class_element("Synthesizer.keep_notes", &Synthesizer::keep_notes);
	ext->declare_class_element("Synthesizer.active_pitch", &Synthesizer::active_pitch);
	ext->declare_class_element("Synthesizer.temperament", &Synthesizer::temperament);
	ext->_declare_class_element("Synthesizer.freq", _offsetof(Synthesizer, temperament.freq));
	ext->declare_class_element("Synthesizer.delta_phi", &Synthesizer::delta_phi);
	ext->declare_class_element("Synthesizer.auto_generate_stereo", &Synthesizer::auto_generate_stereo);
	ext->declare_class_element("Synthesizer.render_by_ref", &Synthesizer::render_by_ref);
	ext->link_class_func("Synthesizer.__init__", &Synthesizer::__init__);
	ext->link_virtual("Synthesizer.__delete__", &Synthesizer::__delete__, &synth);
	ext->link_virtual("Synthesizer.render", &Synthesizer::render, &synth);
	ext->link_virtual("Synthesizer.create_pitch_renderer", &Synthesizer::create_pitch_renderer, &synth);
	ext->link_virtual("Synthesizer.on_config", &Synthesizer::on_config, &synth);
	ext->link_virtual("Synthesizer.reset_state", &Synthesizer::reset_state, &synth);
	ext->link_class_func("Synthesizer.set_sample_rate", &Synthesizer::set_sample_rate);
	//}

	{
		PitchRenderer pren(&synth, 0);
		ext->declare_class_size("PitchRenderer", sizeof(PitchRenderer));
		ext->declare_class_element("PitchRenderer.synth", &PitchRenderer::synth);
		ext->declare_class_element("PitchRenderer.pitch", &PitchRenderer::pitch);
		ext->declare_class_element("PitchRenderer.delta_phi", &PitchRenderer::delta_phi);
		ext->link_class_func("PitchRenderer.__init__", &PitchRenderer::__init__);
		ext->link_virtual("PitchRenderer.__delete__", &PitchRenderer::__delete__, &pren);
		ext->link_virtual("PitchRenderer.render", &PitchRenderer::render, &pren);
		ext->link_virtual("PitchRenderer.on_start", &PitchRenderer::on_start, &pren);
		ext->link_virtual("PitchRenderer.on_end", &PitchRenderer::on_end, &pren);
		ext->link_virtual("PitchRenderer.on_config", &PitchRenderer::on_config, &pren);
	}


	{
		DummySynthesizer dsynth;
		ext->declare_class_size("DummySynthesizer", sizeof(DummySynthesizer));
		ext->link_class_func("DummySynthesizer.__init__", &DummySynthesizer::__init__);
		ext->link_virtual("DummySynthesizer.__delete__", &DummySynthesizer::__delete__, &dsynth);
		ext->link_virtual("DummySynthesizer.render", &DummySynthesizer::render, &dsynth);
		ext->link_virtual("DummySynthesizer.create_pitch_renderer", &DummySynthesizer::create_pitch_renderer, &dsynth);
		ext->link_virtual("DummySynthesizer.on_config", &DummySynthesizer::on_config, &dsynth);
	}

	ext->declare_class_size("EnvelopeADSR", sizeof(EnvelopeADSR));
	ext->link_class_func("EnvelopeADSR.__init__", &EnvelopeADSR::reset);
	ext->link_class_func("EnvelopeADSR.set", &EnvelopeADSR::set);
	ext->link_class_func("EnvelopeADSR.set2", &EnvelopeADSR::set2);
	ext->link_class_func("EnvelopeADSR.reset", &EnvelopeADSR::reset);
	ext->link_class_func("EnvelopeADSR.start", &EnvelopeADSR::start);
	ext->link_class_func("EnvelopeADSR.end", &EnvelopeADSR::end);
	ext->link_class_func("EnvelopeADSR.get", &EnvelopeADSR::get);
	ext->link_class_func("EnvelopeADSR.read", &EnvelopeADSR::read);
	ext->declare_class_element("EnvelopeADSR.just_killed", &EnvelopeADSR::just_killed);

	ext->declare_class_size("BarPattern", sizeof(BarPattern));
	ext->declare_class_element("BarPattern.beats", &BarPattern::beats);
	ext->declare_class_element("BarPattern.divisor", &BarPattern::divisor);
	ext->declare_class_element("BarPattern.length", &BarPattern::length);
	//ext->declare_class_element("BarPattern.type", &BarPattern::type);
	//ext->declare_class_element("BarPattern.count", &BarPattern::count);

	ext->declare_class_size("MidiNote", sizeof(MidiNote));
	ext->declare_class_element("MidiNote.range", &MidiNote::range);
	ext->declare_class_element("MidiNote.pitch", &MidiNote::pitch);
	ext->declare_class_element("MidiNote.volume", &MidiNote::volume);
	ext->declare_class_element("MidiNote.stringno", &MidiNote::stringno);
	ext->declare_class_element("MidiNote.clef_position", &MidiNote::clef_position);
	ext->declare_class_element("MidiNote.modifier", &MidiNote::modifier);
	ext->declare_class_element("MidiNote.flags", &MidiNote::flags);
	ext->declare_class_element("MidiNote." + kaba::Identifier::SHARED_COUNT, &MidiNote::_pointer_ref_counter);
	ext->link_class_func("MidiNote.copy", &MidiNote::copy);
	ext->link_class_func("MidiNote.is", &MidiNote::is);
	ext->link_class_func("MidiNote.set", &MidiNote::set);
	
	ext->declare_class_size("MidiEvent", sizeof(MidiEvent));
	ext->declare_class_element("MidiEvent.pos", &MidiEvent::pos);
	ext->declare_class_element("MidiEvent.pitch", &MidiEvent::pitch);
	ext->declare_class_element("MidiEvent.volume", &MidiEvent::volume);
	ext->declare_class_element("MidiEvent.stringno", &MidiEvent::stringno);
	ext->declare_class_element("MidiEvent.clef_position", &MidiEvent::clef_position);
	ext->declare_class_element("MidiEvent.flags", &MidiEvent::flags);

	ext->declare_class_size("MidiEventBuffer", sizeof(MidiEventBuffer));
	ext->declare_class_element("MidiEventBuffer.samples", &MidiEventBuffer::samples);
	ext->link_class_func("MidiEventBuffer.__init__", &MidiEventBuffer::__init__);
	ext->link_class_func("MidiEventBuffer.get_events", &MidiEventBuffer::get_events);
	ext->link_class_func("MidiEventBuffer.get_notes", &MidiEventBuffer::get_notes);
	ext->link_class_func("MidiEventBuffer.get_range", &MidiEventBuffer::range);
	ext->link_class_func("MidiEventBuffer.add_metronome_click", &MidiEventBuffer::add_metronome_click);

	ext->declare_class_size("MidiNoteBuffer", sizeof(MidiNoteBuffer));
	ext->declare_class_element("MidiNoteBuffer.samples", &MidiNoteBuffer::samples);
	ext->link_class_func("MidiNoteBuffer.__init__", &MidiNoteBuffer::__init__);
	ext->link_class_func("MidiNoteBuffer.__delete__", &MidiNoteBuffer::__delete__);
	ext->link_class_func("MidiNoteBuffer.get_events", &MidiNoteBuffer::get_events);
	ext->link_class_func("MidiNoteBuffer.get_notes", &MidiNoteBuffer::get_notes);
	ext->link_class_func("MidiNoteBuffer.get_range", &MidiNoteBuffer::range);

	{
		BeatSource bsource;
		ext->declare_class_size("BeatSource", sizeof(BeatSource));
		ext->link_class_func("BeatSource.__init__", &BeatSource::__init__);
		ext->link_virtual("BeatSource.__delete__", &BeatSource::__delete__, &bsource);
		ext->link_virtual("BeatSource.read", &BeatSource::read, &bsource);
		ext->link_virtual("BeatSource.reset", &BeatSource::reset, &bsource);
		ext->link_virtual("BeatSource.beats_per_bar", &BeatSource::beats_per_bar, &bsource);
		ext->link_virtual("BeatSource.cur_beat", &BeatSource::cur_beat, &bsource);
		ext->link_virtual("BeatSource.cur_bar", &BeatSource::cur_bar, &bsource);
		ext->link_virtual("BeatSource.beat_fraction", &BeatSource::beat_fraction, &bsource);
	}

	ext->declare_class_size("TrackMarker", sizeof(TrackMarker));
	ext->declare_class_element("TrackMarker.text", &TrackMarker::text);
	ext->declare_class_element("TrackMarker.range", &TrackMarker::range);
	ext->declare_class_element("TrackMarker.fx", &TrackMarker::fx);
	ext->declare_class_element("TrackMarker." + kaba::Identifier::SHARED_COUNT, &TrackMarker::_pointer_ref_counter);

	ext->declare_class_size("TrackLayer", sizeof(TrackLayer));
	ext->declare_class_element("TrackLayer.type", &TrackLayer::type);
	ext->declare_class_element("TrackLayer.buffers", &TrackLayer::buffers);
	ext->declare_class_element("TrackLayer.midi", &TrackLayer::midi);
	ext->declare_class_element("TrackLayer.samples", &TrackLayer::samples);
	ext->declare_class_element("TrackLayer.markers", &TrackLayer::markers);
	ext->declare_class_element("TrackLayer.track", &TrackLayer::track);
	ext->declare_class_element("TrackLayer." + kaba::Identifier::SHARED_COUNT, &TrackLayer::_pointer_ref_counter);
	ext->link_class_func("TrackLayer.get_buffers", &TrackLayer::get_buffers);
	ext->link_class_func("TrackLayer.read_buffers", &TrackLayer::read_buffers);
	ext->link_class_func("TrackLayer.edit_buffers", &TrackLayer::edit_buffers);
	ext->link_class_func("TrackLayer.edit_buffers_finish", &TrackLayer::edit_buffers_finish);
	ext->link_class_func("TrackLayer.insert_midi_data", &TrackLayer::insert_midi_data);
	ext->link_class_func("TrackLayer.add_midi_note", &TrackLayer::add_midi_note);
	//ext->link_class_func("TrackLayer.add_midi_notes", &TrackLayer::addMidiNotes);
	ext->link_class_func("TrackLayer.delete_midi_note", &TrackLayer::delete_midi_note);
	ext->link_class_func("TrackLayer.add_sample_ref", &TrackLayer::add_sample_ref);
	ext->link_class_func("TrackLayer.delete_sample_ref", &TrackLayer::delete_sample_ref);
	ext->link_class_func("TrackLayer.edit_sample_ref", &TrackLayer::edit_sample_ref);
	ext->link_class_func("TrackLayer.add_marker", &TrackLayer::add_marker);
	ext->link_class_func("TrackLayer.delete_marker", &TrackLayer::delete_marker);
	ext->link_class_func("TrackLayer.edit_marker", &TrackLayer::edit_marker);

	ext->declare_class_size("Track", sizeof(Track));
	ext->declare_class_element("Track.type", &Track::type);
	ext->declare_class_element("Track.name", &Track::name);
	ext->declare_class_element("Track.layers", &Track::layers);
	ext->declare_class_element("Track.volume", &Track::volume);
	ext->declare_class_element("Track.panning", &Track::panning);
	ext->declare_class_element("Track.muted", &Track::muted);
	ext->declare_class_element("Track.fx", &Track::fx);
	ext->declare_class_element("Track.synth", &Track::synth);
	ext->declare_class_element("Track.instrument", &Track::instrument);
	ext->declare_class_element("Track.root", &Track::song);
	ext->declare_class_element("Track." + kaba::Identifier::SHARED_COUNT, &Track::_pointer_ref_counter);
	ext->link_class_func("Track.nice_name", &Track::nice_name);
	ext->link_class_func("Track.set_name", &Track::set_name);
	ext->link_class_func("Track.set_muted", &Track::set_muted);
	ext->link_class_func("Track.set_volume", &Track::set_volume);
	ext->link_class_func("Track.set_panning", &Track::set_panning);
	ext->link_class_func("Track.add_effect", &Track::add_effect);
	ext->link_class_func("Track.delete_effect", &Track::delete_effect);
	ext->link_class_func("Track.edit_effect", &Track::edit_effect);
	ext->link_class_func("Track.enable_effect", &Track::enable_effect);
	ext->link_class_func("Track.set_effect_wetness", &Track::set_effect_wetness);
	ext->link_class_func("Track.set_synthesizer", &Track::set_synthesizer);

	{
		Song af(Session::GLOBAL, DEFAULT_SAMPLE_RATE);
		ext->declare_class_size("Song", sizeof(Song));
		ext->declare_class_element("Song.filename", &Song::filename);
		ext->declare_class_element("Song.tag", &Song::tags);
		ext->declare_class_element("Song.sample_rate", &Song::sample_rate);
		ext->declare_class_element("Song.tracks", &Song::tracks);
		ext->declare_class_element("Song.samples", &Song::samples);
	//	ext->declare_class_element("Song.layers", &Song::layers);
		ext->declare_class_element("Song.bars", &Song::bars);
		ext->declare_class_element("Song.secret_data", &Song::secret_data);
		ext->declare_class_element("Song." + kaba::Identifier::SHARED_COUNT, &Song::_pointer_ref_counter);
		ext->link_class_func("Song.__init__", &Song::__init__);
		ext->link_virtual("Song.__delete__", &Song::__delete__, &af);
		ext->link_class_func("Song.add_track", &Song::add_track);
		ext->link_class_func("Song.delete_track", &Song::delete_track);
		ext->link_class_func("Song.range", &Song::range);
		ext->link_class_func("Song.layers", &Song::layers);
		ext->link_class_func("Song.add_bar", &Song::add_bar);
		ext->link_class_func("Song.add_pause", &Song::add_pause);
		ext->link_class_func("Song.edit_bar", &Song::edit_bar);
		ext->link_class_func("Song.delete_bar", &Song::delete_bar);
		ext->link_class_func("Song.add_sample", &Song::add_sample);
		ext->link_class_func("Song.delete_sample", &Song::delete_sample);
		ext->link_class_func("Song.time_track", &Song::time_track);
		ext->link_class_func("Song.undo", &Song::undo);
		ext->link_class_func("Song.redo", &Song::redo);
		ext->link_class_func("Song.reset_history", &Song::reset_history);
		ext->link_class_func("Song.begin_action_group", &Song::begin_action_group);
		ext->link_class_func("Song.end_action_group", &Song::end_action_group);
		ext->link_class_func("Song.get_time_str", &Song::get_time_str);
		ext->link_class_func("Song.get_time_str_fuzzy", &Song::get_time_str_fuzzy);
		ext->link_class_func("Song.get_time_str_long", &Song::get_time_str_long);
	}

	{
		SongRenderer sr(nullptr);
		ext->declare_class_size("SongRenderer", sizeof(SongRenderer));
		ext->link_class_func("SongRenderer.set_range", &SongRenderer::set_range);
		ext->link_class_func("SongRenderer.set_loop", &SongRenderer::set_loop);
		ext->link_class_func("SongRenderer.render", &SongRenderer::render);
		ext->link_class_func("SongRenderer.__init__", &SongRenderer::__init__);
		ext->link_virtual("SongRenderer.__delete__", &SongRenderer::__delete__, &sr);
		ext->link_virtual("SongRenderer.read", &SongRenderer::read, &sr);
		ext->link_virtual("SongRenderer.reset", &SongRenderer::reset_state, &sr);
		ext->link_class_func("SongRenderer.range", &SongRenderer::range);
		ext->link_class_func("SongRenderer.get_pos", &SongRenderer::get_pos);
		ext->link_class_func("SongRenderer.set_pos", &SongRenderer::set_pos);
		ext->link_class_func("SongRenderer.get_beat_source", &SongRenderer::get_beat_source);
	}

	{
		AudioInput input(Session::GLOBAL);
		ext->declare_class_size("AudioInput", sizeof(AudioInput));
		ext->declare_class_element("AudioInput.current_buffer", &AudioInput::buffer);
		//ext->declare_class_element("AudioInput.out", &AudioInput::out);
		ext->link_class_func("AudioInput.__init__", &AudioInput::__init__);
		ext->link_virtual("AudioInput.__delete__", &AudioInput::__delete__, &input);
		ext->link_class_func("AudioInput.start", &AudioInput::start);
		ext->link_class_func("AudioInput.stop",	 &AudioInput::stop);
		ext->link_class_func("AudioInput.is_capturing", &AudioInput::is_capturing);
		ext->link_class_func("AudioInput.sample_rate", &AudioInput::sample_rate);
		ext->link_class_func("AudioInput.samples_recorded", &AudioInput::samples_recorded);
		//ext->link_class_func("AudioInput.set_backup_mode", &AudioInput::set_backup_mode);
	}

	{
		AudioOutput stream(Session::GLOBAL);
		ext->declare_class_size("AudioOutput", sizeof(AudioOutput));
		ext->link_class_func("AudioOutput.__init__", &AudioOutput::__init__);
		ext->link_virtual("AudioOutput.__delete__", &AudioOutput::__delete__, &stream);
		//ext->link_class_func("AudioOutput.setSource", &AudioStream::setSource);
		ext->link_class_func("AudioOutput.start", &AudioOutput::start);
		ext->link_class_func("AudioOutput.stop", &AudioOutput::stop);
		ext->link_class_func("AudioOutput.is_playing", &AudioOutput::is_playing);
		//ext->link_class_func("AudioOutput.sample_rate", &OutputStream::sample_rate);
		ext->link_class_func("AudioOutput.get_volume", &AudioOutput::get_volume);
		ext->link_class_func("AudioOutput.set_volume", &AudioOutput::set_volume);
		ext->link_class_func("AudioOutput.samples_played", &AudioOutput::estimate_samples_played);
		ext->link_virtual("AudioOutput.reset_state", &AudioOutput::reset_state, &stream);
	}
	
	{
		ext->declare_class_size("AudioAccumulator", sizeof(AudioAccumulator));
		ext->link_class_func("AudioAccumulator.__init__", &generic__init__<AudioAccumulator>);
		ext->declare_class_element("AudioAccumulator.samples_skipped", &AudioAccumulator::samples_skipped);
		ext->declare_class_element("AudioAccumulator.buffer", &AudioAccumulator::buf);
	}

	{
		ext->declare_class_size("MidiAccumulator", sizeof(MidiAccumulator));
		ext->link_class_func("MidiAccumulator.__init__", &generic__init__<MidiAccumulator>);
		ext->declare_class_element("MidiAccumulator.buffer", &MidiAccumulator::buffer);
	}

	{
		SignalChain chain(Session::GLOBAL, "");
		ext->declare_class_size("SignalChain", sizeof(SignalChain));
		ext->link_class_func("SignalChain.__init__", &SignalChain::__init__);
		ext->link_virtual("SignalChain.__delete__", &SignalChain::__delete__, &chain);
		ext->link_virtual("SignalChain.reset_state", &SignalChain::reset_state, &chain);
		ext->link_virtual("SignalChain.command", &SignalChain::command, &chain);
		ext->link_class_func("SignalChain.__del_override__", &SignalChain::unregister);
		ext->link_class_func("SignalChain.start", &SignalChain::start);
		ext->link_class_func("SignalChain.stop", &SignalChain::stop);
		ext->link_class_func("SignalChain._add", &XSignalChain::x_add);
		ext->link_class_func("SignalChain.add_basic", &XSignalChain::x_add_basic);
		ext->link_class_func("SignalChain.add_existing", &XSignalChain::x_add_existing);
		ext->link_class_func("SignalChain.delete", &SignalChain::delete_module);
		ext->link_class_func("SignalChain.connect", &XSignalChain::x_connect);
		ext->link_class_func("SignalChain.disconnect", &SignalChain::disconnect);
		ext->link_class_func("SignalChain.set_update_dt", &SignalChain::set_tick_dt);
		ext->link_class_func("SignalChain.set_buffer_size", &SignalChain::set_buffer_size);
		ext->link_class_func("SignalChain.is_prepared", &SignalChain::is_prepared);
		ext->link_class_func("SignalChain.is_active", &SignalChain::is_active);
	}

	ext->declare_class_size("AudioView", sizeof(AudioView));
	ext->declare_class_element("AudioView.cam", &AudioView::cam);
	ext->declare_class_element("AudioView.sel", &AudioView::sel);
	ext->declare_class_element("AudioView.mouse_wheel_speed", &AudioView::mouse_wheel_speed);
	//ext->link_class_func("AudioView.subscribe", &ObservableKabaWrapper<AudioView>::subscribe_kaba);
	ext->link_class_func("AudioView.unsubscribe", &AudioView::unsubscribe);
	ext->link_class_func("AudioView.optimize_view", &AudioView::request_optimize_view);
	ext->link_class_func("AudioView.cur_vlayer", &AudioView::cur_vlayer);
	ext->link_class_func("AudioView.cur_vtrack", &AudioView::cur_vtrack);
	ext->link_class_func("AudioView.update_selection", &AudioView::update_selection);


	ext->declare_class_size("Playback", sizeof(Playback));
	ext->declare_class_element("Playback.renderer", &Playback::renderer);
	ext->declare_class_element("Playback.signal_chain", &Playback::signal_chain);
	ext->declare_class_element("Playback.output_stream", &Playback::output_stream);
	ext->declare_class_element("Playback.out_changed", &Playback::out_changed);
	ext->declare_class_element("Playback.out_state_changed", &Playback::out_state_changed);
	ext->declare_class_element("Playback.out_tick", &Playback::out_tick);
	//ext->link_class_func("Playback.create_sink", &ObservableKabaWrapper<Playback>::subscribe_kaba);
	ext->link_class_func("Playback.unsubscribe", &Playback::unsubscribe);
	ext->link_class_func("Playback.play", &Playback::play);
	ext->link_class_func("Playback.is_active", &Playback::is_active);
	ext->link_class_func("Playback.is_paused", &Playback::is_paused);
	ext->link_class_func("Playback.get_pos", &Playback::get_pos);
	ext->link_class_func("Playback.set_pos", &Playback::set_pos);
	ext->link_class_func("Playback.prepare", &Playback::prepare);
	ext->link_class_func("Playback.set_loop", &Playback::set_loop);

	ext->declare_class_size("SceneGraph.Node", sizeof(scenegraph::Node));
	ext->declare_class_element("SceneGraph.Node.area", &scenegraph::Node::area);
	ext->declare_class_element("SceneGraph.Node." + kaba::Identifier::SHARED_COUNT, &scenegraph::Node::_pointer_ref_counter);

	ext->declare_class_size("AudioView.Layer", sizeof(AudioViewLayer));
	ext->declare_class_element("AudioView.Layer.layer", &AudioViewLayer::layer);

	ext->declare_class_size("AudioView.ViewPort", sizeof(ViewPort));
	ext->declare_class_element("AudioView.ViewPort.area", &ViewPort::area);
	ext->link_class_func("AudioView.ViewPort.__init__", &ViewPort::__init__);
	ext->link_class_func("AudioView.ViewPort.range", &ViewPort::range);
	ext->link_class_func("AudioView.ViewPort.set_range", &ViewPort::set_range);
	ext->link_class_func("AudioView.ViewPort.sample2screen64", &ViewPort::sample2screen);
	ext->link_class_func("AudioView.ViewPort.screen2sample64", &ViewPort::screen2sample);
	ext->link_class_func("AudioView.ViewPort.sample2screen", &ViewPort::sample2screen_f);
	ext->link_class_func("AudioView.ViewPort.screen2sample", &ViewPort::screen2sample_f);

	ext->declare_class_size("ColorScheme", sizeof(ColorScheme));
	ext->declare_class_element("ColorScheme.background", &ColorScheme::background);
	ext->declare_class_element("ColorScheme.background_track", &ColorScheme::background_track);
	ext->declare_class_element("ColorScheme.background_track_selected", &ColorScheme::background_track_selected);
	ext->declare_class_element("ColorScheme.text", &ColorScheme::text);
	ext->declare_class_element("ColorScheme.text_soft1", &ColorScheme::text_soft1);
	ext->declare_class_element("ColorScheme.text_soft2", &ColorScheme::text_soft2);
	ext->declare_class_element("ColorScheme.text_soft3", &ColorScheme::text_soft3);
	ext->declare_class_element("ColorScheme.grid", &ColorScheme::grid);
	ext->declare_class_element("ColorScheme.selection", &ColorScheme::selection);
	ext->declare_class_element("ColorScheme.hover", &ColorScheme::hover);
	ext->declare_class_element("ColorScheme.blob_bg", &ColorScheme::blob_bg);
	ext->declare_class_element("ColorScheme.blob_bg_selected", &ColorScheme::blob_bg_selected);
	ext->declare_class_element("ColorScheme.blob_bg_hidden", &ColorScheme::blob_bg_hidden);
	ext->declare_class_element("ColorScheme.pitch", &ColorScheme::pitch);
	ext->link_class_func("ColorScheme.hoverify", &ColorScheme::hoverify);
	ext->link("ColorScheme.pitch_color", (void*)&ColorScheme::pitch_color);

	ext->link_class_func("Storage.load", &Storage::load);
	ext->link_class_func("Storage.save", &Storage::save);
	ext->link_class_func("Storage.save_via_renderer", &Storage::save_via_renderer);
	ext->link_class_func("Storage.load_buffer", &Storage::load_buffer);
	ext->declare_class_element("Storage.current_directory", &Storage::current_directory);


	ext->declare_class_size("SongSelection", sizeof(SongSelection));
	ext->declare_class_element("SongSelection.range_raw", &SongSelection::range_raw);
	ext->link_class_func("SongSelection.range", &SongSelection::range);
	ext->link_class_func("SongSelection.has_track", (bool (SongSelection::*)(const Track*) const)&SongSelection::has);
	ext->link_class_func("SongSelection.has_layer", (bool (SongSelection::*)(const TrackLayer*) const)&SongSelection::has);
	ext->link_class_func("SongSelection.has_marker", (bool (SongSelection::*)(const TrackMarker*) const)&SongSelection::has);
	ext->link_class_func("SongSelection.has_note",  (bool (SongSelection::*)(const MidiNote*) const)&SongSelection::has);
	ext->link_class_func("SongSelection.has_bar",  (bool (SongSelection::*)(const Bar*) const)&SongSelection::has);


	ext->declare_class_size("MidiPainter", sizeof(MidiPainter));
	ext->declare_class_element("MidiPainter.cam", &MidiPainter::cam);
	ext->link_class_func("MidiPainter.__init__", &MidiPainter::__init__);
	ext->link_class_func("MidiPainter.set_context", &MidiPainter::set_context);
	ext->link_class_func("MidiPainter.draw", &MidiPainter::draw);
	ext->link_class_func("MidiPainter.draw_background", &MidiPainter::draw_background);
	//ext->link_class_func("MidiPainter.pitch_color", &MidiPainter::pitch_color);


	ext->declare_class_size("GridPainter", sizeof(GridPainter));
	ext->link_class_func("GridPainter.__init__", &GridPainter::__init__);
	ext->link_class_func("GridPainter.set_context", &GridPainter::set_context);
	ext->link_class_func("GridPainter.draw_empty_background", &GridPainter::draw_empty_background);
	ext->link_class_func("GridPainter.draw_bars", &GridPainter::draw_bars);
	ext->link_class_func("GridPainter.draw_bar_numbers", &GridPainter::draw_bar_numbers);
	ext->link_class_func("GridPainter.draw_time", &GridPainter::draw_time);
	ext->link_class_func("GridPainter.draw_time_numbers", &GridPainter::draw_time_numbers);

	ext->declare_class_size("MultiLinePainter", sizeof(MultiLinePainter));
	ext->link_class_func("MultiLinePainter.__init__", &MultiLinePainter::__init__);
	ext->link_class_func("MultiLinePainter.__delete__", &MultiLinePainter::__delete__);
	ext->link_class_func("MultiLinePainter.set_context", &MultiLinePainter::set_context);
	ext->link_class_func("MultiLinePainter.set", &MultiLinePainter::set);
	ext->link_class_func("MultiLinePainter.draw_next_line", &MultiLinePainter::draw_next_line);
	ext->link_class_func("MultiLinePainter.next_line_samples", &MultiLinePainter::next_line_samples);
	ext->link_class_func("MultiLinePainter.get_line_dy", &MultiLinePainter::get_line_dy);

	{
		Slider slider{nullptr, "", ""};
		ext->declare_class_size("Slider", sizeof(Slider));
		ext->link_class_func("Slider.__init__", &Slider::__init_ext__);
		ext->link_virtual("Slider.__delete__", &Slider::__delete__, &slider);
		ext->link_class_func("Slider.get", &Slider::get);
		ext->link_class_func("Slider.set", &Slider::set);
		ext->link_class_func("Slider.set_range", &Slider::set_range);
		ext->link_class_func("Slider.set_slider_range", &Slider::set_slider_range);
		ext->link_class_func("Slider.set_scale", &Slider::set_scale);
		ext->link_class_func("Slider.enable", &Slider::enable);
	}

	{
		TsunamiPlugin tsunami_plugin;
		ext->declare_class_size("TsunamiPlugin", sizeof(TsunamiPlugin));
		//ext->declare_class_element("TsunamiPlugin.session", &TsunamiPlugin, session);
		ext->declare_class_element("TsunamiPlugin.args", &TsunamiPlugin::args);
		ext->link_class_func("TsunamiPlugin.__init__", &TsunamiPlugin::__init__);
		ext->link_virtual("TsunamiPlugin.__delete__", &TsunamiPlugin::__delete__, &tsunami_plugin);
		ext->link_virtual("TsunamiPlugin.on_start", &TsunamiPlugin::on_start, &tsunami_plugin);
		ext->link_virtual("TsunamiPlugin.on_stop", &TsunamiPlugin::on_stop, &tsunami_plugin);
		ext->link_virtual("TsunamiPlugin.on_draw_post", &TsunamiPlugin::on_draw_post, &tsunami_plugin);
		ext->link_class_func("TsunamiPlugin.stop", &TsunamiPlugin::stop_request);
	}

	{
		Progress *prog = new Progress("", nullptr);
		ext->declare_class_size("Progress", sizeof(Progress));
		ext->link_class_func("Progress.__init__", &Progress::__init__);
		ext->link_virtual("Progress.__delete__", &Progress::__delete__, prog);
		ext->link_class_func("Progress.set", &Progress::set_kaba);
		delete prog;
	}

	{
		ProgressCancelable prog_can;
		ext->declare_class_size("ProgressX", sizeof(ProgressCancelable));
		ext->link_class_func("ProgressX.__init__", &ProgressCancelable::__init__);
		ext->link_virtual("ProgressX.__delete__", &ProgressCancelable::__delete__, &prog_can);
		ext->link_class_func("ProgressX.set", &ProgressCancelable::set_kaba);
		ext->link_class_func("ProgressX.cancel", &ProgressCancelable::cancel);
		ext->link_class_func("ProgressX.is_cancelled", &ProgressCancelable::is_cancelled);
	}
}

kaba::Class* PluginManager::get_class(const string &name) {
	for (auto c: weak(package->tree->base_class->classes))
		if (c->name == name)
			return (kaba::Class*)c;
	return (kaba::Class*)package->tree->create_new_class(name, kaba::Class::Type::REGULAR, 0, 0, nullptr, {}, package->tree->base_class, -1);
}

void get_plugin_file_data(PluginManager::PluginFile &pf) {
	pf.image = "";
	try {
		string content = os::fs::read_text(pf.filename);
		int p = content.find("// Image = hui:");
		if (p >= 0)
			pf.image = content.sub(p + 11, content.find("\n"));
	} catch(...) {}
}

void PluginManager::find_plugins_in_dir_absolute(const Path &_dir, const string &group, ModuleCategory type) {
	Path dir = _dir;
	if (group.num > 0)
		dir |= group;
	auto list = os::fs::search(dir, "*.kaba", "f");
	for (auto &e: list) {
		PluginManager::PluginFile pf;
		pf.type = type;
		pf.name = e.no_ext().str();
		pf.filename = dir | e;
		pf.group = group;
		get_plugin_file_data(pf);
		plugin_files.add(pf);
	}
}

void PluginManager::find_plugins_in_dir(const Path &rel, const string &group, ModuleCategory type) {
	find_plugins_in_dir_absolute(plugin_dir_static() | rel, group, type);
	if (plugin_dir_local() != plugin_dir_static())
		find_plugins_in_dir_absolute(plugin_dir_local() | rel, group, type);
}

void PluginManager::add_plugins_in_dir(const Path &dir, hui::Menu *m, const string &name_space, TsunamiWindow *win, PluginCallback cb) {
	for (auto &f: plugin_files) {
		if (f.filename.is_in(plugin_dir_static() | dir) or f.filename.is_in(plugin_dir_local() | dir)) {
			string id = format("execute-%s--%s", name_space, f.name);
			m->add_with_image(f.name, f.image, id);
			win->event(id, [cb,f]{ cb(f.name); });
		}
	}
}

void PluginManager::find_plugins() {
	kaba::init();
	kaba::config.show_compiler_stats = false;
	kaba::config.compile_silently = true;

	// "AudioSource"
	find_plugins_in_dir("AudioSource", "", ModuleCategory::AUDIO_SOURCE);

	// "AudioEffect"
	find_plugins_in_dir("AudioEffect", "Channels", ModuleCategory::AUDIO_EFFECT);
	find_plugins_in_dir("AudioEffect", "Dynamics", ModuleCategory::AUDIO_EFFECT);
	find_plugins_in_dir("AudioEffect", "Echo", ModuleCategory::AUDIO_EFFECT);
	find_plugins_in_dir("AudioEffect", "Filter", ModuleCategory::AUDIO_EFFECT);
	find_plugins_in_dir("AudioEffect", "Pitch", ModuleCategory::AUDIO_EFFECT);
	find_plugins_in_dir("AudioEffect", "Repair", ModuleCategory::AUDIO_EFFECT);
	find_plugins_in_dir("AudioEffect", "Sound", ModuleCategory::AUDIO_EFFECT);
	// hidden...
	find_plugins_in_dir("AudioEffect", "Special", ModuleCategory::AUDIO_EFFECT);

	// "AudioVisualizer"
	find_plugins_in_dir("AudioVisualizer", "", ModuleCategory::AUDIO_VISUALIZER);

	// "MidiSource"
	find_plugins_in_dir("MidiSource", "", ModuleCategory::MIDI_SOURCE);

	// "MidiEffect"
	find_plugins_in_dir("MidiEffect", "", ModuleCategory::MIDI_EFFECT);

	// "BeatSource"
	find_plugins_in_dir("BeatSource", "", ModuleCategory::BEAT_SOURCE);

	// "BeatSource"
	find_plugins_in_dir("PitchDetector", "", ModuleCategory::PITCH_DETECTOR);

	// rest
	find_plugins_in_dir("Independent", "Debug", ModuleCategory::TSUNAMI_PLUGIN);
	find_plugins_in_dir("Independent", "File Edit", ModuleCategory::TSUNAMI_PLUGIN);
	find_plugins_in_dir("Independent", "File Management", ModuleCategory::TSUNAMI_PLUGIN);
	find_plugins_in_dir("Independent", "File Visualization", ModuleCategory::TSUNAMI_PLUGIN);
	find_plugins_in_dir("Independent", "Games", ModuleCategory::TSUNAMI_PLUGIN);
	find_plugins_in_dir("Independent", "Live Performance", ModuleCategory::TSUNAMI_PLUGIN);
	find_plugins_in_dir("Independent", "Practice", ModuleCategory::TSUNAMI_PLUGIN);
	find_plugins_in_dir("Independent", "Special", ModuleCategory::TSUNAMI_PLUGIN);

	// "Synthesizer"
	find_plugins_in_dir("Synthesizer", "", ModuleCategory::SYNTHESIZER);
}

void PluginManager::add_plugins_to_menu(TsunamiWindow *win) {
	hui::Menu *m = win->get_menu();

	add_plugins_in_dir("Independent/Debug", m->get_sub_menu_by_id("menu_plugins_debug"), "tsunami", win, [win](const string &name){ win->on_menu_execute_tsunami_plugin(name); });
	add_plugins_in_dir("Independent/File Edit", m->get_sub_menu_by_id("menu_plugins_file_edit"), "tsunami", win, [win](const string &name){ win->on_menu_execute_tsunami_plugin(name); });
	add_plugins_in_dir("Independent/File Management", m->get_sub_menu_by_id("menu_plugins_file_management"), "tsunami", win, [win](const string &name){ win->on_menu_execute_tsunami_plugin(name); });
	add_plugins_in_dir("Independent/File Visualization", m->get_sub_menu_by_id("menu_plugins_file_visualization"), "tsunami", win, [win](const string &name){ win->on_menu_execute_tsunami_plugin(name); });
	add_plugins_in_dir("Independent/Games", m->get_sub_menu_by_id("menu_plugins_games"), "tsunami", win, [win](const string &name){ win->on_menu_execute_tsunami_plugin(name); });
	add_plugins_in_dir("Independent/Live Performance", m->get_sub_menu_by_id("menu_plugins_live_performance"), "tsunami", win, [win](const string &name){ win->on_menu_execute_tsunami_plugin(name); });
	add_plugins_in_dir("Independent/Practice", m->get_sub_menu_by_id("menu_plugins_practice"), "tsunami", win, [win](const string &name){ win->on_menu_execute_tsunami_plugin(name); });
	add_plugins_in_dir("Independent/Special", m->get_sub_menu_by_id("menu_plugins_special"), "tsunami", win, [win](const string &name){ win->on_menu_execute_tsunami_plugin(name); });
}

void PluginManager::apply_module_preset(Module *c, const string &name, bool notify) {
	presets->apply(c, name, notify);
}

void PluginManager::save_module_preset(Module *c, const string &name) {
	presets->save(c, name);
}


base::future<string> PluginManager::select_module_preset_name(hui::Window *win, Module *c, bool save) {
	return presets->select_name(win, c, save);
}

// always push the script... even if an error occurred
//   don't log error...
Plugin *PluginManager::load_and_compile_plugin(ModuleCategory type, const Path &filename) {
	for (Plugin *p: plugins)
		if (filename == p->filename)
			return p;

	//InitPluginData();

	Plugin *p = new Plugin(filename, type);
	p->index = plugins.num;

	plugins.add(p);

	return p;
}


Plugin *PluginManager::get_plugin(Session *session, ModuleCategory type, const string &name) {
	for (PluginFile &pf: plugin_files) {
		if ((pf.name.replace(" ", "") == name.replace(" ", "")) and (pf.type == type)) {
			Plugin *p = load_and_compile_plugin(type, pf.filename);
			return p;
		}
	}
	session->e(format(_("Can't find %s plugin: %s ..."), Module::category_to_str(type), name));
	return nullptr;
}

Path PluginManager::plugin_dir_static() {
	if (tsunami->installed)
		return tsunami->directory_static | "Plugins";
	return "Plugins";
}

Path PluginManager::plugin_dir_local() {
	if (tsunami->installed)
		return tsunami->directory | "Plugins";
	return "Plugins";
}


Array<string> PluginManager::find_module_sub_types(ModuleCategory type) {
	Array<string> names;
	for (auto &pf: plugin_files)
		if (pf.type == type)
			names.add(pf.name);

	if (type == ModuleCategory::AUDIO_SOURCE) {
		names.add("SongRenderer");
		//names.add("BufferStreamer");
	} else if (type == ModuleCategory::MIDI_EFFECT) {
		names.add("Dummy");
	} else if (type == ModuleCategory::BEAT_SOURCE) {
		//names.add("BarStreamer");
	} else if (type == ModuleCategory::AUDIO_VISUALIZER) {
		names.add("PeakMeter");
	} else if (type == ModuleCategory::SYNTHESIZER) {
		names.add("Dummy");
		//names.add("Sample");
	} else if (type == ModuleCategory::STREAM) {
		names.add("AudioInput");
		names.add("AudioOutput");
		names.add("MidiInput");
	} else if (type == ModuleCategory::PLUMBING) {
		names.add("AudioBackup");
		names.add("AudioChannelSelector");
		names.add("AudioJoiner");
		names.add("AudioAccumulator");
		names.add("AudioSucker");
		names.add("BeatMidifier");
		names.add("MidiJoiner");
		names.add("MidiAccumulator");
		names.add("MidiSucker");
	} else if (type == ModuleCategory::PITCH_DETECTOR) {
		names.add("Dummy");
	}
	return names;
}

Array<string> PluginManager::find_module_sub_types_grouped(ModuleCategory type) {
	if ((type == ModuleCategory::AUDIO_EFFECT) or (type == ModuleCategory::TSUNAMI_PLUGIN)) {
		Array<string> names;
		for (auto &pf: plugin_files)
			if (pf.type == type)
				names.add(pf.group + "/" + pf.name);
		return names;
	}
	return find_module_sub_types(type);
}

void PluginManager::set_favorite(Session *session, ModuleCategory type, const string &name, bool favorite) {
	presets->set_favorite(session, type, name, favorite);
}

bool PluginManager::is_favorite(Session *session, ModuleCategory type, const string &name) {
	return presets->is_favorite(session, type, name);
}


