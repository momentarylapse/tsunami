BIN = ../tsunami
CPP = g++
prefix = /usr/local
GLOBALFLAGS = -fno-omit-frame-pointer -O2 -g -std=c++17 -DHAS_LIB_GTK3=1 -DHAS_LIB_ZLIB=1 -DHAS_LIB_FFTW3=1 -DHAS_LIB_OGG=1 -DHAS_LIB_FLAC=1 -DHAS_LIB_ALSA=1 -DHAS_LIB_PULSEAUDIO=1 -DHAS_LIB_PORTAUDIO=1 -DHAS_LIB_UNWIND=1 -DHAS_LIB_DL=1 -DHAS_LIB_GL=1 -D'INSTALL_PREFIX="$(prefix)"'
CXXFLAGS = `pkg-config --cflags gtk+-3.0 zlib fftw3f ogg vorbis vorbisfile vorbisenc flac alsa libpulse portaudio-2.0 libunwind gl` $(GLOBALFLAGS)
CXXFLAGSLINKING = -ldl `pkg-config --cflags --libs gtk+-3.0 zlib fftw3f ogg vorbis vorbisfile vorbisenc flac alsa libpulse portaudio-2.0 libunwind gl` $(GLOBALFLAGS)

OBJ = action/bar/Action__ScaleData.o \
 action/bar/Action__ShiftData.o \
 action/bar/ActionBar__Add.o \
 action/bar/ActionBar__Delete.o \
 action/bar/ActionBar__Edit.o \
 action/bar/ActionBarAdd.o \
 action/bar/ActionBarDelete.o \
 action/bar/ActionBarEdit.o \
 action/sample/ActionSampleAdd.o \
 action/sample/ActionSampleDelete.o \
 action/sample/ActionSampleEditName.o \
 action/sample/ActionSampleReplaceBuffer.o \
 action/song/data/ActionSongChangeAllTrackVolumes.o \
 action/song/data/ActionSongSetDefaultFormat.o \
 action/song/data/ActionSongSetSampleRate.o \
 action/song/ActionSongDeleteSelection.o \
 action/song/ActionSongMoveSelection.o \
 action/tag/ActionTagAdd.o \
 action/tag/ActionTagDelete.o \
 action/tag/ActionTagEdit.o \
 action/track/buffer/ActionTrack__AbsorbBuffer.o \
 action/track/buffer/ActionTrack__AddBuffer.o \
 action/track/buffer/ActionTrack__BufferSetChannels.o \
 action/track/buffer/ActionTrack__DeleteBuffer.o \
 action/track/buffer/ActionTrack__GrowBuffer.o \
 action/track/buffer/ActionTrack__ShrinkBuffer.o \
 action/track/buffer/ActionTrack__SplitBuffer.o \
 action/track/buffer/ActionTrackCreateBuffers.o \
 action/track/buffer/ActionTrackEditBuffer.o \
 action/track/buffer/ActionTrackMoveBuffer.o \
 action/track/buffer/ActionTrackSetChannels.o \
 action/track/curve/ActionTrackAddCurve.o \
 action/track/curve/ActionTrackCurveAddPoint.o \
 action/track/curve/ActionTrackCurveDeletePoint.o \
 action/track/curve/ActionTrackCurveEditPoint.o \
 action/track/curve/ActionTrackDeleteCurve.o \
 action/track/curve/ActionTrackEditCurve.o \
 action/track/data/ActionTrackEditMuted.o \
 action/track/data/ActionTrackEditName.o \
 action/track/data/ActionTrackEditPanning.o \
 action/track/data/ActionTrackEditVolume.o \
 action/track/data/ActionTrackSetInstrument.o \
 action/track/data/ActionTrackSetTarget.o \
 action/track/effect/ActionTrackAddAudioEffect.o \
 action/track/effect/ActionTrackDeleteEffect.o \
 action/track/effect/ActionTrackEditAudioEffect.o \
 action/track/effect/ActionTrackMoveAudioEffect.o \
 action/track/effect/ActionTrackToggleAudioEffectEnabled.o \
 action/track/layer/ActionTrackFadeAdd.o \
 action/track/layer/ActionTrackFadeDelete.o \
 action/track/layer/ActionTrackLayer__Delete.o \
 action/track/layer/ActionTrackLayerAdd.o \
 action/track/layer/ActionTrackLayerDelete.o \
 action/track/layer/ActionTrackLayerMakeTrack.o \
 action/track/layer/ActionTrackLayerMarkDominant.o \
 action/track/layer/ActionTrackLayerMerge.o \
 action/track/layer/ActionTrackLayerMove.o \
 action/track/marker/ActionTrackAddMarker.o \
 action/track/marker/ActionTrackDeleteMarker.o \
 action/track/marker/ActionTrackEditMarker.o \
 action/track/midi/ActionTrackAddMidiEffect.o \
 action/track/midi/ActionTrackAddMidiNote.o \
 action/track/midi/ActionTrackDeleteMidiEffect.o \
 action/track/midi/ActionTrackDeleteMidiNote.o \
 action/track/midi/ActionTrackEditMidiEffect.o \
 action/track/midi/ActionTrackEditMidiNote.o \
 action/track/midi/ActionTrackInsertMidi.o \
 action/track/midi/ActionTrackMoveMidiEffect.o \
 action/track/midi/ActionTrackToggleMidiEffectEnabled.o \
 action/track/sample/ActionTrackAddSample.o \
 action/track/sample/ActionTrackDeleteSample.o \
 action/track/sample/ActionTrackEditSample.o \
 action/track/sample/ActionTrackInsertSample.o \
 action/track/sample/ActionTrackPasteAsSample.o \
 action/track/sample/ActionTrackSampleFromSelection.o \
 action/track/synthesizer/ActionTrackDetuneSynthesizer.o \
 action/track/synthesizer/ActionTrackEditSynthesizer.o \
 action/track/synthesizer/ActionTrackSetSynthesizer.o \
 action/track/ActionTrack__DeleteEmpty.o \
 action/track/ActionTrackAdd.o \
 action/track/ActionTrackDelete.o \
 action/track/ActionTrackMove.o \
 action/Action.o \
 action/ActionGroup.o \
 action/ActionManager.o \
 action/ActionMergable.o \
 data/audio/AudioBuffer.o \
 data/audio/BufferInterpolator.o \
 data/audio/RingBuffer.o \
 data/midi/Clef.o \
 data/midi/Instrument.o \
 data/midi/MidiData.o \
 data/midi/MidiEvent.o \
 data/midi/MidiNote.o \
 data/midi/Scale.o \
 data/midi/Temperament.o \
 data/rhythm/Bar.o \
 data/rhythm/BarCollection.o \
 data/rhythm/Beat.o \
 data/base.o \
 data/CrossFade.o \
 data/Curve.o \
 data/Data.o \
 data/Range.o \
 data/Sample.o \
 data/SampleRef.o \
 data/Song.o \
 data/SongSelection.o \
 data/Track.o \
 data/TrackLayer.o \
 data/TrackMarker.o \
 device/stream/AudioInput.o \
 device/stream/AudioOutput.o \
 device/stream/MidiInput.o \
 device/Device.o \
 device/DeviceManager.o \
 lib/any/any.o \
 lib/base/array.o \
 lib/base/pointer.o \
 lib/base/strings.o \
 lib/file/file.o \
 lib/file/file_op.o \
 lib/file/msg.o \
 lib/file/path.o \
 lib/hui/Controls/Control.o \
 lib/hui/Controls/ControlButtonGtk.o \
 lib/hui/Controls/ControlCheckBoxGtk.o \
 lib/hui/Controls/ControlColorButtonGtk.o \
 lib/hui/Controls/ControlComboBoxGtk.o \
 lib/hui/Controls/ControlDrawingAreaGtk.o \
 lib/hui/Controls/ControlEditGtk.o \
 lib/hui/Controls/ControlExpanderGtk.o \
 lib/hui/Controls/ControlGridGtk.o \
 lib/hui/Controls/ControlGroupGtk.o \
 lib/hui/Controls/ControlHeaderBar.o \
 lib/hui/Controls/ControlLabelGtk.o \
 lib/hui/Controls/ControlListViewGtk.o \
 lib/hui/Controls/ControlMenuButtonGtk.o \
 lib/hui/Controls/ControlMultilineEditGtk.o \
 lib/hui/Controls/ControlPanedGtk.o \
 lib/hui/Controls/ControlProgressBarGtk.o \
 lib/hui/Controls/ControlRadioButtonGtk.o \
 lib/hui/Controls/ControlScrollerGtk.o \
 lib/hui/Controls/ControlSeparatorGtk.o \
 lib/hui/Controls/ControlSliderGtk.o \
 lib/hui/Controls/ControlSpinButtonGtk.o \
 lib/hui/Controls/ControlTabControlGtk.o \
 lib/hui/Controls/ControlToggleButtonGtk.o \
 lib/hui/Controls/ControlTreeViewGtk.o \
 lib/hui/Controls/MenuItemGtk.o \
 lib/hui/Controls/MenuItemSeparatorGtk.o \
 lib/hui/Controls/MenuItemSeparatorWin.o \
 lib/hui/Controls/MenuItemSubmenuGtk.o \
 lib/hui/Controls/MenuItemSubmenuWin.o \
 lib/hui/Controls/MenuItemToggleGtk.o \
 lib/hui/Controls/MenuItemToggleWin.o \
 lib/hui/Controls/MenuItemWin.o \
 lib/hui/Controls/ToolItemButtonGtk.o \
 lib/hui/Controls/ToolItemButtonWin.o \
 lib/hui/Controls/ToolItemMenuButtonGtk.o \
 lib/hui/Controls/ToolItemMenuButtonWin.o \
 lib/hui/Controls/ToolItemSeparatorGtk.o \
 lib/hui/Controls/ToolItemSeparatorWin.o \
 lib/hui/Controls/ToolItemToggleButtonGtk.o \
 lib/hui/Controls/ToolItemToggleButtonWin.o \
 lib/hui/Application.o \
 lib/hui/Callback.o \
 lib/hui/Clipboard.o \
 lib/hui/common_dlg.o \
 lib/hui/common_dlg_gtk.o \
 lib/hui/common_dlg_win.o \
 lib/hui/Config.o \
 lib/hui/error.o \
 lib/hui/Event.o \
 lib/hui/hui.o \
 lib/hui/language.o \
 lib/hui/main_gtk.o \
 lib/hui/main_win.o \
 lib/hui/Menu.o \
 lib/hui/MenuGtk.o \
 lib/hui/MenuWin.o \
 lib/hui/PainterGtk.o \
 lib/hui/Panel.o \
 lib/hui/Resource.o \
 lib/hui/Timer.o \
 lib/hui/Toolbar.o \
 lib/hui/ToolbarGtk.o \
 lib/hui/ToolbarWin.o \
 lib/hui/utility.o \
 lib/hui/Window.o \
 lib/hui/window_control.o \
 lib/hui/window_control_gtk.o \
 lib/hui/window_control_win.o \
 lib/hui/WindowGtk.o \
 lib/hui/WindowWin.o \
 lib/image/color.o \
 lib/image/image.o \
 lib/image/image_bmp.o \
 lib/image/image_jpg.o \
 lib/image/image_png.o \
 lib/image/image_tga.o \
 lib/image/ImagePainter.o \
 lib/kaba/asm/asm.o \
 lib/kaba/asm/asm_arm.o \
 lib/kaba/asm/asm_x86.o \
 lib/kaba/compiler/Backend.o \
 lib/kaba/compiler/BackendAmd64.o \
 lib/kaba/compiler/BackendARM.o \
 lib/kaba/compiler/BackendX86.o \
 lib/kaba/compiler/CommandList.o \
 lib/kaba/compiler/compiler.o \
 lib/kaba/compiler/mapper.o \
 lib/kaba/compiler/serializer.o \
 lib/kaba/compiler/SerializerX.o \
 lib/kaba/compiler/SerialNode.o \
 lib/kaba/dynamic/call.o \
 lib/kaba/dynamic/dynamic.o \
 lib/kaba/dynamic/exception.o \
 lib/kaba/lib/dict.o \
 lib/kaba/lib/extern.o \
 lib/kaba/lib/lib.o \
 lib/kaba/lib/lib_base.o \
 lib/kaba/lib/lib_doc.o \
 lib/kaba/lib/lib_hui.o \
 lib/kaba/lib/lib_image.o \
 lib/kaba/lib/lib_kaba.o \
 lib/kaba/lib/lib_math.o \
 lib/kaba/lib/lib_net.o \
 lib/kaba/lib/lib_nix.o \
 lib/kaba/lib/lib_os.o \
 lib/kaba/lib/lib_threads.o \
 lib/kaba/lib/lib_time.o \
 lib/kaba/lib/lib_vulkan.o \
 lib/kaba/syntax/Block.o \
 lib/kaba/syntax/Class.o \
 lib/kaba/syntax/Constant.o \
 lib/kaba/syntax/Function.o \
 lib/kaba/syntax/Identifier.o \
 lib/kaba/syntax/implicit.o \
 lib/kaba/syntax/lexical.o \
 lib/kaba/syntax/macros.o \
 lib/kaba/syntax/Node.o \
 lib/kaba/syntax/Operator.o \
 lib/kaba/syntax/Parser.o \
 lib/kaba/syntax/preprocessor.o \
 lib/kaba/syntax/Statement.o \
 lib/kaba/syntax/SyntaxTree.o \
 lib/kaba/syntax/Variable.o \
 lib/kaba/CompilerConfiguration.o \
 lib/kaba/Interpreter.o \
 lib/kaba/kaba.o \
 lib/math/complex.o \
 lib/math/interpolation.o \
 lib/math/math.o \
 lib/math/matrix.o \
 lib/math/matrix3.o \
 lib/math/plane.o \
 lib/math/quaternion.o \
 lib/math/random.o \
 lib/math/ray.o \
 lib/math/rect.o \
 lib/math/vec2.o \
 lib/math/vec4.o \
 lib/math/vector.o \
 lib/net/BinaryBuffer.o \
 lib/net/net.o \
 lib/net/Socket.o \
 lib/nix/nix.o \
 lib/nix/nix_buffer.o \
 lib/nix/nix_draw.o \
 lib/nix/nix_framebuffer.o \
 lib/nix/nix_light.o \
 lib/nix/nix_shader.o \
 lib/nix/nix_textures.o \
 lib/nix/nix_vertexbuffer.o \
 lib/nix/nix_view.o \
 lib/threads/Mutex.o \
 lib/threads/Thread.o \
 lib/threads/ThreadedWork.o \
 lib/xfile/chunked.o \
 lib/xfile/pdf.o \
 lib/xfile/xml.o \
 module/audio/AudioAccumulator.o \
 module/audio/AudioBackup.o \
 module/audio/AudioChannelSelector.o \
 module/audio/AudioEffect.o \
 module/audio/AudioJoiner.o \
 module/audio/AudioSource.o \
 module/audio/AudioSucker.o \
 module/audio/AudioVisualizer.o \
 module/audio/BufferStreamer.o \
 module/audio/PeakMeter.o \
 module/audio/PitchDetector.o \
 module/audio/SongRenderer.o \
 module/audio/TrackRenderer.o \
 module/beats/BarStreamer.o \
 module/beats/BeatMidifier.o \
 module/beats/BeatSource.o \
 module/midi/MidiAccumulator.o \
 module/midi/MidiEffect.o \
 module/midi/MidiEventStreamer.o \
 module/midi/MidiJoiner.o \
 module/midi/MidiPreviewSource.o \
 module/midi/MidiSource.o \
 module/midi/MidiSucker.o \
 module/port/Port.o \
 module/synthesizer/DebugSynthesizer.o \
 module/synthesizer/DummySynthesizer.o \
 module/synthesizer/EnvelopeADSR.o \
 module/synthesizer/Synthesizer.o \
 module/AutoConfigPanel.o \
 module/ConfigPanel.o \
 module/Module.o \
 module/ModuleConfiguration.o \
 module/ModuleFactory.o \
 module/SignalChain.o \
 plugins/ExtendedAudioBuffer.o \
 plugins/FastFourierTransform.o \
 plugins/Plugin.o \
 plugins/PluginManager.o \
 plugins/ProfileManager.o \
 plugins/TsunamiPlugin.o \
 storage/dialog/PdfConfigDialog.o \
 storage/dialog/RawConfigDialog.o \
 storage/format/Format.o \
 storage/format/FormatFlac.o \
 storage/format/FormatGuitarPro.o \
 storage/format/FormatM4a.o \
 storage/format/FormatMidi.o \
 storage/format/FormatMp3.o \
 storage/format/FormatNami.o \
 storage/format/FormatOgg.o \
 storage/format/FormatPdf.o \
 storage/format/FormatRaw.o \
 storage/format/FormatSoundFont2.o \
 storage/format/FormatWave.o \
 storage/Storage.o \
 storage/StorageOperationData.o \
 stuff/BackupManager.o \
 stuff/CLIParser.o \
 stuff/Clipboard.o \
 stuff/Diff.o \
 stuff/ErrorHandler.o \
 stuff/Log.o \
 stuff/Observable.o \
 stuff/PerformanceMonitor.o \
 test/TestAudioBuffer.o \
 test/TestInterpolator.o \
 test/TestMidiPreview.o \
 test/TestMixer.o \
 test/TestPlugins.o \
 test/TestPointer.o \
 test/TestRhythm.o \
 test/TestRingBuffer.o \
 test/TestSignalChain.o \
 test/TestStreams.o \
 test/TestThreads.o \
 test/TestTrackVersion.o \
 test/UnitTest.o \
 view/audioview/graph/AudioViewLayer.o \
 view/audioview/graph/AudioViewTrack.o \
 view/audioview/graph/Background.o \
 view/audioview/graph/Cursor.o \
 view/audioview/graph/LayerHeader.o \
 view/audioview/graph/TimeScale.o \
 view/audioview/graph/TrackHeader.o \
 view/audioview/graph/ViewMarker.o \
 view/audioview/AudioView.o \
 view/audioview/TrackHeightManager.o \
 view/audioview/ViewPort.o \
 view/bottombar/signaleditor/SignalEditorBackground.o \
 view/bottombar/signaleditor/SignalEditorCable.o \
 view/bottombar/signaleditor/SignalEditorModule.o \
 view/bottombar/signaleditor/SignalEditorPort.o \
 view/bottombar/signaleditor/SignalEditorTab.o \
 view/bottombar/BottomBar.o \
 view/bottombar/DeviceConsole.o \
 view/bottombar/LogConsole.o \
 view/bottombar/MiniBar.o \
 view/bottombar/MixingConsole.o \
 view/bottombar/PluginConsole.o \
 view/bottombar/SignalEditor.o \
 view/dialog/BarAddDialog.o \
 view/dialog/BarDeleteDialog.o \
 view/dialog/BarEditDialog.o \
 view/dialog/BufferCompressionDialog.o \
 view/dialog/ChannelMapperDialog.o \
 view/dialog/HelpDialog.o \
 view/dialog/MarkerDialog.o \
 view/dialog/ModuleSelectorDialog.o \
 view/dialog/NewDialog.o \
 view/dialog/PauseAddDialog.o \
 view/dialog/PauseEditDialog.o \
 view/dialog/QuestionDialog.o \
 view/dialog/SampleScaleDialog.o \
 view/dialog/SettingsDialog.o \
 view/dialog/TemperamentDialog.o \
 view/dialog/TimeTrackAddDialog.o \
 view/dialog/TrackRoutingDialog.o \
 view/dialog/TuningDialog.o \
 view/helper/graph/Node.o \
 view/helper/graph/SceneGraph.o \
 view/helper/graph/Scrollable.o \
 view/helper/graph/ScrollBar.o \
 view/helper/CpuDisplay.o \
 view/helper/Dial.o \
 view/helper/Drawing.o \
 view/helper/FxListEditor.o \
 view/helper/MidiPreview.o \
 view/helper/ModulePanel.o \
 view/helper/PeakMeterDisplay.o \
 view/helper/PeakThread.o \
 view/helper/Progress.o \
 view/helper/Slider.o \
 view/helper/SymbolRenderer.o \
 view/mode/ViewMode.o \
 view/mode/ViewModeCapture.o \
 view/mode/ViewModeCurve.o \
 view/mode/ViewModeDefault.o \
 view/mode/ViewModeEdit.o \
 view/mode/ViewModeEditAudio.o \
 view/mode/ViewModeEditDummy.o \
 view/mode/ViewModeMidi.o \
 view/mode/ViewModeScaleBars.o \
 view/mode/ViewModeScaleMarker.o \
 view/painter/BasicGridPainter.o \
 view/painter/BufferPainter.o \
 view/painter/GridPainter.o \
 view/painter/MidiPainter.o \
 view/painter/MultiLinePainter.o \
 view/sidebar/captureconsolemodes/CaptureConsoleMode.o \
 view/sidebar/captureconsolemodes/CaptureConsoleModeAudio.o \
 view/sidebar/captureconsolemodes/CaptureConsoleModeMidi.o \
 view/sidebar/captureconsolemodes/CaptureConsoleModeMulti.o \
 view/sidebar/captureconsolemodes/CaptureTrackData.o \
 view/sidebar/AudioEditorConsole.o \
 view/sidebar/CaptureConsole.o \
 view/sidebar/CurveConsole.o \
 view/sidebar/DummyEditorConsole.o \
 view/sidebar/MidiEditorConsole.o \
 view/sidebar/SampleManagerConsole.o \
 view/sidebar/SampleRefConsole.o \
 view/sidebar/SideBar.o \
 view/sidebar/SongConsole.o \
 view/sidebar/TrackConsole.o \
 view/ColorScheme.o \
 view/HoverData.o \
 view/MouseDelayPlanner.o \
 Session.o \
 TsunamiWindow.o

$(BIN) : $(OBJ) Tsunami.o
	$(CPP) $(OBJ) Tsunami.o -o $(BIN) $(CXXFLAGSLINKING)

all : $(BIN)

action/bar/Action__ScaleData.o : action/bar/Action__ScaleData.cpp action/bar/Action__ScaleData.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Range.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h lib/base/set.h data/TrackMarker.h module/audio/AudioEffect.h module/Module.h module/port/Port.h data/CrossFade.h data/Song.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h data/SampleRef.h lib/math/rect.h lib/math/math.h data/audio/AudioBuffer.h
	$(CPP) -c action/bar/Action__ScaleData.cpp -o $@ $(CXXFLAGS)

action/bar/Action__ShiftData.o : action/bar/Action__ShiftData.cpp action/bar/Action__ShiftData.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Track.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h lib/base/set.h data/TrackMarker.h module/audio/AudioEffect.h module/Module.h module/port/Port.h data/CrossFade.h data/Song.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h data/SampleRef.h lib/math/rect.h lib/math/math.h data/audio/AudioBuffer.h
	$(CPP) -c action/bar/Action__ShiftData.cpp -o $@ $(CXXFLAGS)

action/bar/ActionBar__Add.o : action/bar/ActionBar__Add.cpp action/bar/ActionBar__Add.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Song.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h
	$(CPP) -c action/bar/ActionBar__Add.cpp -o $@ $(CXXFLAGS)

action/bar/ActionBar__Delete.o : action/bar/ActionBar__Delete.cpp action/bar/ActionBar__Delete.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Song.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h
	$(CPP) -c action/bar/ActionBar__Delete.cpp -o $@ $(CXXFLAGS)

action/bar/ActionBar__Edit.o : action/bar/ActionBar__Edit.cpp action/bar/ActionBar__Edit.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/rhythm/Bar.h data/Range.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/Song.h data/rhythm/BarCollection.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h
	$(CPP) -c action/bar/ActionBar__Edit.cpp -o $@ $(CXXFLAGS)

action/bar/ActionBarAdd.o : action/bar/ActionBarAdd.cpp action/bar/ActionBarAdd.h action/ActionGroup.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/rhythm/Bar.h data/Range.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h lib/base/set.h data/TrackMarker.h module/audio/AudioEffect.h module/Module.h module/port/Port.h data/Song.h data/rhythm/BarCollection.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h data/audio/AudioBuffer.h action/track/buffer/ActionTrack__SplitBuffer.h action/track/marker/ActionTrackEditMarker.h action/track/midi/ActionTrackEditMidiNote.h action/bar/Action__ShiftData.h action/bar/ActionBar__Add.h
	$(CPP) -c action/bar/ActionBarAdd.cpp -o $@ $(CXXFLAGS)

action/bar/ActionBarDelete.o : action/bar/ActionBarDelete.cpp action/bar/ActionBarDelete.h action/ActionGroup.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h action/song/ActionSongDeleteSelection.h data/Song.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h action/track/marker/ActionTrackEditMarker.h action/track/marker/ActionTrackDeleteMarker.h action/track/midi/ActionTrackEditMidiNote.h action/track/midi/ActionTrackDeleteMidiNote.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h lib/base/set.h data/TrackMarker.h module/audio/AudioEffect.h module/Module.h module/port/Port.h data/SongSelection.h action/bar/Action__ShiftData.h action/bar/ActionBar__Delete.h
	$(CPP) -c action/bar/ActionBarDelete.cpp -o $@ $(CXXFLAGS)

action/bar/ActionBarEdit.o : action/bar/ActionBarEdit.cpp action/bar/ActionBarEdit.h action/ActionGroup.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Track.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h lib/base/set.h data/audio/AudioBuffer.h data/Song.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h data/audio/BufferInterpolator.h action/bar/Action__ScaleData.h action/bar/ActionBar__Edit.h action/track/buffer/ActionTrackEditBuffer.h action/track/buffer/ActionTrack__GrowBuffer.h action/track/buffer/ActionTrack__ShrinkBuffer.h
	$(CPP) -c action/bar/ActionBarEdit.cpp -o $@ $(CXXFLAGS)

action/sample/ActionSampleAdd.o : action/sample/ActionSampleAdd.cpp action/sample/ActionSampleAdd.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/base.h data/Song.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h data/Sample.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h lib/math/rect.h lib/math/math.h data/audio/AudioBuffer.h
	$(CPP) -c action/sample/ActionSampleAdd.cpp -o $@ $(CXXFLAGS)

action/sample/ActionSampleDelete.o : action/sample/ActionSampleDelete.cpp action/sample/ActionSampleDelete.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Song.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h data/Sample.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h lib/math/rect.h lib/math/math.h data/audio/AudioBuffer.h
	$(CPP) -c action/sample/ActionSampleDelete.cpp -o $@ $(CXXFLAGS)

action/sample/ActionSampleEditName.o : action/sample/ActionSampleEditName.cpp action/sample/ActionSampleEditName.h action/ActionMergable.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Sample.h data/midi/MidiData.h data/Range.h data/midi/MidiNote.h data/midi/MidiEvent.h lib/math/rect.h lib/math/math.h data/audio/AudioBuffer.h
	$(CPP) -c action/sample/ActionSampleEditName.cpp -o $@ $(CXXFLAGS)

action/sample/ActionSampleReplaceBuffer.o : action/sample/ActionSampleReplaceBuffer.cpp action/sample/ActionSampleReplaceBuffer.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Sample.h data/midi/MidiData.h data/Range.h data/midi/MidiNote.h data/midi/MidiEvent.h lib/math/rect.h lib/math/math.h data/audio/AudioBuffer.h
	$(CPP) -c action/sample/ActionSampleReplaceBuffer.cpp -o $@ $(CXXFLAGS)

action/song/data/ActionSongChangeAllTrackVolumes.o : action/song/data/ActionSongChangeAllTrackVolumes.cpp action/song/data/ActionSongChangeAllTrackVolumes.h action/ActionMergable.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Song.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h
	$(CPP) -c action/song/data/ActionSongChangeAllTrackVolumes.cpp -o $@ $(CXXFLAGS)

action/song/data/ActionSongSetDefaultFormat.o : action/song/data/ActionSongSetDefaultFormat.cpp action/song/data/ActionSongSetDefaultFormat.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/audio/AudioBuffer.h data/Range.h data/Song.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h
	$(CPP) -c action/song/data/ActionSongSetDefaultFormat.cpp -o $@ $(CXXFLAGS)

action/song/data/ActionSongSetSampleRate.o : action/song/data/ActionSongSetSampleRate.cpp action/song/data/ActionSongSetSampleRate.h action/ActionMergable.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Song.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h
	$(CPP) -c action/song/data/ActionSongSetSampleRate.cpp -o $@ $(CXXFLAGS)

action/song/ActionSongDeleteSelection.o : action/song/ActionSongDeleteSelection.cpp action/song/ActionSongDeleteSelection.h data/Song.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h action/ActionGroup.h action/Action.h action/track/sample/ActionTrackDeleteSample.h action/track/marker/ActionTrackDeleteMarker.h action/track/midi/ActionTrackDeleteMidiNote.h data/SongSelection.h lib/base/set.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h data/SampleRef.h lib/math/rect.h lib/math/math.h action/track/buffer/ActionTrack__DeleteBuffer.h data/audio/AudioBuffer.h action/track/buffer/ActionTrack__ShrinkBuffer.h action/track/buffer/ActionTrack__SplitBuffer.h
	$(CPP) -c action/song/ActionSongDeleteSelection.cpp -o $@ $(CXXFLAGS)

action/song/ActionSongMoveSelection.o : action/song/ActionSongMoveSelection.cpp action/song/ActionSongMoveSelection.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Song.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h lib/base/set.h data/TrackMarker.h module/audio/AudioEffect.h module/Module.h module/port/Port.h data/SongSelection.h data/SampleRef.h lib/math/rect.h lib/math/math.h data/audio/AudioBuffer.h
	$(CPP) -c action/song/ActionSongMoveSelection.cpp -o $@ $(CXXFLAGS)

action/tag/ActionTagAdd.o : action/tag/ActionTagAdd.cpp action/tag/ActionTagAdd.h data/Song.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h action/Action.h
	$(CPP) -c action/tag/ActionTagAdd.cpp -o $@ $(CXXFLAGS)

action/tag/ActionTagDelete.o : action/tag/ActionTagDelete.cpp action/tag/ActionTagDelete.h data/Song.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h action/Action.h
	$(CPP) -c action/tag/ActionTagDelete.cpp -o $@ $(CXXFLAGS)

action/tag/ActionTagEdit.o : action/tag/ActionTagEdit.cpp action/tag/ActionTagEdit.h data/Song.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h action/Action.h
	$(CPP) -c action/tag/ActionTagEdit.cpp -o $@ $(CXXFLAGS)

action/track/buffer/ActionTrack__AbsorbBuffer.o : action/track/buffer/ActionTrack__AbsorbBuffer.cpp action/track/buffer/ActionTrack__AbsorbBuffer.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/TrackLayer.h lib/base/set.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/audio/AudioBuffer.h
	$(CPP) -c action/track/buffer/ActionTrack__AbsorbBuffer.cpp -o $@ $(CXXFLAGS)

action/track/buffer/ActionTrack__AddBuffer.o : action/track/buffer/ActionTrack__AddBuffer.cpp action/track/buffer/ActionTrack__AddBuffer.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Track.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h lib/base/set.h data/audio/AudioBuffer.h
	$(CPP) -c action/track/buffer/ActionTrack__AddBuffer.cpp -o $@ $(CXXFLAGS)

action/track/buffer/ActionTrack__BufferSetChannels.o : action/track/buffer/ActionTrack__BufferSetChannels.cpp action/track/buffer/ActionTrack__BufferSetChannels.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/TrackLayer.h lib/base/set.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/audio/AudioBuffer.h
	$(CPP) -c action/track/buffer/ActionTrack__BufferSetChannels.cpp -o $@ $(CXXFLAGS)

action/track/buffer/ActionTrack__DeleteBuffer.o : action/track/buffer/ActionTrack__DeleteBuffer.cpp action/track/buffer/ActionTrack__DeleteBuffer.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/audio/AudioBuffer.h data/Range.h data/TrackLayer.h lib/base/set.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h
	$(CPP) -c action/track/buffer/ActionTrack__DeleteBuffer.cpp -o $@ $(CXXFLAGS)

action/track/buffer/ActionTrack__GrowBuffer.o : action/track/buffer/ActionTrack__GrowBuffer.cpp action/track/buffer/ActionTrack__GrowBuffer.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/TrackLayer.h lib/base/set.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/audio/AudioBuffer.h
	$(CPP) -c action/track/buffer/ActionTrack__GrowBuffer.cpp -o $@ $(CXXFLAGS)

action/track/buffer/ActionTrack__ShrinkBuffer.o : action/track/buffer/ActionTrack__ShrinkBuffer.cpp action/track/buffer/ActionTrack__ShrinkBuffer.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/audio/AudioBuffer.h data/Range.h data/TrackLayer.h lib/base/set.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h
	$(CPP) -c action/track/buffer/ActionTrack__ShrinkBuffer.cpp -o $@ $(CXXFLAGS)

action/track/buffer/ActionTrack__SplitBuffer.o : action/track/buffer/ActionTrack__SplitBuffer.cpp action/track/buffer/ActionTrack__SplitBuffer.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/TrackLayer.h lib/base/set.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/audio/AudioBuffer.h
	$(CPP) -c action/track/buffer/ActionTrack__SplitBuffer.cpp -o $@ $(CXXFLAGS)

action/track/buffer/ActionTrackCreateBuffers.o : action/track/buffer/ActionTrackCreateBuffers.cpp action/track/buffer/ActionTrackCreateBuffers.h action/ActionGroup.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Range.h action/track/buffer/ActionTrack__AbsorbBuffer.h action/track/buffer/ActionTrack__AddBuffer.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h action/track/buffer/ActionTrack__GrowBuffer.h data/TrackLayer.h lib/base/set.h data/Song.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h data/audio/AudioBuffer.h
	$(CPP) -c action/track/buffer/ActionTrackCreateBuffers.cpp -o $@ $(CXXFLAGS)

action/track/buffer/ActionTrackEditBuffer.o : action/track/buffer/ActionTrackEditBuffer.cpp action/track/buffer/ActionTrackEditBuffer.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/audio/AudioBuffer.h data/Range.h action/ActionManager.h data/TrackLayer.h lib/base/set.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/Song.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h
	$(CPP) -c action/track/buffer/ActionTrackEditBuffer.cpp -o $@ $(CXXFLAGS)

action/track/buffer/ActionTrackMoveBuffer.o : action/track/buffer/ActionTrackMoveBuffer.cpp action/track/buffer/ActionTrackMoveBuffer.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/TrackLayer.h lib/base/set.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/audio/AudioBuffer.h
	$(CPP) -c action/track/buffer/ActionTrackMoveBuffer.cpp -o $@ $(CXXFLAGS)

action/track/buffer/ActionTrackSetChannels.o : action/track/buffer/ActionTrackSetChannels.cpp action/track/buffer/ActionTrackSetChannels.h action/ActionGroup.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Track.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h lib/base/set.h data/Song.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h action/track/buffer/ActionTrack__BufferSetChannels.h
	$(CPP) -c action/track/buffer/ActionTrackSetChannels.cpp -o $@ $(CXXFLAGS)

action/track/curve/ActionTrackAddCurve.o : action/track/curve/ActionTrackAddCurve.cpp action/track/curve/ActionTrackAddCurve.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Track.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/Curve.h
	$(CPP) -c action/track/curve/ActionTrackAddCurve.cpp -o $@ $(CXXFLAGS)

action/track/curve/ActionTrackCurveAddPoint.o : action/track/curve/ActionTrackCurveAddPoint.cpp action/track/curve/ActionTrackCurveAddPoint.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Song.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h data/Curve.h
	$(CPP) -c action/track/curve/ActionTrackCurveAddPoint.cpp -o $@ $(CXXFLAGS)

action/track/curve/ActionTrackCurveDeletePoint.o : action/track/curve/ActionTrackCurveDeletePoint.cpp action/track/curve/ActionTrackCurveDeletePoint.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Curve.h
	$(CPP) -c action/track/curve/ActionTrackCurveDeletePoint.cpp -o $@ $(CXXFLAGS)

action/track/curve/ActionTrackCurveEditPoint.o : action/track/curve/ActionTrackCurveEditPoint.cpp action/track/curve/ActionTrackCurveEditPoint.h action/ActionMergable.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Curve.h
	$(CPP) -c action/track/curve/ActionTrackCurveEditPoint.cpp -o $@ $(CXXFLAGS)

action/track/curve/ActionTrackDeleteCurve.o : action/track/curve/ActionTrackDeleteCurve.cpp action/track/curve/ActionTrackDeleteCurve.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Track.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/Curve.h
	$(CPP) -c action/track/curve/ActionTrackDeleteCurve.cpp -o $@ $(CXXFLAGS)

action/track/curve/ActionTrackEditCurve.o : action/track/curve/ActionTrackEditCurve.cpp action/track/curve/ActionTrackEditCurve.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Curve.h data/Track.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h
	$(CPP) -c action/track/curve/ActionTrackEditCurve.cpp -o $@ $(CXXFLAGS)

action/track/data/ActionTrackEditMuted.o : action/track/data/ActionTrackEditMuted.cpp action/track/data/ActionTrackEditMuted.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Track.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h
	$(CPP) -c action/track/data/ActionTrackEditMuted.cpp -o $@ $(CXXFLAGS)

action/track/data/ActionTrackEditName.o : action/track/data/ActionTrackEditName.cpp action/track/data/ActionTrackEditName.h action/ActionMergable.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Track.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h
	$(CPP) -c action/track/data/ActionTrackEditName.cpp -o $@ $(CXXFLAGS)

action/track/data/ActionTrackEditPanning.o : action/track/data/ActionTrackEditPanning.cpp action/track/data/ActionTrackEditPanning.h action/ActionMergable.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Track.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h
	$(CPP) -c action/track/data/ActionTrackEditPanning.cpp -o $@ $(CXXFLAGS)

action/track/data/ActionTrackEditVolume.o : action/track/data/ActionTrackEditVolume.cpp action/track/data/ActionTrackEditVolume.h action/ActionMergable.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Track.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h
	$(CPP) -c action/track/data/ActionTrackEditVolume.cpp -o $@ $(CXXFLAGS)

action/track/data/ActionTrackSetInstrument.o : action/track/data/ActionTrackSetInstrument.cpp action/track/data/ActionTrackSetInstrument.h action/ActionMergable.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/midi/Instrument.h data/Track.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/TrackLayer.h lib/base/set.h
	$(CPP) -c action/track/data/ActionTrackSetInstrument.cpp -o $@ $(CXXFLAGS)

action/track/data/ActionTrackSetTarget.o : action/track/data/ActionTrackSetTarget.cpp action/track/data/ActionTrackSetTarget.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Track.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h
	$(CPP) -c action/track/data/ActionTrackSetTarget.cpp -o $@ $(CXXFLAGS)

action/track/effect/ActionTrackAddAudioEffect.o : action/track/effect/ActionTrackAddAudioEffect.cpp action/track/effect/ActionTrackAddEffect.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Track.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/Song.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h module/audio/AudioEffect.h module/Module.h module/port/Port.h
	$(CPP) -c action/track/effect/ActionTrackAddAudioEffect.cpp -o $@ $(CXXFLAGS)

action/track/effect/ActionTrackDeleteEffect.o : action/track/effect/ActionTrackDeleteEffect.cpp action/track/effect/ActionTrackDeleteAudioEffect.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Track.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/Song.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h module/audio/AudioEffect.h module/Module.h module/port/Port.h
	$(CPP) -c action/track/effect/ActionTrackDeleteEffect.cpp -o $@ $(CXXFLAGS)

action/track/effect/ActionTrackEditAudioEffect.o : action/track/effect/ActionTrackEditAudioEffect.cpp action/track/effect/ActionTrackEditEffect.h action/ActionMergable.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Track.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h module/audio/AudioEffect.h module/Module.h module/port/Port.h
	$(CPP) -c action/track/effect/ActionTrackEditAudioEffect.cpp -o $@ $(CXXFLAGS)

action/track/effect/ActionTrackMoveAudioEffect.o : action/track/effect/ActionTrackMoveAudioEffect.cpp action/track/effect/ActionTrackMoveAudioEffect.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Track.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h
	$(CPP) -c action/track/effect/ActionTrackMoveAudioEffect.cpp -o $@ $(CXXFLAGS)

action/track/effect/ActionTrackToggleAudioEffectEnabled.o : action/track/effect/ActionTrackToggleAudioEffectEnabled.cpp action/track/effect/ActionTrackToggleEffectEnabled.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h module/audio/AudioEffect.h data/Range.h module/Module.h module/port/Port.h data/Song.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h
	$(CPP) -c action/track/effect/ActionTrackToggleAudioEffectEnabled.cpp -o $@ $(CXXFLAGS)

action/track/layer/ActionTrackFadeAdd.o : action/track/layer/ActionTrackFadeAdd.cpp action/track/layer/ActionTrackFadeAdd.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/CrossFade.h data/Range.h data/TrackLayer.h lib/base/set.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h
	$(CPP) -c action/track/layer/ActionTrackFadeAdd.cpp -o $@ $(CXXFLAGS)

action/track/layer/ActionTrackFadeDelete.o : action/track/layer/ActionTrackFadeDelete.cpp action/track/layer/ActionTrackFadeDelete.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/CrossFade.h data/Range.h data/TrackLayer.h lib/base/set.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h
	$(CPP) -c action/track/layer/ActionTrackFadeDelete.cpp -o $@ $(CXXFLAGS)

action/track/layer/ActionTrackLayer__Delete.o : action/track/layer/ActionTrackLayer__Delete.cpp action/track/layer/ActionTrackLayer__Delete.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Song.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h lib/base/set.h data/CrossFade.h
	$(CPP) -c action/track/layer/ActionTrackLayer__Delete.cpp -o $@ $(CXXFLAGS)

action/track/layer/ActionTrackLayerAdd.o : action/track/layer/ActionTrackLayerAdd.cpp action/track/layer/ActionTrackLayerAdd.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Song.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h lib/base/set.h
	$(CPP) -c action/track/layer/ActionTrackLayerAdd.cpp -o $@ $(CXXFLAGS)

action/track/layer/ActionTrackLayerDelete.o : action/track/layer/ActionTrackLayerDelete.cpp action/track/layer/ActionTrackLayerDelete.h action/ActionGroup.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Track.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h lib/base/set.h data/Song.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h action/track/buffer/ActionTrack__DeleteBuffer.h data/audio/AudioBuffer.h action/track/layer/ActionTrackLayer__Delete.h action/track/layer/ActionTrackFadeDelete.h data/CrossFade.h
	$(CPP) -c action/track/layer/ActionTrackLayerDelete.cpp -o $@ $(CXXFLAGS)

action/track/layer/ActionTrackLayerMakeTrack.o : action/track/layer/ActionTrackLayerMakeTrack.cpp action/track/layer/ActionTrackLayerMakeTrack.h action/ActionGroup.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h action/track/layer/ActionTrackLayerDelete.h action/track/ActionTrackAdd.h data/base.h data/Track.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h lib/base/set.h data/Song.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h module/synthesizer/Synthesizer.h data/midi/Temperament.h module/Module.h module/port/Port.h
	$(CPP) -c action/track/layer/ActionTrackLayerMakeTrack.cpp -o $@ $(CXXFLAGS)

action/track/layer/ActionTrackLayerMarkDominant.o : action/track/layer/ActionTrackLayerMarkDominant.cpp action/track/layer/ActionTrackLayerMarkDominant.h action/ActionGroup.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Range.h action/track/layer/ActionTrackFadeAdd.h data/CrossFade.h action/track/layer/ActionTrackFadeDelete.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h lib/base/set.h
	$(CPP) -c action/track/layer/ActionTrackLayerMarkDominant.cpp -o $@ $(CXXFLAGS)

action/track/layer/ActionTrackLayerMerge.o : action/track/layer/ActionTrackLayerMerge.cpp action/track/layer/ActionTrackLayerMerge.h action/ActionGroup.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Track.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h lib/base/set.h action/track/buffer/ActionTrackEditBuffer.h data/audio/AudioBuffer.h action/track/buffer/ActionTrackCreateBuffers.h action/track/layer/ActionTrackLayerDelete.h action/track/layer/ActionTrackLayerAdd.h module/audio/TrackRenderer.h module/audio/AudioSource.h module/Module.h module/port/Port.h
	$(CPP) -c action/track/layer/ActionTrackLayerMerge.cpp -o $@ $(CXXFLAGS)

action/track/layer/ActionTrackLayerMove.o : action/track/layer/ActionTrackLayerMove.cpp action/track/layer/ActionTrackLayerMove.h
	$(CPP) -c action/track/layer/ActionTrackLayerMove.cpp -o $@ $(CXXFLAGS)

action/track/marker/ActionTrackAddMarker.o : action/track/marker/ActionTrackAddMarker.cpp action/track/marker/ActionTrackAddMarker.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/TrackLayer.h lib/base/set.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackMarker.h module/audio/AudioEffect.h module/Module.h module/port/Port.h
	$(CPP) -c action/track/marker/ActionTrackAddMarker.cpp -o $@ $(CXXFLAGS)

action/track/marker/ActionTrackDeleteMarker.o : action/track/marker/ActionTrackDeleteMarker.cpp action/track/marker/ActionTrackDeleteMarker.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/TrackLayer.h lib/base/set.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackMarker.h module/audio/AudioEffect.h module/Module.h module/port/Port.h
	$(CPP) -c action/track/marker/ActionTrackDeleteMarker.cpp -o $@ $(CXXFLAGS)

action/track/marker/ActionTrackEditMarker.o : action/track/marker/ActionTrackEditMarker.cpp action/track/marker/ActionTrackEditMarker.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Range.h data/TrackLayer.h lib/base/set.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackMarker.h module/audio/AudioEffect.h module/Module.h module/port/Port.h
	$(CPP) -c action/track/marker/ActionTrackEditMarker.cpp -o $@ $(CXXFLAGS)

action/track/midi/ActionTrackAddMidiEffect.o : action/track/midi/ActionTrackAddMidiEffect.cpp action/track/midi/ActionTrackAddMidiEffect.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Track.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h module/midi/MidiEffect.h module/Module.h module/port/Port.h
	$(CPP) -c action/track/midi/ActionTrackAddMidiEffect.cpp -o $@ $(CXXFLAGS)

action/track/midi/ActionTrackAddMidiNote.o : action/track/midi/ActionTrackAddMidiNote.cpp action/track/midi/ActionTrackAddMidiNote.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/TrackLayer.h lib/base/set.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/Track.h
	$(CPP) -c action/track/midi/ActionTrackAddMidiNote.cpp -o $@ $(CXXFLAGS)

action/track/midi/ActionTrackDeleteMidiEffect.o : action/track/midi/ActionTrackDeleteMidiEffect.cpp action/track/midi/ActionTrackDeleteMidiEffect.h module/midi/MidiEffect.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h data/Range.h module/Module.h lib/base/pointer.h stuff/Observable.h module/port/Port.h action/Action.h data/Data.h lib/file/path.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h
	$(CPP) -c action/track/midi/ActionTrackDeleteMidiEffect.cpp -o $@ $(CXXFLAGS)

action/track/midi/ActionTrackDeleteMidiNote.o : action/track/midi/ActionTrackDeleteMidiNote.cpp action/track/midi/ActionTrackDeleteMidiNote.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/TrackLayer.h lib/base/set.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h
	$(CPP) -c action/track/midi/ActionTrackDeleteMidiNote.cpp -o $@ $(CXXFLAGS)

action/track/midi/ActionTrackEditMidiEffect.o : action/track/midi/ActionTrackEditMidiEffect.cpp action/track/midi/ActionTrackEditMidiEffect.h action/ActionMergable.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Track.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h module/midi/MidiEffect.h module/Module.h module/port/Port.h
	$(CPP) -c action/track/midi/ActionTrackEditMidiEffect.cpp -o $@ $(CXXFLAGS)

action/track/midi/ActionTrackEditMidiNote.o : action/track/midi/ActionTrackEditMidiNote.cpp action/track/midi/ActionTrackEditMidiNote.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/TrackLayer.h lib/base/set.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h
	$(CPP) -c action/track/midi/ActionTrackEditMidiNote.cpp -o $@ $(CXXFLAGS)

action/track/midi/ActionTrackInsertMidi.o : action/track/midi/ActionTrackInsertMidi.cpp action/track/midi/ActionTrackInsertMidi.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/midi/MidiData.h data/Range.h data/midi/MidiNote.h data/midi/MidiEvent.h data/TrackLayer.h lib/base/set.h data/midi/Instrument.h
	$(CPP) -c action/track/midi/ActionTrackInsertMidi.cpp -o $@ $(CXXFLAGS)

action/track/midi/ActionTrackMoveMidiEffect.o : action/track/midi/ActionTrackMoveMidiEffect.cpp data/Track.h lib/base/pointer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h stuff/Observable.h action/track/midi/ActionTrackMoveMidiEffect.h action/Action.h data/Data.h lib/file/path.h
	$(CPP) -c action/track/midi/ActionTrackMoveMidiEffect.cpp -o $@ $(CXXFLAGS)

action/track/midi/ActionTrackToggleMidiEffectEnabled.o : action/track/midi/ActionTrackToggleMidiEffectEnabled.cpp action/track/midi/ActionTrackToggleMidiEffectEnabled.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Song.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h module/midi/MidiEffect.h module/Module.h module/port/Port.h
	$(CPP) -c action/track/midi/ActionTrackToggleMidiEffectEnabled.cpp -o $@ $(CXXFLAGS)

action/track/sample/ActionTrackAddSample.o : action/track/sample/ActionTrackAddSample.cpp action/track/sample/ActionTrackAddSample.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/TrackLayer.h lib/base/set.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/SampleRef.h lib/math/rect.h lib/math/math.h data/Sample.h data/audio/AudioBuffer.h
	$(CPP) -c action/track/sample/ActionTrackAddSample.cpp -o $@ $(CXXFLAGS)

action/track/sample/ActionTrackDeleteSample.o : action/track/sample/ActionTrackDeleteSample.cpp action/track/sample/ActionTrackDeleteSample.h action/ActionGroup.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/TrackLayer.h lib/base/set.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/SampleRef.h lib/math/rect.h lib/math/math.h data/Sample.h data/audio/AudioBuffer.h action/sample/ActionSampleDelete.h
	$(CPP) -c action/track/sample/ActionTrackDeleteSample.cpp -o $@ $(CXXFLAGS)

action/track/sample/ActionTrackEditSample.o : action/track/sample/ActionTrackEditSample.cpp action/track/sample/ActionTrackEditSample.h action/ActionMergable.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Track.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/SampleRef.h lib/math/rect.h lib/math/math.h
	$(CPP) -c action/track/sample/ActionTrackEditSample.cpp -o $@ $(CXXFLAGS)

action/track/sample/ActionTrackInsertSample.o : action/track/sample/ActionTrackInsertSample.cpp action/track/sample/ActionTrackInsertSample.h action/ActionGroup.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h action/track/sample/ActionTrackDeleteSample.h action/track/buffer/ActionTrackCreateBuffers.h data/Range.h action/track/buffer/ActionTrackEditBuffer.h data/audio/AudioBuffer.h data/base.h data/Song.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h data/TrackLayer.h lib/base/set.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/Sample.h lib/math/rect.h lib/math/math.h data/SampleRef.h action/track/midi/ActionTrackAddMidiNote.h
	$(CPP) -c action/track/sample/ActionTrackInsertSample.cpp -o $@ $(CXXFLAGS)

action/track/sample/ActionTrackPasteAsSample.o : action/track/sample/ActionTrackPasteAsSample.cpp action/track/sample/ActionTrackPasteAsSample.h action/ActionGroup.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/base.h data/Track.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/Sample.h lib/math/rect.h lib/math/math.h data/audio/AudioBuffer.h data/SampleRef.h action/sample/ActionSampleAdd.h action/track/sample/ActionTrackAddSample.h
	$(CPP) -c action/track/sample/ActionTrackPasteAsSample.cpp -o $@ $(CXXFLAGS)

action/track/sample/ActionTrackSampleFromSelection.o : action/track/sample/ActionTrackSampleFromSelection.cpp action/track/sample/ActionTrackSampleFromSelection.h action/ActionGroup.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h action/track/sample/ActionTrackPasteAsSample.h action/track/buffer/ActionTrackEditBuffer.h data/audio/AudioBuffer.h data/Range.h action/track/midi/ActionTrackDeleteMidiNote.h data/base.h data/SongSelection.h lib/base/set.h data/Song.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h data/TrackLayer.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h action/track/buffer/ActionTrack__DeleteBuffer.h
	$(CPP) -c action/track/sample/ActionTrackSampleFromSelection.cpp -o $@ $(CXXFLAGS)

action/track/synthesizer/ActionTrackDetuneSynthesizer.o : action/track/synthesizer/ActionTrackDetuneSynthesizer.cpp action/track/synthesizer/ActionTrackDetuneSynthesizer.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/midi/Temperament.h data/midi/MidiData.h data/Range.h data/midi/MidiNote.h data/midi/MidiEvent.h data/Song.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h data/Track.h data/midi/Instrument.h module/synthesizer/Synthesizer.h lib/base/set.h module/Module.h module/port/Port.h
	$(CPP) -c action/track/synthesizer/ActionTrackDetuneSynthesizer.cpp -o $@ $(CXXFLAGS)

action/track/synthesizer/ActionTrackEditSynthesizer.o : action/track/synthesizer/ActionTrackEditSynthesizer.cpp action/track/synthesizer/ActionTrackEditSynthesizer.h action/ActionMergable.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Track.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h module/synthesizer/Synthesizer.h lib/base/set.h data/midi/Temperament.h module/Module.h module/port/Port.h
	$(CPP) -c action/track/synthesizer/ActionTrackEditSynthesizer.cpp -o $@ $(CXXFLAGS)

action/track/synthesizer/ActionTrackSetSynthesizer.o : action/track/synthesizer/ActionTrackSetSynthesizer.cpp action/track/synthesizer/ActionTrackSetSynthesizer.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Track.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h module/synthesizer/Synthesizer.h lib/base/set.h data/midi/Temperament.h module/Module.h module/port/Port.h
	$(CPP) -c action/track/synthesizer/ActionTrackSetSynthesizer.cpp -o $@ $(CXXFLAGS)

action/track/ActionTrack__DeleteEmpty.o : action/track/ActionTrack__DeleteEmpty.cpp action/track/ActionTrack__DeleteEmpty.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Track.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h lib/base/set.h data/Song.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h
	$(CPP) -c action/track/ActionTrack__DeleteEmpty.cpp -o $@ $(CXXFLAGS)

action/track/ActionTrackAdd.o : action/track/ActionTrackAdd.cpp action/track/ActionTrackAdd.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Song.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h lib/base/set.h
	$(CPP) -c action/track/ActionTrackAdd.cpp -o $@ $(CXXFLAGS)

action/track/ActionTrackDelete.o : action/track/ActionTrackDelete.cpp action/track/ActionTrackDelete.h action/ActionGroup.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h action/track/sample/ActionTrackDeleteSample.h action/track/ActionTrack__DeleteEmpty.h action/track/buffer/ActionTrack__DeleteBuffer.h data/audio/AudioBuffer.h data/Range.h action/track/data/ActionTrackSetTarget.h data/Song.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h lib/base/set.h data/SampleRef.h lib/math/rect.h lib/math/math.h
	$(CPP) -c action/track/ActionTrackDelete.cpp -o $@ $(CXXFLAGS)

action/track/ActionTrackMove.o : action/track/ActionTrackMove.cpp action/track/ActionTrackMove.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/Song.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h
	$(CPP) -c action/track/ActionTrackMove.cpp -o $@ $(CXXFLAGS)

action/Action.o : action/Action.cpp action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h
	$(CPP) -c action/Action.cpp -o $@ $(CXXFLAGS)

action/ActionGroup.o : action/ActionGroup.cpp action/ActionGroup.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h
	$(CPP) -c action/ActionGroup.cpp -o $@ $(CXXFLAGS)

action/ActionManager.o : action/ActionManager.cpp action/ActionManager.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h action/Action.h data/Data.h lib/base/pointer.h lib/file/path.h stuff/Observable.h action/ActionMergable.h action/ActionGroup.h lib/file/msg.h lib/hui/Timer.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/file_op.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c action/ActionManager.cpp -o $@ $(CXXFLAGS)

action/ActionMergable.o : action/ActionMergable.cpp action/ActionMergable.h action/Action.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h
	$(CPP) -c action/ActionMergable.cpp -o $@ $(CXXFLAGS)

data/audio/AudioBuffer.o : data/audio/AudioBuffer.cpp data/audio/AudioBuffer.h lib/base/pointer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h data/Range.h data/base.h lib/math/math.h lib/file/msg.h
	$(CPP) -c data/audio/AudioBuffer.cpp -o $@ $(CXXFLAGS)

data/audio/BufferInterpolator.o : data/audio/BufferInterpolator.cpp data/audio/BufferInterpolator.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h data/audio/AudioBuffer.h lib/base/pointer.h data/Range.h plugins/FastFourierTransform.h lib/math/complex.h lib/math/math.h
	$(CPP) -c data/audio/BufferInterpolator.cpp -o $@ $(CXXFLAGS)

data/audio/RingBuffer.o : data/audio/RingBuffer.cpp data/audio/RingBuffer.h data/audio/AudioBuffer.h lib/base/pointer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h data/Range.h
	$(CPP) -c data/audio/RingBuffer.cpp -o $@ $(CXXFLAGS)

data/midi/Clef.o : data/midi/Clef.cpp data/midi/Clef.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h data/midi/Scale.h data/midi/MidiData.h lib/base/pointer.h data/Range.h data/midi/MidiNote.h data/midi/MidiEvent.h
	$(CPP) -c data/midi/Clef.cpp -o $@ $(CXXFLAGS)

data/midi/Instrument.o : data/midi/Instrument.cpp data/midi/Instrument.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h data/midi/MidiData.h lib/base/pointer.h data/Range.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Clef.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c data/midi/Instrument.cpp -o $@ $(CXXFLAGS)

data/midi/MidiData.o : data/midi/MidiData.cpp data/midi/MidiData.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h data/Range.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/SongSelection.h lib/base/set.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c data/midi/MidiData.cpp -o $@ $(CXXFLAGS)

data/midi/MidiEvent.o : data/midi/MidiEvent.cpp data/midi/MidiEvent.h data/midi/MidiNote.h lib/base/pointer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h data/Range.h
	$(CPP) -c data/midi/MidiEvent.cpp -o $@ $(CXXFLAGS)

data/midi/MidiNote.o : data/midi/MidiNote.cpp data/midi/MidiNote.h lib/base/pointer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h data/Range.h data/midi/MidiData.h data/midi/MidiEvent.h data/midi/Clef.h data/midi/Instrument.h
	$(CPP) -c data/midi/MidiNote.cpp -o $@ $(CXXFLAGS)

data/midi/Scale.o : data/midi/Scale.cpp lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h data/midi/Scale.h data/midi/MidiData.h data/Range.h data/midi/MidiNote.h data/midi/MidiEvent.h
	$(CPP) -c data/midi/Scale.cpp -o $@ $(CXXFLAGS)

data/midi/Temperament.o : data/midi/Temperament.cpp data/midi/Temperament.h data/midi/MidiData.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h data/Range.h data/midi/MidiNote.h data/midi/MidiEvent.h lib/math/math.h
	$(CPP) -c data/midi/Temperament.cpp -o $@ $(CXXFLAGS)

data/rhythm/Bar.o : data/rhythm/Bar.cpp data/rhythm/Bar.h lib/base/pointer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h data/Range.h data/rhythm/Beat.h
	$(CPP) -c data/rhythm/Bar.cpp -o $@ $(CXXFLAGS)

data/rhythm/BarCollection.o : data/rhythm/BarCollection.cpp data/rhythm/BarCollection.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h data/rhythm/Bar.h data/Range.h data/rhythm/Beat.h
	$(CPP) -c data/rhythm/BarCollection.cpp -o $@ $(CXXFLAGS)

data/rhythm/Beat.o : data/rhythm/Beat.cpp data/rhythm/Beat.h data/Range.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h
	$(CPP) -c data/rhythm/Beat.cpp -o $@ $(CXXFLAGS)

data/base.o : data/base.cpp data/base.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c data/base.cpp -o $@ $(CXXFLAGS)

data/CrossFade.o : data/CrossFade.cpp data/CrossFade.h data/Range.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h
	$(CPP) -c data/CrossFade.cpp -o $@ $(CXXFLAGS)

data/Curve.o : data/Curve.cpp data/Curve.h lib/base/pointer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h stuff/Observable.h data/Track.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/Song.h data/Data.h lib/file/path.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h module/Module.h module/ModuleConfiguration.h module/audio/AudioEffect.h module/port/Port.h module/midi/MidiEffect.h module/synthesizer/Synthesizer.h lib/base/set.h data/midi/Temperament.h lib/kaba/kaba.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h
	$(CPP) -c data/Curve.cpp -o $@ $(CXXFLAGS)

data/Data.o : data/Data.cpp data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h action/ActionManager.h action/Action.h lib/threads/Mutex.h
	$(CPP) -c data/Data.cpp -o $@ $(CXXFLAGS)

data/Range.o : data/Range.cpp data/Range.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h
	$(CPP) -c data/Range.cpp -o $@ $(CXXFLAGS)

data/Sample.o : data/Sample.cpp data/Sample.h data/midi/MidiData.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h data/Range.h data/midi/MidiNote.h data/midi/MidiEvent.h lib/math/rect.h lib/math/math.h stuff/Observable.h data/audio/AudioBuffer.h data/SampleRef.h data/base.h data/Song.h data/Data.h lib/file/path.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h
	$(CPP) -c data/Sample.cpp -o $@ $(CXXFLAGS)

data/SampleRef.o : data/SampleRef.cpp data/SampleRef.h stuff/Observable.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/math/rect.h lib/math/math.h data/Sample.h data/midi/MidiData.h data/Range.h data/midi/MidiNote.h data/midi/MidiEvent.h data/audio/AudioBuffer.h data/TrackLayer.h lib/base/set.h data/midi/Instrument.h data/Song.h data/Data.h lib/file/path.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h
	$(CPP) -c data/SampleRef.cpp -o $@ $(CXXFLAGS)

data/Song.o : data/Song.cpp data/Song.h data/Data.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h data/base.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h lib/base/set.h data/Sample.h lib/math/rect.h lib/math/math.h data/audio/AudioBuffer.h data/CrossFade.h data/SongSelection.h data/TrackMarker.h module/audio/AudioEffect.h module/Module.h module/port/Port.h action/ActionManager.h action/Action.h action/bar/ActionBarAdd.h action/ActionGroup.h action/bar/ActionBarDelete.h action/bar/ActionBarEdit.h action/sample/ActionSampleAdd.h action/sample/ActionSampleDelete.h action/sample/ActionSampleEditName.h action/ActionMergable.h action/sample/ActionSampleReplaceBuffer.h action/song/ActionSongDeleteSelection.h action/song/data/ActionSongChangeAllTrackVolumes.h action/song/data/ActionSongSetDefaultFormat.h action/song/data/ActionSongSetSampleRate.h action/tag/ActionTagAdd.h action/tag/ActionTagDelete.h action/tag/ActionTagEdit.h action/track/ActionTrackAdd.h action/track/ActionTrackDelete.h action/track/sample/ActionTrackInsertSample.h action/track/sample/ActionTrackSampleFromSelection.h module/synthesizer/DummySynthesizer.h module/synthesizer/Synthesizer.h data/midi/Temperament.h module/synthesizer/EnvelopeADSR.h module/midi/MidiEffect.h stuff/Log.h lib/threads/Mutex.h lib/hui/language.h
	$(CPP) -c data/Song.cpp -o $@ $(CXXFLAGS)

data/SongSelection.o : data/SongSelection.cpp data/SongSelection.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/set.h data/Range.h data/rhythm/Bar.h lib/base/pointer.h data/base.h data/Song.h data/Data.h lib/file/path.h stuff/Observable.h data/rhythm/BarCollection.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h data/TrackMarker.h module/audio/AudioEffect.h module/Module.h module/port/Port.h data/SampleRef.h lib/math/rect.h lib/math/math.h
	$(CPP) -c data/SongSelection.cpp -o $@ $(CXXFLAGS)

data/Track.o : data/Track.cpp data/Track.h lib/base/pointer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h stuff/Observable.h data/TrackLayer.h lib/base/set.h data/TrackMarker.h module/audio/AudioEffect.h module/Module.h module/port/Port.h data/CrossFade.h data/Curve.h data/base.h data/Song.h data/Data.h lib/file/path.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h data/audio/AudioBuffer.h action/track/buffer/ActionTrackSetChannels.h action/ActionGroup.h action/Action.h action/track/curve/ActionTrackAddCurve.h action/track/curve/ActionTrackCurveAddPoint.h action/track/curve/ActionTrackCurveDeletePoint.h action/track/curve/ActionTrackCurveEditPoint.h action/ActionMergable.h action/track/curve/ActionTrackDeleteCurve.h action/track/curve/ActionTrackEditCurve.h action/track/data/ActionTrackEditName.h action/track/data/ActionTrackEditMuted.h action/track/data/ActionTrackEditVolume.h action/track/data/ActionTrackEditPanning.h action/track/data/ActionTrackSetInstrument.h action/track/data/ActionTrackSetTarget.h action/track/layer/ActionTrackLayerAdd.h action/track/layer/ActionTrackLayerDelete.h action/track/layer/ActionTrackLayerMerge.h action/track/layer/ActionTrackLayerMarkDominant.h action/track/midi/ActionTrackAddMidiEffect.h action/track/midi/ActionTrackDeleteMidiEffect.h module/midi/MidiEffect.h action/track/midi/ActionTrackEditMidiEffect.h action/track/midi/ActionTrackMoveMidiEffect.h action/track/midi/ActionTrackToggleMidiEffectEnabled.h action/track/ActionTrackMove.h action/track/synthesizer/ActionTrackSetSynthesizer.h action/track/synthesizer/ActionTrackEditSynthesizer.h action/track/synthesizer/ActionTrackDetuneSynthesizer.h data/midi/Temperament.h action/track/effect/ActionTrackAddEffect.h action/track/effect/ActionTrackDeleteAudioEffect.h action/track/effect/ActionTrackEditEffect.h action/track/effect/ActionTrackMoveAudioEffect.h action/track/effect/ActionTrackToggleEffectEnabled.h module/synthesizer/Synthesizer.h plugins/PluginManager.h lib/kaba/kaba.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h Tsunami.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/threads/Mutex.h
	$(CPP) -c data/Track.cpp -o $@ $(CXXFLAGS)

data/TrackLayer.o : data/TrackLayer.cpp data/TrackLayer.h lib/base/pointer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/set.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h stuff/Observable.h data/Track.h data/base.h data/Song.h data/Data.h lib/file/path.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h data/CrossFade.h data/Sample.h lib/math/rect.h lib/math/math.h data/audio/AudioBuffer.h data/SampleRef.h data/TrackMarker.h module/audio/AudioEffect.h module/Module.h module/port/Port.h action/track/buffer/ActionTrackCreateBuffers.h action/ActionGroup.h action/Action.h action/track/buffer/ActionTrackEditBuffer.h action/track/layer/ActionTrackLayerMakeTrack.h action/track/layer/ActionTrackLayerMarkDominant.h action/track/midi/ActionTrackInsertMidi.h action/track/sample/ActionTrackAddSample.h action/track/sample/ActionTrackDeleteSample.h action/track/sample/ActionTrackEditSample.h action/ActionMergable.h action/track/midi/ActionTrackAddMidiNote.h action/track/midi/ActionTrackDeleteMidiNote.h action/track/midi/ActionTrackEditMidiNote.h action/track/marker/ActionTrackAddMarker.h action/track/marker/ActionTrackDeleteMarker.h action/track/marker/ActionTrackEditMarker.h Tsunami.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/threads/Mutex.h
	$(CPP) -c data/TrackLayer.cpp -o $@ $(CXXFLAGS)

data/TrackMarker.o : data/TrackMarker.cpp data/TrackMarker.h data/Range.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h module/audio/AudioEffect.h module/Module.h lib/base/pointer.h stuff/Observable.h module/port/Port.h data/midi/Scale.h lib/file/msg.h
	$(CPP) -c data/TrackMarker.cpp -o $@ $(CXXFLAGS)

device/stream/AudioInput.o : device/stream/AudioInput.cpp device/stream/AudioInput.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h data/audio/RingBuffer.h data/audio/AudioBuffer.h lib/base/pointer.h data/Range.h module/port/Port.h module/Module.h stuff/Observable.h module/ModuleConfiguration.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h Session.h data/base.h device/Device.h device/DeviceManager.h lib/kaba/lib/extern.h plugins/PluginManager.h lib/kaba/kaba.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h
	$(CPP) -c device/stream/AudioInput.cpp -o $@ $(CXXFLAGS)

device/stream/AudioOutput.o : device/stream/AudioOutput.cpp device/stream/AudioOutput.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h data/audio/RingBuffer.h data/audio/AudioBuffer.h lib/base/pointer.h data/Range.h module/Module.h stuff/Observable.h module/ModuleConfiguration.h Session.h data/base.h module/port/Port.h device/Device.h device/DeviceManager.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/kaba/lib/extern.h plugins/PluginManager.h lib/kaba/kaba.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h
	$(CPP) -c device/stream/AudioOutput.cpp -o $@ $(CXXFLAGS)

device/stream/MidiInput.o : device/stream/MidiInput.cpp device/stream/MidiInput.h data/midi/MidiData.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h data/Range.h data/midi/MidiNote.h data/midi/MidiEvent.h module/port/Port.h module/Module.h stuff/Observable.h module/ModuleConfiguration.h Session.h data/base.h device/Device.h device/DeviceManager.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/kaba/lib/extern.h plugins/PluginManager.h lib/kaba/kaba.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h
	$(CPP) -c device/stream/MidiInput.cpp -o $@ $(CXXFLAGS)

device/Device.o : device/Device.cpp device/Device.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c device/Device.cpp -o $@ $(CXXFLAGS)

device/DeviceManager.o : device/DeviceManager.cpp device/DeviceManager.h stuff/Observable.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h Session.h module/Module.h device/Device.h Tsunami.h stuff/Log.h
	$(CPP) -c device/DeviceManager.cpp -o $@ $(CXXFLAGS)

lib/any/any.o : lib/any/any.cpp lib/any/any.h lib/file/file.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/base/map.h lib/base/set.h lib/config.h lib/kaba/kaba.h lib/base/pointer.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h
	$(CPP) -c lib/any/any.cpp -o $@ $(CXXFLAGS)

lib/base/array.o : lib/base/array.cpp lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h
	$(CPP) -c lib/base/array.cpp -o $@ $(CXXFLAGS)

lib/base/pointer.o : lib/base/pointer.cpp lib/base/pointer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/msg.h
	$(CPP) -c lib/base/pointer.cpp -o $@ $(CXXFLAGS)

lib/base/strings.o : lib/base/strings.cpp lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h
	$(CPP) -c lib/base/strings.cpp -o $@ $(CXXFLAGS)

lib/file/file.o : lib/file/file.cpp lib/file/file.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/msg.h lib/file/file_op.h lib/file/path.h
	$(CPP) -c lib/file/file.cpp -o $@ $(CXXFLAGS)

lib/file/file_op.o : lib/file/file_op.cpp lib/file/file.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/msg.h lib/file/file_op.h lib/file/path.h
	$(CPP) -c lib/file/file_op.cpp -o $@ $(CXXFLAGS)

lib/file/msg.o : lib/file/msg.cpp lib/file/file.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/msg.h lib/file/file_op.h lib/file/path.h
	$(CPP) -c lib/file/msg.cpp -o $@ $(CXXFLAGS)

lib/file/path.o : lib/file/path.cpp lib/file/path.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file_op.h
	$(CPP) -c lib/file/path.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/Control.o : lib/hui/Controls/Control.cpp lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h lib/hui/hui.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/hui/Controls/ControlMultilineEdit.h
	$(CPP) -c lib/hui/Controls/Control.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlButtonGtk.o : lib/hui/Controls/ControlButtonGtk.cpp lib/hui/Controls/ControlButton.h lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h lib/hui/hui.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c lib/hui/Controls/ControlButtonGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlCheckBoxGtk.o : lib/hui/Controls/ControlCheckBoxGtk.cpp lib/hui/Controls/ControlCheckBox.h lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h lib/hui/Config.h lib/base/map.h lib/base/set.h
	$(CPP) -c lib/hui/Controls/ControlCheckBoxGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlColorButtonGtk.o : lib/hui/Controls/ControlColorButtonGtk.cpp lib/hui/Controls/ControlColorButton.h lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h
	$(CPP) -c lib/hui/Controls/ControlColorButtonGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlComboBoxGtk.o : lib/hui/Controls/ControlComboBoxGtk.cpp lib/hui/Controls/ControlComboBox.h lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h
	$(CPP) -c lib/hui/Controls/ControlComboBoxGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlDrawingAreaGtk.o : lib/hui/Controls/ControlDrawingAreaGtk.cpp lib/hui/Controls/ControlDrawingArea.h lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h lib/hui/hui.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/hui/internal.h lib/math/rect.h
	$(CPP) -c lib/hui/Controls/ControlDrawingAreaGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlEditGtk.o : lib/hui/Controls/ControlEditGtk.cpp lib/hui/Controls/ControlEdit.h lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h
	$(CPP) -c lib/hui/Controls/ControlEditGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlExpanderGtk.o : lib/hui/Controls/ControlExpanderGtk.cpp lib/hui/Controls/ControlExpander.h lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h lib/hui/Window.h lib/hui/Panel.h lib/hui/hui.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Menu.h lib/hui/common_dlg.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/base/pointer.h
	$(CPP) -c lib/hui/Controls/ControlExpanderGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlGridGtk.o : lib/hui/Controls/ControlGridGtk.cpp lib/hui/Controls/ControlGrid.h lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h lib/hui/hui.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c lib/hui/Controls/ControlGridGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlGroupGtk.o : lib/hui/Controls/ControlGroupGtk.cpp lib/hui/Controls/ControlGroup.h lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h lib/hui/Window.h lib/hui/Panel.h lib/hui/hui.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Menu.h lib/hui/common_dlg.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/base/pointer.h
	$(CPP) -c lib/hui/Controls/ControlGroupGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlHeaderBar.o : lib/hui/Controls/ControlHeaderBar.cpp lib/hui/Controls/ControlHeaderBar.h lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h lib/hui/hui.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c lib/hui/Controls/ControlHeaderBar.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlLabelGtk.o : lib/hui/Controls/ControlLabelGtk.cpp lib/hui/Controls/ControlLabel.h lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h
	$(CPP) -c lib/hui/Controls/ControlLabelGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlListViewGtk.o : lib/hui/Controls/ControlListViewGtk.cpp lib/hui/Controls/ControlListView.h lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h lib/hui/hui.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c lib/hui/Controls/ControlListViewGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlMenuButtonGtk.o : lib/hui/Controls/ControlMenuButtonGtk.cpp lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/hui/Controls/ControlMenuButton.h lib/hui/Controls/Control.h
	$(CPP) -c lib/hui/Controls/ControlMenuButtonGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlMultilineEditGtk.o : lib/hui/Controls/ControlMultilineEditGtk.cpp lib/hui/Controls/ControlMultilineEdit.h lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h
	$(CPP) -c lib/hui/Controls/ControlMultilineEditGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlPanedGtk.o : lib/hui/Controls/ControlPanedGtk.cpp lib/hui/Controls/ControlPaned.h lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h
	$(CPP) -c lib/hui/Controls/ControlPanedGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlProgressBarGtk.o : lib/hui/Controls/ControlProgressBarGtk.cpp lib/hui/Controls/ControlProgressBar.h lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h
	$(CPP) -c lib/hui/Controls/ControlProgressBarGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlRadioButtonGtk.o : lib/hui/Controls/ControlRadioButtonGtk.cpp lib/hui/Controls/ControlRadioButton.h lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h lib/hui/hui.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c lib/hui/Controls/ControlRadioButtonGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlScrollerGtk.o : lib/hui/Controls/ControlScrollerGtk.cpp lib/hui/Controls/ControlScroller.h lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h
	$(CPP) -c lib/hui/Controls/ControlScrollerGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlSeparatorGtk.o : lib/hui/Controls/ControlSeparatorGtk.cpp lib/hui/Controls/ControlSeparator.h lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h
	$(CPP) -c lib/hui/Controls/ControlSeparatorGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlSliderGtk.o : lib/hui/Controls/ControlSliderGtk.cpp lib/hui/Controls/ControlSlider.h lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h
	$(CPP) -c lib/hui/Controls/ControlSliderGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlSpinButtonGtk.o : lib/hui/Controls/ControlSpinButtonGtk.cpp lib/hui/Controls/ControlSpinButton.h lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h
	$(CPP) -c lib/hui/Controls/ControlSpinButtonGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlTabControlGtk.o : lib/hui/Controls/ControlTabControlGtk.cpp lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/hui/Controls/ControlTabControl.h lib/hui/Controls/Control.h
	$(CPP) -c lib/hui/Controls/ControlTabControlGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlToggleButtonGtk.o : lib/hui/Controls/ControlToggleButtonGtk.cpp lib/hui/Controls/ControlToggleButton.h lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h
	$(CPP) -c lib/hui/Controls/ControlToggleButtonGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlTreeViewGtk.o : lib/hui/Controls/ControlTreeViewGtk.cpp lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/hui/Controls/ControlTreeView.h lib/hui/Controls/Control.h
	$(CPP) -c lib/hui/Controls/ControlTreeViewGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/MenuItemGtk.o : lib/hui/Controls/MenuItemGtk.cpp lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/hui/internal.h lib/hui/Controls/MenuItem.h lib/hui/Controls/Control.h
	$(CPP) -c lib/hui/Controls/MenuItemGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/MenuItemSeparatorGtk.o : lib/hui/Controls/MenuItemSeparatorGtk.cpp lib/hui/Controls/MenuItemSeparator.h lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h
	$(CPP) -c lib/hui/Controls/MenuItemSeparatorGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/MenuItemSeparatorWin.o : lib/hui/Controls/MenuItemSeparatorWin.cpp lib/hui/Controls/MenuItemSeparator.h lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h
	$(CPP) -c lib/hui/Controls/MenuItemSeparatorWin.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/MenuItemSubmenuGtk.o : lib/hui/Controls/MenuItemSubmenuGtk.cpp lib/hui/Menu.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Controls/MenuItemSubmenu.h lib/hui/Controls/Control.h lib/image/image.h lib/image/color.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h
	$(CPP) -c lib/hui/Controls/MenuItemSubmenuGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/MenuItemSubmenuWin.o : lib/hui/Controls/MenuItemSubmenuWin.cpp lib/hui/Menu.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Controls/MenuItemSubmenu.h lib/hui/Controls/Control.h lib/image/image.h lib/image/color.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h
	$(CPP) -c lib/hui/Controls/MenuItemSubmenuWin.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/MenuItemToggleGtk.o : lib/hui/Controls/MenuItemToggleGtk.cpp lib/hui/Controls/MenuItemToggle.h lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h
	$(CPP) -c lib/hui/Controls/MenuItemToggleGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/MenuItemToggleWin.o : lib/hui/Controls/MenuItemToggleWin.cpp lib/hui/Controls/MenuItemToggle.h lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h
	$(CPP) -c lib/hui/Controls/MenuItemToggleWin.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/MenuItemWin.o : lib/hui/Controls/MenuItemWin.cpp lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/hui/internal.h lib/hui/Controls/MenuItem.h lib/hui/Controls/Control.h
	$(CPP) -c lib/hui/Controls/MenuItemWin.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ToolItemButtonGtk.o : lib/hui/Controls/ToolItemButtonGtk.cpp lib/hui/Controls/ToolItemButton.h lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h
	$(CPP) -c lib/hui/Controls/ToolItemButtonGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ToolItemButtonWin.o : lib/hui/Controls/ToolItemButtonWin.cpp lib/hui/Controls/ToolItemButton.h lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h
	$(CPP) -c lib/hui/Controls/ToolItemButtonWin.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ToolItemMenuButtonGtk.o : lib/hui/Controls/ToolItemMenuButtonGtk.cpp lib/hui/Menu.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Controls/ToolItemMenuButton.h lib/hui/Controls/Control.h lib/image/image.h lib/image/color.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h lib/hui/hui.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c lib/hui/Controls/ToolItemMenuButtonGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ToolItemMenuButtonWin.o : lib/hui/Controls/ToolItemMenuButtonWin.cpp lib/hui/Menu.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Controls/ToolItemMenuButton.h lib/hui/Controls/Control.h lib/image/image.h lib/image/color.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h
	$(CPP) -c lib/hui/Controls/ToolItemMenuButtonWin.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ToolItemSeparatorGtk.o : lib/hui/Controls/ToolItemSeparatorGtk.cpp lib/hui/Controls/ToolItemSeparator.h lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h
	$(CPP) -c lib/hui/Controls/ToolItemSeparatorGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ToolItemSeparatorWin.o : lib/hui/Controls/ToolItemSeparatorWin.cpp lib/hui/Controls/ToolItemSeparator.h lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h
	$(CPP) -c lib/hui/Controls/ToolItemSeparatorWin.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ToolItemToggleButtonGtk.o : lib/hui/Controls/ToolItemToggleButtonGtk.cpp lib/hui/Controls/ToolItemToggleButton.h lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h
	$(CPP) -c lib/hui/Controls/ToolItemToggleButtonGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ToolItemToggleButtonWin.o : lib/hui/Controls/ToolItemToggleButtonWin.cpp lib/hui/Controls/ToolItemToggleButton.h lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h
	$(CPP) -c lib/hui/Controls/ToolItemToggleButtonWin.cpp -o $@ $(CXXFLAGS)

lib/hui/Application.o : lib/hui/Application.cpp lib/hui/Application.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/map.h lib/base/set.h lib/file/path.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/hui/hui.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/hui/internal.h
	$(CPP) -c lib/hui/Application.cpp -o $@ $(CXXFLAGS)

lib/hui/Callback.o : lib/hui/Callback.cpp lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c lib/hui/Callback.cpp -o $@ $(CXXFLAGS)

lib/hui/Clipboard.o : lib/hui/Clipboard.cpp lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c lib/hui/Clipboard.cpp -o $@ $(CXXFLAGS)

lib/hui/common_dlg.o : lib/hui/common_dlg.cpp lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c lib/hui/common_dlg.cpp -o $@ $(CXXFLAGS)

lib/hui/common_dlg_gtk.o : lib/hui/common_dlg_gtk.cpp lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c lib/hui/common_dlg_gtk.cpp -o $@ $(CXXFLAGS)

lib/hui/common_dlg_win.o : lib/hui/common_dlg_win.cpp lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c lib/hui/common_dlg_win.cpp -o $@ $(CXXFLAGS)

lib/hui/Config.o : lib/hui/Config.cpp lib/hui/Config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/map.h lib/base/set.h lib/file/path.h lib/file/file.h lib/file/msg.h lib/file/file_op.h
	$(CPP) -c lib/hui/Config.cpp -o $@ $(CXXFLAGS)

lib/hui/error.o : lib/hui/error.cpp lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/hui/internal.h lib/net/net.h lib/net/Socket.h lib/net/BinaryBuffer.h
	$(CPP) -c lib/hui/error.cpp -o $@ $(CXXFLAGS)

lib/hui/Event.o : lib/hui/Event.cpp lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c lib/hui/Event.cpp -o $@ $(CXXFLAGS)

lib/hui/hui.o : lib/hui/hui.cpp lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/hui/internal.h
	$(CPP) -c lib/hui/hui.cpp -o $@ $(CXXFLAGS)

lib/hui/language.o : lib/hui/language.cpp lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/hui/internal.h
	$(CPP) -c lib/hui/language.cpp -o $@ $(CXXFLAGS)

lib/hui/main_gtk.o : lib/hui/main_gtk.cpp
	$(CPP) -c lib/hui/main_gtk.cpp -o $@ $(CXXFLAGS)

lib/hui/main_win.o : lib/hui/main_win.cpp
	$(CPP) -c lib/hui/main_win.cpp -o $@ $(CXXFLAGS)

lib/hui/Menu.o : lib/hui/Menu.cpp lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/hui/Controls/MenuItem.h lib/hui/Controls/Control.h lib/hui/Controls/MenuItemSeparator.h lib/hui/Controls/MenuItemSubmenu.h lib/hui/Controls/MenuItemToggle.h
	$(CPP) -c lib/hui/Menu.cpp -o $@ $(CXXFLAGS)

lib/hui/MenuGtk.o : lib/hui/MenuGtk.cpp lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h lib/hui/Controls/MenuItem.h lib/hui/Controls/MenuItemSubmenu.h lib/hui/Controls/MenuItemToggle.h lib/hui/hui.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/hui/internal.h
	$(CPP) -c lib/hui/MenuGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/MenuWin.o : lib/hui/MenuWin.cpp lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c lib/hui/MenuWin.cpp -o $@ $(CXXFLAGS)

lib/hui/PainterGtk.o : lib/hui/PainterGtk.cpp lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/math/rect.h lib/hui/Controls/Control.h lib/hui/Controls/ControlDrawingArea.h
	$(CPP) -c lib/hui/PainterGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Panel.o : lib/hui/Panel.cpp lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h lib/hui/Controls/MenuItem.h lib/hui/Controls/MenuItemToggle.h lib/hui/hui.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/hui/internal.h
	$(CPP) -c lib/hui/Panel.cpp -o $@ $(CXXFLAGS)

lib/hui/Resource.o : lib/hui/Resource.cpp lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/hui/Controls/Control.h lib/hui/internal.h
	$(CPP) -c lib/hui/Resource.cpp -o $@ $(CXXFLAGS)

lib/hui/Timer.o : lib/hui/Timer.cpp lib/hui/Timer.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c lib/hui/Timer.cpp -o $@ $(CXXFLAGS)

lib/hui/Toolbar.o : lib/hui/Toolbar.cpp lib/hui/Toolbar.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/error.h lib/hui/Clipboard.h lib/hui/Controls/ToolItemButton.h lib/hui/Controls/Control.h lib/hui/Controls/ToolItemMenuButton.h lib/hui/Controls/ToolItemSeparator.h lib/hui/Controls/ToolItemToggleButton.h
	$(CPP) -c lib/hui/Toolbar.cpp -o $@ $(CXXFLAGS)

lib/hui/ToolbarGtk.o : lib/hui/ToolbarGtk.cpp lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h lib/hui/Toolbar.h lib/hui/hui.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c lib/hui/ToolbarGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/ToolbarWin.o : lib/hui/ToolbarWin.cpp lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c lib/hui/ToolbarWin.cpp -o $@ $(CXXFLAGS)

lib/hui/utility.o : lib/hui/utility.cpp lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c lib/hui/utility.cpp -o $@ $(CXXFLAGS)

lib/hui/Window.o : lib/hui/Window.cpp lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h lib/hui/hui.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/hui/internal.h
	$(CPP) -c lib/hui/Window.cpp -o $@ $(CXXFLAGS)

lib/hui/window_control.o : lib/hui/window_control.cpp lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h lib/hui/hui.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c lib/hui/window_control.cpp -o $@ $(CXXFLAGS)

lib/hui/window_control_gtk.o : lib/hui/window_control_gtk.cpp lib/hui/Controls/ControlButton.h lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h lib/hui/Controls/ControlCheckBox.h lib/hui/Controls/ControlColorButton.h lib/hui/Controls/ControlComboBox.h lib/hui/Controls/ControlDrawingArea.h lib/hui/Controls/ControlEdit.h lib/hui/Controls/ControlExpander.h lib/hui/Controls/ControlGrid.h lib/hui/Controls/ControlGroup.h lib/hui/Controls/ControlLabel.h lib/hui/Controls/ControlListView.h lib/hui/Controls/ControlMultilineEdit.h lib/hui/Controls/ControlMenuButton.h lib/hui/Controls/ControlPaned.h lib/hui/Controls/ControlProgressBar.h lib/hui/Controls/ControlRadioButton.h lib/hui/Controls/ControlScroller.h lib/hui/Controls/ControlSeparator.h lib/hui/Controls/ControlSlider.h lib/hui/Controls/ControlSpinButton.h lib/hui/Controls/ControlTabControl.h lib/hui/Controls/ControlToggleButton.h lib/hui/Controls/ControlTreeView.h lib/hui/hui.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/hui/internal.h
	$(CPP) -c lib/hui/window_control_gtk.cpp -o $@ $(CXXFLAGS)

lib/hui/window_control_win.o : lib/hui/window_control_win.cpp lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c lib/hui/window_control_win.cpp -o $@ $(CXXFLAGS)

lib/hui/WindowGtk.o : lib/hui/WindowGtk.cpp lib/hui/Controls/Control.h lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Callback.h lib/hui/language.h lib/hui/Controls/ControlHeaderBar.h lib/hui/hui.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/hui/internal.h
	$(CPP) -c lib/hui/WindowGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/WindowWin.o : lib/hui/WindowWin.cpp lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c lib/hui/WindowWin.cpp -o $@ $(CXXFLAGS)

lib/image/color.o : lib/image/color.cpp lib/image/color.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/math/math.h
	$(CPP) -c lib/image/color.cpp -o $@ $(CXXFLAGS)

lib/image/image.o : lib/image/image.cpp lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/math/math.h lib/image/image_bmp.h lib/image/image_tga.h lib/image/image_jpg.h lib/image/image_png.h lib/image/ImagePainter.h lib/image/Painter.h lib/math/rect.h
	$(CPP) -c lib/image/image.cpp -o $@ $(CXXFLAGS)

lib/image/image_bmp.o : lib/image/image_bmp.cpp lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/hui.h lib/hui/common.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c lib/image/image_bmp.cpp -o $@ $(CXXFLAGS)

lib/image/image_jpg.o : lib/image/image_jpg.cpp lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/math/math.h
	$(CPP) -c lib/image/image_jpg.cpp -o $@ $(CXXFLAGS)

lib/image/image_png.o : lib/image/image_png.cpp lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h
	$(CPP) -c lib/image/image_png.cpp -o $@ $(CXXFLAGS)

lib/image/image_tga.o : lib/image/image_tga.cpp lib/image/image.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h
	$(CPP) -c lib/image/image_tga.cpp -o $@ $(CXXFLAGS)

lib/image/ImagePainter.o : lib/image/ImagePainter.cpp lib/image/ImagePainter.h lib/image/Painter.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/math/rect.h lib/math/math.h lib/image/image.h lib/config.h lib/math/vec2.h
	$(CPP) -c lib/image/ImagePainter.cpp -o $@ $(CXXFLAGS)

lib/kaba/asm/asm.o : lib/kaba/asm/asm.cpp lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/asm/internal.h
	$(CPP) -c lib/kaba/asm/asm.cpp -o $@ $(CXXFLAGS)

lib/kaba/asm/asm_arm.o : lib/kaba/asm/asm_arm.cpp lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/msg.h lib/kaba/asm/asm.h lib/kaba/asm/internal.h
	$(CPP) -c lib/kaba/asm/asm_arm.cpp -o $@ $(CXXFLAGS)

lib/kaba/asm/asm_x86.o : lib/kaba/asm/asm_x86.cpp lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/msg.h lib/kaba/asm/asm.h lib/kaba/asm/internal.h
	$(CPP) -c lib/kaba/asm/asm_x86.cpp -o $@ $(CXXFLAGS)

lib/kaba/compiler/Backend.o : lib/kaba/compiler/Backend.cpp lib/kaba/compiler/Backend.h lib/kaba/kaba.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/kaba/compiler/serializer.h lib/kaba/compiler/CommandList.h lib/kaba/compiler/SerialNode.h lib/file/msg.h
	$(CPP) -c lib/kaba/compiler/Backend.cpp -o $@ $(CXXFLAGS)

lib/kaba/compiler/BackendAmd64.o : lib/kaba/compiler/BackendAmd64.cpp lib/kaba/compiler/BackendAmd64.h lib/kaba/kaba.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/kaba/compiler/serializer.h lib/kaba/compiler/CommandList.h lib/kaba/compiler/SerialNode.h lib/kaba/compiler/BackendX86.h lib/kaba/compiler/Backend.h lib/file/msg.h
	$(CPP) -c lib/kaba/compiler/BackendAmd64.cpp -o $@ $(CXXFLAGS)

lib/kaba/compiler/BackendARM.o : lib/kaba/compiler/BackendARM.cpp lib/kaba/compiler/BackendARM.h lib/kaba/kaba.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/kaba/compiler/Backend.h lib/kaba/compiler/serializer.h lib/kaba/compiler/CommandList.h lib/kaba/compiler/SerialNode.h lib/file/msg.h
	$(CPP) -c lib/kaba/compiler/BackendARM.cpp -o $@ $(CXXFLAGS)

lib/kaba/compiler/BackendX86.o : lib/kaba/compiler/BackendX86.cpp lib/kaba/compiler/BackendX86.h lib/kaba/kaba.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/kaba/compiler/Backend.h lib/kaba/compiler/serializer.h lib/kaba/compiler/CommandList.h lib/kaba/compiler/SerialNode.h lib/file/msg.h
	$(CPP) -c lib/kaba/compiler/BackendX86.cpp -o $@ $(CXXFLAGS)

lib/kaba/compiler/CommandList.o : lib/kaba/compiler/CommandList.cpp lib/kaba/compiler/CommandList.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/kaba/compiler/SerialNode.h lib/kaba/compiler/serializer.h lib/kaba/kaba.h lib/base/pointer.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/file/msg.h
	$(CPP) -c lib/kaba/compiler/CommandList.cpp -o $@ $(CXXFLAGS)

lib/kaba/compiler/compiler.o : lib/kaba/compiler/compiler.cpp lib/kaba/kaba.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/base/set.h
	$(CPP) -c lib/kaba/compiler/compiler.cpp -o $@ $(CXXFLAGS)

lib/kaba/compiler/mapper.o : lib/kaba/compiler/mapper.cpp lib/kaba/kaba.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/kaba/compiler/serializer.h lib/kaba/compiler/CommandList.h lib/kaba/compiler/SerialNode.h lib/file/file.h lib/file/msg.h lib/file/file_op.h
	$(CPP) -c lib/kaba/compiler/mapper.cpp -o $@ $(CXXFLAGS)

lib/kaba/compiler/serializer.o : lib/kaba/compiler/serializer.cpp lib/kaba/kaba.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/kaba/compiler/serializer.h lib/kaba/compiler/CommandList.h lib/kaba/compiler/SerialNode.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/kaba/compiler/BackendAmd64.h lib/kaba/compiler/BackendX86.h lib/kaba/compiler/Backend.h lib/kaba/compiler/BackendARM.h lib/kaba/Interpreter.h
	$(CPP) -c lib/kaba/compiler/serializer.cpp -o $@ $(CXXFLAGS)

lib/kaba/compiler/SerializerX.o : lib/kaba/compiler/SerializerX.cpp lib/kaba/compiler/serializer.h lib/kaba/compiler/CommandList.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/kaba/compiler/SerialNode.h lib/kaba/kaba.h lib/base/pointer.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/file/msg.h
	$(CPP) -c lib/kaba/compiler/SerializerX.cpp -o $@ $(CXXFLAGS)

lib/kaba/compiler/SerialNode.o : lib/kaba/compiler/SerialNode.cpp lib/kaba/compiler/SerialNode.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/kaba/kaba.h lib/base/pointer.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/kaba/compiler/serializer.h lib/kaba/compiler/CommandList.h
	$(CPP) -c lib/kaba/compiler/SerialNode.cpp -o $@ $(CXXFLAGS)

lib/kaba/dynamic/call.o : lib/kaba/dynamic/call.cpp lib/kaba/syntax/Function.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/kaba/syntax/Class.h lib/kaba/CompilerConfiguration.h lib/file/path.h lib/file/msg.h
	$(CPP) -c lib/kaba/dynamic/call.cpp -o $@ $(CXXFLAGS)

lib/kaba/dynamic/dynamic.o : lib/kaba/dynamic/dynamic.cpp lib/kaba/dynamic/dynamic.h lib/kaba/kaba.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/kaba/dynamic/exception.h lib/kaba/dynamic/call.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/base/callable.h
	$(CPP) -c lib/kaba/dynamic/dynamic.cpp -o $@ $(CXXFLAGS)

lib/kaba/dynamic/exception.o : lib/kaba/dynamic/exception.cpp lib/kaba/dynamic/exception.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/kaba/kaba.h lib/base/pointer.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/file/msg.h
	$(CPP) -c lib/kaba/dynamic/exception.cpp -o $@ $(CXXFLAGS)

lib/kaba/lib/dict.o : lib/kaba/lib/dict.cpp lib/kaba/kaba.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/kaba/dynamic/exception.h lib/kaba/dynamic/dynamic.h lib/base/map.h lib/base/set.h
	$(CPP) -c lib/kaba/lib/dict.cpp -o $@ $(CXXFLAGS)

lib/kaba/lib/extern.o : lib/kaba/lib/extern.cpp lib/kaba/lib/extern.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/file/path.h lib/kaba/syntax/Flags.h lib/base/pointer.h lib/kaba/kaba.h lib/kaba/asm/asm.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/file/msg.h
	$(CPP) -c lib/kaba/lib/extern.cpp -o $@ $(CXXFLAGS)

lib/kaba/lib/lib.o : lib/kaba/lib/lib.cpp lib/kaba/kaba.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/kaba/lib/dict.h lib/kaba/dynamic/exception.h lib/config.h lib/math/complex.h lib/math/math.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/base/callable.h
	$(CPP) -c lib/kaba/lib/lib.cpp -o $@ $(CXXFLAGS)

lib/kaba/lib/lib_base.o : lib/kaba/lib/lib_base.cpp lib/kaba/kaba.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/kaba/dynamic/exception.h lib/kaba/dynamic/dynamic.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/base/callable.h
	$(CPP) -c lib/kaba/lib/lib_base.cpp -o $@ $(CXXFLAGS)

lib/kaba/lib/lib_doc.o : lib/kaba/lib/lib_doc.cpp lib/kaba/kaba.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/config.h lib/xfile/pdf.h lib/image/Painter.h
	$(CPP) -c lib/kaba/lib/lib_doc.cpp -o $@ $(CXXFLAGS)

lib/kaba/lib/lib_hui.o : lib/kaba/lib/lib_hui.cpp lib/kaba/kaba.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/config.h lib/base/callable.h lib/hui/hui.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c lib/kaba/lib/lib_hui.cpp -o $@ $(CXXFLAGS)

lib/kaba/lib/lib_image.o : lib/kaba/lib/lib_image.cpp lib/file/file.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/kaba/kaba.h lib/base/pointer.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/config.h lib/image/image.h lib/image/color.h lib/image/ImagePainter.h lib/image/Painter.h lib/math/rect.h lib/math/math.h
	$(CPP) -c lib/kaba/lib/lib_image.cpp -o $@ $(CXXFLAGS)

lib/kaba/lib/lib_kaba.o : lib/kaba/lib/lib_kaba.cpp lib/kaba/kaba.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/kaba/dynamic/exception.h lib/kaba/dynamic/dynamic.h
	$(CPP) -c lib/kaba/lib/lib_kaba.cpp -o $@ $(CXXFLAGS)

lib/kaba/lib/lib_math.o : lib/kaba/lib/lib_math.cpp lib/file/file.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/math/math.h lib/math/vec2.h lib/math/vector.h lib/math/complex.h lib/math/quaternion.h lib/math/matrix.h lib/math/matrix3.h lib/math/plane.h lib/math/rect.h lib/math/interpolation.h lib/math/random.h lib/image/color.h lib/base/map.h lib/base/set.h lib/kaba/kaba.h lib/base/pointer.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/config.h lib/kaba/dynamic/exception.h lib/any/any.h
	$(CPP) -c lib/kaba/lib/lib_math.cpp -o $@ $(CXXFLAGS)

lib/kaba/lib/lib_net.o : lib/kaba/lib/lib_net.cpp lib/file/file.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/kaba/kaba.h lib/base/pointer.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/config.h lib/kaba/dynamic/exception.h lib/net/net.h lib/net/Socket.h lib/net/BinaryBuffer.h
	$(CPP) -c lib/kaba/lib/lib_net.cpp -o $@ $(CXXFLAGS)

lib/kaba/lib/lib_nix.o : lib/kaba/lib/lib_nix.cpp lib/file/file.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/kaba/kaba.h lib/base/pointer.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/config.h lib/kaba/dynamic/exception.h lib/nix/nix.h lib/nix/nix_config.h lib/nix/nix_draw.h lib/nix/nix_light.h lib/math/matrix.h lib/math/math.h lib/math/vector.h lib/image/color.h lib/nix/nix_vertexbuffer.h lib/math/rect.h lib/nix/nix_shader.h lib/nix/nix_view.h lib/nix/nix_textures.h lib/nix/nix_framebuffer.h lib/nix/nix_buffer.h
	$(CPP) -c lib/kaba/lib/lib_nix.cpp -o $@ $(CXXFLAGS)

lib/kaba/lib/lib_os.o : lib/kaba/lib/lib_os.cpp lib/file/file.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/kaba/kaba.h lib/base/pointer.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/config.h lib/kaba/dynamic/exception.h
	$(CPP) -c lib/kaba/lib/lib_os.cpp -o $@ $(CXXFLAGS)

lib/kaba/lib/lib_threads.o : lib/kaba/lib/lib_threads.cpp lib/file/file.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/kaba/kaba.h lib/base/pointer.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/config.h lib/threads/Thread.h lib/threads/Mutex.h lib/threads/ThreadedWork.h
	$(CPP) -c lib/kaba/lib/lib_threads.cpp -o $@ $(CXXFLAGS)

lib/kaba/lib/lib_time.o : lib/kaba/lib/lib_time.cpp lib/file/file.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/kaba/kaba.h lib/base/pointer.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/config.h lib/hui/hui.h lib/hui/common.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c lib/kaba/lib/lib_time.cpp -o $@ $(CXXFLAGS)

lib/kaba/lib/lib_vulkan.o : lib/kaba/lib/lib_vulkan.cpp lib/file/file.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/kaba/kaba.h lib/base/pointer.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/config.h lib/math/matrix.h lib/math/math.h lib/kaba/dynamic/exception.h
	$(CPP) -c lib/kaba/lib/lib_vulkan.cpp -o $@ $(CXXFLAGS)

lib/kaba/syntax/Block.o : lib/kaba/syntax/Block.cpp lib/kaba/syntax/Block.h lib/kaba/syntax/Node.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/kaba/syntax/Flags.h lib/kaba/syntax/Function.h lib/kaba/syntax/Variable.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h
	$(CPP) -c lib/kaba/syntax/Block.cpp -o $@ $(CXXFLAGS)

lib/kaba/syntax/Class.o : lib/kaba/syntax/Class.cpp lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/kaba/kaba.h lib/base/pointer.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/file/file.h lib/file/msg.h lib/file/file_op.h
	$(CPP) -c lib/kaba/syntax/Class.cpp -o $@ $(CXXFLAGS)

lib/kaba/syntax/Constant.o : lib/kaba/syntax/Constant.cpp lib/kaba/kaba.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/kaba/dynamic/dynamic.h lib/file/file.h lib/file/msg.h lib/file/file_op.h
	$(CPP) -c lib/kaba/syntax/Constant.cpp -o $@ $(CXXFLAGS)

lib/kaba/syntax/Function.o : lib/kaba/syntax/Function.cpp lib/kaba/kaba.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/base/set.h
	$(CPP) -c lib/kaba/syntax/Function.cpp -o $@ $(CXXFLAGS)

lib/kaba/syntax/Identifier.o : lib/kaba/syntax/Identifier.cpp lib/kaba/syntax/Identifier.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h
	$(CPP) -c lib/kaba/syntax/Identifier.cpp -o $@ $(CXXFLAGS)

lib/kaba/syntax/implicit.o : lib/kaba/syntax/implicit.cpp lib/kaba/kaba.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/kaba/syntax/Parser.h lib/file/file.h lib/file/msg.h lib/file/file_op.h
	$(CPP) -c lib/kaba/syntax/implicit.cpp -o $@ $(CXXFLAGS)

lib/kaba/syntax/lexical.o : lib/kaba/syntax/lexical.cpp lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/kaba/syntax/lexical.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/Class.h lib/base/pointer.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Flags.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h
	$(CPP) -c lib/kaba/syntax/lexical.cpp -o $@ $(CXXFLAGS)

lib/kaba/syntax/macros.o : lib/kaba/syntax/macros.cpp lib/kaba/kaba.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/kaba/syntax/Parser.h lib/file/file.h lib/file/msg.h lib/file/file_op.h
	$(CPP) -c lib/kaba/syntax/macros.cpp -o $@ $(CXXFLAGS)

lib/kaba/syntax/Node.o : lib/kaba/syntax/Node.cpp lib/kaba/kaba.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/file/file.h lib/file/msg.h lib/file/file_op.h
	$(CPP) -c lib/kaba/syntax/Node.cpp -o $@ $(CXXFLAGS)

lib/kaba/syntax/Operator.o : lib/kaba/syntax/Operator.cpp lib/kaba/syntax/Operator.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/kaba/syntax/Identifier.h
	$(CPP) -c lib/kaba/syntax/Operator.cpp -o $@ $(CXXFLAGS)

lib/kaba/syntax/Parser.o : lib/kaba/syntax/Parser.cpp lib/kaba/kaba.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/kaba/syntax/Parser.h lib/config.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/common.h
	$(CPP) -c lib/kaba/syntax/Parser.cpp -o $@ $(CXXFLAGS)

lib/kaba/syntax/preprocessor.o : lib/kaba/syntax/preprocessor.cpp lib/kaba/kaba.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/kaba/dynamic/call.h lib/file/file.h lib/file/msg.h lib/file/file_op.h
	$(CPP) -c lib/kaba/syntax/preprocessor.cpp -o $@ $(CXXFLAGS)

lib/kaba/syntax/Statement.o : lib/kaba/syntax/Statement.cpp lib/kaba/syntax/Statement.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/kaba/syntax/Identifier.h
	$(CPP) -c lib/kaba/syntax/Statement.cpp -o $@ $(CXXFLAGS)

lib/kaba/syntax/SyntaxTree.o : lib/kaba/syntax/SyntaxTree.cpp lib/kaba/kaba.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/kaba/syntax/Parser.h lib/file/file.h lib/file/msg.h lib/file/file_op.h
	$(CPP) -c lib/kaba/syntax/SyntaxTree.cpp -o $@ $(CXXFLAGS)

lib/kaba/syntax/Variable.o : lib/kaba/syntax/Variable.cpp lib/kaba/syntax/Variable.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/kaba/syntax/Flags.h
	$(CPP) -c lib/kaba/syntax/Variable.cpp -o $@ $(CXXFLAGS)

lib/kaba/CompilerConfiguration.o : lib/kaba/CompilerConfiguration.cpp lib/kaba/CompilerConfiguration.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/syntax/Function.h lib/base/pointer.h
	$(CPP) -c lib/kaba/CompilerConfiguration.cpp -o $@ $(CXXFLAGS)

lib/kaba/Interpreter.o : lib/kaba/Interpreter.cpp lib/kaba/Interpreter.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/kaba/kaba.h lib/base/pointer.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/kaba/compiler/serializer.h lib/kaba/compiler/CommandList.h lib/kaba/compiler/SerialNode.h lib/kaba/dynamic/call.h lib/file/msg.h
	$(CPP) -c lib/kaba/Interpreter.cpp -o $@ $(CXXFLAGS)

lib/kaba/kaba.o : lib/kaba/kaba.cpp lib/file/file.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/kaba/kaba.h lib/base/pointer.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/kaba/syntax/Parser.h lib/kaba/Interpreter.h lib/config.h
	$(CPP) -c lib/kaba/kaba.cpp -o $@ $(CXXFLAGS)

lib/math/complex.o : lib/math/complex.cpp lib/math/complex.h lib/math/math.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h
	$(CPP) -c lib/math/complex.cpp -o $@ $(CXXFLAGS)

lib/math/interpolation.o : lib/math/interpolation.cpp lib/math/interpolation.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/math/complex.h lib/math/math.h lib/math/vector.h lib/math/vec2.h lib/math/quaternion.h lib/file/msg.h
	$(CPP) -c lib/math/interpolation.cpp -o $@ $(CXXFLAGS)

lib/math/math.o : lib/math/math.cpp lib/math/math.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h
	$(CPP) -c lib/math/math.cpp -o $@ $(CXXFLAGS)

lib/math/matrix.o : lib/math/matrix.cpp lib/math/matrix.h lib/math/math.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/math/vector.h lib/math/plane.h lib/math/quaternion.h lib/file/msg.h
	$(CPP) -c lib/math/matrix.cpp -o $@ $(CXXFLAGS)

lib/math/matrix3.o : lib/math/matrix3.cpp lib/math/matrix3.h lib/math/math.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/math/vector.h lib/math/quaternion.h lib/file/msg.h
	$(CPP) -c lib/math/matrix3.cpp -o $@ $(CXXFLAGS)

lib/math/plane.o : lib/math/plane.cpp lib/math/plane.h lib/math/math.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/math/vector.h lib/math/matrix.h lib/math/vec2.h
	$(CPP) -c lib/math/plane.cpp -o $@ $(CXXFLAGS)

lib/math/quaternion.o : lib/math/quaternion.cpp lib/math/quaternion.h lib/math/math.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/math/vector.h lib/math/matrix.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h
	$(CPP) -c lib/math/quaternion.cpp -o $@ $(CXXFLAGS)

lib/math/random.o : lib/math/random.cpp lib/math/random.h lib/math/math.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/math/vector.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h
	$(CPP) -c lib/math/random.cpp -o $@ $(CXXFLAGS)

lib/math/ray.o : lib/math/ray.cpp lib/math/ray.h lib/math/math.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/math/vector.h lib/math/plane.h
	$(CPP) -c lib/math/ray.cpp -o $@ $(CXXFLAGS)

lib/math/rect.o : lib/math/rect.cpp lib/math/rect.h lib/math/math.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/math/vec2.h
	$(CPP) -c lib/math/rect.cpp -o $@ $(CXXFLAGS)

lib/math/vec2.o : lib/math/vec2.cpp lib/math/vec2.h lib/math/math.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h
	$(CPP) -c lib/math/vec2.cpp -o $@ $(CXXFLAGS)

lib/math/vec4.o : lib/math/vec4.cpp lib/math/vec4.h lib/math/math.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h
	$(CPP) -c lib/math/vec4.cpp -o $@ $(CXXFLAGS)

lib/math/vector.o : lib/math/vector.cpp lib/math/vector.h lib/math/math.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/math/vec2.h lib/math/matrix.h lib/math/plane.h
	$(CPP) -c lib/math/vector.cpp -o $@ $(CXXFLAGS)

lib/net/BinaryBuffer.o : lib/net/BinaryBuffer.cpp lib/net/BinaryBuffer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/math/vector.h lib/math/math.h
	$(CPP) -c lib/net/BinaryBuffer.cpp -o $@ $(CXXFLAGS)

lib/net/net.o : lib/net/net.cpp lib/net/net.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/net/Socket.h lib/base/pointer.h lib/net/BinaryBuffer.h lib/file/msg.h
	$(CPP) -c lib/net/net.cpp -o $@ $(CXXFLAGS)

lib/net/Socket.o : lib/net/Socket.cpp lib/net/net.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/net/Socket.h lib/base/pointer.h lib/net/BinaryBuffer.h lib/file/msg.h
	$(CPP) -c lib/net/Socket.cpp -o $@ $(CXXFLAGS)

lib/nix/nix.o : lib/nix/nix.cpp lib/nix/nix.h lib/config.h lib/nix/nix_config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/nix/nix_draw.h lib/nix/nix_light.h lib/math/matrix.h lib/math/math.h lib/math/vector.h lib/image/color.h lib/nix/nix_vertexbuffer.h lib/math/rect.h lib/nix/nix_shader.h lib/base/pointer.h lib/file/path.h lib/nix/nix_view.h lib/nix/nix_textures.h lib/nix/nix_framebuffer.h lib/nix/nix_buffer.h lib/nix/nix_common.h lib/file/msg.h
	$(CPP) -c lib/nix/nix.cpp -o $@ $(CXXFLAGS)

lib/nix/nix_buffer.o : lib/nix/nix_buffer.cpp lib/nix/nix.h lib/config.h lib/nix/nix_config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/nix/nix_draw.h lib/nix/nix_light.h lib/math/matrix.h lib/math/math.h lib/math/vector.h lib/image/color.h lib/nix/nix_vertexbuffer.h lib/math/rect.h lib/nix/nix_shader.h lib/base/pointer.h lib/file/path.h lib/nix/nix_view.h lib/nix/nix_textures.h lib/nix/nix_framebuffer.h lib/nix/nix_buffer.h lib/nix/nix_common.h
	$(CPP) -c lib/nix/nix_buffer.cpp -o $@ $(CXXFLAGS)

lib/nix/nix_draw.o : lib/nix/nix_draw.cpp lib/nix/nix.h lib/config.h lib/nix/nix_config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/nix/nix_draw.h lib/nix/nix_light.h lib/math/matrix.h lib/math/math.h lib/math/vector.h lib/image/color.h lib/nix/nix_vertexbuffer.h lib/math/rect.h lib/nix/nix_shader.h lib/base/pointer.h lib/file/path.h lib/nix/nix_view.h lib/nix/nix_textures.h lib/nix/nix_framebuffer.h lib/nix/nix_buffer.h lib/nix/nix_common.h
	$(CPP) -c lib/nix/nix_draw.cpp -o $@ $(CXXFLAGS)

lib/nix/nix_framebuffer.o : lib/nix/nix_framebuffer.cpp lib/nix/nix.h lib/config.h lib/nix/nix_config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/nix/nix_draw.h lib/nix/nix_light.h lib/math/matrix.h lib/math/math.h lib/math/vector.h lib/image/color.h lib/nix/nix_vertexbuffer.h lib/math/rect.h lib/nix/nix_shader.h lib/base/pointer.h lib/file/path.h lib/nix/nix_view.h lib/nix/nix_textures.h lib/nix/nix_framebuffer.h lib/nix/nix_buffer.h lib/nix/nix_common.h lib/file/msg.h
	$(CPP) -c lib/nix/nix_framebuffer.cpp -o $@ $(CXXFLAGS)

lib/nix/nix_light.o : lib/nix/nix_light.cpp lib/nix/nix.h lib/config.h lib/nix/nix_config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/nix/nix_draw.h lib/nix/nix_light.h lib/math/matrix.h lib/math/math.h lib/math/vector.h lib/image/color.h lib/nix/nix_vertexbuffer.h lib/math/rect.h lib/nix/nix_shader.h lib/base/pointer.h lib/file/path.h lib/nix/nix_view.h lib/nix/nix_textures.h lib/nix/nix_framebuffer.h lib/nix/nix_buffer.h lib/nix/nix_common.h
	$(CPP) -c lib/nix/nix_light.cpp -o $@ $(CXXFLAGS)

lib/nix/nix_shader.o : lib/nix/nix_shader.cpp lib/nix/nix.h lib/config.h lib/nix/nix_config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/nix/nix_draw.h lib/nix/nix_light.h lib/math/matrix.h lib/math/math.h lib/math/vector.h lib/image/color.h lib/nix/nix_vertexbuffer.h lib/math/rect.h lib/nix/nix_shader.h lib/base/pointer.h lib/file/path.h lib/nix/nix_view.h lib/nix/nix_textures.h lib/nix/nix_framebuffer.h lib/nix/nix_buffer.h lib/nix/nix_common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h
	$(CPP) -c lib/nix/nix_shader.cpp -o $@ $(CXXFLAGS)

lib/nix/nix_textures.o : lib/nix/nix_textures.cpp lib/nix/nix.h lib/config.h lib/nix/nix_config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/nix/nix_draw.h lib/nix/nix_light.h lib/math/matrix.h lib/math/math.h lib/math/vector.h lib/image/color.h lib/nix/nix_vertexbuffer.h lib/math/rect.h lib/nix/nix_shader.h lib/base/pointer.h lib/file/path.h lib/nix/nix_view.h lib/nix/nix_textures.h lib/nix/nix_framebuffer.h lib/nix/nix_buffer.h lib/nix/nix_common.h lib/image/image.h lib/file/file.h lib/file/msg.h lib/file/file_op.h
	$(CPP) -c lib/nix/nix_textures.cpp -o $@ $(CXXFLAGS)

lib/nix/nix_vertexbuffer.o : lib/nix/nix_vertexbuffer.cpp lib/nix/nix.h lib/config.h lib/nix/nix_config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/nix/nix_draw.h lib/nix/nix_light.h lib/math/matrix.h lib/math/math.h lib/math/vector.h lib/image/color.h lib/nix/nix_vertexbuffer.h lib/math/rect.h lib/nix/nix_shader.h lib/base/pointer.h lib/file/path.h lib/nix/nix_view.h lib/nix/nix_textures.h lib/nix/nix_framebuffer.h lib/nix/nix_buffer.h lib/nix/nix_common.h
	$(CPP) -c lib/nix/nix_vertexbuffer.cpp -o $@ $(CXXFLAGS)

lib/nix/nix_view.o : lib/nix/nix_view.cpp lib/nix/nix.h lib/config.h lib/nix/nix_config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/nix/nix_draw.h lib/nix/nix_light.h lib/math/matrix.h lib/math/math.h lib/math/vector.h lib/image/color.h lib/nix/nix_vertexbuffer.h lib/math/rect.h lib/nix/nix_shader.h lib/base/pointer.h lib/file/path.h lib/nix/nix_view.h lib/nix/nix_textures.h lib/nix/nix_framebuffer.h lib/nix/nix_buffer.h lib/nix/nix_common.h lib/hui/hui.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/image/image.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c lib/nix/nix_view.cpp -o $@ $(CXXFLAGS)

lib/threads/Mutex.o : lib/threads/Mutex.cpp lib/file/file.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/threads/Mutex.h lib/threads/Thread.h
	$(CPP) -c lib/threads/Mutex.cpp -o $@ $(CXXFLAGS)

lib/threads/Thread.o : lib/threads/Thread.cpp lib/threads/Thread.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h
	$(CPP) -c lib/threads/Thread.cpp -o $@ $(CXXFLAGS)

lib/threads/ThreadedWork.o : lib/threads/ThreadedWork.cpp lib/file/file.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/threads/Mutex.h lib/threads/Thread.h lib/threads/ThreadedWork.h
	$(CPP) -c lib/threads/ThreadedWork.cpp -o $@ $(CXXFLAGS)

lib/xfile/chunked.o : lib/xfile/chunked.cpp lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/xfile/chunked.h
	$(CPP) -c lib/xfile/chunked.cpp -o $@ $(CXXFLAGS)

lib/xfile/pdf.o : lib/xfile/pdf.cpp lib/xfile/pdf.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/image/Painter.h lib/image/image.h lib/config.h lib/image/color.h lib/math/rect.h lib/math/math.h lib/math/vec2.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h
	$(CPP) -c lib/xfile/pdf.cpp -o $@ $(CXXFLAGS)

lib/xfile/xml.o : lib/xfile/xml.cpp lib/xfile/xml.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h
	$(CPP) -c lib/xfile/xml.cpp -o $@ $(CXXFLAGS)

module/audio/AudioAccumulator.o : module/audio/AudioAccumulator.cpp module/audio/AudioAccumulator.h data/audio/AudioBuffer.h lib/base/pointer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h data/Range.h module/Module.h stuff/Observable.h module/ModuleConfiguration.h module/port/Port.h Session.h plugins/PluginManager.h lib/kaba/kaba.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h data/base.h
	$(CPP) -c module/audio/AudioAccumulator.cpp -o $@ $(CXXFLAGS)

module/audio/AudioBackup.o : module/audio/AudioBackup.cpp module/audio/AudioBackup.h module/port/Port.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h module/Module.h lib/base/pointer.h stuff/Observable.h module/ModuleConfiguration.h data/base.h data/audio/AudioBuffer.h data/Range.h stuff/BackupManager.h lib/file/path.h lib/file/file.h lib/file/msg.h lib/file/file_op.h Session.h plugins/PluginManager.h lib/kaba/kaba.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h
	$(CPP) -c module/audio/AudioBackup.cpp -o $@ $(CXXFLAGS)

module/audio/AudioChannelSelector.o : module/audio/AudioChannelSelector.cpp module/audio/AudioChannelSelector.h module/port/Port.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h module/Module.h lib/base/pointer.h stuff/Observable.h module/ModuleConfiguration.h module/audio/PeakMeter.h module/audio/AudioVisualizer.h module/ConfigPanel.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h data/base.h data/audio/AudioBuffer.h data/Range.h stuff/BackupManager.h Session.h plugins/PluginManager.h lib/kaba/kaba.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h view/helper/PeakMeterDisplay.h view/helper/graph/Node.h lib/math/rect.h
	$(CPP) -c module/audio/AudioChannelSelector.cpp -o $@ $(CXXFLAGS)

module/audio/AudioEffect.o : module/audio/AudioEffect.cpp module/audio/AudioEffect.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h data/Range.h module/Module.h lib/base/pointer.h stuff/Observable.h module/port/Port.h module/ModuleFactory.h Session.h lib/math/math.h action/track/buffer/ActionTrackEditBuffer.h action/Action.h data/Data.h lib/file/path.h data/audio/AudioBuffer.h data/base.h data/Song.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h data/TrackLayer.h lib/base/set.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h
	$(CPP) -c module/audio/AudioEffect.cpp -o $@ $(CXXFLAGS)

module/audio/AudioJoiner.o : module/audio/AudioJoiner.cpp module/audio/AudioJoiner.h module/port/Port.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h module/Module.h lib/base/pointer.h stuff/Observable.h data/audio/AudioBuffer.h data/Range.h data/base.h
	$(CPP) -c module/audio/AudioJoiner.cpp -o $@ $(CXXFLAGS)

module/audio/AudioSource.o : module/audio/AudioSource.cpp module/audio/AudioSource.h module/Module.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h stuff/Observable.h module/port/Port.h module/ModuleFactory.h data/base.h
	$(CPP) -c module/audio/AudioSource.cpp -o $@ $(CXXFLAGS)

module/audio/AudioSucker.o : module/audio/AudioSucker.cpp module/audio/AudioSucker.h module/Module.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h stuff/Observable.h module/ModuleConfiguration.h module/port/Port.h module/ModuleFactory.h data/base.h data/audio/AudioBuffer.h data/Range.h Session.h plugins/PluginManager.h lib/kaba/kaba.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h
	$(CPP) -c module/audio/AudioSucker.cpp -o $@ $(CXXFLAGS)

module/audio/AudioVisualizer.o : module/audio/AudioVisualizer.cpp module/audio/AudioVisualizer.h module/Module.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h stuff/Observable.h module/port/Port.h module/ModuleFactory.h Session.h data/audio/AudioBuffer.h data/Range.h data/audio/RingBuffer.h data/base.h stuff/PerformanceMonitor.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/file/path.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h
	$(CPP) -c module/audio/AudioVisualizer.cpp -o $@ $(CXXFLAGS)

module/audio/BufferStreamer.o : module/audio/BufferStreamer.cpp module/audio/BufferStreamer.h module/audio/AudioSource.h module/Module.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h stuff/Observable.h module/port/Port.h data/audio/AudioBuffer.h data/Range.h
	$(CPP) -c module/audio/BufferStreamer.cpp -o $@ $(CXXFLAGS)

module/audio/PeakMeter.o : module/audio/PeakMeter.cpp module/audio/PeakMeter.h module/audio/AudioVisualizer.h module/Module.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h stuff/Observable.h module/port/Port.h data/audio/AudioBuffer.h data/Range.h data/audio/RingBuffer.h plugins/FastFourierTransform.h Session.h lib/math/complex.h lib/math/math.h module/ConfigPanel.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h view/helper/PeakMeterDisplay.h view/helper/graph/Node.h lib/math/rect.h
	$(CPP) -c module/audio/PeakMeter.cpp -o $@ $(CXXFLAGS)

module/audio/PitchDetector.o : module/audio/PitchDetector.cpp module/audio/PitchDetector.h module/midi/MidiSource.h module/Module.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h stuff/Observable.h module/port/Port.h data/base.h data/audio/AudioBuffer.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h plugins/FastFourierTransform.h lib/math/complex.h lib/math/math.h
	$(CPP) -c module/audio/PitchDetector.cpp -o $@ $(CXXFLAGS)

module/audio/SongRenderer.o : module/audio/SongRenderer.cpp module/audio/SongRenderer.h data/Range.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h module/audio/AudioSource.h module/Module.h lib/base/pointer.h stuff/Observable.h module/port/Port.h lib/base/set.h module/audio/TrackRenderer.h module/audio/AudioEffect.h module/beats/BarStreamer.h module/beats/BeatSource.h data/rhythm/BarCollection.h data/rhythm/Bar.h module/beats/BeatMidifier.h module/midi/MidiSource.h module/synthesizer/Synthesizer.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/midi/Temperament.h data/base.h data/Song.h data/Data.h lib/file/path.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h data/Track.h data/TrackLayer.h data/audio/AudioBuffer.h lib/math/math.h
	$(CPP) -c module/audio/SongRenderer.cpp -o $@ $(CXXFLAGS)

module/audio/TrackRenderer.o : module/audio/TrackRenderer.cpp module/audio/TrackRenderer.h module/audio/AudioSource.h module/Module.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h stuff/Observable.h module/port/Port.h module/audio/SongRenderer.h data/Range.h lib/base/set.h module/audio/AudioEffect.h module/beats/BarStreamer.h module/beats/BeatSource.h data/rhythm/BarCollection.h data/rhythm/Bar.h module/beats/BeatMidifier.h module/midi/MidiSource.h module/midi/MidiEffect.h module/synthesizer/Synthesizer.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/midi/Temperament.h module/midi/MidiEventStreamer.h data/base.h data/Song.h data/Data.h lib/file/path.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h data/Track.h data/TrackLayer.h data/Curve.h data/CrossFade.h data/Sample.h lib/math/rect.h lib/math/math.h data/audio/AudioBuffer.h data/SampleRef.h Session.h
	$(CPP) -c module/audio/TrackRenderer.cpp -o $@ $(CXXFLAGS)

module/beats/BarStreamer.o : module/beats/BarStreamer.cpp module/beats/BarStreamer.h module/beats/BeatSource.h module/port/Port.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h module/Module.h lib/base/pointer.h stuff/Observable.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h data/rhythm/Beat.h
	$(CPP) -c module/beats/BarStreamer.cpp -o $@ $(CXXFLAGS)

module/beats/BeatMidifier.o : module/beats/BeatMidifier.cpp module/beats/BeatMidifier.h module/midi/MidiSource.h module/Module.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h stuff/Observable.h module/port/Port.h module/ModuleFactory.h data/midi/MidiData.h data/Range.h data/midi/MidiNote.h data/midi/MidiEvent.h data/rhythm/Beat.h data/base.h
	$(CPP) -c module/beats/BeatMidifier.cpp -o $@ $(CXXFLAGS)

module/beats/BeatSource.o : module/beats/BeatSource.cpp module/beats/BeatSource.h module/port/Port.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h module/Module.h lib/base/pointer.h stuff/Observable.h module/ModuleFactory.h Session.h data/base.h
	$(CPP) -c module/beats/BeatSource.cpp -o $@ $(CXXFLAGS)

module/midi/MidiAccumulator.o : module/midi/MidiAccumulator.cpp module/midi/MidiAccumulator.h data/midi/MidiData.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h data/Range.h data/midi/MidiNote.h data/midi/MidiEvent.h module/Module.h stuff/Observable.h module/port/Port.h data/base.h
	$(CPP) -c module/midi/MidiAccumulator.cpp -o $@ $(CXXFLAGS)

module/midi/MidiEffect.o : module/midi/MidiEffect.cpp module/midi/MidiEffect.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h data/Range.h module/Module.h lib/base/pointer.h stuff/Observable.h module/port/Port.h module/ModuleFactory.h Session.h lib/math/math.h data/base.h data/Song.h data/Data.h lib/file/path.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h data/TrackLayer.h lib/base/set.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/SongSelection.h
	$(CPP) -c module/midi/MidiEffect.cpp -o $@ $(CXXFLAGS)

module/midi/MidiEventStreamer.o : module/midi/MidiEventStreamer.cpp module/midi/MidiEventStreamer.h module/midi/MidiSource.h module/Module.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h stuff/Observable.h module/port/Port.h data/midi/MidiData.h data/Range.h data/midi/MidiNote.h data/midi/MidiEvent.h module/beats/BeatSource.h
	$(CPP) -c module/midi/MidiEventStreamer.cpp -o $@ $(CXXFLAGS)

module/midi/MidiJoiner.o : module/midi/MidiJoiner.cpp module/midi/MidiJoiner.h module/port/Port.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h module/Module.h lib/base/pointer.h stuff/Observable.h data/base.h data/midi/MidiData.h data/Range.h data/midi/MidiNote.h data/midi/MidiEvent.h
	$(CPP) -c module/midi/MidiJoiner.cpp -o $@ $(CXXFLAGS)

module/midi/MidiPreviewSource.o : module/midi/MidiPreviewSource.cpp module/midi/MidiPreviewSource.h module/midi/MidiSource.h module/Module.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h stuff/Observable.h module/port/Port.h data/midi/MidiData.h data/Range.h data/midi/MidiNote.h data/midi/MidiEvent.h Session.h lib/file/msg.h
	$(CPP) -c module/midi/MidiPreviewSource.cpp -o $@ $(CXXFLAGS)

module/midi/MidiSource.o : module/midi/MidiSource.cpp module/midi/MidiSource.h module/Module.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h stuff/Observable.h module/port/Port.h Session.h module/ModuleFactory.h data/base.h data/Song.h data/Data.h lib/file/path.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h
	$(CPP) -c module/midi/MidiSource.cpp -o $@ $(CXXFLAGS)

module/midi/MidiSucker.o : module/midi/MidiSucker.cpp module/midi/MidiSucker.h module/Module.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h stuff/Observable.h module/port/Port.h data/base.h data/midi/MidiData.h data/Range.h data/midi/MidiNote.h data/midi/MidiEvent.h
	$(CPP) -c module/midi/MidiSucker.cpp -o $@ $(CXXFLAGS)

module/port/Port.o : module/port/Port.cpp module/port/Port.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h
	$(CPP) -c module/port/Port.cpp -o $@ $(CXXFLAGS)

module/synthesizer/DebugSynthesizer.o : module/synthesizer/DebugSynthesizer.cpp module/synthesizer/DebugSynthesizer.h module/synthesizer/Synthesizer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/set.h data/midi/MidiData.h lib/base/pointer.h data/Range.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/midi/Temperament.h module/Module.h stuff/Observable.h module/port/Port.h module/synthesizer/EnvelopeADSR.h data/audio/AudioBuffer.h
	$(CPP) -c module/synthesizer/DebugSynthesizer.cpp -o $@ $(CXXFLAGS)

module/synthesizer/DummySynthesizer.o : module/synthesizer/DummySynthesizer.cpp module/synthesizer/DummySynthesizer.h module/synthesizer/Synthesizer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/set.h data/midi/MidiData.h lib/base/pointer.h data/Range.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/midi/Temperament.h module/Module.h stuff/Observable.h module/port/Port.h module/synthesizer/EnvelopeADSR.h data/Song.h data/Data.h lib/file/path.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h data/audio/AudioBuffer.h lib/math/math.h
	$(CPP) -c module/synthesizer/DummySynthesizer.cpp -o $@ $(CXXFLAGS)

module/synthesizer/EnvelopeADSR.o : module/synthesizer/EnvelopeADSR.cpp module/synthesizer/EnvelopeADSR.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/math/math.h
	$(CPP) -c module/synthesizer/EnvelopeADSR.cpp -o $@ $(CXXFLAGS)

module/synthesizer/Synthesizer.o : module/synthesizer/Synthesizer.cpp module/synthesizer/Synthesizer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/set.h data/midi/MidiData.h lib/base/pointer.h data/Range.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/midi/Temperament.h module/Module.h stuff/Observable.h module/port/Port.h module/ModuleFactory.h data/base.h data/Song.h data/Data.h lib/file/path.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h data/audio/AudioBuffer.h module/synthesizer/DummySynthesizer.h module/synthesizer/EnvelopeADSR.h lib/math/math.h
	$(CPP) -c module/synthesizer/Synthesizer.cpp -o $@ $(CXXFLAGS)

module/AutoConfigPanel.o : module/AutoConfigPanel.cpp module/AutoConfigPanel.h module/ConfigPanel.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h module/Module.h stuff/Observable.h module/ModuleConfiguration.h view/helper/Slider.h lib/base/callable.h view/sidebar/SampleManagerConsole.h view/sidebar/SideBar.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h data/Sample.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h lib/math/rect.h data/audio/AudioBuffer.h data/SampleRef.h lib/kaba/kaba.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h Session.h device/Device.h device/DeviceManager.h
	$(CPP) -c module/AutoConfigPanel.cpp -o $@ $(CXXFLAGS)

module/ConfigPanel.o : module/ConfigPanel.cpp module/ConfigPanel.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h module/Module.h stuff/Observable.h view/helper/Progress.h view/helper/ModulePanel.h plugins/PluginManager.h lib/kaba/kaba.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h Session.h
	$(CPP) -c module/ConfigPanel.cpp -o $@ $(CXXFLAGS)

module/Module.o : module/Module.cpp module/Module.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h stuff/Observable.h module/ConfigPanel.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h module/AutoConfigPanel.h module/ModuleConfiguration.h module/ModuleFactory.h module/port/Port.h Session.h lib/any/any.h lib/kaba/kaba.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h plugins/PluginManager.h plugins/Plugin.h stuff/PerformanceMonitor.h
	$(CPP) -c module/Module.cpp -o $@ $(CXXFLAGS)

module/ModuleConfiguration.o : module/ModuleConfiguration.cpp lib/kaba/kaba.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h module/ModuleConfiguration.h module/Module.h stuff/Observable.h data/SampleRef.h lib/math/rect.h lib/math/math.h data/Sample.h data/midi/MidiData.h data/Range.h data/midi/MidiNote.h data/midi/MidiEvent.h data/audio/AudioBuffer.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h Session.h
	$(CPP) -c module/ModuleConfiguration.cpp -o $@ $(CXXFLAGS)

module/ModuleFactory.o : module/ModuleFactory.cpp module/ModuleFactory.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h module/Module.h lib/base/pointer.h stuff/Observable.h module/audio/SongRenderer.h data/Range.h module/audio/AudioSource.h module/port/Port.h lib/base/set.h module/audio/AudioBackup.h module/ModuleConfiguration.h module/audio/AudioChannelSelector.h module/audio/PeakMeter.h module/audio/AudioVisualizer.h module/audio/AudioJoiner.h module/audio/AudioEffect.h module/audio/AudioSucker.h module/midi/MidiEffect.h module/midi/MidiJoiner.h module/midi/MidiSource.h module/midi/MidiSucker.h module/audio/PitchDetector.h module/beats/BeatMidifier.h module/synthesizer/Synthesizer.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/midi/Temperament.h module/synthesizer/DummySynthesizer.h module/synthesizer/EnvelopeADSR.h module/audio/AudioAccumulator.h data/audio/AudioBuffer.h module/midi/MidiAccumulator.h device/stream/AudioInput.h data/audio/RingBuffer.h device/stream/AudioOutput.h device/stream/MidiInput.h plugins/Plugin.h lib/file/path.h plugins/PluginManager.h lib/kaba/kaba.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h Session.h
	$(CPP) -c module/ModuleFactory.cpp -o $@ $(CXXFLAGS)

module/SignalChain.o : module/SignalChain.cpp module/SignalChain.h module/Module.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h stuff/Observable.h device/stream/AudioOutput.h data/audio/RingBuffer.h data/audio/AudioBuffer.h data/Range.h module/ModuleConfiguration.h module/ModuleFactory.h module/port/Port.h module/audio/AudioSource.h module/midi/MidiSource.h module/beats/BeatSource.h Session.h plugins/PluginManager.h lib/kaba/kaba.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/xfile/xml.h lib/threads/Thread.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h stuff/PerformanceMonitor.h
	$(CPP) -c module/SignalChain.cpp -o $@ $(CXXFLAGS)

plugins/ExtendedAudioBuffer.o : plugins/ExtendedAudioBuffer.cpp plugins/FastFourierTransform.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/math/math.h plugins/ExtendedAudioBuffer.h data/audio/AudioBuffer.h lib/base/pointer.h data/Range.h
	$(CPP) -c plugins/ExtendedAudioBuffer.cpp -o $@ $(CXXFLAGS)

plugins/FastFourierTransform.o : plugins/FastFourierTransform.cpp plugins/FastFourierTransform.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/math/complex.h lib/math/math.h lib/file/msg.h
	$(CPP) -c plugins/FastFourierTransform.cpp -o $@ $(CXXFLAGS)

plugins/Plugin.o : plugins/Plugin.cpp plugins/Plugin.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h Session.h stuff/Observable.h lib/kaba/kaba.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c plugins/Plugin.cpp -o $@ $(CXXFLAGS)

plugins/PluginManager.o : plugins/PluginManager.cpp plugins/PluginManager.h lib/kaba/kaba.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h plugins/FastFourierTransform.h plugins/Plugin.h plugins/ExtendedAudioBuffer.h data/audio/AudioBuffer.h data/Range.h plugins/ProfileManager.h plugins/TsunamiPlugin.h module/Module.h stuff/Observable.h Tsunami.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h TsunamiWindow.h Session.h view/helper/Slider.h lib/base/callable.h view/helper/Drawing.h data/base.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h data/SongSelection.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h data/TrackMarker.h module/audio/AudioEffect.h module/port/Port.h data/Sample.h lib/math/rect.h data/SampleRef.h data/audio/BufferInterpolator.h module/ModuleFactory.h module/ConfigPanel.h module/SignalChain.h module/ModuleConfiguration.h module/synthesizer/Synthesizer.h data/midi/Temperament.h module/synthesizer/DummySynthesizer.h module/synthesizer/EnvelopeADSR.h module/audio/AudioSource.h module/audio/SongRenderer.h module/audio/AudioVisualizer.h module/audio/PitchDetector.h module/midi/MidiSource.h module/beats/BeatSource.h module/beats/BeatMidifier.h module/midi/MidiEffect.h view/helper/Progress.h storage/Storage.h device/DeviceManager.h device/stream/AudioInput.h data/audio/RingBuffer.h device/stream/AudioOutput.h device/stream/MidiInput.h module/audio/AudioAccumulator.h module/midi/MidiAccumulator.h stuff/Clipboard.h view/audioview/AudioView.h data/midi/Scale.h view/audioview/TrackHeightManager.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h view/dialog/ModuleSelectorDialog.h view/audioview/graph/AudioViewLayer.h view/helper/graph/Node.h view/sidebar/SampleManagerConsole.h view/sidebar/SideBar.h view/mode/ViewModeCapture.h view/mode/ViewModeDefault.h view/mode/ViewMode.h view/painter/GridPainter.h view/painter/BasicGridPainter.h view/painter/MidiPainter.h view/painter/MultiLinePainter.h
	$(CPP) -c plugins/PluginManager.cpp -o $@ $(CXXFLAGS)

plugins/ProfileManager.o : plugins/ProfileManager.cpp module/Module.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h stuff/Observable.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/xfile/xml.h Tsunami.h Session.h plugins/ProfileManager.h
	$(CPP) -c plugins/ProfileManager.cpp -o $@ $(CXXFLAGS)

plugins/TsunamiPlugin.o : plugins/TsunamiPlugin.cpp plugins/TsunamiPlugin.h module/Module.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h stuff/Observable.h Session.h plugins/PluginManager.h lib/kaba/kaba.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h plugins/Plugin.h module/ModuleFactory.h
	$(CPP) -c plugins/TsunamiPlugin.cpp -o $@ $(CXXFLAGS)

storage/dialog/PdfConfigDialog.o : storage/dialog/PdfConfigDialog.cpp storage/dialog/PdfConfigDialog.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h storage/StorageOperationData.h lib/any/any.h data/base.h data/Song.h data/Data.h stuff/Observable.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h
	$(CPP) -c storage/dialog/PdfConfigDialog.cpp -o $@ $(CXXFLAGS)

storage/dialog/RawConfigDialog.o : storage/dialog/RawConfigDialog.cpp storage/dialog/RawConfigDialog.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h storage/StorageOperationData.h lib/any/any.h Session.h stuff/Observable.h data/Track.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/base.h
	$(CPP) -c storage/dialog/RawConfigDialog.cpp -o $@ $(CXXFLAGS)

storage/format/Format.o : storage/format/Format.cpp storage/format/Format.h storage/StorageOperationData.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h Tsunami.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h data/base.h data/Song.h data/Data.h stuff/Observable.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h data/audio/AudioBuffer.h action/ActionManager.h action/Action.h module/audio/SongRenderer.h module/audio/AudioSource.h module/Module.h module/port/Port.h
	$(CPP) -c storage/format/Format.cpp -o $@ $(CXXFLAGS)

storage/format/FormatFlac.o : storage/format/FormatFlac.cpp storage/format/FormatFlac.h storage/format/Format.h storage/StorageOperationData.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h module/port/Port.h Session.h stuff/Observable.h data/Track.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h lib/base/set.h data/TrackMarker.h module/audio/AudioEffect.h module/Module.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/base.h data/audio/AudioBuffer.h
	$(CPP) -c storage/format/FormatFlac.cpp -o $@ $(CXXFLAGS)

storage/format/FormatGuitarPro.o : storage/format/FormatGuitarPro.cpp storage/format/FormatGuitarPro.h storage/format/Format.h storage/StorageOperationData.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h data/rhythm/Bar.h data/Range.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h stuff/Observable.h data/TrackLayer.h lib/base/set.h data/TrackMarker.h module/audio/AudioEffect.h module/Module.h module/port/Port.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/base.h
	$(CPP) -c storage/format/FormatGuitarPro.cpp -o $@ $(CXXFLAGS)

storage/format/FormatM4a.o : storage/format/FormatM4a.cpp storage/format/FormatM4a.h storage/format/Format.h storage/StorageOperationData.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h storage/Storage.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c storage/format/FormatM4a.cpp -o $@ $(CXXFLAGS)

storage/format/FormatMidi.o : storage/format/FormatMidi.cpp storage/format/FormatMidi.h storage/format/Format.h storage/StorageOperationData.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/base/map.h lib/base/set.h data/Track.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h stuff/Observable.h data/TrackLayer.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/base.h
	$(CPP) -c storage/format/FormatMidi.cpp -o $@ $(CXXFLAGS)

storage/format/FormatMp3.o : storage/format/FormatMp3.cpp storage/format/FormatMp3.h storage/format/Format.h storage/StorageOperationData.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h storage/Storage.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h data/Track.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h stuff/Observable.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/base.h storage/helper/minimp3.h
	$(CPP) -c storage/format/FormatMp3.cpp -o $@ $(CXXFLAGS)

storage/format/FormatNami.o : storage/format/FormatNami.cpp storage/format/FormatNami.h storage/format/Format.h storage/StorageOperationData.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h storage/Storage.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h Session.h stuff/Observable.h plugins/PluginManager.h lib/kaba/kaba.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h data/Track.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h data/TrackMarker.h module/audio/AudioEffect.h module/Module.h module/port/Port.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/base.h data/Curve.h data/CrossFade.h data/SampleRef.h lib/math/rect.h data/Sample.h data/audio/AudioBuffer.h module/midi/MidiEffect.h module/synthesizer/Synthesizer.h data/midi/Temperament.h lib/xfile/chunked.h
	$(CPP) -c storage/format/FormatNami.cpp -o $@ $(CXXFLAGS)

storage/format/FormatOgg.o : storage/format/FormatOgg.cpp storage/format/FormatOgg.h storage/format/Format.h storage/StorageOperationData.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h module/port/Port.h Session.h stuff/Observable.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h data/base.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h data/TrackMarker.h module/audio/AudioEffect.h module/Module.h data/audio/AudioBuffer.h
	$(CPP) -c storage/format/FormatOgg.cpp -o $@ $(CXXFLAGS)

storage/format/FormatPdf.o : storage/format/FormatPdf.cpp storage/format/FormatPdf.h storage/format/Format.h storage/StorageOperationData.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h data/base.h data/Song.h data/Data.h stuff/Observable.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/SongSelection.h lib/base/set.h lib/xfile/pdf.h lib/image/Painter.h lib/math/rect.h lib/math/math.h storage/dialog/PdfConfigDialog.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h view/ColorScheme.h view/helper/SymbolRenderer.h view/HoverData.h view/painter/MultiLinePainter.h
	$(CPP) -c storage/format/FormatPdf.cpp -o $@ $(CXXFLAGS)

storage/format/FormatRaw.o : storage/format/FormatRaw.cpp storage/format/FormatRaw.h storage/format/Format.h storage/StorageOperationData.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h module/audio/SongRenderer.h data/Range.h module/audio/AudioSource.h module/Module.h stuff/Observable.h module/port/Port.h lib/base/set.h lib/math/math.h data/base.h data/audio/AudioBuffer.h Session.h TsunamiWindow.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h storage/dialog/RawConfigDialog.h
	$(CPP) -c storage/format/FormatRaw.cpp -o $@ $(CXXFLAGS)

storage/format/FormatSoundFont2.o : storage/format/FormatSoundFont2.cpp storage/format/FormatSoundFont2.h storage/format/Format.h storage/StorageOperationData.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h data/base.h data/Song.h data/Data.h stuff/Observable.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h data/Sample.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h lib/math/rect.h lib/math/math.h data/audio/AudioBuffer.h Session.h
	$(CPP) -c storage/format/FormatSoundFont2.cpp -o $@ $(CXXFLAGS)

storage/format/FormatWave.o : storage/format/FormatWave.cpp storage/format/FormatWave.h storage/format/Format.h storage/StorageOperationData.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h module/port/Port.h Session.h stuff/Observable.h data/base.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/audio/AudioBuffer.h lib/math/math.h lib/hui/language.h
	$(CPP) -c storage/format/FormatWave.cpp -o $@ $(CXXFLAGS)

storage/Storage.o : storage/Storage.cpp storage/Storage.h lib/file/file.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h module/audio/SongRenderer.h data/Range.h module/audio/AudioSource.h module/Module.h stuff/Observable.h module/port/Port.h storage/format/Format.h storage/StorageOperationData.h lib/any/any.h storage/format/FormatWave.h storage/format/FormatRaw.h storage/format/FormatOgg.h storage/format/FormatFlac.h storage/format/FormatM4a.h storage/format/FormatMidi.h storage/format/FormatPdf.h storage/format/FormatMp3.h storage/format/FormatSoundFont2.h storage/format/FormatGuitarPro.h storage/format/FormatNami.h action/ActionManager.h action/Action.h data/Data.h Tsunami.h TsunamiWindow.h Session.h view/helper/Progress.h stuff/Log.h data/base.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h data/Song.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/audio/AudioBuffer.h data/SongSelection.h module/audio/BufferStreamer.h
	$(CPP) -c storage/Storage.cpp -o $@ $(CXXFLAGS)

storage/StorageOperationData.o : storage/StorageOperationData.cpp storage/StorageOperationData.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h storage/Storage.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h view/helper/Progress.h stuff/Observable.h Session.h TsunamiWindow.h data/Track.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h data/audio/AudioBuffer.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h
	$(CPP) -c storage/StorageOperationData.cpp -o $@ $(CXXFLAGS)

stuff/BackupManager.o : stuff/BackupManager.cpp stuff/BackupManager.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/path.h data/Track.h lib/base/pointer.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h stuff/Observable.h Tsunami.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h Session.h
	$(CPP) -c stuff/BackupManager.cpp -o $@ $(CXXFLAGS)

stuff/CLIParser.o : stuff/CLIParser.cpp stuff/CLIParser.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/msg.h
	$(CPP) -c stuff/CLIParser.cpp -o $@ $(CXXFLAGS)

stuff/Clipboard.o : stuff/Clipboard.cpp stuff/Clipboard.h lib/base/pointer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h stuff/Observable.h view/audioview/AudioView.h data/SongSelection.h lib/base/set.h data/Range.h data/midi/Scale.h view/audioview/TrackHeightManager.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/math/rect.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h action/track/sample/ActionTrackPasteAsSample.h action/ActionGroup.h action/Action.h data/Data.h Session.h data/base.h data/Song.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h data/TrackMarker.h module/audio/AudioEffect.h module/Module.h module/port/Port.h data/Sample.h data/audio/AudioBuffer.h
	$(CPP) -c stuff/Clipboard.cpp -o $@ $(CXXFLAGS)

stuff/Diff.o : stuff/Diff.cpp stuff/Diff.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h data/Song.h data/Data.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h lib/base/set.h data/Sample.h lib/math/rect.h lib/math/math.h data/audio/AudioBuffer.h data/CrossFade.h module/audio/AudioEffect.h module/Module.h module/port/Port.h module/midi/MidiEffect.h module/synthesizer/Synthesizer.h data/midi/Temperament.h
	$(CPP) -c stuff/Diff.cpp -o $@ $(CXXFLAGS)

stuff/ErrorHandler.o : stuff/ErrorHandler.cpp stuff/ErrorHandler.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/msg.h lib/kaba/kaba.h lib/base/pointer.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/hui/error.h lib/hui/Callback.h
	$(CPP) -c stuff/ErrorHandler.cpp -o $@ $(CXXFLAGS)

stuff/Log.o : stuff/Log.cpp stuff/Log.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h stuff/Observable.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h Session.h
	$(CPP) -c stuff/Log.cpp -o $@ $(CXXFLAGS)

stuff/Observable.o : stuff/Observable.cpp stuff/Observable.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/kaba/kaba.h lib/base/pointer.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h lib/file/msg.h
	$(CPP) -c stuff/Observable.cpp -o $@ $(CXXFLAGS)

stuff/PerformanceMonitor.o : stuff/PerformanceMonitor.cpp stuff/PerformanceMonitor.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h stuff/Observable.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h module/Module.h
	$(CPP) -c stuff/PerformanceMonitor.cpp -o $@ $(CXXFLAGS)

test/TestAudioBuffer.o : test/TestAudioBuffer.cpp test/TestAudioBuffer.h test/UnitTest.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h data/audio/AudioBuffer.h lib/base/pointer.h data/Range.h
	$(CPP) -c test/TestAudioBuffer.cpp -o $@ $(CXXFLAGS)

test/TestInterpolator.o : test/TestInterpolator.cpp test/TestInterpolator.h test/UnitTest.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h data/audio/BufferInterpolator.h lib/file/msg.h
	$(CPP) -c test/TestInterpolator.cpp -o $@ $(CXXFLAGS)

test/TestMidiPreview.o : test/TestMidiPreview.cpp test/TestMidiPreview.h test/UnitTest.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/msg.h view/helper/MidiPreview.h lib/base/pointer.h module/SignalChain.h module/Module.h stuff/Observable.h module/synthesizer/Synthesizer.h lib/base/set.h data/midi/MidiData.h data/Range.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/midi/Temperament.h module/port/Port.h module/midi/MidiPreviewSource.h module/midi/MidiSource.h Session.h
	$(CPP) -c test/TestMidiPreview.cpp -o $@ $(CXXFLAGS)

test/TestMixer.o : test/TestMixer.cpp test/TestMixer.h test/UnitTest.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h Session.h lib/base/pointer.h stuff/Observable.h data/base.h data/Song.h data/Data.h lib/file/path.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h lib/base/set.h data/audio/AudioBuffer.h module/audio/SongRenderer.h module/audio/AudioSource.h module/Module.h module/port/Port.h
	$(CPP) -c test/TestMixer.cpp -o $@ $(CXXFLAGS)

test/TestPlugins.o : test/TestPlugins.cpp test/TestPlugins.h test/UnitTest.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h data/audio/AudioBuffer.h lib/base/pointer.h data/Range.h module/audio/AudioEffect.h module/Module.h stuff/Observable.h module/port/Port.h module/audio/AudioSource.h module/midi/MidiEffect.h module/midi/MidiSource.h module/synthesizer/Synthesizer.h lib/base/set.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/midi/Temperament.h module/SignalChain.h plugins/TsunamiPlugin.h plugins/PluginManager.h lib/kaba/kaba.h lib/file/path.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h plugins/Plugin.h Session.h
	$(CPP) -c test/TestPlugins.cpp -o $@ $(CXXFLAGS)

test/TestPointer.o : test/TestPointer.cpp test/TestPointer.h test/UnitTest.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/file/msg.h
	$(CPP) -c test/TestPointer.cpp -o $@ $(CXXFLAGS)

test/TestRhythm.o : test/TestRhythm.cpp test/TestRhythm.h test/UnitTest.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h data/rhythm/Bar.h lib/base/pointer.h data/Range.h data/rhythm/Beat.h lib/file/msg.h
	$(CPP) -c test/TestRhythm.cpp -o $@ $(CXXFLAGS)

test/TestRingBuffer.o : test/TestRingBuffer.cpp test/TestRingBuffer.h test/UnitTest.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h data/audio/RingBuffer.h data/audio/AudioBuffer.h lib/base/pointer.h data/Range.h
	$(CPP) -c test/TestRingBuffer.cpp -o $@ $(CXXFLAGS)

test/TestSignalChain.o : test/TestSignalChain.cpp test/TestSignalChain.h test/UnitTest.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h module/synthesizer/DebugSynthesizer.h module/synthesizer/Synthesizer.h lib/base/set.h data/midi/MidiData.h lib/base/pointer.h data/Range.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/midi/Temperament.h module/Module.h stuff/Observable.h module/port/Port.h module/synthesizer/EnvelopeADSR.h
	$(CPP) -c test/TestSignalChain.cpp -o $@ $(CXXFLAGS)

test/TestStreams.o : test/TestStreams.cpp test/TestStreams.h test/UnitTest.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/msg.h lib/math/math.h data/base.h module/audio/AudioSource.h module/Module.h lib/base/pointer.h stuff/Observable.h module/port/Port.h module/SignalChain.h Session.h device/stream/AudioOutput.h data/audio/RingBuffer.h data/audio/AudioBuffer.h data/Range.h module/ModuleConfiguration.h
	$(CPP) -c test/TestStreams.cpp -o $@ $(CXXFLAGS)

test/TestThreads.o : test/TestThreads.cpp test/TestThreads.h test/UnitTest.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/msg.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/threads/Thread.h
	$(CPP) -c test/TestThreads.cpp -o $@ $(CXXFLAGS)

test/TestTrackVersion.o : test/TestTrackVersion.cpp test/TestTrackVersion.h test/UnitTest.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h data/base.h data/Song.h data/Data.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h lib/base/set.h data/CrossFade.h data/audio/AudioBuffer.h Session.h
	$(CPP) -c test/TestTrackVersion.cpp -o $@ $(CXXFLAGS)

test/UnitTest.o : test/UnitTest.cpp test/UnitTest.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/msg.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h data/audio/AudioBuffer.h data/Range.h test/TestAudioBuffer.h test/TestRingBuffer.h test/TestInterpolator.h test/TestRhythm.h test/TestStreams.h test/TestThreads.h test/TestMidiPreview.h test/TestPlugins.h test/TestMixer.h test/TestTrackVersion.h test/TestPointer.h test/TestSignalChain.h
	$(CPP) -c test/UnitTest.cpp -o $@ $(CXXFLAGS)

view/audioview/graph/AudioViewLayer.o : view/audioview/graph/AudioViewLayer.cpp view/audioview/graph/AudioViewLayer.h view/helper/graph/Node.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/math/rect.h lib/math/math.h lib/math/vec2.h stuff/Observable.h lib/base/map.h lib/base/set.h view/audioview/graph/AudioViewTrack.h view/audioview/graph/LayerHeader.h view/audioview/AudioView.h data/SongSelection.h data/Range.h data/midi/Scale.h view/audioview/TrackHeightManager.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/hui/main.h lib/hui/Event.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h view/MouseDelayPlanner.h view/mode/ViewMode.h view/mode/ViewModeMidi.h view/mode/ViewModeDefault.h view/mode/ViewModeCurve.h view/mode/ViewModeEdit.h view/painter/BufferPainter.h view/painter/GridPainter.h view/painter/BasicGridPainter.h view/painter/MidiPainter.h view/helper/graph/ScrollBar.h view/helper/SymbolRenderer.h view/helper/Drawing.h Tsunami.h Session.h EditModes.h data/base.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h data/TrackMarker.h module/audio/AudioEffect.h module/Module.h module/port/Port.h data/SampleRef.h data/Sample.h data/audio/AudioBuffer.h data/CrossFade.h data/rhythm/Beat.h data/midi/Clef.h module/synthesizer/Synthesizer.h data/midi/Temperament.h module/audio/SongRenderer.h module/audio/AudioSource.h stuff/PerformanceMonitor.h lib/math/vector.h
	$(CPP) -c view/audioview/graph/AudioViewLayer.cpp -o $@ $(CXXFLAGS)

view/audioview/graph/AudioViewTrack.o : view/audioview/graph/AudioViewTrack.cpp view/audioview/graph/AudioViewTrack.h view/helper/graph/Node.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/math/rect.h lib/math/math.h lib/math/vec2.h stuff/Observable.h view/audioview/AudioView.h data/SongSelection.h lib/base/set.h data/Range.h data/midi/Scale.h view/audioview/TrackHeightManager.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/hui/main.h lib/hui/Event.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h view/mode/ViewMode.h view/mode/ViewModeMidi.h view/mode/ViewModeDefault.h Session.h EditModes.h data/base.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h data/audio/AudioBuffer.h data/CrossFade.h module/audio/SongRenderer.h module/audio/AudioSource.h module/Module.h module/port/Port.h view/audioview/graph/AudioViewLayer.h view/audioview/graph/TrackHeader.h view/helper/SymbolRenderer.h view/painter/BufferPainter.h
	$(CPP) -c view/audioview/graph/AudioViewTrack.cpp -o $@ $(CXXFLAGS)

view/audioview/graph/Background.o : view/audioview/graph/Background.cpp view/audioview/graph/Background.h view/helper/graph/Node.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/math/rect.h lib/math/math.h lib/math/vec2.h stuff/Observable.h view/audioview/AudioView.h data/SongSelection.h lib/base/set.h data/Range.h data/midi/Scale.h view/audioview/TrackHeightManager.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/hui/main.h lib/hui/Event.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h view/mode/ViewMode.h view/painter/GridPainter.h view/painter/BasicGridPainter.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h view/audioview/graph/AudioViewLayer.h lib/math/vector.h
	$(CPP) -c view/audioview/graph/Background.cpp -o $@ $(CXXFLAGS)

view/audioview/graph/Cursor.o : view/audioview/graph/Cursor.cpp view/audioview/graph/Cursor.h view/helper/graph/Node.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/math/rect.h lib/math/math.h lib/math/vec2.h stuff/Observable.h data/Range.h view/audioview/AudioView.h data/SongSelection.h lib/base/set.h data/midi/Scale.h view/audioview/TrackHeightManager.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/hui/main.h lib/hui/Event.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h data/base.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h data/TrackLayer.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h view/audioview/graph/AudioViewLayer.h lib/math/vector.h Session.h TsunamiWindow.h
	$(CPP) -c view/audioview/graph/Cursor.cpp -o $@ $(CXXFLAGS)

view/audioview/graph/LayerHeader.o : view/audioview/graph/LayerHeader.cpp view/audioview/graph/LayerHeader.h view/helper/graph/Node.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/math/rect.h lib/math/math.h lib/math/vec2.h stuff/Observable.h view/audioview/graph/AudioViewLayer.h lib/base/map.h lib/base/set.h view/audioview/graph/AudioViewTrack.h view/audioview/AudioView.h data/SongSelection.h data/Range.h data/midi/Scale.h view/audioview/TrackHeightManager.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/hui/main.h lib/hui/Event.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h view/helper/Drawing.h data/base.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h lib/math/vector.h
	$(CPP) -c view/audioview/graph/LayerHeader.cpp -o $@ $(CXXFLAGS)

view/audioview/graph/TimeScale.o : view/audioview/graph/TimeScale.cpp view/audioview/graph/TimeScale.h view/helper/graph/Node.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/math/rect.h lib/math/math.h lib/math/vec2.h stuff/Observable.h view/audioview/AudioView.h data/SongSelection.h lib/base/set.h data/Range.h data/midi/Scale.h view/audioview/TrackHeightManager.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/hui/main.h lib/hui/Event.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h view/painter/GridPainter.h view/painter/BasicGridPainter.h module/audio/SongRenderer.h module/audio/AudioSource.h module/Module.h module/port/Port.h lib/math/vector.h
	$(CPP) -c view/audioview/graph/TimeScale.cpp -o $@ $(CXXFLAGS)

view/audioview/graph/TrackHeader.o : view/audioview/graph/TrackHeader.cpp view/audioview/graph/TrackHeader.h view/helper/graph/Node.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/math/rect.h lib/math/math.h lib/math/vec2.h stuff/Observable.h view/helper/graph/SceneGraph.h view/HoverData.h data/Range.h view/helper/Drawing.h view/audioview/AudioView.h data/SongSelection.h lib/base/set.h data/midi/Scale.h view/audioview/TrackHeightManager.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/hui/main.h lib/hui/Event.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h view/audioview/ViewPort.h view/ColorScheme.h view/MouseDelayPlanner.h data/base.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h Session.h EditModes.h view/audioview/graph/AudioViewTrack.h lib/math/vector.h
	$(CPP) -c view/audioview/graph/TrackHeader.cpp -o $@ $(CXXFLAGS)

view/audioview/graph/ViewMarker.o : view/audioview/graph/ViewMarker.cpp view/audioview/graph/ViewMarker.h view/helper/graph/Node.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/math/rect.h lib/math/math.h lib/math/vec2.h stuff/Observable.h view/HoverData.h data/Range.h
	$(CPP) -c view/audioview/graph/ViewMarker.cpp -o $@ $(CXXFLAGS)

view/audioview/AudioView.o : view/audioview/AudioView.cpp view/audioview/AudioView.h lib/base/pointer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h data/SongSelection.h lib/base/set.h data/Range.h data/midi/Scale.h stuff/Observable.h view/audioview/TrackHeightManager.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/math/rect.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h view/audioview/graph/AudioViewLayer.h view/helper/graph/Node.h view/audioview/graph/AudioViewTrack.h view/audioview/graph/Background.h view/audioview/graph/Cursor.h view/audioview/graph/TimeScale.h view/MouseDelayPlanner.h view/helper/graph/SceneGraph.h view/helper/graph/ScrollBar.h view/helper/Dial.h view/helper/Drawing.h view/helper/PeakThread.h lib/threads/Thread.h view/helper/CpuDisplay.h view/helper/PeakMeterDisplay.h view/mode/ViewModeDefault.h view/mode/ViewMode.h view/mode/ViewModeEdit.h view/mode/ViewModeEditDummy.h view/mode/ViewModeEditAudio.h view/mode/ViewModeMidi.h view/mode/ViewModeCurve.h view/mode/ViewModeCapture.h view/mode/ViewModeScaleBars.h view/mode/ViewModeScaleMarker.h view/painter/BufferPainter.h view/painter/GridPainter.h view/painter/BasicGridPainter.h view/painter/MidiPainter.h view/sidebar/SideBar.h view/bottombar/BottomBar.h data/base.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h data/TrackMarker.h module/audio/AudioEffect.h module/Module.h module/port/Port.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h data/Sample.h data/audio/AudioBuffer.h data/rhythm/Beat.h data/SampleRef.h device/stream/AudioOutput.h data/audio/RingBuffer.h module/ModuleConfiguration.h module/audio/SongRenderer.h module/audio/AudioSource.h module/synthesizer/Synthesizer.h data/midi/Temperament.h module/audio/PeakMeter.h module/audio/AudioVisualizer.h module/SignalChain.h plugins/TsunamiPlugin.h lib/threads/Mutex.h stuff/PerformanceMonitor.h Session.h EditModes.h Tsunami.h TsunamiWindow.h
	$(CPP) -c view/audioview/AudioView.cpp -o $@ $(CXXFLAGS)

view/audioview/TrackHeightManager.o : view/audioview/TrackHeightManager.cpp view/audioview/TrackHeightManager.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/math/rect.h view/audioview/AudioView.h data/SongSelection.h data/Range.h data/midi/Scale.h stuff/Observable.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h view/mode/ViewMode.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h view/audioview/graph/AudioViewLayer.h view/helper/graph/Node.h view/audioview/graph/AudioViewTrack.h view/helper/graph/ScrollBar.h
	$(CPP) -c view/audioview/TrackHeightManager.cpp -o $@ $(CXXFLAGS)

view/audioview/ViewPort.o : view/audioview/ViewPort.cpp view/audioview/ViewPort.h lib/math/rect.h lib/math/math.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h view/audioview/AudioView.h lib/base/pointer.h data/SongSelection.h lib/base/set.h data/Range.h data/midi/Scale.h stuff/Observable.h view/audioview/TrackHeightManager.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h view/ColorScheme.h view/HoverData.h
	$(CPP) -c view/audioview/ViewPort.cpp -o $@ $(CXXFLAGS)

view/bottombar/signaleditor/SignalEditorBackground.o : view/bottombar/signaleditor/SignalEditorBackground.cpp view/bottombar/signaleditor/SignalEditorBackground.h view/helper/graph/Node.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/math/rect.h lib/math/math.h lib/math/vec2.h stuff/Observable.h view/helper/graph/Scrollable.h view/bottombar/signaleditor/SignalEditorTab.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h view/audioview/AudioView.h data/SongSelection.h data/Range.h data/midi/Scale.h view/audioview/TrackHeightManager.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h
	$(CPP) -c view/bottombar/signaleditor/SignalEditorBackground.cpp -o $@ $(CXXFLAGS)

view/bottombar/signaleditor/SignalEditorCable.o : view/bottombar/signaleditor/SignalEditorCable.cpp view/bottombar/signaleditor/SignalEditorCable.h view/helper/graph/Node.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/math/rect.h lib/math/math.h lib/math/vec2.h stuff/Observable.h view/bottombar/signaleditor/SignalEditorTab.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h view/bottombar/signaleditor/SignalEditorModule.h view/bottombar/signaleditor/SignalEditorPort.h module/SignalChain.h module/Module.h view/audioview/AudioView.h data/SongSelection.h data/Range.h data/midi/Scale.h view/audioview/TrackHeightManager.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h lib/math/vector.h lib/math/interpolation.h
	$(CPP) -c view/bottombar/signaleditor/SignalEditorCable.cpp -o $@ $(CXXFLAGS)

view/bottombar/signaleditor/SignalEditorModule.o : view/bottombar/signaleditor/SignalEditorModule.cpp view/bottombar/signaleditor/SignalEditorModule.h view/helper/graph/Node.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/math/rect.h lib/math/math.h lib/math/vec2.h stuff/Observable.h view/bottombar/signaleditor/SignalEditorTab.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h view/bottombar/signaleditor/SignalEditorPort.h module/Module.h module/port/Port.h view/audioview/AudioView.h data/SongSelection.h data/Range.h data/midi/Scale.h view/audioview/TrackHeightManager.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h view/MouseDelayPlanner.h view/helper/Drawing.h view/helper/graph/SceneGraph.h
	$(CPP) -c view/bottombar/signaleditor/SignalEditorModule.cpp -o $@ $(CXXFLAGS)

view/bottombar/signaleditor/SignalEditorPort.o : view/bottombar/signaleditor/SignalEditorPort.cpp view/bottombar/signaleditor/SignalEditorPort.h view/helper/graph/Node.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/math/rect.h lib/math/math.h lib/math/vec2.h stuff/Observable.h view/bottombar/signaleditor/SignalEditorTab.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h data/base.h module/SignalChain.h module/Module.h lib/math/vector.h view/MouseDelayPlanner.h view/helper/graph/SceneGraph.h view/HoverData.h data/Range.h view/audioview/AudioView.h data/SongSelection.h data/midi/Scale.h view/audioview/TrackHeightManager.h view/audioview/ViewPort.h view/ColorScheme.h
	$(CPP) -c view/bottombar/signaleditor/SignalEditorPort.cpp -o $@ $(CXXFLAGS)

view/bottombar/signaleditor/SignalEditorTab.o : view/bottombar/signaleditor/SignalEditorTab.cpp view/bottombar/signaleditor/SignalEditorTab.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h view/helper/graph/Node.h lib/math/rect.h stuff/Observable.h view/bottombar/signaleditor/SignalEditorBackground.h view/helper/graph/Scrollable.h view/bottombar/signaleditor/SignalEditorModule.h view/bottombar/signaleditor/SignalEditorCable.h view/bottombar/SignalEditor.h view/bottombar/BottomBar.h view/audioview/AudioView.h data/SongSelection.h data/Range.h data/midi/Scale.h view/audioview/TrackHeightManager.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h module/port/Port.h module/Module.h module/SignalChain.h view/helper/graph/SceneGraph.h view/helper/graph/ScrollBar.h view/helper/Drawing.h view/MouseDelayPlanner.h data/base.h Session.h storage/Storage.h plugins/PluginManager.h lib/kaba/kaba.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h
	$(CPP) -c view/bottombar/signaleditor/SignalEditorTab.cpp -o $@ $(CXXFLAGS)

view/bottombar/BottomBar.o : view/bottombar/BottomBar.cpp view/bottombar/BottomBar.h lib/base/pointer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h stuff/Observable.h view/bottombar/MixingConsole.h view/bottombar/LogConsole.h view/bottombar/SignalEditor.h view/bottombar/PluginConsole.h view/audioview/AudioView.h data/SongSelection.h data/Range.h data/midi/Scale.h view/audioview/TrackHeightManager.h lib/math/rect.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h view/bottombar/MiniBar.h Session.h
	$(CPP) -c view/bottombar/BottomBar.cpp -o $@ $(CXXFLAGS)

view/bottombar/DeviceConsole.o : view/bottombar/DeviceConsole.cpp view/bottombar/DeviceConsole.h view/bottombar/BottomBar.h lib/base/pointer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h stuff/Observable.h Session.h device/Device.h device/DeviceManager.h
	$(CPP) -c view/bottombar/DeviceConsole.cpp -o $@ $(CXXFLAGS)

view/bottombar/LogConsole.o : view/bottombar/LogConsole.cpp view/bottombar/BottomBar.h lib/base/pointer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h stuff/Observable.h Session.h stuff/Log.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h view/bottombar/LogConsole.h
	$(CPP) -c view/bottombar/LogConsole.cpp -o $@ $(CXXFLAGS)

view/bottombar/MiniBar.o : view/bottombar/MiniBar.cpp view/bottombar/MiniBar.h lib/base/pointer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h view/bottombar/BottomBar.h stuff/Observable.h view/audioview/AudioView.h data/SongSelection.h data/Range.h data/midi/Scale.h view/audioview/TrackHeightManager.h lib/math/rect.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h view/helper/PeakMeterDisplay.h view/helper/graph/Node.h Session.h device/DeviceManager.h device/stream/AudioOutput.h data/audio/RingBuffer.h data/audio/AudioBuffer.h module/Module.h module/ModuleConfiguration.h
	$(CPP) -c view/bottombar/MiniBar.cpp -o $@ $(CXXFLAGS)

view/bottombar/MixingConsole.o : view/bottombar/MixingConsole.cpp view/bottombar/MixingConsole.h view/bottombar/BottomBar.h lib/base/pointer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h stuff/Observable.h view/helper/PeakMeterDisplay.h view/helper/graph/Node.h lib/math/rect.h view/helper/ModulePanel.h view/helper/FxListEditor.h view/helper/Drawing.h view/audioview/AudioView.h data/SongSelection.h data/Range.h data/midi/Scale.h view/audioview/TrackHeightManager.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h view/audioview/graph/AudioViewTrack.h view/audioview/graph/TrackHeader.h data/base.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h module/audio/AudioEffect.h module/Module.h module/port/Port.h module/audio/SongRenderer.h module/audio/AudioSource.h module/ConfigPanel.h module/SignalChain.h Session.h EditModes.h plugins/PluginManager.h lib/kaba/kaba.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h device/DeviceManager.h device/stream/AudioOutput.h data/audio/RingBuffer.h data/audio/AudioBuffer.h module/ModuleConfiguration.h lib/math/vector.h
	$(CPP) -c view/bottombar/MixingConsole.cpp -o $@ $(CXXFLAGS)

view/bottombar/PluginConsole.o : view/bottombar/PluginConsole.cpp view/bottombar/PluginConsole.h view/bottombar/BottomBar.h lib/base/pointer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h stuff/Observable.h Session.h plugins/TsunamiPlugin.h module/Module.h plugins/PluginManager.h lib/kaba/kaba.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h view/helper/ModulePanel.h
	$(CPP) -c view/bottombar/PluginConsole.cpp -o $@ $(CXXFLAGS)

view/bottombar/SignalEditor.o : view/bottombar/SignalEditor.cpp view/bottombar/SignalEditor.h view/bottombar/BottomBar.h lib/base/pointer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h stuff/Observable.h view/helper/ModulePanel.h view/bottombar/signaleditor/SignalEditorTab.h view/helper/graph/Node.h lib/math/rect.h module/port/Port.h module/Module.h module/ConfigPanel.h module/SignalChain.h Session.h storage/Storage.h data/base.h TsunamiWindow.h
	$(CPP) -c view/bottombar/SignalEditor.cpp -o $@ $(CXXFLAGS)

view/dialog/BarAddDialog.o : view/dialog/BarAddDialog.cpp view/dialog/BarAddDialog.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h data/Range.h data/rhythm/Bar.h view/audioview/AudioView.h data/SongSelection.h data/midi/Scale.h stuff/Observable.h view/audioview/TrackHeightManager.h lib/math/rect.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h data/base.h data/Song.h data/Data.h data/rhythm/BarCollection.h lib/any/any.h
	$(CPP) -c view/dialog/BarAddDialog.cpp -o $@ $(CXXFLAGS)

view/dialog/BarDeleteDialog.o : view/dialog/BarDeleteDialog.cpp view/dialog/BarDeleteDialog.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h view/audioview/AudioView.h data/SongSelection.h data/Range.h data/midi/Scale.h stuff/Observable.h view/audioview/TrackHeightManager.h lib/math/rect.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h data/base.h data/rhythm/Bar.h data/Song.h data/Data.h data/rhythm/BarCollection.h lib/any/any.h
	$(CPP) -c view/dialog/BarDeleteDialog.cpp -o $@ $(CXXFLAGS)

view/dialog/BarEditDialog.o : view/dialog/BarEditDialog.cpp view/dialog/BarEditDialog.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h data/rhythm/Bar.h data/Range.h data/Song.h data/Data.h stuff/Observable.h data/rhythm/BarCollection.h lib/any/any.h
	$(CPP) -c view/dialog/BarEditDialog.cpp -o $@ $(CXXFLAGS)

view/dialog/BufferCompressionDialog.o : view/dialog/BufferCompressionDialog.cpp view/dialog/BufferCompressionDialog.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h data/rhythm/Bar.h data/Range.h
	$(CPP) -c view/dialog/BufferCompressionDialog.cpp -o $@ $(CXXFLAGS)

view/dialog/ChannelMapperDialog.o : view/dialog/ChannelMapperDialog.cpp view/dialog/ChannelMapperDialog.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h module/audio/AudioChannelSelector.h module/port/Port.h module/Module.h stuff/Observable.h module/ModuleConfiguration.h module/audio/PeakMeter.h module/audio/AudioVisualizer.h module/ConfigPanel.h
	$(CPP) -c view/dialog/ChannelMapperDialog.cpp -o $@ $(CXXFLAGS)

view/dialog/HelpDialog.o : view/dialog/HelpDialog.cpp view/dialog/HelpDialog.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c view/dialog/HelpDialog.cpp -o $@ $(CXXFLAGS)

view/dialog/MarkerDialog.o : view/dialog/MarkerDialog.cpp view/dialog/MarkerDialog.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h data/Range.h data/TrackLayer.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h stuff/Observable.h data/TrackMarker.h module/audio/AudioEffect.h module/Module.h module/port/Port.h data/midi/Scale.h
	$(CPP) -c view/dialog/MarkerDialog.cpp -o $@ $(CXXFLAGS)

view/dialog/ModuleSelectorDialog.o : view/dialog/ModuleSelectorDialog.cpp view/dialog/ModuleSelectorDialog.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h Session.h stuff/Observable.h plugins/PluginManager.h lib/kaba/kaba.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h module/Module.h
	$(CPP) -c view/dialog/ModuleSelectorDialog.cpp -o $@ $(CXXFLAGS)

view/dialog/NewDialog.o : view/dialog/NewDialog.cpp view/dialog/NewDialog.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h data/rhythm/Bar.h data/Range.h data/base.h data/Song.h data/Data.h stuff/Observable.h data/rhythm/BarCollection.h lib/any/any.h action/ActionManager.h action/Action.h Tsunami.h TsunamiWindow.h Session.h
	$(CPP) -c view/dialog/NewDialog.cpp -o $@ $(CXXFLAGS)

view/dialog/PauseAddDialog.o : view/dialog/PauseAddDialog.cpp view/dialog/PauseAddDialog.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h data/Range.h view/audioview/AudioView.h data/SongSelection.h data/midi/Scale.h stuff/Observable.h view/audioview/TrackHeightManager.h lib/math/rect.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h data/rhythm/Bar.h data/Song.h data/Data.h data/rhythm/BarCollection.h lib/any/any.h data/base.h
	$(CPP) -c view/dialog/PauseAddDialog.cpp -o $@ $(CXXFLAGS)

view/dialog/PauseEditDialog.o : view/dialog/PauseEditDialog.cpp view/dialog/PauseEditDialog.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h data/rhythm/Bar.h data/Range.h data/Song.h data/Data.h stuff/Observable.h data/rhythm/BarCollection.h lib/any/any.h
	$(CPP) -c view/dialog/PauseEditDialog.cpp -o $@ $(CXXFLAGS)

view/dialog/QuestionDialog.o : view/dialog/QuestionDialog.cpp view/dialog/QuestionDialog.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c view/dialog/QuestionDialog.cpp -o $@ $(CXXFLAGS)

view/dialog/SampleScaleDialog.o : view/dialog/SampleScaleDialog.cpp view/dialog/SampleScaleDialog.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h data/Song.h data/Data.h stuff/Observable.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h data/Sample.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h lib/math/rect.h data/audio/AudioBuffer.h data/audio/BufferInterpolator.h
	$(CPP) -c view/dialog/SampleScaleDialog.cpp -o $@ $(CXXFLAGS)

view/dialog/SettingsDialog.o : view/dialog/SettingsDialog.cpp view/dialog/SettingsDialog.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h view/bottombar/DeviceConsole.h view/bottombar/BottomBar.h stuff/Observable.h view/bottombar/MiniBar.h view/helper/CpuDisplay.h view/helper/graph/Node.h lib/math/rect.h view/helper/Slider.h lib/base/callable.h view/audioview/AudioView.h data/SongSelection.h data/Range.h data/midi/Scale.h view/audioview/TrackHeightManager.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h device/DeviceManager.h device/stream/AudioOutput.h data/audio/RingBuffer.h data/audio/AudioBuffer.h module/Module.h module/ModuleConfiguration.h module/SignalChain.h stuff/Log.h Session.h Tsunami.h TsunamiWindow.h
	$(CPP) -c view/dialog/SettingsDialog.cpp -o $@ $(CXXFLAGS)

view/dialog/TemperamentDialog.o : view/dialog/TemperamentDialog.cpp view/dialog/TemperamentDialog.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h data/midi/Temperament.h data/midi/MidiData.h data/Range.h data/midi/MidiNote.h data/midi/MidiEvent.h view/audioview/AudioView.h data/SongSelection.h data/midi/Scale.h stuff/Observable.h view/audioview/TrackHeightManager.h lib/math/rect.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h data/Track.h data/midi/Instrument.h module/synthesizer/Synthesizer.h module/Module.h module/port/Port.h lib/math/vector.h Tsunami.h
	$(CPP) -c view/dialog/TemperamentDialog.cpp -o $@ $(CXXFLAGS)

view/dialog/TimeTrackAddDialog.o : view/dialog/TimeTrackAddDialog.cpp view/dialog/TimeTrackAddDialog.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h data/rhythm/Bar.h data/Range.h data/base.h data/Song.h data/Data.h stuff/Observable.h data/rhythm/BarCollection.h lib/any/any.h action/ActionManager.h action/Action.h Tsunami.h TsunamiWindow.h Session.h
	$(CPP) -c view/dialog/TimeTrackAddDialog.cpp -o $@ $(CXXFLAGS)

view/dialog/TrackRoutingDialog.o : view/dialog/TrackRoutingDialog.cpp view/dialog/TrackRoutingDialog.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h data/base.h data/Song.h data/Data.h stuff/Observable.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h
	$(CPP) -c view/dialog/TrackRoutingDialog.cpp -o $@ $(CXXFLAGS)

view/dialog/TuningDialog.o : view/dialog/TuningDialog.cpp view/dialog/TuningDialog.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h data/Track.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h stuff/Observable.h
	$(CPP) -c view/dialog/TuningDialog.cpp -o $@ $(CXXFLAGS)

view/helper/graph/Node.o : view/helper/graph/Node.cpp view/helper/graph/Node.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/math/rect.h lib/math/math.h lib/math/vec2.h stuff/Observable.h view/helper/graph/SceneGraph.h view/HoverData.h data/Range.h stuff/PerformanceMonitor.h lib/image/Painter.h lib/image/color.h lib/math/vector.h
	$(CPP) -c view/helper/graph/Node.cpp -o $@ $(CXXFLAGS)

view/helper/graph/SceneGraph.o : view/helper/graph/SceneGraph.cpp view/helper/graph/SceneGraph.h view/helper/graph/Node.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/math/rect.h lib/math/math.h lib/math/vec2.h stuff/Observable.h view/HoverData.h data/Range.h view/MouseDelayPlanner.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c view/helper/graph/SceneGraph.cpp -o $@ $(CXXFLAGS)

view/helper/graph/Scrollable.o : view/helper/graph/Scrollable.cpp view/helper/graph/Scrollable.h view/helper/graph/Node.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/math/rect.h lib/math/math.h lib/math/vec2.h stuff/Observable.h view/helper/graph/ScrollBar.h lib/hui/Event.h lib/hui/Callback.h
	$(CPP) -c view/helper/graph/Scrollable.cpp -o $@ $(CXXFLAGS)

view/helper/graph/ScrollBar.o : view/helper/graph/ScrollBar.cpp view/helper/graph/ScrollBar.h view/helper/graph/Node.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/math/rect.h lib/math/math.h lib/math/vec2.h stuff/Observable.h view/helper/graph/SceneGraph.h view/HoverData.h data/Range.h view/ColorScheme.h lib/image/color.h lib/image/Painter.h
	$(CPP) -c view/helper/graph/ScrollBar.cpp -o $@ $(CXXFLAGS)

view/helper/CpuDisplay.o : view/helper/CpuDisplay.cpp view/helper/CpuDisplay.h stuff/Observable.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h view/helper/graph/Node.h lib/base/pointer.h lib/math/rect.h lib/math/math.h lib/math/vec2.h view/helper/graph/SceneGraph.h view/HoverData.h data/Range.h view/MouseDelayPlanner.h stuff/PerformanceMonitor.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h Session.h module/Module.h module/SignalChain.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h TsunamiWindow.h view/audioview/graph/AudioViewLayer.h view/audioview/graph/AudioViewTrack.h view/ColorScheme.h view/painter/BasicGridPainter.h lib/math/vector.h
	$(CPP) -c view/helper/CpuDisplay.cpp -o $@ $(CXXFLAGS)

view/helper/Dial.o : view/helper/Dial.cpp view/helper/Dial.h view/helper/graph/Node.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/math/rect.h lib/math/math.h lib/math/vec2.h stuff/Observable.h view/helper/graph/SceneGraph.h view/HoverData.h data/Range.h view/ColorScheme.h lib/image/color.h lib/math/complex.h lib/math/vector.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c view/helper/Dial.cpp -o $@ $(CXXFLAGS)

view/helper/Drawing.o : view/helper/Drawing.cpp view/helper/Drawing.h view/ColorScheme.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/image/Painter.h lib/math/rect.h lib/math/math.h lib/math/vec2.h
	$(CPP) -c view/helper/Drawing.cpp -o $@ $(CXXFLAGS)

view/helper/FxListEditor.o : view/helper/FxListEditor.cpp view/helper/FxListEditor.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h view/helper/ModulePanel.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h stuff/Observable.h data/base.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h module/audio/AudioEffect.h module/Module.h module/port/Port.h module/midi/MidiEffect.h module/ConfigPanel.h Session.h plugins/PluginManager.h lib/kaba/kaba.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h
	$(CPP) -c view/helper/FxListEditor.cpp -o $@ $(CXXFLAGS)

view/helper/MidiPreview.o : view/helper/MidiPreview.cpp view/helper/MidiPreview.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h module/SignalChain.h module/Module.h stuff/Observable.h module/synthesizer/Synthesizer.h lib/base/set.h data/midi/MidiData.h data/Range.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/midi/Temperament.h module/port/Port.h module/midi/MidiPreviewSource.h module/midi/MidiSource.h device/stream/MidiInput.h module/ModuleConfiguration.h Session.h
	$(CPP) -c view/helper/MidiPreview.cpp -o $@ $(CXXFLAGS)

view/helper/ModulePanel.o : view/helper/ModulePanel.cpp view/helper/ModulePanel.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h stuff/Observable.h module/Module.h module/ConfigPanel.h plugins/PluginManager.h lib/kaba/kaba.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h Session.h TsunamiWindow.h lib/hui/Controls/ControlMenuButton.h lib/hui/Controls/Control.h
	$(CPP) -c view/helper/ModulePanel.cpp -o $@ $(CXXFLAGS)

view/helper/PeakMeterDisplay.o : view/helper/PeakMeterDisplay.cpp view/helper/PeakMeterDisplay.h stuff/Observable.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h view/helper/graph/Node.h lib/base/pointer.h lib/math/rect.h lib/math/math.h lib/math/vec2.h view/ColorScheme.h lib/image/color.h Tsunami.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h module/audio/PeakMeter.h module/audio/AudioVisualizer.h module/Module.h module/port/Port.h
	$(CPP) -c view/helper/PeakMeterDisplay.cpp -o $@ $(CXXFLAGS)

view/helper/PeakThread.o : view/helper/PeakThread.cpp view/helper/PeakThread.h lib/threads/Thread.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h view/audioview/AudioView.h lib/base/pointer.h data/SongSelection.h lib/base/set.h data/Range.h data/midi/Scale.h stuff/Observable.h view/audioview/TrackHeightManager.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/math/rect.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h data/Sample.h data/audio/AudioBuffer.h stuff/PerformanceMonitor.h
	$(CPP) -c view/helper/PeakThread.cpp -o $@ $(CXXFLAGS)

view/helper/Progress.o : view/helper/Progress.cpp view/helper/Progress.h lib/base/pointer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h stuff/Observable.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c view/helper/Progress.cpp -o $@ $(CXXFLAGS)

view/helper/Slider.o : view/helper/Slider.cpp view/helper/Slider.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/base/callable.h
	$(CPP) -c view/helper/Slider.cpp -o $@ $(CXXFLAGS)

view/helper/SymbolRenderer.o : view/helper/SymbolRenderer.cpp view/helper/SymbolRenderer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/image.h lib/config.h lib/image/color.h lib/image/Painter.h lib/hui/hui.h lib/hui/common.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/base/pointer.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/math/vector.h
	$(CPP) -c view/helper/SymbolRenderer.cpp -o $@ $(CXXFLAGS)

view/mode/ViewMode.o : view/mode/ViewMode.cpp view/mode/ViewMode.h stuff/Observable.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/set.h view/audioview/AudioView.h lib/base/pointer.h data/SongSelection.h data/Range.h data/midi/Scale.h view/audioview/TrackHeightManager.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/math/rect.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h view/sidebar/SideBar.h TsunamiWindow.h
	$(CPP) -c view/mode/ViewMode.cpp -o $@ $(CXXFLAGS)

view/mode/ViewModeCapture.o : view/mode/ViewModeCapture.cpp view/mode/ViewModeCapture.h view/mode/ViewModeDefault.h view/mode/ViewMode.h stuff/Observable.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/set.h view/sidebar/captureconsolemodes/CaptureTrackData.h view/audioview/AudioView.h lib/base/pointer.h data/SongSelection.h data/Range.h data/midi/Scale.h view/audioview/TrackHeightManager.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/math/rect.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h view/audioview/graph/AudioViewLayer.h view/helper/graph/Node.h view/painter/BufferPainter.h view/sidebar/SideBar.h data/base.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h data/TrackLayer.h module/SignalChain.h module/Module.h module/audio/AudioAccumulator.h data/audio/AudioBuffer.h module/ModuleConfiguration.h module/port/Port.h module/midi/MidiAccumulator.h Session.h
	$(CPP) -c view/mode/ViewModeCapture.cpp -o $@ $(CXXFLAGS)

view/mode/ViewModeCurve.o : view/mode/ViewModeCurve.cpp view/mode/ViewModeCurve.h view/mode/ViewModeDefault.h view/mode/ViewMode.h stuff/Observable.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/set.h view/audioview/AudioView.h lib/base/pointer.h data/SongSelection.h data/Range.h data/midi/Scale.h view/audioview/TrackHeightManager.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/math/rect.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h view/audioview/graph/AudioViewLayer.h view/helper/graph/Node.h view/audioview/graph/AudioViewTrack.h view/sidebar/SideBar.h data/Curve.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h device/stream/AudioOutput.h data/audio/RingBuffer.h data/audio/AudioBuffer.h module/Module.h module/ModuleConfiguration.h module/audio/SongRenderer.h module/audio/AudioSource.h module/port/Port.h TsunamiWindow.h lib/math/vector.h
	$(CPP) -c view/mode/ViewModeCurve.cpp -o $@ $(CXXFLAGS)

view/mode/ViewModeDefault.o : view/mode/ViewModeDefault.cpp view/mode/ViewModeDefault.h view/mode/ViewMode.h stuff/Observable.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/set.h view/audioview/AudioView.h lib/base/pointer.h data/SongSelection.h data/Range.h data/midi/Scale.h view/audioview/TrackHeightManager.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/math/rect.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h view/helper/graph/SceneGraph.h view/helper/graph/Node.h view/painter/BufferPainter.h view/painter/GridPainter.h view/painter/BasicGridPainter.h view/painter/MidiPainter.h view/audioview/graph/AudioViewLayer.h view/audioview/graph/AudioViewTrack.h view/helper/graph/ScrollBar.h view/MouseDelayPlanner.h module/audio/SongRenderer.h module/audio/AudioSource.h module/Module.h module/port/Port.h module/SignalChain.h data/base.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h data/rhythm/Beat.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h data/TrackMarker.h module/audio/AudioEffect.h data/CrossFade.h data/Sample.h data/audio/AudioBuffer.h data/SampleRef.h lib/hui/Controls/Control.h lib/math/vector.h action/song/ActionSongMoveSelection.h action/Action.h device/stream/AudioOutput.h data/audio/RingBuffer.h module/ModuleConfiguration.h TsunamiWindow.h Session.h
	$(CPP) -c view/mode/ViewModeDefault.cpp -o $@ $(CXXFLAGS)

view/mode/ViewModeEdit.o : view/mode/ViewModeEdit.cpp view/mode/ViewModeEdit.h view/mode/ViewModeDefault.h view/mode/ViewMode.h stuff/Observable.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/set.h view/mode/ViewModeEditDummy.h view/mode/ViewModeMidi.h lib/base/pointer.h lib/math/math.h view/mode/ViewModeEditAudio.h view/audioview/AudioView.h data/SongSelection.h data/Range.h data/midi/Scale.h view/audioview/TrackHeightManager.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/math/rect.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h view/audioview/graph/AudioViewLayer.h view/helper/graph/Node.h data/base.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h
	$(CPP) -c view/mode/ViewModeEdit.cpp -o $@ $(CXXFLAGS)

view/mode/ViewModeEditAudio.o : view/mode/ViewModeEditAudio.cpp view/mode/ViewModeEditAudio.h view/mode/ViewModeDefault.h view/mode/ViewMode.h stuff/Observable.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/set.h view/sidebar/AudioEditorConsole.h view/sidebar/SideBar.h lib/base/pointer.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h view/audioview/AudioView.h data/SongSelection.h data/Range.h data/midi/Scale.h view/audioview/TrackHeightManager.h lib/math/rect.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h view/audioview/graph/AudioViewLayer.h view/helper/graph/Node.h view/MouseDelayPlanner.h data/base.h data/TrackLayer.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/audio/AudioBuffer.h data/audio/BufferInterpolator.h plugins/FastFourierTransform.h lib/math/complex.h
	$(CPP) -c view/mode/ViewModeEditAudio.cpp -o $@ $(CXXFLAGS)

view/mode/ViewModeEditDummy.o : view/mode/ViewModeEditDummy.cpp view/mode/ViewModeEditDummy.h view/mode/ViewModeDefault.h view/mode/ViewMode.h stuff/Observable.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/set.h view/sidebar/SideBar.h lib/base/pointer.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h
	$(CPP) -c view/mode/ViewModeEditDummy.cpp -o $@ $(CXXFLAGS)

view/mode/ViewModeMidi.o : view/mode/ViewModeMidi.cpp view/mode/ViewModeMidi.h view/mode/ViewModeDefault.h view/mode/ViewMode.h stuff/Observable.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/set.h lib/base/pointer.h lib/math/math.h view/mode/ViewModeEdit.h view/audioview/AudioView.h data/SongSelection.h data/Range.h data/midi/Scale.h view/audioview/TrackHeightManager.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/math/rect.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h view/audioview/graph/AudioViewLayer.h view/helper/graph/Node.h view/helper/graph/ScrollBar.h view/MouseDelayPlanner.h view/helper/MidiPreview.h module/SignalChain.h module/Module.h view/painter/GridPainter.h view/painter/BasicGridPainter.h view/painter/MidiPainter.h view/sidebar/SideBar.h module/synthesizer/Synthesizer.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/midi/Temperament.h module/port/Port.h data/base.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h data/Track.h data/TrackLayer.h data/Sample.h data/audio/AudioBuffer.h data/midi/Clef.h device/Device.h device/DeviceManager.h device/stream/MidiInput.h module/ModuleConfiguration.h module/midi/MidiAccumulator.h TsunamiWindow.h Session.h
	$(CPP) -c view/mode/ViewModeMidi.cpp -o $@ $(CXXFLAGS)

view/mode/ViewModeScaleBars.o : view/mode/ViewModeScaleBars.cpp view/mode/ViewModeScaleBars.h view/mode/ViewModeDefault.h view/mode/ViewMode.h stuff/Observable.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/set.h data/Range.h view/audioview/AudioView.h lib/base/pointer.h data/SongSelection.h data/midi/Scale.h view/audioview/TrackHeightManager.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/math/rect.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h Session.h EditModes.h
	$(CPP) -c view/mode/ViewModeScaleBars.cpp -o $@ $(CXXFLAGS)

view/mode/ViewModeScaleMarker.o : view/mode/ViewModeScaleMarker.cpp view/mode/ViewModeScaleMarker.h view/mode/ViewModeDefault.h view/mode/ViewMode.h stuff/Observable.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/set.h data/Range.h view/audioview/AudioView.h lib/base/pointer.h data/SongSelection.h data/midi/Scale.h view/audioview/TrackHeightManager.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h lib/math/rect.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h Session.h EditModes.h data/TrackMarker.h module/audio/AudioEffect.h module/Module.h module/port/Port.h data/TrackLayer.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h
	$(CPP) -c view/mode/ViewModeScaleMarker.cpp -o $@ $(CXXFLAGS)

view/painter/BasicGridPainter.o : view/painter/BasicGridPainter.cpp view/painter/BasicGridPainter.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/math/rect.h lib/math/math.h lib/image/color.h lib/image/Painter.h lib/math/vec2.h
	$(CPP) -c view/painter/BasicGridPainter.cpp -o $@ $(CXXFLAGS)

view/painter/BufferPainter.o : view/painter/BufferPainter.cpp view/painter/BufferPainter.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/math/rect.h lib/math/math.h lib/image/color.h view/audioview/AudioView.h lib/base/pointer.h data/SongSelection.h lib/base/set.h data/Range.h data/midi/Scale.h stuff/Observable.h view/audioview/TrackHeightManager.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h view/audioview/graph/AudioViewTrack.h view/helper/graph/Node.h data/audio/AudioBuffer.h lib/math/complex.h plugins/FastFourierTransform.h Session.h
	$(CPP) -c view/painter/BufferPainter.cpp -o $@ $(CXXFLAGS)

view/painter/GridPainter.o : view/painter/GridPainter.cpp view/painter/GridPainter.h view/painter/BasicGridPainter.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h lib/math/rect.h lib/math/math.h lib/image/color.h view/audioview/graph/AudioViewTrack.h view/helper/graph/Node.h lib/math/vec2.h stuff/Observable.h view/audioview/AudioView.h data/SongSelection.h lib/base/set.h data/Range.h data/midi/Scale.h view/audioview/TrackHeightManager.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/hui/main.h lib/hui/Event.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h view/helper/Drawing.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h data/rhythm/Beat.h
	$(CPP) -c view/painter/GridPainter.cpp -o $@ $(CXXFLAGS)

view/painter/MidiPainter.o : view/painter/MidiPainter.cpp view/painter/MidiPainter.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/math/rect.h lib/math/math.h data/Range.h data/midi/Scale.h view/audioview/AudioView.h lib/base/pointer.h data/SongSelection.h lib/base/set.h stuff/Observable.h view/audioview/TrackHeightManager.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h view/audioview/graph/AudioViewLayer.h view/helper/graph/Node.h view/helper/SymbolRenderer.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Clef.h data/midi/Instrument.h data/SampleRef.h data/Sample.h data/audio/AudioBuffer.h module/synthesizer/Synthesizer.h data/midi/Temperament.h module/Module.h module/port/Port.h module/ModuleConfiguration.h
	$(CPP) -c view/painter/MidiPainter.cpp -o $@ $(CXXFLAGS)

view/painter/MultiLinePainter.o : view/painter/MultiLinePainter.cpp view/painter/MultiLinePainter.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h view/painter/MidiPainter.h lib/math/rect.h lib/math/math.h data/Range.h data/midi/Scale.h view/audioview/ViewPort.h view/ColorScheme.h lib/image/color.h view/HoverData.h data/base.h data/Song.h data/Data.h lib/base/pointer.h lib/file/path.h stuff/Observable.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h lib/base/set.h data/TrackMarker.h module/audio/AudioEffect.h module/Module.h module/port/Port.h data/rhythm/Beat.h data/midi/Clef.h data/SongSelection.h lib/image/Painter.h lib/math/vec2.h
	$(CPP) -c view/painter/MultiLinePainter.cpp -o $@ $(CXXFLAGS)

view/sidebar/captureconsolemodes/CaptureConsoleMode.o : view/sidebar/captureconsolemodes/CaptureConsoleMode.cpp view/sidebar/captureconsolemodes/CaptureConsoleMode.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h view/sidebar/captureconsolemodes/CaptureTrackData.h view/sidebar/CaptureConsole.h view/sidebar/SideBar.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h stuff/Observable.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h view/helper/PeakMeterDisplay.h view/helper/graph/Node.h lib/math/rect.h view/audioview/AudioView.h data/SongSelection.h data/midi/Scale.h view/audioview/TrackHeightManager.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h view/mode/ViewModeCapture.h view/mode/ViewModeDefault.h view/mode/ViewMode.h view/dialog/ChannelMapperDialog.h module/SignalChain.h module/Module.h module/audio/AudioChannelSelector.h module/port/Port.h module/ModuleConfiguration.h module/audio/PeakMeter.h module/audio/AudioVisualizer.h data/base.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h Session.h device/DeviceManager.h device/Device.h
	$(CPP) -c view/sidebar/captureconsolemodes/CaptureConsoleMode.cpp -o $@ $(CXXFLAGS)

view/sidebar/captureconsolemodes/CaptureConsoleModeAudio.o : view/sidebar/captureconsolemodes/CaptureConsoleModeAudio.cpp view/sidebar/captureconsolemodes/CaptureConsoleModeAudio.h view/sidebar/captureconsolemodes/CaptureConsoleMode.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h view/sidebar/captureconsolemodes/CaptureTrackData.h view/sidebar/CaptureConsole.h view/sidebar/SideBar.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h stuff/Observable.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h view/helper/PeakMeterDisplay.h view/helper/graph/Node.h lib/math/rect.h view/audioview/AudioView.h data/SongSelection.h data/midi/Scale.h view/audioview/TrackHeightManager.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h view/mode/ViewModeCapture.h view/mode/ViewModeDefault.h view/mode/ViewMode.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/base.h Session.h module/SignalChain.h module/Module.h device/Device.h module/audio/AudioChannelSelector.h module/port/Port.h module/ModuleConfiguration.h module/audio/PeakMeter.h module/audio/AudioVisualizer.h device/DeviceManager.h
	$(CPP) -c view/sidebar/captureconsolemodes/CaptureConsoleModeAudio.cpp -o $@ $(CXXFLAGS)

view/sidebar/captureconsolemodes/CaptureConsoleModeMidi.o : view/sidebar/captureconsolemodes/CaptureConsoleModeMidi.cpp view/sidebar/captureconsolemodes/CaptureConsoleModeMidi.h view/sidebar/captureconsolemodes/CaptureConsoleMode.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h view/sidebar/captureconsolemodes/CaptureTrackData.h view/sidebar/CaptureConsole.h view/sidebar/SideBar.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h stuff/Observable.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h view/helper/PeakMeterDisplay.h view/helper/graph/Node.h lib/math/rect.h view/audioview/AudioView.h data/SongSelection.h data/midi/Scale.h view/audioview/TrackHeightManager.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h view/mode/ViewModeCapture.h view/mode/ViewModeDefault.h view/mode/ViewMode.h data/base.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h module/SignalChain.h module/Module.h Session.h
	$(CPP) -c view/sidebar/captureconsolemodes/CaptureConsoleModeMidi.cpp -o $@ $(CXXFLAGS)

view/sidebar/captureconsolemodes/CaptureConsoleModeMulti.o : view/sidebar/captureconsolemodes/CaptureConsoleModeMulti.cpp view/sidebar/captureconsolemodes/CaptureConsoleModeMulti.h view/sidebar/captureconsolemodes/CaptureConsoleMode.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h view/sidebar/captureconsolemodes/CaptureTrackData.h view/sidebar/CaptureConsole.h view/sidebar/SideBar.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h stuff/Observable.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h view/helper/PeakMeterDisplay.h view/helper/graph/Node.h lib/math/rect.h view/audioview/AudioView.h data/SongSelection.h data/midi/Scale.h view/audioview/TrackHeightManager.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h view/mode/ViewModeCapture.h view/mode/ViewModeDefault.h view/mode/ViewMode.h data/base.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h module/audio/PeakMeter.h module/audio/AudioVisualizer.h module/Module.h module/port/Port.h module/SignalChain.h Session.h
	$(CPP) -c view/sidebar/captureconsolemodes/CaptureConsoleModeMulti.cpp -o $@ $(CXXFLAGS)

view/sidebar/captureconsolemodes/CaptureTrackData.o : view/sidebar/captureconsolemodes/CaptureTrackData.cpp view/sidebar/captureconsolemodes/CaptureTrackData.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h view/helper/PeakMeterDisplay.h stuff/Observable.h view/helper/graph/Node.h lib/base/pointer.h lib/math/rect.h lib/math/math.h lib/math/vec2.h data/base.h data/Track.h data/Range.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/Song.h data/Data.h lib/file/path.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h lib/file/file.h lib/file/msg.h lib/file/file_op.h data/TrackLayer.h lib/base/set.h module/SignalChain.h module/Module.h stuff/BackupManager.h Session.h device/Device.h device/DeviceManager.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/hui/main.h lib/hui/Event.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h device/stream/AudioInput.h data/audio/RingBuffer.h data/audio/AudioBuffer.h module/port/Port.h module/ModuleConfiguration.h device/stream/MidiInput.h device/stream/AudioOutput.h module/audio/AudioAccumulator.h module/audio/AudioBackup.h module/audio/AudioChannelSelector.h module/audio/PeakMeter.h module/audio/AudioVisualizer.h module/audio/AudioSucker.h module/midi/MidiAccumulator.h module/synthesizer/Synthesizer.h data/midi/Temperament.h
	$(CPP) -c view/sidebar/captureconsolemodes/CaptureTrackData.cpp -o $@ $(CXXFLAGS)

view/sidebar/AudioEditorConsole.o : view/sidebar/AudioEditorConsole.cpp view/sidebar/AudioEditorConsole.h view/sidebar/SideBar.h lib/base/pointer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h stuff/Observable.h view/audioview/AudioView.h data/SongSelection.h data/Range.h data/midi/Scale.h view/audioview/TrackHeightManager.h lib/math/rect.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h view/audioview/graph/AudioViewTrack.h view/helper/graph/Node.h view/mode/ViewModeEditAudio.h view/mode/ViewModeDefault.h view/mode/ViewMode.h module/Module.h plugins/PluginManager.h lib/kaba/kaba.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h TsunamiWindow.h Session.h EditModes.h
	$(CPP) -c view/sidebar/AudioEditorConsole.cpp -o $@ $(CXXFLAGS)

view/sidebar/CaptureConsole.o : view/sidebar/CaptureConsole.cpp view/sidebar/CaptureConsole.h view/sidebar/SideBar.h lib/base/pointer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h stuff/Observable.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h view/helper/PeakMeterDisplay.h view/helper/graph/Node.h lib/math/rect.h view/sidebar/captureconsolemodes/CaptureConsoleMode.h view/sidebar/captureconsolemodes/CaptureConsoleModeAudio.h view/sidebar/captureconsolemodes/CaptureConsoleModeMidi.h view/sidebar/captureconsolemodes/CaptureConsoleModeMulti.h view/sidebar/captureconsolemodes/CaptureTrackData.h view/audioview/AudioView.h data/SongSelection.h data/midi/Scale.h view/audioview/TrackHeightManager.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h view/mode/ViewModeCapture.h view/mode/ViewModeDefault.h view/mode/ViewMode.h data/base.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h device/stream/AudioOutput.h data/audio/RingBuffer.h data/audio/AudioBuffer.h module/Module.h module/ModuleConfiguration.h module/SignalChain.h Session.h EditModes.h
	$(CPP) -c view/sidebar/CaptureConsole.cpp -o $@ $(CXXFLAGS)

view/sidebar/CurveConsole.o : view/sidebar/CurveConsole.cpp view/sidebar/CurveConsole.h view/sidebar/SideBar.h lib/base/pointer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h stuff/Observable.h view/audioview/AudioView.h data/SongSelection.h data/Range.h data/midi/Scale.h view/audioview/TrackHeightManager.h lib/math/rect.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h view/mode/ViewModeCurve.h view/mode/ViewModeDefault.h view/mode/ViewMode.h data/base.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/Curve.h Session.h EditModes.h
	$(CPP) -c view/sidebar/CurveConsole.cpp -o $@ $(CXXFLAGS)

view/sidebar/DummyEditorConsole.o : view/sidebar/DummyEditorConsole.cpp view/sidebar/DummyEditorConsole.h view/sidebar/SideBar.h lib/base/pointer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h stuff/Observable.h Session.h EditModes.h
	$(CPP) -c view/sidebar/DummyEditorConsole.cpp -o $@ $(CXXFLAGS)

view/sidebar/MidiEditorConsole.o : view/sidebar/MidiEditorConsole.cpp view/sidebar/MidiEditorConsole.h view/sidebar/SideBar.h lib/base/pointer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h stuff/Observable.h view/audioview/AudioView.h data/SongSelection.h data/Range.h data/midi/Scale.h view/audioview/TrackHeightManager.h lib/math/rect.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h view/audioview/graph/AudioViewLayer.h view/helper/graph/Node.h view/audioview/graph/AudioViewTrack.h view/dialog/MarkerDialog.h view/dialog/QuestionDialog.h view/mode/ViewModeMidi.h view/mode/ViewModeDefault.h view/mode/ViewMode.h data/base.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h data/rhythm/Beat.h device/DeviceManager.h device/Device.h module/synthesizer/Synthesizer.h data/midi/Temperament.h module/Module.h module/port/Port.h module/ConfigPanel.h plugins/PluginManager.h lib/kaba/kaba.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h Session.h TsunamiWindow.h EditModes.h
	$(CPP) -c view/sidebar/MidiEditorConsole.cpp -o $@ $(CXXFLAGS)

view/sidebar/SampleManagerConsole.o : view/sidebar/SampleManagerConsole.cpp view/sidebar/SampleManagerConsole.h view/sidebar/SideBar.h lib/base/pointer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h stuff/Observable.h view/audioview/AudioView.h data/SongSelection.h data/Range.h data/midi/Scale.h view/audioview/TrackHeightManager.h lib/math/rect.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h view/audioview/graph/AudioViewLayer.h view/helper/graph/Node.h view/painter/MidiPainter.h view/helper/Progress.h view/dialog/SampleScaleDialog.h action/ActionManager.h action/Action.h data/Data.h data/base.h data/Song.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h data/TrackLayer.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/Sample.h data/audio/AudioBuffer.h device/stream/AudioOutput.h data/audio/RingBuffer.h module/Module.h module/ModuleConfiguration.h module/SignalChain.h module/audio/BufferStreamer.h module/audio/AudioSource.h module/port/Port.h module/audio/SongRenderer.h module/midi/MidiEventStreamer.h module/midi/MidiSource.h storage/Storage.h Session.h EditModes.h
	$(CPP) -c view/sidebar/SampleManagerConsole.cpp -o $@ $(CXXFLAGS)

view/sidebar/SampleRefConsole.o : view/sidebar/SampleRefConsole.cpp view/sidebar/SampleRefConsole.h view/sidebar/SideBar.h lib/base/pointer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h stuff/Observable.h view/sidebar/SampleManagerConsole.h view/audioview/AudioView.h data/SongSelection.h data/Range.h data/midi/Scale.h view/audioview/TrackHeightManager.h lib/math/rect.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h view/helper/Slider.h lib/base/callable.h data/base.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h data/Sample.h data/audio/AudioBuffer.h data/SampleRef.h Session.h EditModes.h
	$(CPP) -c view/sidebar/SampleRefConsole.cpp -o $@ $(CXXFLAGS)

view/sidebar/SideBar.o : view/sidebar/SideBar.cpp view/sidebar/SideBar.h lib/base/pointer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h stuff/Observable.h view/sidebar/TrackConsole.h view/sidebar/CurveConsole.h view/sidebar/DummyEditorConsole.h view/sidebar/AudioEditorConsole.h view/sidebar/MidiEditorConsole.h view/sidebar/SampleManagerConsole.h view/sidebar/SampleRefConsole.h view/sidebar/SongConsole.h view/sidebar/CaptureConsole.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h view/helper/PeakMeterDisplay.h view/helper/graph/Node.h lib/math/rect.h view/audioview/AudioView.h data/SongSelection.h data/midi/Scale.h view/audioview/TrackHeightManager.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h view/bottombar/BottomBar.h Session.h EditModes.h TsunamiWindow.h
	$(CPP) -c view/sidebar/SideBar.cpp -o $@ $(CXXFLAGS)

view/sidebar/SongConsole.o : view/sidebar/SongConsole.cpp view/sidebar/SongConsole.h view/sidebar/SideBar.h lib/base/pointer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h stuff/Observable.h view/audioview/AudioView.h data/SongSelection.h data/Range.h data/midi/Scale.h view/audioview/TrackHeightManager.h lib/math/rect.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h view/bottombar/BottomBar.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h lib/any/any.h data/base.h Session.h EditModes.h
	$(CPP) -c view/sidebar/SongConsole.cpp -o $@ $(CXXFLAGS)

view/sidebar/TrackConsole.o : view/sidebar/TrackConsole.cpp view/sidebar/TrackConsole.h view/sidebar/SideBar.h lib/base/pointer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h stuff/Observable.h view/audioview/AudioView.h data/SongSelection.h data/Range.h data/midi/Scale.h view/audioview/TrackHeightManager.h lib/math/rect.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h view/helper/Slider.h lib/base/callable.h view/helper/ModulePanel.h view/helper/FxListEditor.h view/dialog/TemperamentDialog.h data/midi/Temperament.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h view/dialog/TuningDialog.h view/dialog/ModuleSelectorDialog.h data/Track.h data/midi/Instrument.h data/base.h module/synthesizer/Synthesizer.h module/Module.h module/port/Port.h module/ConfigPanel.h plugins/PluginManager.h lib/kaba/kaba.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h Session.h EditModes.h
	$(CPP) -c view/sidebar/TrackConsole.cpp -o $@ $(CXXFLAGS)

view/ColorScheme.o : view/ColorScheme.cpp view/ColorScheme.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/image/color.h lib/base/map.h lib/base/set.h
	$(CPP) -c view/ColorScheme.cpp -o $@ $(CXXFLAGS)

view/HoverData.o : view/HoverData.cpp view/HoverData.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h data/Range.h view/audioview/graph/AudioViewLayer.h view/helper/graph/Node.h lib/base/pointer.h lib/math/rect.h lib/math/math.h lib/math/vec2.h stuff/Observable.h lib/base/map.h lib/base/set.h view/audioview/graph/AudioViewTrack.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h
	$(CPP) -c view/HoverData.cpp -o $@ $(CXXFLAGS)

view/MouseDelayPlanner.o : view/MouseDelayPlanner.cpp view/MouseDelayPlanner.h lib/base/pointer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h view/helper/graph/SceneGraph.h view/helper/graph/Node.h lib/math/rect.h stuff/Observable.h view/HoverData.h data/Range.h
	$(CPP) -c view/MouseDelayPlanner.cpp -o $@ $(CXXFLAGS)

Session.o : Session.cpp Session.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/base/pointer.h stuff/Observable.h EditModes.h TsunamiWindow.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h stuff/Log.h storage/Storage.h plugins/TsunamiPlugin.h module/Module.h data/base.h data/Song.h data/Data.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h module/SignalChain.h view/audioview/AudioView.h data/SongSelection.h data/midi/Scale.h view/audioview/TrackHeightManager.h lib/math/rect.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h view/mode/ViewModeDefault.h view/mode/ViewMode.h view/mode/ViewModeCapture.h view/mode/ViewModeCurve.h view/mode/ViewModeEdit.h view/mode/ViewModeEditAudio.h view/mode/ViewModeMidi.h view/mode/ViewModeScaleBars.h view/mode/ViewModeScaleMarker.h view/sidebar/SideBar.h view/sidebar/TrackConsole.h view/bottombar/BottomBar.h view/bottombar/MixingConsole.h
	$(CPP) -c Session.cpp -o $@ $(CXXFLAGS)

Tsunami.o : Tsunami.cpp Tsunami.h lib/base/pointer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h data/base.h data/Song.h data/Data.h stuff/Observable.h data/rhythm/BarCollection.h data/rhythm/Bar.h data/Range.h lib/any/any.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h module/SignalChain.h module/Module.h module/audio/AudioEffect.h module/port/Port.h module/audio/SongRenderer.h module/audio/AudioSource.h module/audio/PeakMeter.h module/audio/AudioVisualizer.h module/midi/MidiEffect.h module/synthesizer/Synthesizer.h data/midi/Temperament.h TsunamiWindow.h Session.h EditModes.h storage/Storage.h stuff/Log.h stuff/Clipboard.h stuff/CLIParser.h stuff/PerformanceMonitor.h stuff/BackupManager.h stuff/Diff.h stuff/ErrorHandler.h plugins/PluginManager.h lib/kaba/kaba.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h plugins/TsunamiPlugin.h device/DeviceManager.h device/stream/AudioOutput.h data/audio/RingBuffer.h data/audio/AudioBuffer.h module/ModuleConfiguration.h test/TestRingBuffer.h test/UnitTest.h module/ModuleFactory.h module/ConfigPanel.h data/Sample.h lib/math/rect.h data/SampleRef.h
	$(CPP) -c Tsunami.cpp -o $@ $(CXXFLAGS)

TsunamiWindow.o : TsunamiWindow.cpp TsunamiWindow.h lib/base/pointer.h lib/base/base.h lib/base/macros.h lib/base/array.h lib/base/strings.h lib/hui/hui.h lib/hui/common.h lib/config.h lib/file/file.h lib/file/msg.h lib/file/file_op.h lib/file/path.h lib/image/image.h lib/image/color.h lib/hui/Callback.h lib/hui/Application.h lib/base/map.h lib/base/set.h lib/hui/main.h lib/hui/Event.h lib/math/vec2.h lib/math/math.h lib/hui/Menu.h lib/hui/Panel.h lib/hui/Window.h lib/hui/common_dlg.h lib/hui/language.h lib/hui/Config.h lib/hui/Resource.h lib/hui/utility.h lib/hui/Painter.h lib/image/Painter.h lib/hui/Timer.h lib/hui/Toolbar.h lib/hui/error.h lib/hui/Clipboard.h Session.h stuff/Observable.h EditModes.h module/audio/SongRenderer.h data/Range.h module/audio/AudioSource.h module/Module.h module/port/Port.h Tsunami.h view/dialog/NewDialog.h data/rhythm/Bar.h view/dialog/HelpDialog.h view/dialog/SettingsDialog.h view/dialog/MarkerDialog.h view/dialog/BarAddDialog.h view/dialog/BarDeleteDialog.h view/dialog/BarEditDialog.h view/dialog/PauseAddDialog.h view/dialog/PauseEditDialog.h view/dialog/TrackRoutingDialog.h view/dialog/TimeTrackAddDialog.h view/dialog/QuestionDialog.h view/dialog/BufferCompressionDialog.h view/bottombar/BottomBar.h view/bottombar/MiniBar.h view/sidebar/SideBar.h view/sidebar/CaptureConsole.h data/Song.h data/Data.h data/rhythm/BarCollection.h lib/any/any.h view/helper/PeakMeterDisplay.h view/helper/graph/Node.h lib/math/rect.h view/mode/ViewModeDefault.h view/mode/ViewMode.h view/mode/ViewModeMidi.h view/mode/ViewModeEdit.h view/mode/ViewModeCapture.h view/mode/ViewModeScaleBars.h view/helper/Slider.h lib/base/callable.h view/helper/Progress.h view/helper/ModulePanel.h view/audioview/AudioView.h data/SongSelection.h data/midi/Scale.h view/audioview/TrackHeightManager.h view/audioview/ViewPort.h view/ColorScheme.h view/HoverData.h plugins/PluginManager.h lib/kaba/kaba.h lib/kaba/asm/asm.h lib/kaba/lib/lib.h lib/kaba/CompilerConfiguration.h lib/kaba/syntax/Flags.h lib/kaba/lib/extern.h lib/kaba/syntax/SyntaxTree.h lib/kaba/syntax/lexical.h lib/kaba/syntax/Class.h lib/kaba/syntax/Constant.h lib/kaba/syntax/Function.h lib/kaba/syntax/Node.h lib/kaba/syntax/Block.h lib/kaba/syntax/Operator.h lib/kaba/syntax/Variable.h lib/kaba/syntax/Identifier.h lib/kaba/syntax/Inline.h lib/kaba/syntax/Statement.h plugins/TsunamiPlugin.h storage/Storage.h stuff/Log.h stuff/Clipboard.h stuff/BackupManager.h data/base.h data/Track.h data/midi/MidiData.h data/midi/MidiNote.h data/midi/MidiEvent.h data/midi/Instrument.h data/TrackLayer.h action/ActionManager.h action/Action.h action/track/buffer/ActionTrackEditBuffer.h data/audio/AudioBuffer.h action/song/ActionSongMoveSelection.h module/audio/AudioEffect.h module/midi/MidiEffect.h module/midi/MidiSource.h module/ConfigPanel.h module/SignalChain.h plugins/FastFourierTransform.h device/DeviceManager.h view/audioview/graph/AudioViewLayer.h view/audioview/graph/AudioViewTrack.h
	$(CPP) -c TsunamiWindow.cpp -o $@ $(CXXFLAGS)

install :
	mkdir -p $(prefix)/share/tsunami
	xdg-desktop-menu install ../static/michisoft-tsunami.desktop
	xdg-mime install ../static/michisoft-nami.xml
	xdg-icon-resource install --context mimetypes --size 32 ../static/icons/32.png application-michisoft-nami --noupdate
	xdg-icon-resource install --context mimetypes --size 48 ../static/icons/48.png application-michisoft-nami --noupdate
	xdg-icon-resource install --context mimetypes --size 64 ../static/icons/64.png application-michisoft-nami --noupdate
	xdg-icon-resource install --context mimetypes --size 128 ../static/icons/128.png application-michisoft-nami
	cp -ruf '../static/icons' $(prefix)/share/tsunami
	cp -ruf '../static/Midi Fonts' $(prefix)/share/tsunami
	cp -ruf '../static/SignalChains' $(prefix)/share/tsunami
	cp -uf '../static/favorites_demo.txt' $(prefix)/share/tsunami
	cp -uf '../static/hui_resources.txt' $(prefix)/share/tsunami
	cp -uf '../static/icon.png' $(prefix)/share/tsunami
	cp -uf '../static/icon.svg' $(prefix)/share/tsunami
	cp -uf '../static/license.txt' $(prefix)/share/tsunami
	cp -uf '../static/license_small.txt' $(prefix)/share/tsunami
	cp -ruf '../Plugins' $(prefix)/share/tsunami
	cp $(BIN) $(prefix)/bin

clean :
	rm -f $(BIN)
	find . -name "*.o" -exec rm -f {} \;
	find . -name "*.a" -exec rm -f {} \;

