/*
 * PluginManager.cpp
 *
 *  Created on: 26.03.2012
 *      Author: michi
 */

#include "PluginManager.h"

#include "../Tsunami.h"
#include "../TsunamiWindow.h"
#include "../Session.h"
#include "FastFourierTransform.h"
#include "../View/Helper/Slider.h"
#include "../Data/base.h"
#include "../Data/Song.h"
#include "../Data/SongSelection.h"
#include "../Data/Track.h"
#include "../Data/Midi/Instrument.h"
#include "../Data/TrackLayer.h"
#include "../Data/TrackMarker.h"
#include "../Data/Sample.h"
#include "../Data/SampleRef.h"
#include "../Data/Audio/AudioBuffer.h"
#include "../Data/Audio/BufferInterpolator.h"
#include "../Data/Rhythm/Bar.h"
#include "../Module/ModuleFactory.h"
#include "../Module/ConfigPanel.h"
#include "../Module/SignalChain.h"
#include "../Module/ModuleConfiguration.h"
#include "../Module/Synth/Synthesizer.h"
#include "../Module/Synth/DummySynthesizer.h"
#include "../Module/Port/Port.h"
#include "../Module/Audio/AudioSource.h"
#include "../Module/Audio/SongRenderer.h"
#include "../Module/Audio/AudioVisualizer.h"
#include "../Module/Audio/PitchDetector.h"
#include "../Module/Audio/AudioRecorder.h"
#include "../Module/Midi/MidiSource.h"
#include "../Module/Audio/AudioEffect.h"
#include "../Module/Beats/BeatSource.h"
#include "../Module/Beats/BeatMidifier.h"
#include "../Module/Midi/MidiEffect.h"
#include "../View/Helper/Progress.h"
#include "../Storage/Storage.h"
#include "../Device/DeviceManager.h"
#include "../Device/Stream/AudioInput.h"
#include "../Device/Stream/AudioOutput.h"
#include "../Device/Stream/MidiInput.h"
#include "../Stuff/Clipboard.h"
#include "../View/AudioView.h"
#include "../View/Dialog/ConfigurableSelectorDialog.h"
#include "../View/SideBar/SampleManagerConsole.h"
#include "../View/Mode/ViewModeCapture.h"
#include "../View/Painter/MidiPainter.h"
#include "../View/Painter/GridPainter.h"
#include "Plugin.h"
#include "ExtendedAudioBuffer.h"
#include "SongPlugin.h"
#include "TsunamiPlugin.h"
#include "FavoriteManager.h"

namespace Kaba{
	extern Array<Script*> _public_scripts_;
};

#define _offsetof(CLASS, ELEMENT) (int)( (char*)&((CLASS*)1)->ELEMENT - (char*)((CLASS*)1) )

PluginManager::PluginManager() {
	favorites = new FavoriteManager;

	find_plugins();

	package = new Kaba::Script;
	package->filename = "-tsunami-internal-";
	package->used_by_default = false;
	Kaba::Packages.add(package);
	Kaba::_public_scripts_.add(package);

	auto *type_dev = package->syntax->make_class("Device", Kaba::Class::Type::OTHER, 0, 0, nullptr, nullptr, package->syntax->base_class);
	package->syntax->make_class("Device*", Kaba::Class::Type::POINTER, sizeof(void*), 0, nullptr, type_dev, package->syntax->base_class);
}

PluginManager::~PluginManager() {
	delete favorites;
	Kaba::clean_up();
}


Module *_CreateBeatMidifier(Session *s) {
	return new BeatMidifier();
}

template<class T>
class ObservableKabaWrapper : public T {
public:
	void _cdecl subscribe_kaba(hui::EventHandler *handler, Kaba::Function *f, const string &message) {
		auto *ff = (hui::kaba_member_callback*)f->address;
		T::subscribe(handler, [=] { ff(handler); }, message);
	}
};


void PluginManager::link_app_script_data() {
	Kaba::config.directory = "";

	// api definition
	Kaba::link_external("device_manager", &tsunami->device_manager);
	Kaba::link_external("colors", &AudioView::colors);
	Kaba::link_external("clipboard", &tsunami->clipboard);
	//Kaba::link_external("view_input", &export_view_input);
	Kaba::link_external("db2amp", (void*)&db2amplitude);
	Kaba::link_external("amp2db", (void*)&amplitude2db);
	Kaba::link_external("fft_c2c", (void*)&FastFourierTransform::fft_c2c);
	Kaba::link_external("fft_r2c", (void*)&FastFourierTransform::fft_r2c);
	Kaba::link_external("fft_c2r_inv", (void*)&FastFourierTransform::fft_c2r_inv);
	Kaba::link_external("CreateModule", (void*)&ModuleFactory::create);
	Kaba::link_external("CreateSynthesizer", (void*)&CreateSynthesizer);
	Kaba::link_external("CreateAudioEffect", (void*)&CreateAudioEffect);
	Kaba::link_external("CreateAudioSource", (void*)&CreateAudioSource);
	Kaba::link_external("CreateMidiEffect", (void*)&CreateMidiEffect);
	Kaba::link_external("CreateMidiSource", (void*)&CreateMidiSource);
	Kaba::link_external("CreateBeatSource", (void*)&CreateBeatSource);
	Kaba::link_external("CreateBeatMidifier", (void*)&_CreateBeatMidifier);
	Kaba::link_external("SelectSample", (void*)&SampleManagerConsole::select);
	Kaba::link_external("ChooseModule", (void*)&choose_module);
	Kaba::link_external("draw_boxed_str", (void*)&AudioView::draw_boxed_str);
	Kaba::link_external("interpolate_buffer", (void*)&BufferInterpolator::interpolate);

	Kaba::declare_class_size("Clipboard", sizeof(Clipboard));
	Kaba::declare_class_element("Clipboard.temp", &Clipboard::temp);
	Kaba::link_external_class_func("Clipboard.has_data", &Clipboard::has_data);
	Kaba::link_external_class_func("Clipboard.prepare_layer_map", &Clipboard::prepare_layer_map);

	Kaba::declare_class_size("Range", sizeof(Range));
	Kaba::declare_class_element("Range.offset", &Range::offset);
	Kaba::declare_class_element("Range.length", &Range::length);

	Kaba::declare_class_size("Bar", sizeof(Bar));
	Kaba::declare_class_element("Bar.beats", &Bar::beats);
	Kaba::declare_class_element("Bar.divisor", &Bar::divisor);
	Kaba::declare_class_element("Bar.length", &Bar::length);
	Kaba::declare_class_element("Bar.index", &Bar::index);
	Kaba::declare_class_element("Bar.index_text", &Bar::index_text);
	Kaba::declare_class_element("Bar.offset", &Bar::offset);
	Kaba::link_external_class_func("Bar.range", &Bar::range);
	Kaba::link_external_class_func("Bar.bpm", &Bar::bpm);

	Kaba::declare_class_size("Session", sizeof(Session));
	Kaba::declare_class_element("Session.id", &Session::id);
	Kaba::declare_class_element("Session.storage", &Session::storage);
	Kaba::declare_class_element("Session.win", &Session::_kaba_win);
	Kaba::declare_class_element("Session.view", &Session::view);
	Kaba::declare_class_element("Session.song", &Session::song);
	Kaba::link_external_class_func("Session.sample_rate", &Session::sample_rate);
	Kaba::link_external_class_func("Session.i", &Session::i);
	Kaba::link_external_class_func("Session.w", &Session::w);
	Kaba::link_external_class_func("Session.e", &Session::e);
	Kaba::link_external_class_func("Session.add_signal_chain", &Session::add_signal_chain);
	Kaba::link_external_class_func("Session.create_child", &Session::create_child);


	Module module(ModuleType::AUDIO_EFFECT, "");
	Kaba::declare_class_size("Module", sizeof(Module));
	Kaba::declare_class_element("Module.name", &Module::module_subtype);
	Kaba::declare_class_element("Module.session", &Module::session);
	Kaba::link_external_class_func("Module.__init__", &Module::__init__);
	Kaba::link_external_virtual("Module.__delete__", &Module::__delete__, &module);
	Kaba::link_external_virtual("Module.create_panel", &Module::create_panel, &module);
	Kaba::link_external_class_func("Module.reset_config", &Module::reset_config);
	Kaba::link_external_virtual("Module.reset_state", &Module::reset_state, &module);
	Kaba::link_external_class_func("Module.changed", &Module::changed);
	Kaba::link_external_virtual("Module.get_config", &Module::get_config, &module);
	Kaba::link_external_class_func("Module.config_to_string", &Module::config_to_string);
	Kaba::link_external_class_func("Module.config_from_string", &Module::config_from_string);
	Kaba::link_external_virtual("Module.on_config", &Module::on_config, &module);
	Kaba::link_external_virtual("Module.command", &Module::command, &module);
	Kaba::link_external_class_func("Module.subscribe", &ObservableKabaWrapper<Module>::subscribe_kaba);
	Kaba::link_external_class_func("Module.unsubscribe", &Module::unsubscribe);
	Kaba::link_external_class_func("Module.copy", &Module::copy);
	Kaba::link_external_class_func("Module.plug_in", &Module::_plug_in);
	Kaba::link_external_class_func("Module.unplug_in", &Module::_unplug_in);


	ModuleConfiguration plugin_data;
	Kaba::declare_class_size("Module.Config", sizeof(ModuleConfiguration));
	Kaba::link_external_class_func("Module.Config.__init__", &ModuleConfiguration::__init__);
	Kaba::link_external_virtual("Module.Config.__delete__", &ModuleConfiguration::__delete__, &plugin_data);
	Kaba::link_external_virtual("Module.Config.reset", &ModuleConfiguration::reset, &plugin_data);
	Kaba::link_external_virtual("Module.Config.from_string", &ModuleConfiguration::from_string, &plugin_data);
	Kaba::link_external_virtual("Module.Config.to_string", &ModuleConfiguration::to_string, &plugin_data);
	Kaba::link_external_virtual("Module.Config.auto_conf", &ModuleConfiguration::auto_conf, &plugin_data);


	ConfigPanel config_panel(nullptr);
	Kaba::declare_class_size("ConfigPanel", sizeof(ConfigPanel));
	Kaba::link_external_class_func("ConfigPanel.__init__", &ConfigPanel::__init__);
	Kaba::link_external_virtual("ConfigPanel.__delete__", &ConfigPanel::__delete__, &config_panel);
	Kaba::link_external_virtual("ConfigPanel.update", &ConfigPanel::update, &config_panel);
	Kaba::link_external_virtual("ConfigPanel.set_large", &ConfigPanel::set_large, &config_panel);
	Kaba::link_external_class_func("ConfigPanel.changed", &ConfigPanel::changed);
	Kaba::declare_class_element("ConfigPanel.c", &ConfigPanel::c);

	AudioSource asource;
	Kaba::declare_class_size("AudioSource", sizeof(AudioSource));
	Kaba::link_external_class_func("AudioSource.__init__", &AudioSource::__init__);
	Kaba::link_external_virtual("AudioSource.__delete__", &AudioSource::__delete__, &asource);
	Kaba::link_external_virtual("AudioSource.read", &AudioSource::read, &asource);

	AudioEffect aeffect;
	Kaba::declare_class_size("AudioEffect", sizeof(AudioEffect));
	Kaba::declare_class_element("AudioEffect.sample_rate", &AudioEffect::sample_rate);
	Kaba::declare_class_element("AudioEffect.out", &AudioEffect::out);
	Kaba::declare_class_element("AudioEffect.source", &AudioEffect::source);
	Kaba::link_external_class_func("AudioEffect.__init__", &AudioEffect::__init__);
	Kaba::link_external_virtual("AudioEffect.__delete__", &AudioEffect::__delete__, &aeffect);
	Kaba::link_external_virtual("AudioEffect.process", &AudioEffect::process, &aeffect);
	Kaba::link_external_virtual("AudioEffect.read", &AudioEffect::read, &aeffect);

	MidiEffect meffect;
	Kaba::declare_class_size("MidiEffect", sizeof(MidiEffect));
	Kaba::declare_class_element("MidiEffect.only_on_selection", &MidiEffect::only_on_selection);
	Kaba::declare_class_element("MidiEffect.range", &MidiEffect::range);
	Kaba::link_external_class_func("MidiEffect.__init__", &MidiEffect::__init__);
	Kaba::link_external_virtual("MidiEffect.__delete__", &MidiEffect::__delete__, &meffect);
	Kaba::link_external_virtual("MidiEffect.process", &MidiEffect::process, &meffect);

	AudioVisualizer avis;
	Kaba::declare_class_size("AudioVisualizer", sizeof(AudioVisualizer));
	Kaba::declare_class_element("AudioVisualizer.chunk_size", &AudioVisualizer::chunk_size);
	Kaba::link_external_class_func("AudioVisualizer.__init__", &AudioVisualizer::__init__);
	Kaba::link_external_virtual("AudioVisualizer.__delete__", &AudioVisualizer::__delete__, &avis);
	Kaba::link_external_virtual("AudioVisualizer.process", &AudioVisualizer::process, &avis);
	Kaba::link_external_class_func("AudioVisualizer.set_chunk_size", &AudioVisualizer::set_chunk_size);

	//PitchDetector pd;
	Kaba::declare_class_size("PitchDetector", sizeof(PitchDetector));
	Kaba::declare_class_element("PitchDetector.frequency", &PitchDetector::frequency);
	Kaba::declare_class_element("PitchDetector.volume", &PitchDetector::volume);
	Kaba::declare_class_element("PitchDetector.loud_enough", &PitchDetector::loud_enough);

	Kaba::declare_class_size("AudioBuffer", sizeof(AudioBuffer));
	Kaba::declare_class_element("AudioBuffer.offset", &AudioBuffer::offset);
	Kaba::declare_class_element("AudioBuffer.length", &AudioBuffer::length);
	Kaba::declare_class_element("AudioBuffer.channels", &AudioBuffer::channels);
	Kaba::declare_class_element("AudioBuffer.c", &AudioBuffer::c);
	Kaba::_declare_class_element("AudioBuffer.l", _offsetof(AudioBuffer, c[0]));
	Kaba::_declare_class_element("AudioBuffer.r", _offsetof(AudioBuffer, c[1]));
	Kaba::declare_class_element("AudioBuffer.peaks", &AudioBuffer::peaks);
	Kaba::link_external_class_func("AudioBuffer.__init__", &AudioBuffer::__init__);
	Kaba::link_external_class_func("AudioBuffer.__delete__", &AudioBuffer::__delete__);
	Kaba::link_external_class_func("AudioBuffer.clear", &AudioBuffer::clear);
	Kaba::link_external_class_func("AudioBuffer.__assign__", &AudioBuffer::__assign__);
	Kaba::link_external_class_func("AudioBuffer.range", &AudioBuffer::range);
	Kaba::link_external_class_func("AudioBuffer.resize", &AudioBuffer::resize);
	Kaba::link_external_class_func("AudioBuffer.add", &AudioBuffer::add);
	Kaba::link_external_class_func("AudioBuffer.set", &AudioBuffer::set);
	Kaba::link_external_class_func("AudioBuffer.set_as_ref", &AudioBuffer::set_as_ref);
	Kaba::link_external_class_func("AudioBuffer." + Kaba::IDENTIFIER_FUNC_SUBARRAY, &AudioBuffer::ref);
	Kaba::link_external_class_func("AudioBuffer.get_spectrum", &ExtendedAudioBuffer::get_spectrum);


	Kaba::declare_class_size("RingBuffer", sizeof(RingBuffer));
	Kaba::link_external_class_func("RingBuffer.__init__", &RingBuffer::__init__);
	Kaba::link_external_class_func("RingBuffer.__delete__", &RingBuffer::__delete__);
	Kaba::link_external_class_func("RingBuffer.available", &RingBuffer::available);
	Kaba::link_external_class_func("RingBuffer.read", &RingBuffer::read);
	Kaba::link_external_class_func("RingBuffer.write", &RingBuffer::write);
	Kaba::link_external_class_func("RingBuffer.read_ref", &RingBuffer::read_ref);
	Kaba::link_external_class_func("RingBuffer.read_ref_done", &RingBuffer::read_ref_done);
	Kaba::link_external_class_func("RingBuffer.peek_ref", &RingBuffer::peek_ref);
	Kaba::link_external_class_func("RingBuffer.write_ref", &RingBuffer::write_ref);
	Kaba::link_external_class_func("RingBuffer.write_ref_done", &RingBuffer::write_ref_done);
//	Kaba::link_external_class_func("RingBuffer.move_read_pos", &RingBuffer::move_read_pos);
//	Kaba::link_external_class_func("RingBuffer.move_write_pos", &RingBuffer::move_write_pos);
	Kaba::link_external_class_func("RingBuffer.clear", &RingBuffer::clear);

	Kaba::declare_class_size("Sample", sizeof(Sample));
	Kaba::declare_class_element("Sample.name", &Sample::name);
	Kaba::declare_class_element("Sample.type", &Sample::type);
	Kaba::declare_class_element("Sample.buf", &Sample::buf);
	Kaba::declare_class_element("Sample.midi", &Sample::midi);
	Kaba::declare_class_element("Sample.volume", &Sample::volume);
	Kaba::declare_class_element("Sample.uid", &Sample::uid);
	Kaba::declare_class_element("Sample.tags", &Sample::tags);
	Kaba::link_external_class_func("Sample.create_ref", &Sample::create_ref);
	Kaba::link_external_class_func("Sample.get_value", &Sample::get_value);
	Kaba::link_external_class_func("Sample.set_value", &Sample::set_value);

	Sample sample(SignalType::AUDIO);
	sample._pointer_ref(); // stack allocated... don't auto-delete!
	//sample.owner = tsunami->song;
	SampleRef sampleref(&sample);
	Kaba::declare_class_size("SampleRef", sizeof(SampleRef));
	Kaba::link_external_class_func("SampleRef.buf", &SampleRef::buf);
	Kaba::link_external_class_func("SampleRef.midi", &SampleRef::midi);
	Kaba::declare_class_element("SampleRef.origin", &SampleRef::origin);
	Kaba::link_external_class_func("SampleRef.__init__", &SampleRef::__init__);
	Kaba::link_external_virtual("SampleRef.__delete__", &SampleRef::__delete__, &sampleref);



	Port port(SignalType::AUDIO, "");
	Kaba::declare_class_size("Module.Port", sizeof(Port));
	Kaba::link_external_class_func("Module.Port.__init__", &Port::__init__);
	Kaba::link_external_virtual("Module.Port.__delete__", &Port::__delete__, &port);
	Kaba::link_external_virtual("Module.Port.read_audio", &Port::read_audio, &port);
	Kaba::link_external_virtual("Module.Port.read_midi", &Port::read_midi, &port);
	Kaba::link_external_virtual("Module.Port.read_beats", &Port::read_beats, &port);

	MidiSource msource;
	Kaba::declare_class_size("MidiSource", sizeof(MidiSource));
	Kaba::declare_class_element("MidiSource.bh_midi", &MidiSource::bh_midi);
	Kaba::link_external_class_func("MidiSource.__init__", &MidiSource::__init__);
	Kaba::link_external_virtual("MidiSource.__delete__", &MidiSource::__delete__, &msource);
	Kaba::link_external_virtual("MidiSource.read", &MidiSource::read, &msource);
	Kaba::link_external_virtual("MidiSource.reset", &MidiSource::reset, &msource);
	Kaba::link_external_class_func("MidiSource.note", &MidiSource::note);
	Kaba::link_external_class_func("MidiSource.skip", &MidiSource::skip);
	Kaba::link_external_class_func("MidiSource.note_x", &MidiSource::note_x);
	Kaba::link_external_class_func("MidiSource.skip_x", &MidiSource::skip_x);


	BeatMidifier bmidifier;
	Kaba::declare_class_size("BeatMidifier", sizeof(BeatMidifier));
	Kaba::link_external_class_func("BeatMidifier.__init__", &BeatMidifier::__init__);
	//Kaba::link_external_virtual("BeatMidifier.__delete__", &MidiSource::__delete__, &bmidifier);
	Kaba::link_external_virtual("BeatMidifier.read", &BeatMidifier::read, &bmidifier);
	Kaba::link_external_virtual("BeatMidifier.reset", &BeatMidifier::reset, &bmidifier);
	Kaba::declare_class_element("BeatMidifier.volume", &BeatMidifier::volume);

	Synthesizer synth;
	Kaba::declare_class_size("Synthesizer", sizeof(Synthesizer));
	Kaba::declare_class_element("Synthesizer.sample_rate", &Synthesizer::sample_rate);
	Kaba::declare_class_element("Synthesizer.events", &Synthesizer::events);
	Kaba::declare_class_element("Synthesizer.keep_notes", &Synthesizer::keep_notes);
	Kaba::declare_class_element("Synthesizer.active_pitch", &Synthesizer::active_pitch);
	Kaba::_declare_class_element("Synthesizer.freq", _offsetof(Synthesizer, tuning.freq));
	Kaba::declare_class_element("Synthesizer.delta_phi", &Synthesizer::delta_phi);
	Kaba::declare_class_element("Synthesizer.out", &Synthesizer::out);
	Kaba::declare_class_element("Synthesizer.auto_generate_stereo", &Synthesizer::auto_generate_stereo);
	Kaba::link_external_class_func("Synthesizer.__init__", &Synthesizer::__init__);
	Kaba::link_external_virtual("Synthesizer.__delete__", &Synthesizer::__delete__, &synth);
	Kaba::link_external_virtual("Synthesizer.render", &Synthesizer::render, &synth);
	Kaba::link_external_virtual("Synthesizer.create_pitch_renderer", &Synthesizer::create_pitch_renderer, &synth);
	Kaba::link_external_virtual("Synthesizer.on_config", &Synthesizer::on_config, &synth);
	Kaba::link_external_virtual("Synthesizer.reset_state", &Synthesizer::reset_state, &synth);
	Kaba::link_external_class_func("Synthesizer.set_sample_rate", &Synthesizer::set_sample_rate);

	PitchRenderer pren(&synth, 0);
	Kaba::declare_class_size("PitchRenderer", sizeof(PitchRenderer));
	Kaba::declare_class_element("PitchRenderer.synth", &PitchRenderer::synth);
	Kaba::declare_class_element("PitchRenderer.pitch", &PitchRenderer::pitch);
	Kaba::declare_class_element("PitchRenderer.delta_phi", &PitchRenderer::delta_phi);
	Kaba::link_external_class_func("PitchRenderer.__init__", &PitchRenderer::__init__);
	Kaba::link_external_virtual("PitchRenderer.__delete__", &PitchRenderer::__delete__, &pren);
	Kaba::link_external_virtual("PitchRenderer.render", &PitchRenderer::render, &pren);
	Kaba::link_external_virtual("PitchRenderer.on_start", &PitchRenderer::on_start, &pren);
	Kaba::link_external_virtual("PitchRenderer.on_end", &PitchRenderer::on_end, &pren);
	Kaba::link_external_virtual("PitchRenderer.on_config", &PitchRenderer::on_config, &pren);


	DummySynthesizer dsynth;
	Kaba::declare_class_size("DummySynthesizer", sizeof(DummySynthesizer));
	Kaba::link_external_class_func("DummySynthesizer.__init__", &DummySynthesizer::__init__);
	Kaba::link_external_virtual("DummySynthesizer.__delete__", &DummySynthesizer::__delete__, &dsynth);
	Kaba::link_external_virtual("DummySynthesizer.render", &DummySynthesizer::render, &dsynth);
	Kaba::link_external_virtual("DummySynthesizer.create_pitch_renderer", &DummySynthesizer::create_pitch_renderer, &dsynth);
	Kaba::link_external_virtual("DummySynthesizer.on_config", &DummySynthesizer::on_config, &dsynth);

	Kaba::declare_class_size("EnvelopeADSR", sizeof(EnvelopeADSR));
	Kaba::link_external_class_func("EnvelopeADSR.set", &EnvelopeADSR::set);
	Kaba::link_external_class_func("EnvelopeADSR.set2", &EnvelopeADSR::set2);
	Kaba::link_external_class_func("EnvelopeADSR.reset", &EnvelopeADSR::reset);
	Kaba::link_external_class_func("EnvelopeADSR.start", &EnvelopeADSR::start);
	Kaba::link_external_class_func("EnvelopeADSR.end", &EnvelopeADSR::end);
	Kaba::link_external_class_func("EnvelopeADSR.get", &EnvelopeADSR::get);
	Kaba::link_external_class_func("EnvelopeADSR.read", &EnvelopeADSR::read);
	Kaba::declare_class_element("EnvelopeADSR.just_killed", &EnvelopeADSR::just_killed);

	Kaba::declare_class_size("BarPattern", sizeof(BarPattern));
	Kaba::declare_class_element("BarPattern.beats", &BarPattern::beats);
	Kaba::declare_class_element("BarPattern.divisor", &BarPattern::divisor);
	Kaba::declare_class_element("BarPattern.length", &BarPattern::length);
	//Kaba::declare_class_element("BarPattern.type", &BarPattern::type);
	//Kaba::declare_class_element("BarPattern.count", &BarPattern::count);

	Kaba::declare_class_size("MidiNote", sizeof(MidiNote));
	Kaba::declare_class_element("MidiNote.range", &MidiNote::range);
	Kaba::declare_class_element("MidiNote.pitch", &MidiNote::pitch);
	Kaba::declare_class_element("MidiNote.volume", &MidiNote::volume);
	Kaba::declare_class_element("MidiNote.stringno", &MidiNote::stringno);
	Kaba::declare_class_element("MidiNote.clef_position", &MidiNote::clef_position);
	Kaba::declare_class_element("MidiNote.modifier", &MidiNote::modifier);
	Kaba::declare_class_element("MidiNote.flags", &MidiNote::flags);
	Kaba::link_external_class_func("MidiNote.copy", &MidiNote::copy);
	Kaba::link_external_class_func("MidiNote.is", &MidiNote::is);
	Kaba::link_external_class_func("MidiNote.set", &MidiNote::set);
	
	Kaba::declare_class_size("MidiEvent", sizeof(MidiEvent));
	Kaba::declare_class_element("MidiEvent.pos", &MidiEvent::pos);
	Kaba::declare_class_element("MidiEvent.pitch", &MidiEvent::pitch);
	Kaba::declare_class_element("MidiEvent.volume", &MidiEvent::volume);
	Kaba::declare_class_element("MidiEvent.stringno", &MidiEvent::stringno);
	Kaba::declare_class_element("MidiEvent.clef_position", &MidiEvent::clef_position);
	Kaba::declare_class_element("MidiEvent.flags", &MidiEvent::flags);

	Kaba::declare_class_size("MidiEventBuffer", sizeof(MidiEventBuffer));
	Kaba::declare_class_element("MidiEventBuffer.samples", &MidiEventBuffer::samples);
	Kaba::link_external_class_func("MidiEventBuffer.__init__", &MidiEventBuffer::__init__);
	Kaba::link_external_class_func("MidiEventBuffer.get_events", &MidiEventBuffer::get_events);
	Kaba::link_external_class_func("MidiEventBuffer.get_notes", &MidiEventBuffer::get_notes);
	Kaba::link_external_class_func("MidiEventBuffer.get_range", &MidiEventBuffer::range);
	Kaba::link_external_class_func("MidiEventBuffer.add_metronome_click", &MidiEventBuffer::add_metronome_click);

	Kaba::declare_class_size("MidiNoteBuffer", sizeof(MidiNoteBuffer));
	Kaba::declare_class_element("MidiNoteBuffer.samples", &MidiNoteBuffer::samples);
	Kaba::link_external_class_func("MidiNoteBuffer.__init__", &MidiNoteBuffer::__init__);
	Kaba::link_external_class_func("MidiNoteBuffer.get_events", &MidiNoteBuffer::get_events);
	Kaba::link_external_class_func("MidiNoteBuffer.get_notes", &MidiNoteBuffer::get_notes);
	Kaba::link_external_class_func("MidiNoteBuffer.get_range", &MidiNoteBuffer::range);

	BeatSource bsource;
	Kaba::declare_class_size("BeatSource", sizeof(BeatSource));
	Kaba::link_external_class_func("BeatSource.__init__", &BeatSource::__init__);
	Kaba::link_external_virtual("BeatSource.__delete__", &BeatSource::__delete__, &bsource);
	Kaba::link_external_virtual("BeatSource.read", &BeatSource::read, &bsource);
	Kaba::link_external_virtual("BeatSource.reset", &BeatSource::reset, &bsource);
	Kaba::link_external_virtual("BeatSource.beats_per_bar", &BeatSource::beats_per_bar, &bsource);
	Kaba::link_external_virtual("BeatSource.cur_beat", &BeatSource::cur_beat, &bsource);
	Kaba::link_external_virtual("BeatSource.cur_bar", &BeatSource::cur_bar, &bsource);
	Kaba::link_external_virtual("BeatSource.beat_fraction", &BeatSource::beat_fraction, &bsource);

	Kaba::declare_class_size("TrackMarker", sizeof(TrackMarker));
	Kaba::declare_class_element("TrackMarker.text", &TrackMarker::text);
	Kaba::declare_class_element("TrackMarker.range", &TrackMarker::range);
	Kaba::declare_class_element("TrackMarker.fx", &TrackMarker::fx);

	Kaba::declare_class_size("TrackLayer", sizeof(TrackLayer));
	Kaba::declare_class_element("TrackLayer.buffers", &TrackLayer::buffers);
	Kaba::declare_class_element("TrackLayer.midi", &TrackLayer::midi);
	Kaba::declare_class_element("TrackLayer.samples", &TrackLayer::samples);
	Kaba::declare_class_element("TrackLayer.markers", &TrackLayer::markers);
	Kaba::declare_class_element("TrackLayer.track", &TrackLayer::track);
	Kaba::link_external_class_func("TrackLayer.get_buffers", &TrackLayer::get_buffers);
	Kaba::link_external_class_func("TrackLayer.read_buffers", &TrackLayer::read_buffers);
	Kaba::link_external_class_func("TrackLayer.edit_buffers", &TrackLayer::edit_buffers);
	Kaba::link_external_class_func("TrackLayer.edit_buffers_finish", &TrackLayer::edit_buffers_finish);
	Kaba::link_external_class_func("TrackLayer.insert_midi_data", &TrackLayer::insert_midi_data);
	Kaba::link_external_class_func("TrackLayer.add_midi_note", &TrackLayer::add_midi_note);
	//Kaba::link_external_class_func("TrackLayer.add_midi_notes", &TrackLayer::addMidiNotes);
	Kaba::link_external_class_func("TrackLayer.delete_midi_note", &TrackLayer::delete_midi_note);
	Kaba::link_external_class_func("TrackLayer.add_sample_ref", &TrackLayer::add_sample_ref);
	Kaba::link_external_class_func("TrackLayer.delete_sample_ref", &TrackLayer::delete_sample_ref);
	Kaba::link_external_class_func("TrackLayer.edit_sample_ref", &TrackLayer::edit_sample_ref);
	Kaba::link_external_class_func("TrackLayer.add_marker", &TrackLayer::add_marker);
	Kaba::link_external_class_func("TrackLayer.delete_marker", &TrackLayer::delete_marker);
	Kaba::link_external_class_func("TrackLayer.edit_marker", &TrackLayer::edit_marker);

	Kaba::declare_class_size("Track", sizeof(Track));
	Kaba::declare_class_element("Track.type", &Track::type);
	Kaba::declare_class_element("Track.name", &Track::name);
	Kaba::declare_class_element("Track.layers", &Track::layers);
	Kaba::declare_class_element("Track.volume", &Track::volume);
	Kaba::declare_class_element("Track.panning", &Track::panning);
	Kaba::declare_class_element("Track.muted", &Track::muted);
	Kaba::declare_class_element("Track.fx", &Track::fx);
	Kaba::declare_class_element("Track.synth", &Track::synth);
	Kaba::declare_class_element("Track.instrument", &Track::instrument);
	Kaba::declare_class_element("Track.root", &Track::song);
	Kaba::link_external_class_func("Track.nice_name", &Track::nice_name);
	Kaba::link_external_class_func("Track.set_name", &Track::set_name);
	Kaba::link_external_class_func("Track.set_muted", &Track::set_muted);
	Kaba::link_external_class_func("Track.set_volume", &Track::set_volume);
	Kaba::link_external_class_func("Track.set_panning", &Track::set_panning);
	Kaba::link_external_class_func("Track.add_effect", &Track::add_effect);
	Kaba::link_external_class_func("Track.delete_effect", &Track::delete_effect);
	Kaba::link_external_class_func("Track.edit_effect", &Track::edit_effect);
	Kaba::link_external_class_func("Track.enable_effect", &Track::enable_effect);
	Kaba::link_external_class_func("Track.set_synthesizer", &Track::set_synthesizer);

	Song af(Session::GLOBAL, DEFAULT_SAMPLE_RATE);
	Kaba::declare_class_size("Song", sizeof(Song));
	Kaba::declare_class_element("Song.filename", &Song::filename);
	Kaba::declare_class_element("Song.tag", &Song::tags);
	Kaba::declare_class_element("Song.sample_rate", &Song::sample_rate);
	Kaba::declare_class_element("Song.tracks", &Song::tracks);
	Kaba::declare_class_element("Song.samples", &Song::samples);
//	Kaba::declare_class_element("Song.layers", &Song::layers);
	Kaba::declare_class_element("Song.bars", &Song::bars);
	Kaba::link_external_class_func("Song.__init__", &Song::__init__);
	Kaba::link_external_virtual("Song.__delete__", &Song::__delete__, &af);
	Kaba::link_external_class_func("Song.add_track", &Song::add_track);
	Kaba::link_external_class_func("Song.delete_track", &Song::delete_track);
	Kaba::link_external_class_func("Song.range", &Song::range);
	Kaba::link_external_class_func("Song.layers", &Song::layers);
	Kaba::link_external_class_func("Song.add_bar", &Song::add_bar);
	Kaba::link_external_class_func("Song.add_pause", &Song::add_pause);
	Kaba::link_external_class_func("Song.edit_bar", &Song::edit_bar);
	Kaba::link_external_class_func("Song.delete_bar", &Song::delete_bar);
	Kaba::link_external_class_func("Song.add_sample", &Song::add_sample);
	Kaba::link_external_class_func("Song.delete_sample", &Song::delete_sample);
	Kaba::link_external_class_func("Song.time_track", &Song::time_track);
	Kaba::link_external_class_func("Song.begin_action_group", &Song::begin_action_group);
	Kaba::link_external_class_func("Song.end_action_group", &Song::end_action_group);
	Kaba::link_external_class_func("Song.get_time_str", &Song::get_time_str);
	Kaba::link_external_class_func("Song.get_time_str_fuzzy", &Song::get_time_str_fuzzy);
	Kaba::link_external_class_func("Song.get_time_str_long", &Song::get_time_str_long);

	SongRenderer sr(nullptr);
	Kaba::declare_class_size("SongRenderer", sizeof(SongRenderer));
	Kaba::link_external_class_func("SongRenderer.prepare", &SongRenderer::prepare);
	Kaba::link_external_class_func("SongRenderer.render", &SongRenderer::render);
	Kaba::link_external_class_func("SongRenderer.__init__", &SongRenderer::__init__);
	Kaba::link_external_virtual("SongRenderer.__delete__", &SongRenderer::__delete__, &sr);
	Kaba::link_external_virtual("SongRenderer.read", &SongRenderer::read, &sr);
	Kaba::link_external_virtual("SongRenderer.reset", &SongRenderer::reset_state, &sr);
	Kaba::link_external_class_func("SongRenderer.range", &SongRenderer::range);
	Kaba::link_external_class_func("SongRenderer.get_pos", &SongRenderer::get_pos);
	Kaba::link_external_class_func("SongRenderer.set_pos", &SongRenderer::set_pos);
	Kaba::link_external_class_func("SongRenderer.get_beat_source", &SongRenderer::get_beat_source);

	{
	AudioInput input(Session::GLOBAL);
	Kaba::declare_class_size("AudioInput", sizeof(AudioInput));
	Kaba::declare_class_element("AudioInput.current_buffer", &AudioInput::buffer);
	Kaba::declare_class_element("AudioInput.out", &AudioInput::out);
	Kaba::link_external_class_func("AudioInput.__init__", &AudioInput::__init__);
	Kaba::link_external_virtual("AudioInput.__delete__", &AudioInput::__delete__, &input);
	Kaba::link_external_class_func("AudioInput.start", &AudioInput::start);
	Kaba::link_external_class_func("AudioInput.stop",	 &AudioInput::stop);
	Kaba::link_external_class_func("AudioInput.is_capturing", &AudioInput::is_capturing);
	Kaba::link_external_class_func("AudioInput.sample_rate", &AudioInput::sample_rate);
	Kaba::link_external_class_func("AudioInput.samples_recorded", &AudioInput::samples_recorded);
	//Kaba::link_external_class_func("AudioInput.set_backup_mode", &AudioInput::set_backup_mode);
	}

	{
	AudioOutput stream(Session::GLOBAL);
	Kaba::declare_class_size("AudioOutput", sizeof(AudioOutput));
	Kaba::link_external_class_func("AudioOutput.__init__", &AudioOutput::__init__);
	Kaba::link_external_virtual("AudioOutput.__delete__", &AudioOutput::__delete__, &stream);
	//Kaba::link_external_class_func("AudioOutput.setSource", &AudioStream::setSource);
	Kaba::link_external_class_func("AudioOutput.start", &AudioOutput::start);
	Kaba::link_external_class_func("AudioOutput.stop", &AudioOutput::stop);
	Kaba::link_external_class_func("AudioOutput.is_playing", &AudioOutput::is_playing);
	//Kaba::link_external_class_func("AudioOutput.sample_rate", &OutputStream::sample_rate);
	Kaba::link_external_class_func("AudioOutput.get_volume", &AudioOutput::get_volume);
	Kaba::link_external_class_func("AudioOutput.set_volume", &AudioOutput::set_volume);
	Kaba::link_external_class_func("AudioOutput.samples_played", &AudioOutput::samples_played);
	Kaba::link_external_virtual("AudioOutput.reset_state", &AudioOutput::reset_state, &stream);
	}
	
	Kaba::declare_class_element("AudioRecorder.samples_skipped", &AudioRecorder::samples_skipped);
	Kaba::declare_class_element("AudioRecorder.buffer", &AudioRecorder::buf);

	SignalChain chain(Session::GLOBAL, "");
	Kaba::declare_class_size("SignalChain", sizeof(SignalChain));
	Kaba::link_external_class_func("SignalChain.__init__", &SignalChain::__init__);
	Kaba::link_external_virtual("SignalChain.__delete__", &SignalChain::__delete__, &chain);
	Kaba::link_external_virtual("SignalChain.reset_state", &SignalChain::reset_state, &chain);
	Kaba::link_external_virtual("SignalChain.command", &SignalChain::command, &chain);
	Kaba::link_external_class_func("SignalChain.start", &SignalChain::start);
	Kaba::link_external_class_func("SignalChain.stop", &SignalChain::stop);
	Kaba::link_external_class_func("SignalChain.add", &SignalChain::add);
	Kaba::link_external_class_func("SignalChain._add", &SignalChain::_add);
	Kaba::link_external_class_func("SignalChain.delete", &SignalChain::delete_module);
	Kaba::link_external_class_func("SignalChain.connect", &SignalChain::connect);
	Kaba::link_external_class_func("SignalChain.disconnect", &SignalChain::disconnect);
	Kaba::link_external_class_func("SignalChain.set_update_dt", &SignalChain::set_tick_dt);
	Kaba::link_external_class_func("SignalChain.set_buffer_size", &SignalChain::set_buffer_size);
	Kaba::link_external_class_func("SignalChain.is_paused", &SignalChain::is_paused);
	Kaba::link_external_class_func("SignalChain.is_active", &SignalChain::is_playback_active);

	Kaba::declare_class_size("AudioView", sizeof(AudioView));
	Kaba::declare_class_element("AudioView.cam", &AudioView::cam);
	Kaba::declare_class_element("AudioView.sel", &AudioView::sel);
	Kaba::declare_class_element("AudioView.renderer", &AudioView::renderer);
	Kaba::declare_class_element("AudioView.signal_chain", &AudioView::signal_chain);
	Kaba::declare_class_element("AudioView.output_stream", &AudioView::output_stream);
	Kaba::link_external_class_func("AudioView.subscribe", &ObservableKabaWrapper<AudioView>::subscribe_kaba);
	Kaba::link_external_class_func("AudioView.unsubscribe", &AudioView::unsubscribe);
	Kaba::link_external_class_func("AudioView.play", &AudioView::play);
	Kaba::link_external_class_func("AudioView.set_playback_loop", &AudioView::set_playback_loop);
	Kaba::link_external_class_func("AudioView.optimize_view", &AudioView::optimize_view);

	Kaba::declare_class_size("ViewPort", sizeof(ViewPort));
	Kaba::declare_class_element("ViewPort.area", &ViewPort::area);
	Kaba::link_external_class_func("ViewPort.__init__", &ViewPort::__init__);
	Kaba::link_external_class_func("ViewPort.range", &ViewPort::range);
	Kaba::link_external_class_func("ViewPort.set_range", &ViewPort::set_range);

	Kaba::declare_class_size("ColorScheme", sizeof(ColorScheme));
	Kaba::declare_class_element("ColorScheme.background", &ColorScheme::background);
	Kaba::declare_class_element("ColorScheme.background_track", &ColorScheme::background_track);
	Kaba::declare_class_element("ColorScheme.background_track_selected", &ColorScheme::background_track_selected);
	Kaba::declare_class_element("ColorScheme.text", &ColorScheme::text);
	Kaba::declare_class_element("ColorScheme.text_soft1", &ColorScheme::text_soft1);
	Kaba::declare_class_element("ColorScheme.text_soft2", &ColorScheme::text_soft2);
	Kaba::declare_class_element("ColorScheme.text_soft3", &ColorScheme::text_soft3);
	Kaba::declare_class_element("ColorScheme.grid", &ColorScheme::grid);
	Kaba::declare_class_element("ColorScheme.selection", &ColorScheme::selection);
	Kaba::declare_class_element("ColorScheme.hover", &ColorScheme::hover);
	Kaba::declare_class_element("ColorScheme.blob_bg", &ColorScheme::blob_bg);
	Kaba::declare_class_element("ColorScheme.blob_bg_selected", &ColorScheme::blob_bg_selected);
	Kaba::declare_class_element("ColorScheme.blob_bg_hidden", &ColorScheme::blob_bg_hidden);
	Kaba::link_external_class_func("ColorScheme.hoverify", &ColorScheme::hoverify);

	Kaba::link_external_class_func("Storage.load", &Storage::load);
	Kaba::link_external_class_func("Storage.save", &Storage::save);
	Kaba::link_external_class_func("Storage.save_via_renderer", &Storage::save_via_renderer);
	Kaba::link_external_class_func("Storage.load_buffer", &Storage::load_buffer);
	Kaba::declare_class_element("Storage.current_directory", &Storage::current_directory);


	Kaba::declare_class_size("SongSelection", sizeof(SongSelection));
	Kaba::declare_class_element("SongSelection.range_raw", &SongSelection::range_raw);
	Kaba::link_external_class_func("SongSelection.range", &SongSelection::range);
	Kaba::link_external_class_func("SongSelection.has_track", (bool (SongSelection::*)(const Track*) const)&SongSelection::has);
	Kaba::link_external_class_func("SongSelection.has_layer", (bool (SongSelection::*)(const TrackLayer*) const)&SongSelection::has);
	Kaba::link_external_class_func("SongSelection.has_marker", (bool (SongSelection::*)(const TrackMarker*) const)&SongSelection::has);
	Kaba::link_external_class_func("SongSelection.has_note",  (bool (SongSelection::*)(const MidiNote*) const)&SongSelection::has);
	Kaba::link_external_class_func("SongSelection.has_bar",  (bool (SongSelection::*)(const Bar*) const)&SongSelection::has);


	Kaba::declare_class_size("MidiPainter", sizeof(MidiPainter));
	Kaba::declare_class_element("MidiPainter.cam", &MidiPainter::cam);
	Kaba::link_external_class_func("MidiPainter.__init__", &MidiPainter::__init__);
	Kaba::link_external_class_func("MidiPainter.set_context", &MidiPainter::set_context);
	Kaba::link_external_class_func("MidiPainter.draw", &MidiPainter::draw);
	Kaba::link_external_class_func("MidiPainter.draw_background", &MidiPainter::draw_background);


	Kaba::declare_class_size("GridPainter", sizeof(GridPainter));
	Kaba::link_external_class_func("GridPainter.__init__", &GridPainter::__init__);
	Kaba::link_external_class_func("GridPainter.set_context", &GridPainter::set_context);
	Kaba::link_external_class_func("GridPainter.draw_empty_background", &GridPainter::draw_empty_background);
	Kaba::link_external_class_func("GridPainter.draw_bars", &GridPainter::draw_bars);
	Kaba::link_external_class_func("GridPainter.draw_bar_numbers", &GridPainter::draw_bar_numbers);
	Kaba::link_external_class_func("GridPainter.draw_time", &GridPainter::draw_time);
	Kaba::link_external_class_func("GridPainter.draw_time_numbers", &GridPainter::draw_time_numbers);

	Slider slider;
	Kaba::declare_class_size("Slider", sizeof(Slider));
	Kaba::link_external_class_func("Slider.__init__", &Slider::__init_ext__);
	Kaba::link_external_virtual("Slider.__delete__", &Slider::__delete__, &slider);
	Kaba::link_external_class_func("Slider.get", &Slider::get);
	Kaba::link_external_class_func("Slider.set", &Slider::set);

	SongPlugin song_plugin;
	Kaba::declare_class_size("SongPlugin", sizeof(SongPlugin));
	Kaba::declare_class_element("SongPlugin.session", &SongPlugin::session);
	Kaba::declare_class_element("SongPlugin.song", &SongPlugin::song);
	Kaba::link_external_class_func("SongPlugin.__init__", &SongPlugin::__init__);
	Kaba::link_external_virtual("SongPlugin.__delete__", &SongPlugin::__delete__, &song_plugin);
	Kaba::link_external_virtual("SongPlugin.apply", &SongPlugin::apply, &song_plugin);

	TsunamiPlugin tsunami_plugin;
	Kaba::declare_class_size("TsunamiPlugin", sizeof(TsunamiPlugin));
	//Kaba::declare_class_element("TsunamiPlugin.session", &TsunamiPlugin, session);
	Kaba::declare_class_element("TsunamiPlugin.args", &TsunamiPlugin::args);
	Kaba::link_external_class_func("TsunamiPlugin.__init__", &TsunamiPlugin::__init__);
	Kaba::link_external_virtual("TsunamiPlugin.__delete__", &TsunamiPlugin::__delete__, &tsunami_plugin);
	Kaba::link_external_virtual("TsunamiPlugin.on_start", &TsunamiPlugin::on_start, &tsunami_plugin);
	Kaba::link_external_virtual("TsunamiPlugin.on_stop", &TsunamiPlugin::on_stop, &tsunami_plugin);
	Kaba::link_external_class_func("TsunamiPlugin.stop", &TsunamiPlugin::stop_request);

	Progress *prog = new Progress("", nullptr);
	Kaba::declare_class_size("Progress", sizeof(Progress));
	Kaba::link_external_class_func("Progress.__init__", &Progress::__init__);
	Kaba::link_external_virtual("Progress.__delete__", &Progress::__delete__, prog);
	Kaba::link_external_class_func("Progress.set", &Progress::set_kaba);
	delete prog;

	ProgressCancelable prog_can;
	Kaba::declare_class_size("ProgressX", sizeof(ProgressCancelable));
	Kaba::link_external_class_func("ProgressX.__init__", &ProgressCancelable::__init__);
	Kaba::link_external_virtual("ProgressX.__delete__", &ProgressCancelable::__delete__, &prog_can);
	Kaba::link_external_class_func("ProgressX.set", &ProgressCancelable::set_kaba);
	Kaba::link_external_class_func("ProgressX.cancel", &ProgressCancelable::cancel);
	Kaba::link_external_class_func("ProgressX.is_cancelled", &ProgressCancelable::is_cancelled);
}

Kaba::Class* PluginManager::get_class(const string &name) {
	return (Kaba::Class*)package->syntax->make_class(name, Kaba::Class::Type::OTHER, 0, 0, nullptr, nullptr, package->syntax->base_class);
}

void get_plugin_file_data(PluginManager::PluginFile &pf) {
	pf.image = "";
	try {
		string content = FileRead(pf.filename);
		int p = content.find("// Image = hui:");
		if (p >= 0)
			pf.image = content.substr(p + 11, content.find("\n") - p - 11);
	} catch(...) {}
}

void find_plugins_in_dir_absolute(const string &_dir, const string &group, ModuleType type, PluginManager *pm) {
	string dir = _dir;
	if (group.num > 0)
		dir += group + "/";
	auto list = dir_search(dir, "*.kaba", false);
	for (DirEntry &e: list) {
		PluginManager::PluginFile pf;
		pf.type = type;
		pf.name = e.name.replace(".kaba", "");
		pf.filename = dir + e.name;
		pf.group = group;
		get_plugin_file_data(pf);
		pm->plugin_files.add(pf);
	}
}

void find_plugins_in_dir(const string &rel, const string &group, ModuleType type, PluginManager *pm) {
	find_plugins_in_dir_absolute(pm->plugin_dir_static() + rel, group, type, pm);
	if (pm->plugin_dir_local() != pm->plugin_dir_static())
		find_plugins_in_dir_absolute(pm->plugin_dir_local() + rel, group, type, pm);
}

void add_plugins_in_dir(const string &dir, PluginManager *pm, hui::Menu *m, const string &name_space, TsunamiWindow *win, void (TsunamiWindow::*function)()) {
	for (PluginManager::PluginFile &f: pm->plugin_files) {
		if (f.filename.find(dir) >= 0) {
			string id = "execute-" + name_space + "--" + f.name;
			m->add_with_image(f.name, f.image, id);
			win->event(id, std::bind(function, win));
		}
	}
}

void PluginManager::find_plugins() {
	Kaba::init();
	Kaba::config.show_compiler_stats = false;
	Kaba::config.compile_silently = true;

	// "AudioSource"
#ifndef OS_WINDOWS
	find_plugins_in_dir("AudioSource/", "", ModuleType::AUDIO_SOURCE, this);

	// "AudioEffect"
	find_plugins_in_dir("AudioEffect/", "Channels", ModuleType::AUDIO_EFFECT, this);
	find_plugins_in_dir("AudioEffect/", "Dynamics", ModuleType::AUDIO_EFFECT, this);
	find_plugins_in_dir("AudioEffect/", "Echo", ModuleType::AUDIO_EFFECT, this);
	find_plugins_in_dir("AudioEffect/", "Filter", ModuleType::AUDIO_EFFECT, this);
	find_plugins_in_dir("AudioEffect/", "Pitch", ModuleType::AUDIO_EFFECT, this);
	find_plugins_in_dir("AudioEffect/", "Repair", ModuleType::AUDIO_EFFECT, this);
	find_plugins_in_dir("AudioEffect/", "Sound", ModuleType::AUDIO_EFFECT, this);
	// hidden...
	find_plugins_in_dir("AudioEffect/", "Special", ModuleType::AUDIO_EFFECT, this);

	// "AudioVisualizer"
	find_plugins_in_dir("AudioVisualizer/", "", ModuleType::AUDIO_VISUALIZER, this);

	// "MidiSource"
	find_plugins_in_dir("MidiSource/", "", ModuleType::MIDI_SOURCE, this);

	// "MidiEffect"
	find_plugins_in_dir("MidiEffect/", "", ModuleType::MIDI_EFFECT, this);

	// "BeatSource"
	find_plugins_in_dir("BeatSource/", "", ModuleType::BEAT_SOURCE, this);

	// "All"
	find_plugins_in_dir("All/", "", ModuleType::SONG_PLUGIN, this);

	// rest
	find_plugins_in_dir("Independent/", "", ModuleType::TSUNAMI_PLUGIN, this);

	// "Synthesizer"
	find_plugins_in_dir("Synthesizer/", "", ModuleType::SYNTHESIZER, this);
#endif
}

void PluginManager::add_plugins_to_menu(TsunamiWindow *win) {
	hui::Menu *m = win->get_menu();

	// "Buffer"
	add_plugins_in_dir("AudioEffect/Channels/", this, m->get_sub_menu_by_id("menu_plugins_channels"), "audio-effect", win, &TsunamiWindow::on_menu_execute_audio_effect);
	add_plugins_in_dir("AudioEffect/Dynamics/", this, m->get_sub_menu_by_id("menu_plugins_dynamics"), "audio-effect", win, &TsunamiWindow::on_menu_execute_audio_effect);
	add_plugins_in_dir("AudioEffect/Echo/", this, m->get_sub_menu_by_id("menu_plugins_echo"), "audio-effect", win, &TsunamiWindow::on_menu_execute_audio_effect);
	add_plugins_in_dir("AudioEffect/Filter/", this, m->get_sub_menu_by_id("menu_plugins_filter"), "audio-effect", win, &TsunamiWindow::on_menu_execute_audio_effect);
	add_plugins_in_dir("AudioEffect/Pitch/", this, m->get_sub_menu_by_id("menu_plugins_pitch"), "audio-effect", win, &TsunamiWindow::on_menu_execute_audio_effect);
	add_plugins_in_dir("AudioEffect/Repair/", this, m->get_sub_menu_by_id("menu_plugins_repair"), "audio-effect", win, &TsunamiWindow::on_menu_execute_audio_effect);
	add_plugins_in_dir("AudioEffect/Sound/", this, m->get_sub_menu_by_id("menu_plugins_sound"), "audio-effect", win, &TsunamiWindow::on_menu_execute_audio_effect);

	add_plugins_in_dir("AudioSource/", this, m->get_sub_menu_by_id("menu_plugins_audio_source"), "source", win, &TsunamiWindow::on_menu_execute_audio_source);

	// "Midi"
	add_plugins_in_dir("MidiEffect/", this, m->get_sub_menu_by_id("menu_plugins_midi_effects"), "midi-effect", win, &TsunamiWindow::on_menu_execute_midi_effect);
	add_plugins_in_dir("MidiSource/", this, m->get_sub_menu_by_id("menu_plugins_midi_source"), "midi-source", win, &TsunamiWindow::on_menu_execute_midi_source);

	// "All"
	add_plugins_in_dir("All/", this, m->get_sub_menu_by_id("menu_plugins_on_all"), "song", win, &TsunamiWindow::on_menu_execute_song_plugin);

	// rest
	add_plugins_in_dir("Independent/", this, m->get_sub_menu_by_id("menu_plugins_other"), "tsunami", win, &TsunamiWindow::on_menu_execute_tsunami_plugin);
}

void PluginManager::apply_favorite(Module *c, const string &name) {
	favorites->apply(c, name);
}

void PluginManager::save_favorite(Module *c, const string &name)
{
	favorites->save(c, name);
}


string PluginManager::select_favorite_name(hui::Window *win, Module *c, bool save)
{
	return favorites->select_name(win, c, save);
}

// always push the script... even if an error occurred
//   don't log error...
Plugin *PluginManager::load_and_compile_plugin(ModuleType type, const string &filename)
{
	for (Plugin *p: plugins)
		if (filename == p->filename)
			return p;

	//InitPluginData();

	Plugin *p = new Plugin(filename, type);
	p->index = plugins.num;

	plugins.add(p);

	return p;
}


Plugin *PluginManager::get_plugin(Session *session, ModuleType type, const string &name)
{
	for (PluginFile &pf: plugin_files){
		if ((pf.name.replace(" ", "") == name.replace(" ", "")) and (pf.type == type)){
			Plugin *p = load_and_compile_plugin(type, pf.filename);
			return p;
		}
	}
	session->e(format(_("Can't find %s plugin: %s ..."), Module::type_to_name(type).c_str(), name.c_str()));
	return nullptr;
}

string PluginManager::plugin_dir_static()
{
	if (tsunami->installed)
		return tsunami->directory_static + "Plugins/";
	return "Plugins/";
}

string PluginManager::plugin_dir_local()
{
	if (tsunami->installed)
		return tsunami->directory + "Plugins/";
	return "Plugins/";
}


Array<string> PluginManager::find_module_sub_types(ModuleType type)
{
	Array<string> names;
	for (auto &pf: plugin_files)
		if (pf.type == type)
			names.add(pf.name);

	if (type == ModuleType::AUDIO_SOURCE){
		names.add("SongRenderer");
		//names.add("BufferStreamer");
	}else if (type == ModuleType::MIDI_EFFECT){
		names.add("Dummy");
	}else if (type == ModuleType::BEAT_SOURCE){
		//names.add("BarStreamer");
	}else if (type == ModuleType::AUDIO_VISUALIZER){
		names.add("PeakMeter");
	}else if (type == ModuleType::SYNTHESIZER){
		names.add("Dummy");
		//names.add("Sample");
	}else if (type == ModuleType::STREAM){
		names.add("AudioInput");
		names.add("AudioOutput");
		names.add("MidiInput");
	}else if (type == ModuleType::PLUMBING){
		names.add("AudioBackup");
		names.add("AudioJoiner");
		names.add("AudioRecorder");
		names.add("AudioSucker");
		names.add("BeatMidifier");
		names.add("MidiJoiner");
		names.add("MidiRecorder");
		names.add("MidiSucker");
	}
	return names;
}

Array<string> PluginManager::find_module_sub_types_grouped(ModuleType type)
{
	if (type == ModuleType::AUDIO_EFFECT){
		Array<string> names;
		for (auto &pf: plugin_files)
			if (pf.type == type)
				names.add(pf.group + "/" + pf.name);
		return names;
	}
	return find_module_sub_types(type);
}

string PluginManager::choose_module(hui::Panel *parent, Session *session, ModuleType type, const string &old_name)
{
	Array<string> names = session->plugin_manager->find_module_sub_types(type);
	if (names.num == 1)
		return names[0];
	if (names.num == 0)
		return "";

	ConfigurableSelectorDialog *dlg = new ConfigurableSelectorDialog(parent->win, type, session, old_name);
	dlg->run();
	string name = dlg->_return;
	delete(dlg);
	return name;
}


