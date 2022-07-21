/*
 * PluginManager.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "PluginManager.h"
#include "FastFourierTransform.h"
#include "Plugin.h"
#include "ExtendedAudioBuffer.h"
#include "ProfileManager.h"
#include "TsunamiPlugin.h"
#include "../Tsunami.h"
#include "../TsunamiWindow.h"
#include "../Session.h"
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
#include "../data/audio/BufferInterpolator.h"
#include "../data/rhythm/Bar.h"
#include "../module/ModuleFactory.h"
#include "../module/ConfigPanel.h"
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
#include "../view/audioview/graph/AudioViewLayer.h"
#include "../view/sidebar/SampleManagerConsole.h"
#include "../view/mode/ViewModeCapture.h"
#include "../view/helper/graph/Node.h"
#include "../view/painter/GridPainter.h"
#include "../view/painter/MidiPainter.h"
#include "../view/painter/MultiLinePainter.h"
#include "../lib/base/callable.h"
#include "../lib/os/filesystem.h"

namespace kaba {
	extern shared_array<Module> public_modules;
};

namespace hui {
	void get_style_colors(Panel *p, const string &id, Map<string,color> &colors);
}

#define _offsetof(CLASS, ELEMENT) (int)( (char*)&((CLASS*)1)->ELEMENT - (char*)((CLASS*)1) )

PluginManager::PluginManager() {
	profiles = new ProfileManager;

	find_plugins();

	package = new kaba::Module;
	package->filename = "-tsunami-internal-";
	package->used_by_default = false;
	kaba::packages.add(package);
	kaba::public_modules.add(package.get());

	auto *type_dev = package->syntax->make_class("Device", kaba::Class::Type::OTHER, 0, 0, nullptr, {}, package->syntax->base_class, -1);
	package->syntax->make_class("Device*", kaba::Class::Type::POINTER, sizeof(void*), 0, nullptr, {type_dev}, package->syntax->base_class, -1);
}

PluginManager::~PluginManager() {
	//plugins.clear();
	//kaba::clean_up();
}


Module *_CreateBeatMidifier(Session *s) {
	return new BeatMidifier();
}

template<class T>
class ObservableKabaWrapper : public T {
public:
	void _cdecl subscribe_kaba(hui::EventHandler *handler, Callable<void(VirtualBase*)> *f, const string &message) {
		T::subscribe(handler, [f, handler] {
			(*f)(handler);
		}, message);
	}
};


void PluginManager::link_app_data() {
	kaba::config.directory = Path::EMPTY;

	// api definition
	kaba::link_external("device_manager", &tsunami->device_manager);
	kaba::link_external("colors", &theme);
	kaba::link_external("clipboard", &tsunami->clipboard);
	//kaba::link_external("view_input", &export_view_input);
	kaba::link_external("db2amp", (void*)&db2amplitude);
	kaba::link_external("amp2db", (void*)&amplitude2db);
	kaba::link_external("fft_c2c", (void*)&FastFourierTransform::fft_c2c);
	kaba::link_external("fft_r2c", (void*)&FastFourierTransform::fft_r2c);
	kaba::link_external("fft_c2r_inv", (void*)&FastFourierTransform::fft_c2r_inv);
	kaba::link_external("CreateModule", (void*)&ModuleFactory::create);
	kaba::link_external("CreateSynthesizer", (void*)&CreateSynthesizer);
	kaba::link_external("CreateAudioEffect", (void*)&CreateAudioEffect);
	kaba::link_external("CreateAudioSource", (void*)&CreateAudioSource);
	kaba::link_external("CreateMidiEffect", (void*)&CreateMidiEffect);
	kaba::link_external("CreateMidiSource", (void*)&CreateMidiSource);
	kaba::link_external("CreateBeatSource", (void*)&CreateBeatSource);
	kaba::link_external("CreateBeatMidifier", (void*)&_CreateBeatMidifier);
	kaba::link_external("SelectSample", (void*)&SampleManagerConsole::select);
	kaba::link_external("ChooseModule", (void*)&choose_module);
	kaba::link_external("draw_boxed_str", (void*)&draw_boxed_str);
	kaba::link_external("interpolate_buffer", (void*)&BufferInterpolator::interpolate);
	kaba::link_external("get_style_colors", (void*)&hui::get_style_colors);

	kaba::declare_class_size("Clipboard", sizeof(Clipboard));
	kaba::declare_class_element("Clipboard.temp", &Clipboard::temp);
	kaba::link_external_class_func("Clipboard.has_data", &Clipboard::has_data);
	kaba::link_external_class_func("Clipboard.prepare_layer_map", &Clipboard::prepare_layer_map);

	kaba::declare_class_size("Range", sizeof(Range));
	kaba::declare_class_element("Range.offset", &Range::offset);
	kaba::declare_class_element("Range.length", &Range::length);
	kaba::link_external_class_func("Range.__and__", &Range::intersect);
	kaba::link_external_class_func("Range.__str__", &Range::str);
	kaba::link_external_class_func("Range.start", &Range::start);
	kaba::link_external_class_func("Range.end", &Range::end);
	kaba::link_external_class_func("Range.covers", &Range::covers);
	kaba::link_external_class_func("Range.overlaps", &Range::overlaps);
	kaba::link_external_class_func("Range.is_inside", &Range::is_inside);

	kaba::declare_class_size("Bar", sizeof(Bar));
	kaba::declare_class_element("Bar.beats", &Bar::beats);
	kaba::declare_class_element("Bar.divisor", &Bar::divisor);
	kaba::declare_class_element("Bar.total_sub_beats", &Bar::total_sub_beats);
	kaba::declare_class_element("Bar.length", &Bar::length);
	kaba::declare_class_element("Bar.index", &Bar::index);
	kaba::declare_class_element("Bar.index_text", &Bar::index_text);
	kaba::declare_class_element("Bar.offset", &Bar::offset);
	kaba::declare_class_element("Bar." + kaba::IDENTIFIER_SHARED_COUNT, &Bar::_pointer_ref_counter);
	kaba::link_external_class_func("Bar." + kaba::IDENTIFIER_FUNC_INIT, &Bar::__init__);
	kaba::link_external_class_func("Bar.range", &Bar::range);
	kaba::link_external_class_func("Bar.bpm", &Bar::bpm);

	kaba::declare_class_size("BarCollection", sizeof(BarCollection));
	kaba::link_external_class_func("BarCollection.get_bars", &BarCollection::get_bars);
	kaba::link_external_class_func("BarCollection.get_beats", &BarCollection::get_beats);
	kaba::link_external_class_func("BarCollection.get_next_beat", &BarCollection::get_next_beat);
	kaba::link_external_class_func("BarCollection.get_prev_beat", &BarCollection::get_prev_beat);

	kaba::declare_class_size("Session", sizeof(Session));
	kaba::declare_class_element("Session.id", &Session::id);
	kaba::declare_class_element("Session.storage", &Session::storage);
	kaba::declare_class_element("Session.win", &Session::_kaba_win);
	kaba::declare_class_element("Session.view", &Session::view);
	kaba::declare_class_element("Session.song", &Session::song);
	kaba::declare_class_element("Session." + kaba::IDENTIFIER_SHARED_COUNT, &Session::_pointer_ref_counter);
	kaba::link_external_class_func("Session.sample_rate", &Session::sample_rate);
	kaba::link_external_class_func("Session.i", &Session::i);
	kaba::link_external_class_func("Session.w", &Session::w);
	kaba::link_external_class_func("Session.e", &Session::e);
	kaba::link_external_class_func("Session.add_signal_chain", &Session::add_signal_chain);
	kaba::link_external_class_func("Session.create_signal_chain", &Session::create_signal_chain);
	kaba::link_external_class_func("Session.load_signal_chain", &Session::load_signal_chain);
	kaba::link_external_class_func("Session.create_child", &Session::create_child);


	{
		Module module(ModuleCategory::AUDIO_EFFECT, "");
		kaba::declare_class_size("Module", sizeof(Module));
		kaba::declare_class_element("Module.name", &Module::module_class);
		kaba::declare_class_element("Module.session", &Module::session);
		kaba::declare_class_element("Module.port_out", &Module::port_out);
		kaba::declare_class_element("Module." + kaba::IDENTIFIER_SHARED_COUNT, &Module::_pointer_ref_counter);
		kaba::link_external_class_func("Module.__init__", &Module::__init__);
		kaba::link_external_virtual("Module.__delete__", &Module::__delete__, &module);
		kaba::link_external_virtual("Module.create_panel", &Module::create_panel, &module);
		kaba::link_external_class_func("Module.reset_config", &Module::reset_config);
		kaba::link_external_virtual("Module.reset_state", &Module::reset_state, &module);
		kaba::link_external_class_func("Module.changed", &Module::changed);
		kaba::link_external_virtual("Module.get_config", &Module::get_config, &module);
		kaba::link_external_class_func("Module.config_to_string", &Module::config_to_string);
		kaba::link_external_class_func("Module.config_from_string", &Module::config_from_string);
		kaba::link_external_class_func("Module.config_to_any", &Module::config_to_any);
		kaba::link_external_class_func("Module.config_from_any", &Module::config_from_any);
		kaba::link_external_virtual("Module.on_config", &Module::on_config, &module);
		kaba::link_external_virtual("Module.command", &Module::command, &module);
		kaba::link_external_class_func("Module.subscribe", &ObservableKabaWrapper<Module>::subscribe_kaba);
		kaba::link_external_class_func("Module.unsubscribe", &Module::unsubscribe);
		kaba::link_external_class_func("Module.copy", &Module::copy);
		kaba::link_external_class_func("Module.plug_in", &Module::_plug_in);
		kaba::link_external_class_func("Module.unplug_in", &Module::_unplug_in);
	}


	{
		ModuleConfiguration plugin_data;
		kaba::declare_class_size("Module.Config", sizeof(ModuleConfiguration));
		kaba::link_external_class_func("Module.Config.__init__", &ModuleConfiguration::__init__);
		kaba::link_external_virtual("Module.Config.__delete__", &ModuleConfiguration::__delete__, &plugin_data);
		kaba::link_external_virtual("Module.Config.reset", &ModuleConfiguration::reset, &plugin_data);
		kaba::link_external_class_func("Module.Config.from_string", &ModuleConfiguration::from_string);
		kaba::link_external_class_func("Module.Config.to_string", &ModuleConfiguration::to_string);
		kaba::link_external_virtual("Module.Config.auto_conf", &ModuleConfiguration::auto_conf, &plugin_data);
	}


	{
		ConfigPanel::_hidden_parent_check_ = false;
		ConfigPanel config_panel(nullptr);
		kaba::declare_class_size("ConfigPanel", sizeof(ConfigPanel));
		kaba::link_external_class_func("ConfigPanel.__init__", &ConfigPanel::__init__);
		kaba::link_external_virtual("ConfigPanel.__delete__", &ConfigPanel::__delete__, &config_panel);
		kaba::link_external_virtual("ConfigPanel.update", &ConfigPanel::update, &config_panel);
		kaba::link_external_virtual("ConfigPanel.set_large", &ConfigPanel::set_large, &config_panel);
		kaba::link_external_class_func("ConfigPanel.changed", &ConfigPanel::changed);
		kaba::declare_class_element("ConfigPanel.c", &ConfigPanel::c);
		ConfigPanel::_hidden_parent_check_ = true;
	}

	{
		AudioSource asource;
		kaba::declare_class_size("AudioSource", sizeof(AudioSource));
		kaba::link_external_class_func("AudioSource.__init__", &AudioSource::__init__);
		kaba::link_external_virtual("AudioSource.__delete__", &AudioSource::__delete__, &asource);
		kaba::link_external_virtual("AudioSource.read", &AudioSource::read, &asource);
	}

	{
		AudioEffect aeffect;
		kaba::declare_class_size("AudioEffect", sizeof(AudioEffect));
		kaba::declare_class_element("AudioEffect.sample_rate", &AudioEffect::sample_rate);
		kaba::declare_class_element("AudioEffect.apply_to_whole_buffer", &AudioEffect::apply_to_whole_buffer);
		kaba::declare_class_element("AudioEffect.source", &AudioEffect::source);
		kaba::link_external_class_func("AudioEffect.__init__", &AudioEffect::__init__);
		kaba::link_external_virtual("AudioEffect.__delete__", &AudioEffect::__delete__, &aeffect);
		kaba::link_external_virtual("AudioEffect.process", &AudioEffect::process, &aeffect);
		kaba::link_external_virtual("AudioEffect.read", &AudioEffect::read, &aeffect);
	}

	{
		MidiEffect meffect;
		kaba::declare_class_size("MidiEffect", sizeof(MidiEffect));
		kaba::link_external_class_func("MidiEffect.__init__", &MidiEffect::__init__);
		kaba::link_external_virtual("MidiEffect.__delete__", &MidiEffect::__delete__, &meffect);
		kaba::link_external_virtual("MidiEffect.process", &MidiEffect::process, &meffect);
	}

	{
		AudioVisualizer avis;
		kaba::declare_class_size("AudioVisualizer", sizeof(AudioVisualizer));
		kaba::declare_class_element("AudioVisualizer.chunk_size", &AudioVisualizer::chunk_size);
		kaba::declare_class_element("AudioVisualizer.ring_buffer", &AudioVisualizer::buffer);
		kaba::declare_class_element("AudioVisualizer.next_writing", &AudioVisualizer::next_writing);
		kaba::declare_class_element("AudioVisualizer.current_reading", &AudioVisualizer::current_reading);
		kaba::link_external_class_func("AudioVisualizer.__init__", &AudioVisualizer::__init__);
		kaba::link_external_virtual("AudioVisualizer.__delete__", &AudioVisualizer::__delete__, &avis);
		kaba::link_external_virtual("AudioVisualizer.process", &AudioVisualizer::process, &avis);
		kaba::link_external_class_func("AudioVisualizer.set_chunk_size", &AudioVisualizer::set_chunk_size);
		kaba::link_external_class_func("AudioVisualizer.lock", &AudioVisualizer::lock);
		kaba::link_external_class_func("AudioVisualizer.unlock", &AudioVisualizer::unlock);
		kaba::link_external_class_func("AudioVisualizer.flip", &AudioVisualizer::flip);
	}

	{
		DummyPitchDetector pd;
		kaba::declare_class_size("PitchDetector", sizeof(PitchDetector));
		kaba::declare_class_element("PitchDetector.frequency", &PitchDetector::frequency);
		kaba::declare_class_element("PitchDetector.pitch", &PitchDetector::pitch);
		kaba::declare_class_element("PitchDetector.volume", &PitchDetector::volume);
		kaba::declare_class_element("PitchDetector.loud_enough", &PitchDetector::loud_enough);
		kaba::link_external_class_func("PitchDetector.__init__", &DummyPitchDetector::__init__);
		kaba::link_external_virtual("PitchDetector.read", &PitchDetector::read, &pd);
		kaba::link_external_virtual("PitchDetector.process", &DummyPitchDetector::process, &pd);
	}

	kaba::declare_class_size("AudioBuffer", sizeof(AudioBuffer));
	kaba::declare_class_element("AudioBuffer.offset", &AudioBuffer::offset);
	kaba::declare_class_element("AudioBuffer.length", &AudioBuffer::length);
	kaba::declare_class_element("AudioBuffer.channels", &AudioBuffer::channels);
	kaba::declare_class_element("AudioBuffer.c", &AudioBuffer::c);
	//kaba::_declare_class_element("AudioBuffer.l", _offsetof(AudioBuffer, c[0]));
	//kaba::_declare_class_element("AudioBuffer.r", _offsetof(AudioBuffer, c[1]));
	kaba::declare_class_element("AudioBuffer.peaks", &AudioBuffer::peaks);
	kaba::link_external_class_func("AudioBuffer.__init__", &AudioBuffer::__init__);
	kaba::link_external_class_func("AudioBuffer.__delete__", &AudioBuffer::__delete__);
	kaba::link_external_class_func("AudioBuffer.clear", &AudioBuffer::clear);
	kaba::link_external_class_func("AudioBuffer.__assign__", &AudioBuffer::__assign__);
	kaba::link_external_class_func("AudioBuffer.range", &AudioBuffer::range);
	kaba::link_external_class_func("AudioBuffer.resize", &AudioBuffer::resize);
	kaba::link_external_class_func("AudioBuffer.set_channels", &AudioBuffer::set_channels);
	kaba::link_external_class_func("AudioBuffer.add", &AudioBuffer::add);
	kaba::link_external_class_func("AudioBuffer.set", &AudioBuffer::set);
	kaba::link_external_class_func("AudioBuffer.set_as_ref", &AudioBuffer::set_as_ref);
	kaba::link_external_class_func("AudioBuffer." + kaba::IDENTIFIER_FUNC_SUBARRAY, &AudioBuffer::ref);
	kaba::link_external_class_func("AudioBuffer.get_spectrum", &ExtendedAudioBuffer::get_spectrum);


	kaba::declare_class_size("RingBuffer", sizeof(RingBuffer));
	kaba::link_external_class_func("RingBuffer.__init__", &RingBuffer::__init__);
	kaba::link_external_class_func("RingBuffer.__delete__", &RingBuffer::__delete__);
	kaba::link_external_class_func("RingBuffer.available", &RingBuffer::available);
	kaba::link_external_class_func("RingBuffer.read", &RingBuffer::read);
	kaba::link_external_class_func("RingBuffer.write", &RingBuffer::write);
	kaba::link_external_class_func("RingBuffer.read_ref", &RingBuffer::read_ref);
	kaba::link_external_class_func("RingBuffer.read_ref_done", &RingBuffer::read_ref_done);
	kaba::link_external_class_func("RingBuffer.peek", &RingBuffer::peek);
	kaba::link_external_class_func("RingBuffer.write_ref", &RingBuffer::write_ref);
	kaba::link_external_class_func("RingBuffer.write_ref_done", &RingBuffer::write_ref_done);
	kaba::link_external_class_func("RingBuffer.clear", &RingBuffer::clear);

	kaba::declare_class_size("Sample", sizeof(Sample));
	kaba::declare_class_element("Sample.name", &Sample::name);
	kaba::declare_class_element("Sample.type", &Sample::type);
	kaba::declare_class_element("Sample.buf", &Sample::buf);
	kaba::declare_class_element("Sample.midi", &Sample::midi);
	kaba::declare_class_element("Sample.volume", &Sample::volume);
	kaba::declare_class_element("Sample.uid", &Sample::uid);
	kaba::declare_class_element("Sample.ref_count", &Sample::ref_count);
	kaba::declare_class_element("Sample.tags", &Sample::tags);
	kaba::declare_class_element("Sample." + kaba::IDENTIFIER_SHARED_COUNT, &Sample::_pointer_ref_counter);
	kaba::link_external_class_func("Sample.__init__", &Sample::__init__);
	kaba::link_external_class_func("Sample.create_ref", &Sample::create_ref);
	kaba::link_external_class_func("Sample.get_value", &Sample::get_value);
	kaba::link_external_class_func("Sample.set_value", &Sample::set_value);

	{
		shared<Sample> sample = new Sample(SignalType::AUDIO);
		//sample.owner = tsunami->song;
		SampleRef sampleref(sample.get());
		kaba::declare_class_size("SampleRef", sizeof(SampleRef));
		kaba::link_external_class_func("SampleRef.buf", &SampleRef::buf);
		kaba::link_external_class_func("SampleRef.midi", &SampleRef::midi);
		kaba::declare_class_element("SampleRef.origin", &SampleRef::origin);
		kaba::declare_class_element("SampleRef." + kaba::IDENTIFIER_SHARED_COUNT, &SampleRef::_pointer_ref_counter);
		kaba::link_external_class_func("SampleRef.__init__", &SampleRef::__init__);
		kaba::link_external_virtual("SampleRef.__delete__", &SampleRef::__delete__, &sampleref);
	}



	{
		Port port(SignalType::AUDIO, "");
		kaba::declare_class_size("Module.Port", sizeof(Port));
		kaba::link_external_class_func("Module.Port.__init__", &Port::__init__);
		kaba::link_external_virtual("Module.Port.__delete__", &Port::__delete__, &port);
		kaba::link_external_virtual("Module.Port.read_audio", &Port::read_audio, &port);
		kaba::link_external_virtual("Module.Port.read_midi", &Port::read_midi, &port);
		kaba::link_external_virtual("Module.Port.read_beats", &Port::read_beats, &port);
	}

	{
		MidiSource msource;
		kaba::declare_class_size("MidiSource", sizeof(MidiSource));
		kaba::declare_class_element("MidiSource.bh_midi", &MidiSource::bh_midi);
		kaba::link_external_class_func("MidiSource.__init__", &MidiSource::__init__);
		kaba::link_external_virtual("MidiSource.__delete__", &MidiSource::__delete__, &msource);
		kaba::link_external_virtual("MidiSource.read", &MidiSource::read, &msource);
		kaba::link_external_virtual("MidiSource.reset", &MidiSource::reset, &msource);
		kaba::link_external_virtual("MidiSource.on_produce", &MidiSource::on_produce, &msource);
		kaba::link_external_class_func("MidiSource.note", &MidiSource::note);
		kaba::link_external_class_func("MidiSource.skip", &MidiSource::skip);
		kaba::link_external_class_func("MidiSource.note_x", &MidiSource::note_x);
		kaba::link_external_class_func("MidiSource.skip_x", &MidiSource::skip_x);
	}


	{
		BeatMidifier bmidifier;
		kaba::declare_class_size("BeatMidifier", sizeof(BeatMidifier));
		kaba::link_external_class_func("BeatMidifier.__init__", &BeatMidifier::__init__);
		//kaba::link_external_virtual("BeatMidifier.__delete__", &MidiSource::__delete__, &bmidifier);
		kaba::link_external_virtual("BeatMidifier.read", &BeatMidifier::read, &bmidifier);
		kaba::link_external_virtual("BeatMidifier.reset", &BeatMidifier::reset, &bmidifier);
		kaba::declare_class_element("BeatMidifier.volume", &BeatMidifier::volume);
	}

	//{
	Synthesizer synth;
	kaba::declare_class_size("Synthesizer", sizeof(Synthesizer));
	kaba::declare_class_element("Synthesizer.sample_rate", &Synthesizer::sample_rate);
	kaba::declare_class_element("Synthesizer.events", &Synthesizer::events);
	kaba::declare_class_element("Synthesizer.keep_notes", &Synthesizer::keep_notes);
	kaba::declare_class_element("Synthesizer.active_pitch", &Synthesizer::active_pitch);
	kaba::declare_class_element("Synthesizer.temperament", &Synthesizer::temperament);
	kaba::_declare_class_element("Synthesizer.freq", _offsetof(Synthesizer, temperament.freq));
	kaba::declare_class_element("Synthesizer.delta_phi", &Synthesizer::delta_phi);
	kaba::declare_class_element("Synthesizer.auto_generate_stereo", &Synthesizer::auto_generate_stereo);
	kaba::declare_class_element("Synthesizer.render_by_ref", &Synthesizer::render_by_ref);
	kaba::link_external_class_func("Synthesizer.__init__", &Synthesizer::__init__);
	kaba::link_external_virtual("Synthesizer.__delete__", &Synthesizer::__delete__, &synth);
	kaba::link_external_virtual("Synthesizer.render", &Synthesizer::render, &synth);
	kaba::link_external_virtual("Synthesizer.create_pitch_renderer", &Synthesizer::create_pitch_renderer, &synth);
	kaba::link_external_virtual("Synthesizer.on_config", &Synthesizer::on_config, &synth);
	kaba::link_external_virtual("Synthesizer.reset_state", &Synthesizer::reset_state, &synth);
	kaba::link_external_class_func("Synthesizer.set_sample_rate", &Synthesizer::set_sample_rate);
	//}

	{
		PitchRenderer pren(&synth, 0);
		kaba::declare_class_size("PitchRenderer", sizeof(PitchRenderer));
		kaba::declare_class_element("PitchRenderer.synth", &PitchRenderer::synth);
		kaba::declare_class_element("PitchRenderer.pitch", &PitchRenderer::pitch);
		kaba::declare_class_element("PitchRenderer.delta_phi", &PitchRenderer::delta_phi);
		kaba::link_external_class_func("PitchRenderer.__init__", &PitchRenderer::__init__);
		kaba::link_external_virtual("PitchRenderer.__delete__", &PitchRenderer::__delete__, &pren);
		kaba::link_external_virtual("PitchRenderer.render", &PitchRenderer::render, &pren);
		kaba::link_external_virtual("PitchRenderer.on_start", &PitchRenderer::on_start, &pren);
		kaba::link_external_virtual("PitchRenderer.on_end", &PitchRenderer::on_end, &pren);
		kaba::link_external_virtual("PitchRenderer.on_config", &PitchRenderer::on_config, &pren);
	}


	{
		DummySynthesizer dsynth;
		kaba::declare_class_size("DummySynthesizer", sizeof(DummySynthesizer));
		kaba::link_external_class_func("DummySynthesizer.__init__", &DummySynthesizer::__init__);
		kaba::link_external_virtual("DummySynthesizer.__delete__", &DummySynthesizer::__delete__, &dsynth);
		kaba::link_external_virtual("DummySynthesizer.render", &DummySynthesizer::render, &dsynth);
		kaba::link_external_virtual("DummySynthesizer.create_pitch_renderer", &DummySynthesizer::create_pitch_renderer, &dsynth);
		kaba::link_external_virtual("DummySynthesizer.on_config", &DummySynthesizer::on_config, &dsynth);
	}

	kaba::declare_class_size("EnvelopeADSR", sizeof(EnvelopeADSR));
	kaba::link_external_class_func("EnvelopeADSR.__init__", &EnvelopeADSR::reset);
	kaba::link_external_class_func("EnvelopeADSR.set", &EnvelopeADSR::set);
	kaba::link_external_class_func("EnvelopeADSR.set2", &EnvelopeADSR::set2);
	kaba::link_external_class_func("EnvelopeADSR.reset", &EnvelopeADSR::reset);
	kaba::link_external_class_func("EnvelopeADSR.start", &EnvelopeADSR::start);
	kaba::link_external_class_func("EnvelopeADSR.end", &EnvelopeADSR::end);
	kaba::link_external_class_func("EnvelopeADSR.get", &EnvelopeADSR::get);
	kaba::link_external_class_func("EnvelopeADSR.read", &EnvelopeADSR::read);
	kaba::declare_class_element("EnvelopeADSR.just_killed", &EnvelopeADSR::just_killed);

	kaba::declare_class_size("BarPattern", sizeof(BarPattern));
	kaba::declare_class_element("BarPattern.beats", &BarPattern::beats);
	kaba::declare_class_element("BarPattern.divisor", &BarPattern::divisor);
	kaba::declare_class_element("BarPattern.length", &BarPattern::length);
	//kaba::declare_class_element("BarPattern.type", &BarPattern::type);
	//kaba::declare_class_element("BarPattern.count", &BarPattern::count);

	kaba::declare_class_size("MidiNote", sizeof(MidiNote));
	kaba::declare_class_element("MidiNote.range", &MidiNote::range);
	kaba::declare_class_element("MidiNote.pitch", &MidiNote::pitch);
	kaba::declare_class_element("MidiNote.volume", &MidiNote::volume);
	kaba::declare_class_element("MidiNote.stringno", &MidiNote::stringno);
	kaba::declare_class_element("MidiNote.clef_position", &MidiNote::clef_position);
	kaba::declare_class_element("MidiNote.modifier", &MidiNote::modifier);
	kaba::declare_class_element("MidiNote.flags", &MidiNote::flags);
	kaba::declare_class_element("MidiNote." + kaba::IDENTIFIER_SHARED_COUNT, &MidiNote::_pointer_ref_counter);
	kaba::link_external_class_func("MidiNote.copy", &MidiNote::copy);
	kaba::link_external_class_func("MidiNote.is", &MidiNote::is);
	kaba::link_external_class_func("MidiNote.set", &MidiNote::set);
	
	kaba::declare_class_size("MidiEvent", sizeof(MidiEvent));
	kaba::declare_class_element("MidiEvent.pos", &MidiEvent::pos);
	kaba::declare_class_element("MidiEvent.pitch", &MidiEvent::pitch);
	kaba::declare_class_element("MidiEvent.volume", &MidiEvent::volume);
	kaba::declare_class_element("MidiEvent.stringno", &MidiEvent::stringno);
	kaba::declare_class_element("MidiEvent.clef_position", &MidiEvent::clef_position);
	kaba::declare_class_element("MidiEvent.flags", &MidiEvent::flags);

	kaba::declare_class_size("MidiEventBuffer", sizeof(MidiEventBuffer));
	kaba::declare_class_element("MidiEventBuffer.samples", &MidiEventBuffer::samples);
	kaba::link_external_class_func("MidiEventBuffer.__init__", &MidiEventBuffer::__init__);
	kaba::link_external_class_func("MidiEventBuffer.get_events", &MidiEventBuffer::get_events);
	kaba::link_external_class_func("MidiEventBuffer.get_notes", &MidiEventBuffer::get_notes);
	kaba::link_external_class_func("MidiEventBuffer.get_range", &MidiEventBuffer::range);
	kaba::link_external_class_func("MidiEventBuffer.add_metronome_click", &MidiEventBuffer::add_metronome_click);

	kaba::declare_class_size("MidiNoteBuffer", sizeof(MidiNoteBuffer));
	kaba::declare_class_element("MidiNoteBuffer.samples", &MidiNoteBuffer::samples);
	kaba::link_external_class_func("MidiNoteBuffer.__init__", &MidiNoteBuffer::__init__);
	kaba::link_external_class_func("MidiNoteBuffer.__delete__", &MidiNoteBuffer::__delete__);
	kaba::link_external_class_func("MidiNoteBuffer.get_events", &MidiNoteBuffer::get_events);
	kaba::link_external_class_func("MidiNoteBuffer.get_notes", &MidiNoteBuffer::get_notes);
	kaba::link_external_class_func("MidiNoteBuffer.get_range", &MidiNoteBuffer::range);

	{
		BeatSource bsource;
		kaba::declare_class_size("BeatSource", sizeof(BeatSource));
		kaba::link_external_class_func("BeatSource.__init__", &BeatSource::__init__);
		kaba::link_external_virtual("BeatSource.__delete__", &BeatSource::__delete__, &bsource);
		kaba::link_external_virtual("BeatSource.read", &BeatSource::read, &bsource);
		kaba::link_external_virtual("BeatSource.reset", &BeatSource::reset, &bsource);
		kaba::link_external_virtual("BeatSource.beats_per_bar", &BeatSource::beats_per_bar, &bsource);
		kaba::link_external_virtual("BeatSource.cur_beat", &BeatSource::cur_beat, &bsource);
		kaba::link_external_virtual("BeatSource.cur_bar", &BeatSource::cur_bar, &bsource);
		kaba::link_external_virtual("BeatSource.beat_fraction", &BeatSource::beat_fraction, &bsource);
	}

	kaba::declare_class_size("TrackMarker", sizeof(TrackMarker));
	kaba::declare_class_element("TrackMarker.text", &TrackMarker::text);
	kaba::declare_class_element("TrackMarker.range", &TrackMarker::range);
	kaba::declare_class_element("TrackMarker.fx", &TrackMarker::fx);
	kaba::declare_class_element("TrackMarker." + kaba::IDENTIFIER_SHARED_COUNT, &TrackMarker::_pointer_ref_counter);

	kaba::declare_class_size("TrackLayer", sizeof(TrackLayer));
	kaba::declare_class_element("TrackLayer.type", &TrackLayer::type);
	kaba::declare_class_element("TrackLayer.buffers", &TrackLayer::buffers);
	kaba::declare_class_element("TrackLayer.midi", &TrackLayer::midi);
	kaba::declare_class_element("TrackLayer.samples", &TrackLayer::samples);
	kaba::declare_class_element("TrackLayer.markers", &TrackLayer::markers);
	kaba::declare_class_element("TrackLayer.track", &TrackLayer::track);
	kaba::declare_class_element("TrackLayer." + kaba::IDENTIFIER_SHARED_COUNT, &TrackLayer::_pointer_ref_counter);
	kaba::link_external_class_func("TrackLayer.get_buffers", &TrackLayer::get_buffers);
	kaba::link_external_class_func("TrackLayer.read_buffers", &TrackLayer::read_buffers);
	kaba::link_external_class_func("TrackLayer.edit_buffers", &TrackLayer::edit_buffers);
	kaba::link_external_class_func("TrackLayer.edit_buffers_finish", &TrackLayer::edit_buffers_finish);
	kaba::link_external_class_func("TrackLayer.insert_midi_data", &TrackLayer::insert_midi_data);
	kaba::link_external_class_func("TrackLayer.add_midi_note", &TrackLayer::add_midi_note);
	//kaba::link_external_class_func("TrackLayer.add_midi_notes", &TrackLayer::addMidiNotes);
	kaba::link_external_class_func("TrackLayer.delete_midi_note", &TrackLayer::delete_midi_note);
	kaba::link_external_class_func("TrackLayer.add_sample_ref", &TrackLayer::add_sample_ref);
	kaba::link_external_class_func("TrackLayer.delete_sample_ref", &TrackLayer::delete_sample_ref);
	kaba::link_external_class_func("TrackLayer.edit_sample_ref", &TrackLayer::edit_sample_ref);
	kaba::link_external_class_func("TrackLayer.add_marker", &TrackLayer::add_marker);
	kaba::link_external_class_func("TrackLayer.delete_marker", &TrackLayer::delete_marker);
	kaba::link_external_class_func("TrackLayer.edit_marker", &TrackLayer::edit_marker);

	kaba::declare_class_size("Track", sizeof(Track));
	kaba::declare_class_element("Track.type", &Track::type);
	kaba::declare_class_element("Track.name", &Track::name);
	kaba::declare_class_element("Track.layers", &Track::layers);
	kaba::declare_class_element("Track.volume", &Track::volume);
	kaba::declare_class_element("Track.panning", &Track::panning);
	kaba::declare_class_element("Track.muted", &Track::muted);
	kaba::declare_class_element("Track.fx", &Track::fx);
	kaba::declare_class_element("Track.synth", &Track::synth);
	kaba::declare_class_element("Track.instrument", &Track::instrument);
	kaba::declare_class_element("Track.root", &Track::song);
	kaba::declare_class_element("Track." + kaba::IDENTIFIER_SHARED_COUNT, &Track::_pointer_ref_counter);
	kaba::link_external_class_func("Track.nice_name", &Track::nice_name);
	kaba::link_external_class_func("Track.set_name", &Track::set_name);
	kaba::link_external_class_func("Track.set_muted", &Track::set_muted);
	kaba::link_external_class_func("Track.set_volume", &Track::set_volume);
	kaba::link_external_class_func("Track.set_panning", &Track::set_panning);
	kaba::link_external_class_func("Track.add_effect", &Track::add_effect);
	kaba::link_external_class_func("Track.delete_effect", &Track::delete_effect);
	kaba::link_external_class_func("Track.edit_effect", &Track::edit_effect);
	kaba::link_external_class_func("Track.enable_effect", &Track::enable_effect);
	kaba::link_external_class_func("Track.set_synthesizer", &Track::set_synthesizer);

	{
		Song af(Session::GLOBAL, DEFAULT_SAMPLE_RATE);
		kaba::declare_class_size("Song", sizeof(Song));
		kaba::declare_class_element("Song.filename", &Song::filename);
		kaba::declare_class_element("Song.tag", &Song::tags);
		kaba::declare_class_element("Song.sample_rate", &Song::sample_rate);
		kaba::declare_class_element("Song.tracks", &Song::tracks);
		kaba::declare_class_element("Song.samples", &Song::samples);
	//	kaba::declare_class_element("Song.layers", &Song::layers);
		kaba::declare_class_element("Song.bars", &Song::bars);
		kaba::declare_class_element("Song.secret_data", &Song::secret_data);
		kaba::declare_class_element("Song." + kaba::IDENTIFIER_SHARED_COUNT, &Song::_pointer_ref_counter);
		kaba::link_external_class_func("Song.__init__", &Song::__init__);
		kaba::link_external_virtual("Song.__delete__", &Song::__delete__, &af);
		kaba::link_external_class_func("Song.add_track", &Song::add_track);
		kaba::link_external_class_func("Song.delete_track", &Song::delete_track);
		kaba::link_external_class_func("Song.range", &Song::range);
		kaba::link_external_class_func("Song.layers", &Song::layers);
		kaba::link_external_class_func("Song.add_bar", &Song::add_bar);
		kaba::link_external_class_func("Song.add_pause", &Song::add_pause);
		kaba::link_external_class_func("Song.edit_bar", &Song::edit_bar);
		kaba::link_external_class_func("Song.delete_bar", &Song::delete_bar);
		kaba::link_external_class_func("Song.add_sample", &Song::add_sample);
		kaba::link_external_class_func("Song.delete_sample", &Song::delete_sample);
		kaba::link_external_class_func("Song.time_track", &Song::time_track);
		kaba::link_external_class_func("Song.undo", &Song::undo);
		kaba::link_external_class_func("Song.redo", &Song::redo);
		kaba::link_external_class_func("Song.reset_history", &Song::reset_history);
		kaba::link_external_class_func("Song.begin_action_group", &Song::begin_action_group);
		kaba::link_external_class_func("Song.end_action_group", &Song::end_action_group);
		kaba::link_external_class_func("Song.get_time_str", &Song::get_time_str);
		kaba::link_external_class_func("Song.get_time_str_fuzzy", &Song::get_time_str_fuzzy);
		kaba::link_external_class_func("Song.get_time_str_long", &Song::get_time_str_long);
	}

	{
		SongRenderer sr(nullptr);
		kaba::declare_class_size("SongRenderer", sizeof(SongRenderer));
		kaba::link_external_class_func("SongRenderer.set_range", &SongRenderer::set_range);
		kaba::link_external_class_func("SongRenderer.set_loop", &SongRenderer::set_loop);
		kaba::link_external_class_func("SongRenderer.render", &SongRenderer::render);
		kaba::link_external_class_func("SongRenderer.__init__", &SongRenderer::__init__);
		kaba::link_external_virtual("SongRenderer.__delete__", &SongRenderer::__delete__, &sr);
		kaba::link_external_virtual("SongRenderer.read", &SongRenderer::read, &sr);
		kaba::link_external_virtual("SongRenderer.reset", &SongRenderer::reset_state, &sr);
		kaba::link_external_class_func("SongRenderer.range", &SongRenderer::range);
		kaba::link_external_class_func("SongRenderer.get_pos", &SongRenderer::get_pos);
		kaba::link_external_class_func("SongRenderer.set_pos", &SongRenderer::set_pos);
		kaba::link_external_class_func("SongRenderer.get_beat_source", &SongRenderer::get_beat_source);
	}

	{
		AudioInput input(Session::GLOBAL);
		kaba::declare_class_size("AudioInput", sizeof(AudioInput));
		kaba::declare_class_element("AudioInput.current_buffer", &AudioInput::buffer);
		//kaba::declare_class_element("AudioInput.out", &AudioInput::out);
		kaba::link_external_class_func("AudioInput.__init__", &AudioInput::__init__);
		kaba::link_external_virtual("AudioInput.__delete__", &AudioInput::__delete__, &input);
		kaba::link_external_class_func("AudioInput.start", &AudioInput::start);
		kaba::link_external_class_func("AudioInput.stop",	 &AudioInput::stop);
		kaba::link_external_class_func("AudioInput.is_capturing", &AudioInput::is_capturing);
		kaba::link_external_class_func("AudioInput.sample_rate", &AudioInput::sample_rate);
		kaba::link_external_class_func("AudioInput.samples_recorded", &AudioInput::samples_recorded);
		//kaba::link_external_class_func("AudioInput.set_backup_mode", &AudioInput::set_backup_mode);
	}

	{
		AudioOutput stream(Session::GLOBAL);
		kaba::declare_class_size("AudioOutput", sizeof(AudioOutput));
		kaba::link_external_class_func("AudioOutput.__init__", &AudioOutput::__init__);
		kaba::link_external_virtual("AudioOutput.__delete__", &AudioOutput::__delete__, &stream);
		//kaba::link_external_class_func("AudioOutput.setSource", &AudioStream::setSource);
		kaba::link_external_class_func("AudioOutput.start", &AudioOutput::start);
		kaba::link_external_class_func("AudioOutput.stop", &AudioOutput::stop);
		kaba::link_external_class_func("AudioOutput.is_playing", &AudioOutput::is_playing);
		//kaba::link_external_class_func("AudioOutput.sample_rate", &OutputStream::sample_rate);
		kaba::link_external_class_func("AudioOutput.get_volume", &AudioOutput::get_volume);
		kaba::link_external_class_func("AudioOutput.set_volume", &AudioOutput::set_volume);
		kaba::link_external_class_func("AudioOutput.samples_played", &AudioOutput::samples_played);
		kaba::link_external_virtual("AudioOutput.reset_state", &AudioOutput::reset_state, &stream);
	}
	
	kaba::declare_class_element("AudioAccumulator.samples_skipped", &AudioAccumulator::samples_skipped);
	kaba::declare_class_element("AudioAccumulator.buffer", &AudioAccumulator::buf);

	kaba::declare_class_element("MidiAccumulator.buffer", &MidiAccumulator::buffer);

	{
		SignalChain chain(Session::GLOBAL, "");
		kaba::declare_class_size("SignalChain", sizeof(SignalChain));
		kaba::link_external_class_func("SignalChain.__init__", &SignalChain::__init__);
		kaba::link_external_virtual("SignalChain.__delete__", &SignalChain::__delete__, &chain);
		kaba::link_external_virtual("SignalChain.reset_state", &SignalChain::reset_state, &chain);
		kaba::link_external_virtual("SignalChain.command", &SignalChain::command, &chain);
		kaba::link_external_class_func("SignalChain.__del_override__", &SignalChain::unregister);
		kaba::link_external_class_func("SignalChain.start", &SignalChain::start);
		kaba::link_external_class_func("SignalChain.stop", &SignalChain::stop);
		kaba::link_external_class_func("SignalChain.add", &SignalChain::add);
		kaba::link_external_class_func("SignalChain._add", &SignalChain::_add);
		kaba::link_external_class_func("SignalChain.delete", &SignalChain::delete_module);
		kaba::link_external_class_func("SignalChain.connect", &SignalChain::connect);
		kaba::link_external_class_func("SignalChain.disconnect", &SignalChain::disconnect);
		kaba::link_external_class_func("SignalChain.set_update_dt", &SignalChain::set_tick_dt);
		kaba::link_external_class_func("SignalChain.set_buffer_size", &SignalChain::set_buffer_size);
		kaba::link_external_class_func("SignalChain.is_prepared", &SignalChain::is_prepared);
		kaba::link_external_class_func("SignalChain.is_active", &SignalChain::is_active);
	}

	kaba::declare_class_size("AudioView", sizeof(AudioView));
	kaba::declare_class_element("AudioView.cam", &AudioView::cam);
	kaba::declare_class_element("AudioView.sel", &AudioView::sel);
	kaba::declare_class_element("AudioView.mouse_wheel_speed", &AudioView::mouse_wheel_speed);
	kaba::declare_class_element("AudioView.renderer", &AudioView::renderer);
	kaba::declare_class_element("AudioView.signal_chain", &AudioView::signal_chain);
	kaba::declare_class_element("AudioView.output_stream", &AudioView::output_stream);
	kaba::link_external_class_func("AudioView.subscribe", &ObservableKabaWrapper<AudioView>::subscribe_kaba);
	kaba::link_external_class_func("AudioView.unsubscribe", &AudioView::unsubscribe);
	kaba::link_external_class_func("AudioView.play", &AudioView::play);
	kaba::link_external_class_func("AudioView.is_playback_active", &AudioView::is_playback_active);
	kaba::link_external_class_func("AudioView.is_paused", &AudioView::is_paused);
	kaba::link_external_class_func("AudioView.playback_pos", &AudioView::playback_pos);
	kaba::link_external_class_func("AudioView.set_playback_pos", &AudioView::set_playback_pos);
	kaba::link_external_class_func("AudioView.prepare_playback", &AudioView::prepare_playback);
	kaba::link_external_class_func("AudioView.set_playback_loop", &AudioView::set_playback_loop);
	kaba::link_external_class_func("AudioView.optimize_view", &AudioView::request_optimize_view);
	kaba::link_external_class_func("AudioView.cur_vlayer", &AudioView::cur_vlayer);
	kaba::link_external_class_func("AudioView.cur_vtrack", &AudioView::cur_vtrack);
	kaba::link_external_class_func("AudioView.update_selection", &AudioView::update_selection);

	kaba::declare_class_size("SceneGraph.Node", sizeof(scenegraph::Node));
	kaba::declare_class_element("SceneGraph.Node.area", &scenegraph::Node::area);
	kaba::declare_class_element("SceneGraph.Node." + kaba::IDENTIFIER_SHARED_COUNT, &scenegraph::Node::_pointer_ref_counter);

	kaba::declare_class_size("AudioView.Layer", sizeof(AudioViewLayer));
	kaba::declare_class_element("AudioView.Layer.layer", &AudioViewLayer::layer);

	kaba::declare_class_size("AudioView.ViewPort", sizeof(ViewPort));
	kaba::declare_class_element("AudioView.ViewPort.area", &ViewPort::area);
	kaba::link_external_class_func("AudioView.ViewPort.__init__", &ViewPort::__init__);
	kaba::link_external_class_func("AudioView.ViewPort.range", &ViewPort::range);
	kaba::link_external_class_func("AudioView.ViewPort.set_range", &ViewPort::set_range);
	kaba::link_external_class_func("AudioView.ViewPort.sample2screen64", &ViewPort::sample2screen);
	kaba::link_external_class_func("AudioView.ViewPort.screen2sample64", &ViewPort::screen2sample);
	kaba::link_external_class_func("AudioView.ViewPort.sample2screen", &ViewPort::sample2screen_f);
	kaba::link_external_class_func("AudioView.ViewPort.screen2sample", &ViewPort::screen2sample_f);

	kaba::declare_class_size("ColorScheme", sizeof(ColorScheme));
	kaba::declare_class_element("ColorScheme.background", &ColorScheme::background);
	kaba::declare_class_element("ColorScheme.background_track", &ColorScheme::background_track);
	kaba::declare_class_element("ColorScheme.background_track_selected", &ColorScheme::background_track_selected);
	kaba::declare_class_element("ColorScheme.text", &ColorScheme::text);
	kaba::declare_class_element("ColorScheme.text_soft1", &ColorScheme::text_soft1);
	kaba::declare_class_element("ColorScheme.text_soft2", &ColorScheme::text_soft2);
	kaba::declare_class_element("ColorScheme.text_soft3", &ColorScheme::text_soft3);
	kaba::declare_class_element("ColorScheme.grid", &ColorScheme::grid);
	kaba::declare_class_element("ColorScheme.selection", &ColorScheme::selection);
	kaba::declare_class_element("ColorScheme.hover", &ColorScheme::hover);
	kaba::declare_class_element("ColorScheme.blob_bg", &ColorScheme::blob_bg);
	kaba::declare_class_element("ColorScheme.blob_bg_selected", &ColorScheme::blob_bg_selected);
	kaba::declare_class_element("ColorScheme.blob_bg_hidden", &ColorScheme::blob_bg_hidden);
	kaba::declare_class_element("ColorScheme.pitch", &ColorScheme::pitch);
	kaba::link_external_class_func("ColorScheme.hoverify", &ColorScheme::hoverify);
	kaba::link_external("ColorScheme.pitch_color", (void*)&ColorScheme::pitch_color);

	kaba::link_external_class_func("Storage.load", &Storage::load);
	kaba::link_external_class_func("Storage.save", &Storage::save);
	kaba::link_external_class_func("Storage.save_via_renderer", &Storage::save_via_renderer);
	kaba::link_external_class_func("Storage.load_buffer", &Storage::load_buffer);
	kaba::declare_class_element("Storage.current_directory", &Storage::current_directory);


	kaba::declare_class_size("SongSelection", sizeof(SongSelection));
	kaba::declare_class_element("SongSelection.range_raw", &SongSelection::range_raw);
	kaba::link_external_class_func("SongSelection.range", &SongSelection::range);
	kaba::link_external_class_func("SongSelection.has_track", (bool (SongSelection::*)(const Track*) const)&SongSelection::has);
	kaba::link_external_class_func("SongSelection.has_layer", (bool (SongSelection::*)(const TrackLayer*) const)&SongSelection::has);
	kaba::link_external_class_func("SongSelection.has_marker", (bool (SongSelection::*)(const TrackMarker*) const)&SongSelection::has);
	kaba::link_external_class_func("SongSelection.has_note",  (bool (SongSelection::*)(const MidiNote*) const)&SongSelection::has);
	kaba::link_external_class_func("SongSelection.has_bar",  (bool (SongSelection::*)(const Bar*) const)&SongSelection::has);


	kaba::declare_class_size("MidiPainter", sizeof(MidiPainter));
	kaba::declare_class_element("MidiPainter.cam", &MidiPainter::cam);
	kaba::link_external_class_func("MidiPainter.__init__", &MidiPainter::__init__);
	kaba::link_external_class_func("MidiPainter.set_context", &MidiPainter::set_context);
	kaba::link_external_class_func("MidiPainter.draw", &MidiPainter::draw);
	kaba::link_external_class_func("MidiPainter.draw_background", &MidiPainter::draw_background);
	//kaba::link_external_class_func("MidiPainter.pitch_color", &MidiPainter::pitch_color);


	kaba::declare_class_size("GridPainter", sizeof(GridPainter));
	kaba::link_external_class_func("GridPainter.__init__", &GridPainter::__init__);
	kaba::link_external_class_func("GridPainter.set_context", &GridPainter::set_context);
	kaba::link_external_class_func("GridPainter.draw_empty_background", &GridPainter::draw_empty_background);
	kaba::link_external_class_func("GridPainter.draw_bars", &GridPainter::draw_bars);
	kaba::link_external_class_func("GridPainter.draw_bar_numbers", &GridPainter::draw_bar_numbers);
	kaba::link_external_class_func("GridPainter.draw_time", &GridPainter::draw_time);
	kaba::link_external_class_func("GridPainter.draw_time_numbers", &GridPainter::draw_time_numbers);

	kaba::declare_class_size("MultiLinePainter", sizeof(MultiLinePainter));
	kaba::link_external_class_func("MultiLinePainter.__init__", &MultiLinePainter::__init__);
	kaba::link_external_class_func("MultiLinePainter.__delete__", &MultiLinePainter::__delete__);
	kaba::link_external_class_func("MultiLinePainter.set_context", &MultiLinePainter::set_context);
	kaba::link_external_class_func("MultiLinePainter.set", &MultiLinePainter::set);
	kaba::link_external_class_func("MultiLinePainter.draw_next_line", &MultiLinePainter::draw_next_line);
	kaba::link_external_class_func("MultiLinePainter.next_line_samples", &MultiLinePainter::next_line_samples);
	kaba::link_external_class_func("MultiLinePainter.get_line_dy", &MultiLinePainter::get_line_dy);

	{
		Slider slider;
		kaba::declare_class_size("Slider", sizeof(Slider));
		kaba::link_external_class_func("Slider.__init__", &Slider::__init_ext__);
		kaba::link_external_virtual("Slider.__delete__", &Slider::__delete__, &slider);
		kaba::link_external_class_func("Slider.get", &Slider::get);
		kaba::link_external_class_func("Slider.set", &Slider::set);
	}

	{
		TsunamiPlugin tsunami_plugin;
		kaba::declare_class_size("TsunamiPlugin", sizeof(TsunamiPlugin));
		//kaba::declare_class_element("TsunamiPlugin.session", &TsunamiPlugin, session);
		kaba::declare_class_element("TsunamiPlugin.args", &TsunamiPlugin::args);
		kaba::link_external_class_func("TsunamiPlugin.__init__", &TsunamiPlugin::__init__);
		kaba::link_external_virtual("TsunamiPlugin.__delete__", &TsunamiPlugin::__delete__, &tsunami_plugin);
		kaba::link_external_virtual("TsunamiPlugin.on_start", &TsunamiPlugin::on_start, &tsunami_plugin);
		kaba::link_external_virtual("TsunamiPlugin.on_stop", &TsunamiPlugin::on_stop, &tsunami_plugin);
		kaba::link_external_virtual("TsunamiPlugin.on_draw_post", &TsunamiPlugin::on_draw_post, &tsunami_plugin);
		kaba::link_external_class_func("TsunamiPlugin.stop", &TsunamiPlugin::stop_request);
	}

	{
		Progress *prog = new Progress("", nullptr);
		kaba::declare_class_size("Progress", sizeof(Progress));
		kaba::link_external_class_func("Progress.__init__", &Progress::__init__);
		kaba::link_external_virtual("Progress.__delete__", &Progress::__delete__, prog);
		kaba::link_external_class_func("Progress.set", &Progress::set_kaba);
		delete prog;
	}

	{
		ProgressCancelable prog_can;
		kaba::declare_class_size("ProgressX", sizeof(ProgressCancelable));
		kaba::link_external_class_func("ProgressX.__init__", &ProgressCancelable::__init__);
		kaba::link_external_virtual("ProgressX.__delete__", &ProgressCancelable::__delete__, &prog_can);
		kaba::link_external_class_func("ProgressX.set", &ProgressCancelable::set_kaba);
		kaba::link_external_class_func("ProgressX.cancel", &ProgressCancelable::cancel);
		kaba::link_external_class_func("ProgressX.is_cancelled", &ProgressCancelable::is_cancelled);
	}
}

kaba::Class* PluginManager::get_class(const string &name) {
	return (kaba::Class*)package->syntax->make_class(name, kaba::Class::Type::OTHER, 0, 0, nullptr, {}, package->syntax->base_class, -1);
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
		dir <<= group;
	auto list = os::fs::search(dir, "*.kaba", "f");
	for (auto &e: list) {
		PluginManager::PluginFile pf;
		pf.type = type;
		pf.name = e.no_ext().str();
		pf.filename = dir << e;
		pf.group = group;
		get_plugin_file_data(pf);
		plugin_files.add(pf);
	}
}

void PluginManager::find_plugins_in_dir(const Path &rel, const string &group, ModuleCategory type) {
	find_plugins_in_dir_absolute(plugin_dir_static() << rel, group, type);
	if (plugin_dir_local() != plugin_dir_static())
		find_plugins_in_dir_absolute(plugin_dir_local() << rel, group, type);
}

void PluginManager::add_plugins_in_dir(const Path &dir, hui::Menu *m, const string &name_space, TsunamiWindow *win, PluginCallback cb) {
	for (auto &f: plugin_files) {
		if (f.filename.is_in(plugin_dir_static() << dir) or f.filename.is_in(plugin_dir_local() << dir)) {
			string id = "execute-" + name_space + "--" + f.name;
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

void PluginManager::apply_profile(Module *c, const string &name, bool notify) {
	profiles->apply(c, name, notify);
}

void PluginManager::save_profile(Module *c, const string &name) {
	profiles->save(c, name);
}


void PluginManager::select_profile_name(hui::Window *win, Module *c, bool save, std::function<void(const string&)> cb) {
	profiles->select_name(win, c, save, cb);
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
		return tsunami->directory_static << "Plugins";
	return "Plugins";
}

Path PluginManager::plugin_dir_local() {
	if (tsunami->installed)
		return tsunami->directory << "Plugins";
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

void PluginManager::choose_module(hui::Panel *parent, Session *session, ModuleCategory type, std::function<void(const string&)> cb, const string &old_name) {
	auto names = session->plugin_manager->find_module_sub_types(type);
	if (names.num == 1) {
		cb(names[0]);
		return;
	}
	if (names.num == 0) {
		cb("");
		return;
	}

	auto *dlg = new ModuleSelectorDialog(parent->win, type, session, old_name);
	hui::fly(dlg, [dlg, cb] {
		cb(dlg->_return);
	});
}


