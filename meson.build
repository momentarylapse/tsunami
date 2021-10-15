project('tsunami', 'cpp', default_options : ['cpp_std=c++14', 'warning_level=1', 'optimization=2', 'debug=true'])
add_project_arguments('-Wno-unused', '-Wstrict-aliasing=0', '-fno-omit-frame-pointer', language : ['cpp'])

#install_data
#meson.add_install_script('')
#install_subdir

cc = meson.get_compiler('cpp')

# mandatory dependencies
mdep = cc.find_library('m', required : false)
threaddep = dependency('threads')
gtk3dep = dependency('gtk+-3.0')
add_project_arguments('-DHAS_LIB_GTK3=1', language : ['cpp'])
zlibdep = dependency('zlib')
add_project_arguments('-DHAS_LIB_ZLIB=1', language : ['cpp'])
fftw3dep = dependency('fftw3f')
add_project_arguments('-DHAS_LIB_FFTW3=1', language : ['cpp'])
deps = [mdep, threaddep, gtk3dep, zlibdep, fftw3dep]

# optional dependency: ogg
d1 = cc.find_library('ogg', required : false)
d2 = cc.find_library('vorbis', required : false)
d3 = cc.find_library('vorbisfile', required : false)
d4 = cc.find_library('vorbisenc', required : false)
if d1.found() and d2.found() and d3.found() and d4.found()
	add_project_arguments('-DHAS_LIB_OGG=1', language : ['cpp'])
	deps += [d1,d2,d3,d4]
else
	add_project_arguments('-DHAS_LIB_OGG=0', language : ['cpp'])
endif

# optional dependency: flac
d = cc.find_library('FLAC', required : false)
if d.found()
	add_project_arguments('-DHAS_LIB_FLAC=1', language : ['cpp'])
	deps += [d]
else
	add_project_arguments('-DHAS_LIB_FLAC=0', language : ['cpp'])
endif

# optional dependency: alsa
d = cc.find_library('asound', required : false)
if d.found()
	add_project_arguments('-DHAS_LIB_ALSA=1', language : ['cpp'])
	deps += [d]
else
	add_project_arguments('-DHAS_LIB_ALSA=0', language : ['cpp'])
endif

# optional dependency: pulseaudio
d = cc.find_library('pulse', required : false)
if d.found()
	add_project_arguments('-DHAS_LIB_PULSEAUDIO=1', language : ['cpp'])
	deps += [d]
else
	add_project_arguments('-DHAS_LIB_PULSEAUDIO=0', language : ['cpp'])
endif

# optional dependency: portaudio
d = cc.find_library('portaudio', required : false)
if d.found()
	add_project_arguments('-DHAS_LIB_PORTAUDIO=1', language : ['cpp'])
	deps += [d]
else
	add_project_arguments('-DHAS_LIB_PORTAUDIO=0', language : ['cpp'])
endif

# optional dependency: unwind
d = cc.find_library('unwind', required : false)
if d.found()
	add_project_arguments('-DHAS_LIB_UNWIND=1', language : ['cpp'])
	deps += [d]
else
	add_project_arguments('-DHAS_LIB_UNWIND=0', language : ['cpp'])
endif

# optional dependency: dl
d = cc.find_library('dl', required : false)
if d.found()
	add_project_arguments('-DHAS_LIB_DL=1', language : ['cpp'])
	deps += [d]
else
	add_project_arguments('-DHAS_LIB_DL=0', language : ['cpp'])
endif

# optional dependency: gl
d = cc.find_library('GL', required : false)
if d.found()
	add_project_arguments('-DHAS_LIB_GL=1', language : ['cpp'])
	deps += [d]
else
	add_project_arguments('-DHAS_LIB_GL=0', language : ['cpp'])
endif


sources = [
	'src/Action/Bar/Action__ScaleData.cpp',
	'src/Action/Bar/Action__ShiftData.cpp',
	'src/Action/Bar/ActionBar__Add.cpp',
	'src/Action/Bar/ActionBar__Delete.cpp',
	'src/Action/Bar/ActionBar__Edit.cpp',
	'src/Action/Bar/ActionBarAdd.cpp',
	'src/Action/Bar/ActionBarDelete.cpp',
	'src/Action/Bar/ActionBarEdit.cpp',
	'src/Action/Sample/ActionSampleAdd.cpp',
	'src/Action/Sample/ActionSampleDelete.cpp',
	'src/Action/Sample/ActionSampleEditName.cpp',
	'src/Action/Sample/ActionSampleReplaceBuffer.cpp',
	'src/Action/Song/Data/ActionSongChangeAllTrackVolumes.cpp',
	'src/Action/Song/Data/ActionSongSetDefaultFormat.cpp',
	'src/Action/Song/Data/ActionSongSetSampleRate.cpp',
	'src/Action/Song/ActionSongDeleteSelection.cpp',
	'src/Action/Song/ActionSongMoveSelection.cpp',
	'src/Action/Tag/ActionTagAdd.cpp',
	'src/Action/Tag/ActionTagDelete.cpp',
	'src/Action/Tag/ActionTagEdit.cpp',
	'src/Action/Track/Buffer/ActionTrack__AbsorbBuffer.cpp',
	'src/Action/Track/Buffer/ActionTrack__AddBuffer.cpp',
	'src/Action/Track/Buffer/ActionTrack__BufferSetChannels.cpp',
	'src/Action/Track/Buffer/ActionTrack__DeleteBuffer.cpp',
	'src/Action/Track/Buffer/ActionTrack__GrowBuffer.cpp',
	'src/Action/Track/Buffer/ActionTrack__ShrinkBuffer.cpp',
	'src/Action/Track/Buffer/ActionTrack__SplitBuffer.cpp',
	'src/Action/Track/Buffer/ActionTrackCreateBuffers.cpp',
	'src/Action/Track/Buffer/ActionTrackEditBuffer.cpp',
	'src/Action/Track/Buffer/ActionTrackMoveBuffer.cpp',
	'src/Action/Track/Buffer/ActionTrackSetChannels.cpp',
	'src/Action/Track/Curve/ActionTrackAddCurve.cpp',
	'src/Action/Track/Curve/ActionTrackCurveAddPoint.cpp',
	'src/Action/Track/Curve/ActionTrackCurveDeletePoint.cpp',
	'src/Action/Track/Curve/ActionTrackCurveEditPoint.cpp',
	'src/Action/Track/Curve/ActionTrackDeleteCurve.cpp',
	'src/Action/Track/Curve/ActionTrackEditCurve.cpp',
	'src/Action/Track/Data/ActionTrackEditMuted.cpp',
	'src/Action/Track/Data/ActionTrackEditName.cpp',
	'src/Action/Track/Data/ActionTrackEditPanning.cpp',
	'src/Action/Track/Data/ActionTrackEditVolume.cpp',
	'src/Action/Track/Data/ActionTrackSetInstrument.cpp',
	'src/Action/Track/Data/ActionTrackSetTarget.cpp',
	'src/Action/Track/Effect/ActionTrackAddAudioEffect.cpp',
	'src/Action/Track/Effect/ActionTrackDeleteEffect.cpp',
	'src/Action/Track/Effect/ActionTrackEditAudioEffect.cpp',
	'src/Action/Track/Effect/ActionTrackMoveAudioEffect.cpp',
	'src/Action/Track/Effect/ActionTrackToggleAudioEffectEnabled.cpp',
	'src/Action/Track/Layer/ActionTrackFadeAdd.cpp',
	'src/Action/Track/Layer/ActionTrackFadeDelete.cpp',
	'src/Action/Track/Layer/ActionTrackLayer__Delete.cpp',
	'src/Action/Track/Layer/ActionTrackLayerAdd.cpp',
	'src/Action/Track/Layer/ActionTrackLayerDelete.cpp',
	'src/Action/Track/Layer/ActionTrackLayerMakeTrack.cpp',
	'src/Action/Track/Layer/ActionTrackLayerMarkDominant.cpp',
	'src/Action/Track/Layer/ActionTrackLayerMerge.cpp',
	'src/Action/Track/Layer/ActionTrackLayerMove.cpp',
	'src/Action/Track/Marker/ActionTrackAddMarker.cpp',
	'src/Action/Track/Marker/ActionTrackDeleteMarker.cpp',
	'src/Action/Track/Marker/ActionTrackEditMarker.cpp',
	'src/Action/Track/Midi/ActionTrackAddMidiEffect.cpp',
	'src/Action/Track/Midi/ActionTrackAddMidiNote.cpp',
	'src/Action/Track/Midi/ActionTrackDeleteMidiEffect.cpp',
	'src/Action/Track/Midi/ActionTrackDeleteMidiNote.cpp',
	'src/Action/Track/Midi/ActionTrackEditMidiEffect.cpp',
	'src/Action/Track/Midi/ActionTrackEditMidiNote.cpp',
	'src/Action/Track/Midi/ActionTrackInsertMidi.cpp',
	'src/Action/Track/Midi/ActionTrackMoveMidiEffect.cpp',
	'src/Action/Track/Midi/ActionTrackToggleMidiEffectEnabled.cpp',
	'src/Action/Track/Sample/ActionTrackAddSample.cpp',
	'src/Action/Track/Sample/ActionTrackDeleteSample.cpp',
	'src/Action/Track/Sample/ActionTrackEditSample.cpp',
	'src/Action/Track/Sample/ActionTrackInsertSample.cpp',
	'src/Action/Track/Sample/ActionTrackInsertSelectedSamples.cpp',
	'src/Action/Track/Sample/ActionTrackPasteAsSample.cpp',
	'src/Action/Track/Sample/ActionTrackSampleFromSelection.cpp',
	'src/Action/Track/Synthesizer/ActionTrackDetuneSynthesizer.cpp',
	'src/Action/Track/Synthesizer/ActionTrackEditSynthesizer.cpp',
	'src/Action/Track/Synthesizer/ActionTrackSetSynthesizer.cpp',
	'src/Action/Track/ActionTrack__DeleteEmpty.cpp',
	'src/Action/Track/ActionTrackAdd.cpp',
	'src/Action/Track/ActionTrackDelete.cpp',
	'src/Action/Track/ActionTrackMove.cpp',
	'src/Action/Action.cpp',
	'src/Action/ActionGroup.cpp',
	'src/Action/ActionManager.cpp',
	'src/Action/ActionMergable.cpp',
	'src/Data/Audio/AudioBuffer.cpp',
	'src/Data/Audio/BufferInterpolator.cpp',
	'src/Data/Audio/RingBuffer.cpp',
	'src/Data/Midi/Clef.cpp',
	'src/Data/Midi/Instrument.cpp',
	'src/Data/Midi/MidiData.cpp',
	'src/Data/Midi/MidiEvent.cpp',
	'src/Data/Midi/MidiNote.cpp',
	'src/Data/Midi/Scale.cpp',
	'src/Data/Rhythm/Bar.cpp',
	'src/Data/Rhythm/BarCollection.cpp',
	'src/Data/Rhythm/Beat.cpp',
	'src/Data/base.cpp',
	'src/Data/CrossFade.cpp',
	'src/Data/Curve.cpp',
	'src/Data/Data.cpp',
	'src/Data/Range.cpp',
	'src/Data/Sample.cpp',
	'src/Data/SampleRef.cpp',
	'src/Data/Song.cpp',
	'src/Data/SongSelection.cpp',
	'src/Data/Track.cpp',
	'src/Data/TrackLayer.cpp',
	'src/Data/TrackMarker.cpp',
	'src/Device/Stream/AudioInput.cpp',
	'src/Device/Stream/AudioOutput.cpp',
	'src/Device/Stream/MidiInput.cpp',
	'src/Device/Device.cpp',
	'src/Device/DeviceManager.cpp',
	'src/lib/any/any.cpp',
	'src/lib/base/array.cpp',
	'src/lib/base/pointer.cpp',
	'src/lib/base/strings.cpp',
	'src/lib/file/file.cpp',
	'src/lib/file/file_op.cpp',
	'src/lib/file/msg.cpp',
	'src/lib/file/path.cpp',
	'src/lib/hui/Controls/Control.cpp',
	'src/lib/hui/Controls/ControlButtonGtk.cpp',
	'src/lib/hui/Controls/ControlCheckBoxGtk.cpp',
	'src/lib/hui/Controls/ControlColorButtonGtk.cpp',
	'src/lib/hui/Controls/ControlComboBoxGtk.cpp',
	'src/lib/hui/Controls/ControlDrawingAreaGtk.cpp',
	'src/lib/hui/Controls/ControlEditGtk.cpp',
	'src/lib/hui/Controls/ControlExpanderGtk.cpp',
	'src/lib/hui/Controls/ControlGridGtk.cpp',
	'src/lib/hui/Controls/ControlGroupGtk.cpp',
	'src/lib/hui/Controls/ControlLabelGtk.cpp',
	'src/lib/hui/Controls/ControlListViewGtk.cpp',
	'src/lib/hui/Controls/ControlMenuButtonGtk.cpp',
	'src/lib/hui/Controls/ControlMultilineEditGtk.cpp',
	'src/lib/hui/Controls/ControlPanedGtk.cpp',
	'src/lib/hui/Controls/ControlProgressBarGtk.cpp',
	'src/lib/hui/Controls/ControlRadioButtonGtk.cpp',
	'src/lib/hui/Controls/ControlRevealerGtk.cpp',
	'src/lib/hui/Controls/ControlScrollerGtk.cpp',
	'src/lib/hui/Controls/ControlSeparatorGtk.cpp',
	'src/lib/hui/Controls/ControlSliderGtk.cpp',
	'src/lib/hui/Controls/ControlSpinButtonGtk.cpp',
	'src/lib/hui/Controls/ControlTabControlGtk.cpp',
	'src/lib/hui/Controls/ControlToggleButtonGtk.cpp',
	'src/lib/hui/Controls/ControlTreeViewGtk.cpp',
	'src/lib/hui/Controls/MenuItemGtk.cpp',
	'src/lib/hui/Controls/MenuItemSeparatorGtk.cpp',
	'src/lib/hui/Controls/MenuItemSeparatorWin.cpp',
	'src/lib/hui/Controls/MenuItemSubmenuGtk.cpp',
	'src/lib/hui/Controls/MenuItemSubmenuWin.cpp',
	'src/lib/hui/Controls/MenuItemToggleGtk.cpp',
	'src/lib/hui/Controls/MenuItemToggleWin.cpp',
	'src/lib/hui/Controls/MenuItemWin.cpp',
	'src/lib/hui/Controls/ToolItemButtonGtk.cpp',
	'src/lib/hui/Controls/ToolItemButtonWin.cpp',
	'src/lib/hui/Controls/ToolItemMenuButtonGtk.cpp',
	'src/lib/hui/Controls/ToolItemMenuButtonWin.cpp',
	'src/lib/hui/Controls/ToolItemSeparatorGtk.cpp',
	'src/lib/hui/Controls/ToolItemSeparatorWin.cpp',
	'src/lib/hui/Controls/ToolItemToggleButtonGtk.cpp',
	'src/lib/hui/Controls/ToolItemToggleButtonWin.cpp',
	'src/lib/hui/Application.cpp',
	'src/lib/hui/Callback.cpp',
	'src/lib/hui/Clipboard.cpp',
	'src/lib/hui/common_dlg.cpp',
	'src/lib/hui/common_dlg_gtk.cpp',
	'src/lib/hui/common_dlg_win.cpp',
	'src/lib/hui/Config.cpp',
	'src/lib/hui/error.cpp',
	'src/lib/hui/Event.cpp',
	'src/lib/hui/hui.cpp',
	'src/lib/hui/language.cpp',
	'src/lib/hui/main_gtk.cpp',
	'src/lib/hui/main_win.cpp',
	'src/lib/hui/Menu.cpp',
	'src/lib/hui/MenuGtk.cpp',
	'src/lib/hui/MenuWin.cpp',
	'src/lib/hui/PainterGtk.cpp',
	'src/lib/hui/Panel.cpp',
	'src/lib/hui/Resource.cpp',
	'src/lib/hui/Timer.cpp',
	'src/lib/hui/Toolbar.cpp',
	'src/lib/hui/ToolbarGtk.cpp',
	'src/lib/hui/ToolbarWin.cpp',
	'src/lib/hui/utility.cpp',
	'src/lib/hui/Window.cpp',
	'src/lib/hui/window_control.cpp',
	'src/lib/hui/window_control_gtk.cpp',
	'src/lib/hui/window_control_win.cpp',
	'src/lib/hui/WindowGtk.cpp',
	'src/lib/hui/WindowWin.cpp',
	'src/lib/image/color.cpp',
	'src/lib/image/image.cpp',
	'src/lib/image/image_bmp.cpp',
	'src/lib/image/image_jpg.cpp',
	'src/lib/image/image_png.cpp',
	'src/lib/image/image_tga.cpp',
	'src/lib/image/ImagePainter.cpp',
	'src/lib/kaba/asm/asm.cpp',
	'src/lib/kaba/asm/asm_arm.cpp',
	'src/lib/kaba/asm/asm_x86.cpp',
	'src/lib/kaba/compiler/Backend.cpp',
	'src/lib/kaba/compiler/BackendAmd64.cpp',
	'src/lib/kaba/compiler/BackendARM.cpp',
	'src/lib/kaba/compiler/BackendX86.cpp',
	'src/lib/kaba/compiler/CommandList.cpp',
	'src/lib/kaba/compiler/compiler.cpp',
	'src/lib/kaba/compiler/mapper.cpp',
	'src/lib/kaba/compiler/serializer.cpp',
	'src/lib/kaba/compiler/SerializerX.cpp',
	'src/lib/kaba/compiler/SerialNode.cpp',
	'src/lib/kaba/dynamic/call.cpp',
	'src/lib/kaba/dynamic/dynamic.cpp',
	'src/lib/kaba/dynamic/exception.cpp',
	'src/lib/kaba/lib/dict.cpp',
	'src/lib/kaba/lib/extern.cpp',
	'src/lib/kaba/lib/lib.cpp',
	'src/lib/kaba/lib/lib_base.cpp',
	'src/lib/kaba/lib/lib_doc.cpp',
	'src/lib/kaba/lib/lib_hui.cpp',
	'src/lib/kaba/lib/lib_image.cpp',
	'src/lib/kaba/lib/lib_kaba.cpp',
	'src/lib/kaba/lib/lib_math.cpp',
	'src/lib/kaba/lib/lib_net.cpp',
	'src/lib/kaba/lib/lib_nix.cpp',
	'src/lib/kaba/lib/lib_os.cpp',
	'src/lib/kaba/lib/lib_threads.cpp',
	'src/lib/kaba/lib/lib_time.cpp',
	'src/lib/kaba/lib/lib_vulkan.cpp',
	'src/lib/kaba/syntax/Block.cpp',
	'src/lib/kaba/syntax/Class.cpp',
	'src/lib/kaba/syntax/Constant.cpp',
	'src/lib/kaba/syntax/Function.cpp',
	'src/lib/kaba/syntax/Identifier.cpp',
	'src/lib/kaba/syntax/implicit.cpp',
	'src/lib/kaba/syntax/lexical.cpp',
	'src/lib/kaba/syntax/macros.cpp',
	'src/lib/kaba/syntax/Node.cpp',
	'src/lib/kaba/syntax/Operator.cpp',
	'src/lib/kaba/syntax/Parser.cpp',
	'src/lib/kaba/syntax/preprocessor.cpp',
	'src/lib/kaba/syntax/Statement.cpp',
	'src/lib/kaba/syntax/SyntaxTree.cpp',
	'src/lib/kaba/syntax/Variable.cpp',
	'src/lib/kaba/CompilerConfiguration.cpp',
	'src/lib/kaba/Interpreter.cpp',
	'src/lib/kaba/kaba.cpp',
	'src/lib/math/complex.cpp',
	'src/lib/math/interpolation.cpp',
	'src/lib/math/math.cpp',
	'src/lib/math/matrix.cpp',
	'src/lib/math/matrix3.cpp',
	'src/lib/math/plane.cpp',
	'src/lib/math/quaternion.cpp',
	'src/lib/math/random.cpp',
	'src/lib/math/ray.cpp',
	'src/lib/math/rect.cpp',
	'src/lib/math/vec2.cpp',
	'src/lib/math/vec4.cpp',
	'src/lib/math/vector.cpp',
	'src/lib/net/BinaryBuffer.cpp',
	'src/lib/net/net.cpp',
	'src/lib/net/Socket.cpp',
	'src/lib/nix/nix.cpp',
	'src/lib/nix/nix_buffer.cpp',
	'src/lib/nix/nix_draw.cpp',
	'src/lib/nix/nix_framebuffer.cpp',
	'src/lib/nix/nix_light.cpp',
	'src/lib/nix/nix_shader.cpp',
	'src/lib/nix/nix_textures.cpp',
	'src/lib/nix/nix_vertexbuffer.cpp',
	'src/lib/nix/nix_view.cpp',
	'src/lib/threads/Mutex.cpp',
	'src/lib/threads/Thread.cpp',
	'src/lib/threads/ThreadedWork.cpp',
	'src/lib/xfile/chunked.cpp',
	'src/lib/xfile/pdf.cpp',
	'src/lib/xfile/xml.cpp',
	'src/Module/Audio/AudioAccumulator.cpp',
	'src/Module/Audio/AudioBackup.cpp',
	'src/Module/Audio/AudioChannelSelector.cpp',
	'src/Module/Audio/AudioEffect.cpp',
	'src/Module/Audio/AudioJoiner.cpp',
	'src/Module/Audio/AudioSource.cpp',
	'src/Module/Audio/AudioSucker.cpp',
	'src/Module/Audio/AudioVisualizer.cpp',
	'src/Module/Audio/BufferStreamer.cpp',
	'src/Module/Audio/PeakMeter.cpp',
	'src/Module/Audio/PitchDetector.cpp',
	'src/Module/Audio/SongRenderer.cpp',
	'src/Module/Audio/TrackRenderer.cpp',
	'src/Module/Beats/BarStreamer.cpp',
	'src/Module/Beats/BeatMidifier.cpp',
	'src/Module/Beats/BeatSource.cpp',
	'src/Module/Midi/MidiAccumulator.cpp',
	'src/Module/Midi/MidiEffect.cpp',
	'src/Module/Midi/MidiEventStreamer.cpp',
	'src/Module/Midi/MidiJoiner.cpp',
	'src/Module/Midi/MidiSource.cpp',
	'src/Module/Midi/MidiSucker.cpp',
	'src/Module/Port/Port.cpp',
	'src/Module/Synth/DummySynthesizer.cpp',
	'src/Module/Synth/EnvelopeADSR.cpp',
	'src/Module/Synth/Synthesizer.cpp',
	'src/Module/AutoConfigPanel.cpp',
	'src/Module/ConfigPanel.cpp',
	'src/Module/Module.cpp',
	'src/Module/ModuleConfiguration.cpp',
	'src/Module/ModuleFactory.cpp',
	'src/Module/SignalChain.cpp',
	'src/Plugins/ExtendedAudioBuffer.cpp',
	'src/Plugins/FastFourierTransform.cpp',
	'src/Plugins/Plugin.cpp',
	'src/Plugins/PluginManager.cpp',
	'src/Plugins/ProfileManager.cpp',
	'src/Plugins/SongPlugin.cpp',
	'src/Plugins/TsunamiPlugin.cpp',
	'src/Storage/Dialog/PdfConfigDialog.cpp',
	'src/Storage/Dialog/RawConfigDialog.cpp',
	'src/Storage/Format/Format.cpp',
	'src/Storage/Format/FormatFlac.cpp',
	'src/Storage/Format/FormatGuitarPro.cpp',
	'src/Storage/Format/FormatM4a.cpp',
	'src/Storage/Format/FormatMidi.cpp',
	'src/Storage/Format/FormatMp3.cpp',
	'src/Storage/Format/FormatNami.cpp',
	'src/Storage/Format/FormatOgg.cpp',
	'src/Storage/Format/FormatPdf.cpp',
	'src/Storage/Format/FormatRaw.cpp',
	'src/Storage/Format/FormatSoundFont2.cpp',
	'src/Storage/Format/FormatWave.cpp',
	'src/Storage/Storage.cpp',
	'src/Storage/StorageOperationData.cpp',
	'src/Stuff/BackupManager.cpp',
	'src/Stuff/CLIParser.cpp',
	'src/Stuff/Clipboard.cpp',
	'src/Stuff/Diff.cpp',
	'src/Stuff/ErrorHandler.cpp',
	'src/Stuff/Log.cpp',
	'src/Stuff/Observable.cpp',
	'src/Stuff/PerformanceMonitor.cpp',
	'src/Test/TestAudioBuffer.cpp',
	'src/Test/TestInterpolator.cpp',
	'src/Test/TestMidiPreview.cpp',
	'src/Test/TestMixer.cpp',
	'src/Test/TestPlugins.cpp',
	'src/Test/TestPointer.cpp',
	'src/Test/TestRhythm.cpp',
	'src/Test/TestRingBuffer.cpp',
	'src/Test/TestStreams.cpp',
	'src/Test/TestThreads.cpp',
	'src/Test/TestTrackVersion.cpp',
	'src/Test/UnitTest.cpp',
	'src/View/BottomBar/SignalEditor/SignalEditorBackground.cpp',
	'src/View/BottomBar/SignalEditor/SignalEditorCable.cpp',
	'src/View/BottomBar/SignalEditor/SignalEditorModule.cpp',
	'src/View/BottomBar/SignalEditor/SignalEditorPort.cpp',
	'src/View/BottomBar/SignalEditor/SignalEditorTab.cpp',
	'src/View/BottomBar/BottomBar.cpp',
	'src/View/BottomBar/DeviceConsole.cpp',
	'src/View/BottomBar/LogConsole.cpp',
	'src/View/BottomBar/MiniBar.cpp',
	'src/View/BottomBar/MixingConsole.cpp',
	'src/View/BottomBar/PluginConsole.cpp',
	'src/View/BottomBar/SignalEditor.cpp',
	'src/View/Dialog/BarAddDialog.cpp',
	'src/View/Dialog/BarDeleteDialog.cpp',
	'src/View/Dialog/BarEditDialog.cpp',
	'src/View/Dialog/BufferCompressionDialog.cpp',
	'src/View/Dialog/ChannelMapperDialog.cpp',
	'src/View/Dialog/DetuneSynthesizerDialog.cpp',
	'src/View/Dialog/HelpDialog.cpp',
	'src/View/Dialog/MarkerDialog.cpp',
	'src/View/Dialog/ModuleSelectorDialog.cpp',
	'src/View/Dialog/NewDialog.cpp',
	'src/View/Dialog/PauseAddDialog.cpp',
	'src/View/Dialog/PauseEditDialog.cpp',
	'src/View/Dialog/QuestionDialog.cpp',
	'src/View/Dialog/SampleScaleDialog.cpp',
	'src/View/Dialog/SettingsDialog.cpp',
	'src/View/Dialog/TrackRoutingDialog.cpp',
	'src/View/Dialog/TuningDialog.cpp',
	'src/View/Graph/AudioViewLayer.cpp',
	'src/View/Graph/AudioViewTrack.cpp',
	'src/View/Graph/Background.cpp',
	'src/View/Graph/Cursor.cpp',
	'src/View/Graph/LayerHeader.cpp',
	'src/View/Graph/TimeScale.cpp',
	'src/View/Graph/TrackHeader.cpp',
	'src/View/Graph/ViewMarker.cpp',
	'src/View/Helper/Graph/Node.cpp',
	'src/View/Helper/Graph/SceneGraph.cpp',
	'src/View/Helper/Graph/Scrollable.cpp',
	'src/View/Helper/Graph/ScrollBar.cpp',
	'src/View/Helper/CpuDisplay.cpp',
	'src/View/Helper/Dial.cpp',
	'src/View/Helper/Drawing.cpp',
	'src/View/Helper/FxListEditor.cpp',
	'src/View/Helper/MidiPreview.cpp',
	'src/View/Helper/ModulePanel.cpp',
	'src/View/Helper/PeakMeterDisplay.cpp',
	'src/View/Helper/PeakThread.cpp',
	'src/View/Helper/Progress.cpp',
	'src/View/Helper/Slider.cpp',
	'src/View/Helper/SymbolRenderer.cpp',
	'src/View/Mode/ViewMode.cpp',
	'src/View/Mode/ViewModeCapture.cpp',
	'src/View/Mode/ViewModeCurve.cpp',
	'src/View/Mode/ViewModeDefault.cpp',
	'src/View/Mode/ViewModeEdit.cpp',
	'src/View/Mode/ViewModeEditAudio.cpp',
	'src/View/Mode/ViewModeEditDummy.cpp',
	'src/View/Mode/ViewModeMidi.cpp',
	'src/View/Mode/ViewModeScaleBars.cpp',
	'src/View/Mode/ViewModeScaleMarker.cpp',
	'src/View/Painter/BasicGridPainter.cpp',
	'src/View/Painter/BufferPainter.cpp',
	'src/View/Painter/GridPainter.cpp',
	'src/View/Painter/MidiPainter.cpp',
	'src/View/SideBar/CaptureConsoleModes/CaptureConsoleMode.cpp',
	'src/View/SideBar/CaptureConsoleModes/CaptureConsoleModeAudio.cpp',
	'src/View/SideBar/CaptureConsoleModes/CaptureConsoleModeMidi.cpp',
	'src/View/SideBar/CaptureConsoleModes/CaptureConsoleModeMulti.cpp',
	'src/View/SideBar/CaptureConsoleModes/CaptureTrackData.cpp',
	'src/View/SideBar/AudioEditorConsole.cpp',
	'src/View/SideBar/CaptureConsole.cpp',
	'src/View/SideBar/CurveConsole.cpp',
	'src/View/SideBar/DummyEditorConsole.cpp',
	'src/View/SideBar/MidiEditorConsole.cpp',
	'src/View/SideBar/SampleManagerConsole.cpp',
	'src/View/SideBar/SampleRefConsole.cpp',
	'src/View/SideBar/SideBar.cpp',
	'src/View/SideBar/SongConsole.cpp',
	'src/View/SideBar/TrackConsole.cpp',
	'src/View/AudioView.cpp',
	'src/View/ColorScheme.cpp',
	'src/View/HoverData.cpp',
	'src/View/MouseDelayPlanner.cpp',
	'src/View/TrackHeightManager.cpp',
	'src/View/ViewPort.cpp',
	'src/Session.cpp',
	'src/Tsunami.cpp',
	'src/TsunamiWindow.cpp'
]


executable('tsunami', sources, dependencies : deps, install : true)

install_subdir('static/icons', install_dir : join_paths(get_option('datadir'), 'tsunami'))
install_subdir('static/Midi Fonts', install_dir : join_paths(get_option('datadir'), 'tsunami'))
install_subdir('static/SignalChains', install_dir : join_paths(get_option('datadir'), 'tsunami'))
install_data('static/favorites_demo.txt', install_dir : join_paths(get_option('datadir'), 'tsunami'))
install_data('static/hui_resources.txt', install_dir : join_paths(get_option('datadir'), 'tsunami'))
install_data('static/icon.png', install_dir : join_paths(get_option('datadir'), 'tsunami'))
install_data('static/icon.svg', install_dir : join_paths(get_option('datadir'), 'tsunami'))
install_data('static/license.txt', install_dir : join_paths(get_option('datadir'), 'tsunami'))
install_data('static/license_small.txt', install_dir : join_paths(get_option('datadir'), 'tsunami'))
install_subdir('Plugins', install_dir : join_paths(get_option('datadir'), 'tsunami'))
install_data('static/michisoft-tsunami.desktop', install_dir : join_paths(get_option('datadir'), 'applications'))
