# Project: Tsunami

BIN = ../tsunami
all : $(BIN)
OBJ = \
 Action/Bar/ActionBar__Add.o\
 Action/Bar/ActionBar__Delete.o\
 Action/Bar/ActionBar__Edit.o\
 Action/Bar/Action__ScaleData.o\
 Action/Bar/Action__ShiftData.o\
 Action/Bar/ActionBarAdd.o\
 Action/Bar/ActionBarDelete.o\
 Action/Bar/ActionBarEdit.o\
 Action/Curve/ActionCurveAdd.o\
 Action/Curve/ActionCurveAddPoint.o\
 Action/Curve/ActionCurveDelete.o\
 Action/Curve/ActionCurveDeletePoint.o\
 Action/Curve/ActionCurveEdit.o\
 Action/Curve/ActionCurveEditPoint.o\
 Action/Song/Data/ActionSongChangeAllTrackVolumes.o\
 Action/Song/Data/ActionSongSetDefaultFormat.o\
 Action/Song/Data/ActionSongSetSampleRate.o\
 Action/Song/Data/ActionSongSetVolume.o\
 Action/Layer/ActionLayer__Delete.o\
 Action/Layer/ActionLayerAdd.o\
 Action/Layer/ActionLayerDelete.o\
 Action/Layer/ActionLayerMove.o\
 Action/Layer/ActionLayerMerge.o\
 Action/Layer/ActionLayerRename.o\
 Action/Sample/ActionSampleAdd.o\
 Action/Sample/ActionSampleDelete.o\
 Action/Sample/ActionSampleEditName.o\
 Action/Sample/ActionSampleScale.o\
 Action/Tag/ActionTagAdd.o\
 Action/Tag/ActionTagDelete.o\
 Action/Tag/ActionTagEdit.o\
 Action/Song/ActionSongDeleteSelection.o\
 Action/Track/Buffer/ActionTrack__AbsorbBuffer.o\
 Action/Track/Buffer/ActionTrack__AddBuffer.o\
 Action/Track/Buffer/ActionTrack__SplitBuffer.o\
 Action/Track/Buffer/ActionTrack__DeleteBuffer.o\
 Action/Track/Buffer/ActionTrack__GrowBuffer.o\
 Action/Track/Buffer/ActionTrack__ShrinkBuffer.o\
 Action/Track/Buffer/ActionTrackCreateBuffers.o\
 Action/Track/Buffer/ActionTrackEditBuffer.o\
 Action/Track/Data/ActionTrackEditMuted.o\
 Action/Track/Data/ActionTrackEditName.o\
 Action/Track/Data/ActionTrackEditPanning.o\
 Action/Track/Data/ActionTrackEditVolume.o\
 Action/Track/Data/ActionTrackSetInstrument.o\
 Action/Track/Effect/ActionTrackAddEffect.o\
 Action/Track/Effect/ActionTrackDeleteEffect.o\
 Action/Track/Effect/ActionTrackEditEffect.o\
 Action/Track/Effect/ActionTrackToggleEffectEnabled.o\
 Action/Track/Marker/ActionTrackAddMarker.o\
 Action/Track/Marker/ActionTrackDeleteMarker.o\
 Action/Track/Marker/ActionTrackEditMarker.o\
 Action/Track/Marker/ActionTrackMoveMarker.o\
 Action/Track/Midi/ActionTrackAddMidiEffect.o\
 Action/Track/Midi/ActionTrackDeleteMidiEffect.o\
 Action/Track/Midi/ActionTrackEditMidiEffect.o\
 Action/Track/Midi/ActionTrackToggleMidiEffectEnabled.o\
 Action/Track/Midi/ActionTrackAddMidiNote.o\
 Action/Track/Midi/ActionTrackDeleteMidiNote.o\
 Action/Track/Midi/ActionTrackInsertMidi.o\
 Action/Track/Sample/ActionTrackAddSample.o\
 Action/Track/Sample/ActionTrackDeleteSample.o\
 Action/Track/Sample/ActionTrackEditSample.o\
 Action/Track/Sample/ActionTrackInsertSample.o\
 Action/Track/Sample/ActionTrackInsertSelectedSamples.o\
 Action/Track/Sample/ActionTrackMoveSample.o\
 Action/Track/Sample/ActionTrackPasteAsSample.o\
 Action/Track/Sample/ActionTrackSampleFromSelection.o\
 Action/Track/Synthesizer/ActionTrackDetuneSynthesizer.o\
 Action/Track/Synthesizer/ActionTrackEditSynthesizer.o\
 Action/Track/Synthesizer/ActionTrackSetSynthesizer.o\
 Action/Track/ActionTrack__DeleteEmpty.o\
 Action/Track/ActionTrackAdd.o\
 Action/Track/ActionTrackDelete.o\
 Action/Track/ActionTrackMove.o\
 Action/Action.o\
 Action/ActionGroup.o\
 Action/ActionManager.o\
 Action/ActionMergable.o\
 Audio/AudioBuffer.o\
 Audio/AudioSucker.o\
 Audio/BufferInterpolator.o\
 Audio/RingBuffer.o\
 Audio/Source/AudioSource.o\
 Audio/Source/BufferStreamer.o\
 Audio/Source/SongRenderer.o\
 Audio/Synth/DummySynthesizer.o\
 Audio/Synth/EnvelopeADSR.o\
 Audio/Synth/SampleSynthesizer.o\
 Audio/Synth/Synthesizer.o\
 Data/Curve.o\
 Data/Data.o\
 Data/Range.o\
 Data/Sample.o\
 Data/SampleRef.o\
 Data/Song.o\
 Data/SongSelection.o\
 Data/Track.o\
 Device/Device.o\
 Device/DeviceManager.o\
 Device/InputStreamAudio.o\
 Device/InputStreamMidi.o\
 Device/OutputStream.o\
 lib/base/array.o\
 lib/base/strings.o\
 lib/file/file.o\
 lib/file/file_op.o\
 lib/file/msg.o\
 lib/hui/Controls/Control.o\
 lib/hui/Controls/ControlButtonGtk.o\
 lib/hui/Controls/ControlCheckBoxGtk.o\
 lib/hui/Controls/ControlColorButtonGtk.o\
 lib/hui/Controls/ControlComboBoxGtk.o\
 lib/hui/Controls/ControlDrawingAreaGtk.o\
 lib/hui/Controls/ControlEditGtk.o\
 lib/hui/Controls/ControlExpanderGtk.o\
 lib/hui/Controls/ControlGridGtk.o\
 lib/hui/Controls/ControlGroupGtk.o\
 lib/hui/Controls/ControlLabelGtk.o\
 lib/hui/Controls/ControlListViewGtk.o\
 lib/hui/Controls/ControlMultilineEditGtk.o\
 lib/hui/Controls/ControlPanedGtk.o\
 lib/hui/Controls/ControlProgressBarGtk.o\
 lib/hui/Controls/ControlRadioButtonGtk.o\
 lib/hui/Controls/ControlRevealerGtk.o\
 lib/hui/Controls/ControlScrollerGtk.o\
 lib/hui/Controls/ControlSeparatorGtk.o\
 lib/hui/Controls/ControlSliderGtk.o\
 lib/hui/Controls/ControlSpinButtonGtk.o\
 lib/hui/Controls/ControlTabControlGtk.o\
 lib/hui/Controls/ControlToggleButtonGtk.o\
 lib/hui/Controls/ControlTreeViewGtk.o\
 lib/hui/Controls/MenuItemGtk.o\
 lib/hui/Controls/MenuItemSeparatorGtk.o\
 lib/hui/Controls/MenuItemSubmenuGtk.o\
 lib/hui/Controls/MenuItemToggleGtk.o\
 lib/hui/Controls/ToolItemButtonGtk.o\
 lib/hui/Controls/ToolItemMenuButtonGtk.o\
 lib/hui/Controls/ToolItemSeparatorGtk.o\
 lib/hui/Controls/ToolItemToggleButtonGtk.o\
 lib/hui/hui.o\
 lib/hui/common_dlg.o\
 lib/hui/common_dlg_gtk.o\
 lib/hui/common_dlg_win.o\
 lib/hui/error.o\
 lib/hui/language.o\
 lib/hui/main_gtk.o\
 lib/hui/main_win.o\
 lib/hui/utility.o\
 lib/hui/window_control.o\
 lib/hui/window_control_gtk.o\
 lib/hui/window_control_win.o\
 lib/hui/Application.o\
 lib/hui/Callback.o\
 lib/hui/Clipboard.o\
 lib/hui/Config.o\
 lib/hui/Event.o\
 lib/hui/Menu.o\
 lib/hui/MenuGtk.o\
 lib/hui/MenuWin.o\
 lib/hui/PainterGtk.o\
 lib/hui/Panel.o\
 lib/hui/Resource.o\
 lib/hui/Timer.o\
 lib/hui/Toolbar.o\
 lib/hui/ToolbarGtk.o\
 lib/hui/ToolbarWin.o\
 lib/hui/Window.o\
 lib/hui/WindowGtk.o\
 lib/hui/WindowWin.o\
 lib/image/color.o\
 lib/image/image.o\
 lib/image/image_bmp.o\
 lib/image/image_jpg.o\
 lib/image/image_png.o\
 lib/image/image_tga.o\
 lib/image/ImagePainter.o\
 lib/kaba/asm/asm.o\
 lib/kaba/compiler/compiler.o\
 lib/kaba/compiler/serializer.o\
 lib/kaba/compiler/serializer_amd64.o\
 lib/kaba/compiler/serializer_arm.o\
 lib/kaba/compiler/serializer_x86.o\
 lib/kaba/lib/lib.o\
 lib/kaba/lib/lib_file.o\
 lib/kaba/lib/lib_hui.o\
 lib/kaba/lib/lib_image.o\
 lib/kaba/lib/lib_math.o\
 lib/kaba/lib/lib_net.o\
 lib/kaba/lib/lib_nix.o\
 lib/kaba/lib/lib_sound.o\
 lib/kaba/lib/lib_threads.o\
 lib/kaba/lib/lib_x.o\
 lib/kaba/syntax/class.o\
 lib/kaba/syntax/implicit.o\
 lib/kaba/syntax/lexical.o\
 lib/kaba/syntax/parser.o\
 lib/kaba/syntax/precompiler.o\
 lib/kaba/syntax/preprocessor.o\
 lib/kaba/syntax/syntax_tree.o\
 lib/kaba/kaba.o\
 lib/math/complex.o\
 lib/math/interpolation.o\
 lib/math/math.o\
 lib/math/matrix.o\
 lib/math/matrix3.o\
 lib/math/plane.o\
 lib/math/quaternion.o\
 lib/math/random.o\
 lib/math/ray.o\
 lib/math/rect.o\
 lib/math/vector.o\
 lib/net/net.o\
 lib/threads/Mutex.o\
 lib/threads/Thread.o\
 lib/threads/ThreadedWork.o\
 lib/xfile/chunked.o\
 Midi/Clef.o\
 Midi/Instrument.o\
 Midi/MidiData.o\
 Midi/MidiEvent.o\
 Midi/MidiEventStreamer.o\
 Midi/MidiNote.o\
 Midi/MidiSource.o\
 Midi/Scale.o\
 Plugins/AutoConfigPanel.o\
 Plugins/ConfigPanel.o\
 Plugins/Configurable.o\
 Plugins/Effect.o\
 Plugins/EffectManager.o\
 Plugins/ExtendedAudioBuffer.o\
 Plugins/FastFourierTransform.o\
 Plugins/FavoriteManager.o\
 Plugins/MidiEffect.o\
 Plugins/Plugin.o\
 Plugins/PluginManager.o\
 Plugins/SongPlugin.o\
 Plugins/TsunamiPlugin.o\
 Rhythm/Bar.o\
 Rhythm/BarCollection.o\
 Rhythm/BarStreamer.o\
 Rhythm/Beat.o\
 Rhythm/BeatSource.o\
 Storage/Dialog/RawConfigDialog.o\
 Storage/Format/Format.o\
 Storage/Format/FormatFlac.o\
 Storage/Format/FormatGuitarPro.o\
 Storage/Format/FormatMidi.o\
 Storage/Format/FormatM4a.o\
 Storage/Format/FormatMp3.o\
 Storage/Format/FormatNami.o\
 Storage/Format/FormatOgg.o\
 Storage/Format/FormatRaw.o\
 Storage/Format/FormatSoundFont2.o\
 Storage/Format/FormatWave.o\
 Storage/Storage.o\
 Storage/StorageOperationData.o\
 Stuff/Clipboard.o\
 Stuff/Log.o\
 Stuff/Observable.o\
 Stuff/PerformanceMonitor.o\
 View/BottomBar/BottomBar.o\
 View/BottomBar/DeviceConsole.o\
 View/BottomBar/LogConsole.o\
 View/BottomBar/MiniBar.o\
 View/BottomBar/MixingConsole.o\
 View/Dialog/BarAddDialog.o\
 View/Dialog/BarEditDialog.o\
 View/Dialog/ConfigurableSelectorDialog.o\
 View/Dialog/DetuneSynthesizerDialog.o\
 View/Dialog/MarkerDialog.o\
 View/Dialog/NewDialog.o\
 View/Dialog/PauseAddDialog.o\
 View/Dialog/PauseEditDialog.o\
 View/Dialog/RecordDialog.o\
 View/Dialog/SampleScaleDialog.o\
 View/Dialog/SettingsDialog.o\
 View/Dialog/TuningDialog.o\
 View/Helper/PeakMeter.o\
 View/Helper/Progress.o\
 View/Helper/Slider.o\
 View/Mode/ViewMode.o\
 View/Mode/ViewModeCapture.o\
 View/Mode/ViewModeCurve.o\
 View/Mode/ViewModeDefault.o\
 View/Mode/ViewModeMidi.o\
 View/Mode/ViewModeScaleBars.o\
 View/SideBar/CaptureConsoleModes/CaptureConsoleMode.o\
 View/SideBar/CaptureConsoleModes/CaptureConsoleModeAudio.o\
 View/SideBar/CaptureConsoleModes/CaptureConsoleModeMidi.o\
 View/SideBar/CaptureConsoleModes/CaptureConsoleModeMulti.o\
 View/SideBar/CaptureConsole.o\
 View/SideBar/CurveConsole.o\
 View/SideBar/FxConsole.o\
 View/SideBar/LayerConsole.o\
 View/SideBar/MidiEditorConsole.o\
 View/SideBar/MidiFxConsole.o\
 View/SideBar/SampleManagerConsole.o\
 View/SideBar/SampleRefConsole.o\
 View/SideBar/SideBar.o\
 View/SideBar/SongConsole.o\
 View/SideBar/SynthConsole.o\
 View/SideBar/TrackConsole.o\
 View/AudioView.o\
 View/AudioViewTrack.o\
 View/ColorScheme.o\
 View/Selection.o\
 View/TrackHeightManager.o\
 View/ViewPort.o\
 TsunamiWindow.o#\
# Tsunami.o
CPP = g++
GLOBALFLAGS = -O2 -g
#GLOBALFLAGS = -O1 -g
LIB_DIR = lib
CXXFLAGS = -std=c++11 `pkg-config --cflags gtk+-3.0` -DUSE_UNWIND=1 $(GLOBALFLAGS)
CXXFLAGSLINKING = -std=c++11 `pkg-config --cflags --libs gtk+-3.0` -lX11 -lz -lunwind -lpulse -lasound -lfftw3f -lFLAC -logg -lvorbis -lvorbisfile -lvorbisenc $(GLOBALFLAGS)

$(BIN) : $(OBJ) Tsunami.o
	$(CPP) $(OBJ) Tsunami.o -o $(BIN) $(CXXFLAGSLINKING)

Action/Bar/ActionBar__Add.o : Action/Bar/ActionBar__Add.cpp
	$(CPP) -c Action/Bar/ActionBar__Add.cpp -o $@ $(CXXFLAGS)

Action/Bar/ActionBar__Delete.o : Action/Bar/ActionBar__Delete.cpp
	$(CPP) -c Action/Bar/ActionBar__Delete.cpp -o $@ $(CXXFLAGS)

Action/Bar/ActionBar__Edit.o : Action/Bar/ActionBar__Edit.cpp
	$(CPP) -c Action/Bar/ActionBar__Edit.cpp -o $@ $(CXXFLAGS)

Action/Bar/Action__ScaleData.o : Action/Bar/Action__ScaleData.cpp
	$(CPP) -c Action/Bar/Action__ScaleData.cpp -o $@ $(CXXFLAGS)

Action/Bar/Action__ShiftData.o : Action/Bar/Action__ShiftData.cpp
	$(CPP) -c Action/Bar/Action__ShiftData.cpp -o $@ $(CXXFLAGS)

Action/Bar/ActionBarAdd.o : Action/Bar/ActionBarAdd.cpp
	$(CPP) -c Action/Bar/ActionBarAdd.cpp -o $@ $(CXXFLAGS)

Action/Bar/ActionBarDelete.o : Action/Bar/ActionBarDelete.cpp
	$(CPP) -c Action/Bar/ActionBarDelete.cpp -o $@ $(CXXFLAGS)

Action/Bar/ActionBarEdit.o : Action/Bar/ActionBarEdit.cpp
	$(CPP) -c Action/Bar/ActionBarEdit.cpp -o $@ $(CXXFLAGS)

Action/Curve/ActionCurveAdd.o : Action/Curve/ActionCurveAdd.cpp
	$(CPP) -c Action/Curve/ActionCurveAdd.cpp -o $@ $(CXXFLAGS)

Action/Curve/ActionCurveAddPoint.o : Action/Curve/ActionCurveAddPoint.cpp
	$(CPP) -c Action/Curve/ActionCurveAddPoint.cpp -o $@ $(CXXFLAGS)

Action/Curve/ActionCurveDelete.o : Action/Curve/ActionCurveDelete.cpp
	$(CPP) -c Action/Curve/ActionCurveDelete.cpp -o $@ $(CXXFLAGS)

Action/Curve/ActionCurveDeletePoint.o : Action/Curve/ActionCurveDeletePoint.cpp
	$(CPP) -c Action/Curve/ActionCurveDeletePoint.cpp -o $@ $(CXXFLAGS)

Action/Curve/ActionCurveEdit.o : Action/Curve/ActionCurveEdit.cpp
	$(CPP) -c Action/Curve/ActionCurveEdit.cpp -o $@ $(CXXFLAGS)

Action/Curve/ActionCurveEditPoint.o : Action/Curve/ActionCurveEditPoint.cpp
	$(CPP) -c Action/Curve/ActionCurveEditPoint.cpp -o $@ $(CXXFLAGS)

Action/Song/Data/ActionSongChangeAllTrackVolumes.o : Action/Song/Data/ActionSongChangeAllTrackVolumes.cpp
	$(CPP) -c Action/Song/Data/ActionSongChangeAllTrackVolumes.cpp -o $@ $(CXXFLAGS)

Action/Song/Data/ActionSongSetDefaultFormat.o : Action/Song/Data/ActionSongSetDefaultFormat.cpp
	$(CPP) -c Action/Song/Data/ActionSongSetDefaultFormat.cpp -o $@ $(CXXFLAGS)

Action/Song/Data/ActionSongSetSampleRate.o : Action/Song/Data/ActionSongSetSampleRate.cpp
	$(CPP) -c Action/Song/Data/ActionSongSetSampleRate.cpp -o $@ $(CXXFLAGS)

Action/Song/Data/ActionSongSetVolume.o : Action/Song/Data/ActionSongSetVolume.cpp
	$(CPP) -c Action/Song/Data/ActionSongSetVolume.cpp -o $@ $(CXXFLAGS)

Action/Layer/ActionLayer__Delete.o : Action/Layer/ActionLayer__Delete.cpp
	$(CPP) -c Action/Layer/ActionLayer__Delete.cpp -o $@ $(CXXFLAGS)

Action/Layer/ActionLayerAdd.o : Action/Layer/ActionLayerAdd.cpp
	$(CPP) -c Action/Layer/ActionLayerAdd.cpp -o $@ $(CXXFLAGS)

Action/Layer/ActionLayerDelete.o : Action/Layer/ActionLayerDelete.cpp
	$(CPP) -c Action/Layer/ActionLayerDelete.cpp -o $@ $(CXXFLAGS)

Action/Layer/ActionLayerMove.o : Action/Layer/ActionLayerMove.cpp
	$(CPP) -c Action/Layer/ActionLayerMove.cpp -o $@ $(CXXFLAGS)

Action/Layer/ActionLayerMerge.o : Action/Layer/ActionLayerMerge.cpp
	$(CPP) -c Action/Layer/ActionLayerMerge.cpp -o $@ $(CXXFLAGS)

Action/Layer/ActionLayerRename.o : Action/Layer/ActionLayerRename.cpp
	$(CPP) -c Action/Layer/ActionLayerRename.cpp -o $@ $(CXXFLAGS)

Action/Sample/ActionSampleAdd.o : Action/Sample/ActionSampleAdd.cpp
	$(CPP) -c Action/Sample/ActionSampleAdd.cpp -o $@ $(CXXFLAGS)

Action/Sample/ActionSongSampleDelete.o : Action/Sample/ActionSampleDelete.cpp
	$(CPP) -c Action/Sample/ActionSampleDelete.cpp -o $@ $(CXXFLAGS)

Action/Sample/ActionSongSampleEditName.o : Action/Sample/ActionSampleEditName.cpp
	$(CPP) -c Action/Sample/ActionSongSampleEditName.cpp -o $@ $(CXXFLAGS)

Action/Sample/ActionSampleScale.o : Action/Sample/ActionSampleScale.cpp
	$(CPP) -c Action/Sample/ActionSampleScale.cpp -o $@ $(CXXFLAGS)

Action/Tag/ActionTagAdd.o : Action/Tag/ActionTagAdd.cpp
	$(CPP) -c Action/Tag/ActionTagAdd.cpp -o $@ $(CXXFLAGS)

Action/Tag/ActionTagDelete.o : Action/Tag/ActionTagDelete.cpp
	$(CPP) -c Action/Tag/ActionTagDelete.cpp -o $@ $(CXXFLAGS)

Action/Tag/ActionTagEdit.o : Action/Tag/ActionTagEdit.cpp
	$(CPP) -c Action/Tag/ActionTagEdit.cpp -o $@ $(CXXFLAGS)

Action/Song/ActionSongDeleteSelection.o : Action/Song/ActionSongDeleteSelection.cpp
	$(CPP) -c Action/Song/ActionSongDeleteSelection.cpp -o $@ $(CXXFLAGS)

Action/Track/Buffer/ActionTrack__AbsorbBuffer.o : Action/Track/Buffer/ActionTrack__AbsorbBuffer.cpp
	$(CPP) -c Action/Track/Buffer/ActionTrack__AbsorbBuffer.cpp -o $@ $(CXXFLAGS)

Action/Track/Buffer/ActionTrack__AddBuffer.o : Action/Track/Buffer/ActionTrack__AddBuffer.cpp
	$(CPP) -c Action/Track/Buffer/ActionTrack__AddBuffer.cpp -o $@ $(CXXFLAGS)

Action/Track/Buffer/ActionTrack__SplitBuffer.o : Action/Track/Buffer/ActionTrack__SplitBuffer.cpp
	$(CPP) -c Action/Track/Buffer/ActionTrack__SplitBuffer.cpp -o $@ $(CXXFLAGS)

Action/Track/Buffer/ActionTrack__DeleteBuffer.o : Action/Track/Buffer/ActionTrack__DeleteBuffer.cpp
	$(CPP) -c Action/Track/Buffer/ActionTrack__DeleteBuffer.cpp -o $@ $(CXXFLAGS)

Action/Track/Buffer/ActionTrack__GrowBuffer.o : Action/Track/Buffer/ActionTrack__GrowBuffer.cpp
	$(CPP) -c Action/Track/Buffer/ActionTrack__GrowBuffer.cpp -o $@ $(CXXFLAGS)

Action/Track/Buffer/ActionTrack__ShrinkBuffer.o : Action/Track/Buffer/ActionTrack__ShrinkBuffer.cpp
	$(CPP) -c Action/Track/Buffer/ActionTrack__ShrinkBuffer.cpp -o $@ $(CXXFLAGS)

Action/Track/Buffer/ActionTrackCreateBuffers.o : Action/Track/Buffer/ActionTrackCreateBuffers.cpp
	$(CPP) -c Action/Track/Buffer/ActionTrackCreateBuffers.cpp -o $@ $(CXXFLAGS)

Action/Track/Buffer/ActionTrackEditBuffer.o : Action/Track/Buffer/ActionTrackEditBuffer.cpp
	$(CPP) -c Action/Track/Buffer/ActionTrackEditBuffer.cpp -o $@ $(CXXFLAGS)

Action/Track/Data/ActionTrackEditMuted.o : Action/Track/Data/ActionTrackEditMuted.cpp
	$(CPP) -c Action/Track/Data/ActionTrackEditMuted.cpp -o $@ $(CXXFLAGS)

Action/Track/Data/ActionTrackEditName.o : Action/Track/Data/ActionTrackEditName.cpp
	$(CPP) -c Action/Track/Data/ActionTrackEditName.cpp -o $@ $(CXXFLAGS)

Action/Track/Data/ActionTrackEditPanning.o : Action/Track/Data/ActionTrackEditPanning.cpp
	$(CPP) -c Action/Track/Data/ActionTrackEditPanning.cpp -o $@ $(CXXFLAGS)

Action/Track/Data/ActionTrackEditVolume.o : Action/Track/Data/ActionTrackEditVolume.cpp
	$(CPP) -c Action/Track/Data/ActionTrackEditVolume.cpp -o $@ $(CXXFLAGS)

Action/Track/Data/ActionTrackSetInstrument.o : Action/Track/Data/ActionTrackSetInstrument.cpp
	$(CPP) -c Action/Track/Data/ActionTrackSetInstrument.cpp -o $@ $(CXXFLAGS)

Action/Track/Effect/ActionTrackAddEffect.o : Action/Track/Effect/ActionTrackAddEffect.cpp
	$(CPP) -c Action/Track/Effect/ActionTrackAddEffect.cpp -o $@ $(CXXFLAGS)

Action/Track/Effect/ActionTrackDeleteEffect.o : Action/Track/Effect/ActionTrackDeleteEffect.cpp
	$(CPP) -c Action/Track/Effect/ActionTrackDeleteEffect.cpp -o $@ $(CXXFLAGS)

Action/Track/Effect/ActionTrackEditEffect.o : Action/Track/Effect/ActionTrackEditEffect.cpp
	$(CPP) -c Action/Track/Effect/ActionTrackEditEffect.cpp -o $@ $(CXXFLAGS)

Action/Track/Effect/ActionTrackToggleEffectEnabled.o : Action/Track/Effect/ActionTrackToggleEffectEnabled.cpp
	$(CPP) -c Action/Track/Effect/ActionTrackToggleEffectEnabled.cpp -o $@ $(CXXFLAGS)

Action/Track/Marker/ActionTrackAddMarker.o : Action/Track/Marker/ActionTrackAddMarker.cpp
	$(CPP) -c Action/Track/Marker/ActionTrackAddMarker.cpp -o $@ $(CXXFLAGS)

Action/Track/Marker/ActionTrackDeleteMarker.o : Action/Track/Marker/ActionTrackDeleteMarker.cpp
	$(CPP) -c Action/Track/Marker/ActionTrackDeleteMarker.cpp -o $@ $(CXXFLAGS)

Action/Track/Marker/ActionTrackMoveMarker.o : Action/Track/Marker/ActionTrackMoveMarker.cpp
	$(CPP) -c Action/Track/Marker/ActionTrackMoveMarker.cpp -o $@ $(CXXFLAGS)

Action/Track/Marker/ActionTrackEditMarker.o : Action/Track/Marker/ActionTrackEditMarker.cpp
	$(CPP) -c Action/Track/Marker/ActionTrackEditMarker.cpp -o $@ $(CXXFLAGS)

Action/Track/Midi/ActionTrackAddMidiEffect.o : Action/Track/Midi/ActionTrackAddMidiEffect.cpp
	$(CPP) -c Action/Track/Midi/ActionTrackAddMidiEffect.cpp -o $@ $(CXXFLAGS)

Action/Track/Midi/ActionTrackDeleteMidiEffect.o : Action/Track/Midi/ActionTrackDeleteMidiEffect.cpp
	$(CPP) -c Action/Track/Midi/ActionTrackDeleteMidiEffect.cpp -o $@ $(CXXFLAGS)

Action/Track/Midi/ActionTrackEditMidiEffect.o : Action/Track/Midi/ActionTrackEditMidiEffect.cpp
	$(CPP) -c Action/Track/Midi/ActionTrackEditMidiEffect.cpp -o $@ $(CXXFLAGS)

Action/Track/Midi/ActionTrackToggleMidiEffectEnabled.o : Action/Track/Midi/ActionTrackToggleMidiEffectEnabled.cpp
	$(CPP) -c Action/Track/Midi/ActionTrackToggleMidiEffectEnabled.cpp -o $@ $(CXXFLAGS)

Action/Track/Midi/ActionTrackAddMidiNote.o : Action/Track/Midi/ActionTrackAddMidiNote.cpp
	$(CPP) -c Action/Track/Midi/ActionTrackAddMidiNote.cpp -o $@ $(CXXFLAGS)

Action/Track/Midi/ActionTrackDeleteMidiNote.o : Action/Track/Midi/ActionTrackDeleteMidiNote.cpp
	$(CPP) -c Action/Track/Midi/ActionTrackDeleteMidiNote.cpp -o $@ $(CXXFLAGS)

Action/Track/Midi/ActionTrackInsertMidi.o : Action/Track/Midi/ActionTrackInsertMidi.cpp
	$(CPP) -c Action/Track/Midi/ActionTrackInsertMidi.cpp -o $@ $(CXXFLAGS)

Action/Track/Sample/ActionTrackAddSample.o : Action/Track/Sample/ActionTrackAddSample.cpp
	$(CPP) -c Action/Track/Sample/ActionTrackAddSample.cpp -o $@ $(CXXFLAGS)

Action/Track/Sample/ActionTrackDeleteSample.o : Action/Track/Sample/ActionTrackDeleteSample.cpp
	$(CPP) -c Action/Track/Sample/ActionTrackDeleteSample.cpp -o $@ $(CXXFLAGS)

Action/Track/Sample/ActionTrackEditSample.o : Action/Track/Sample/ActionTrackEditSample.cpp
	$(CPP) -c Action/Track/Sample/ActionTrackEditSample.cpp -o $@ $(CXXFLAGS)

Action/Track/Sample/ActionTrackInsertSample.o : Action/Track/Sample/ActionTrackInsertSample.cpp
	$(CPP) -c Action/Track/Sample/ActionTrackInsertSample.cpp -o $@ $(CXXFLAGS)

Action/Track/Sample/ActionTrackInsertSelectedSamples.o : Action/Track/Sample/ActionTrackInsertSelectedSamples.cpp
	$(CPP) -c Action/Track/Sample/ActionTrackInsertSelectedSamples.cpp -o $@ $(CXXFLAGS)

Action/Track/Sample/ActionTrackMoveSample.o : Action/Track/Sample/ActionTrackMoveSample.cpp
	$(CPP) -c Action/Track/Sample/ActionTrackMoveSample.cpp -o $@ $(CXXFLAGS)

Action/Track/Sample/ActionTrackPasteAsSample.o : Action/Track/Sample/ActionTrackPasteAsSample.cpp
	$(CPP) -c Action/Track/Sample/ActionTrackPasteAsSample.cpp -o $@ $(CXXFLAGS)

Action/Track/Sample/ActionTrackSampleFromSelection.o : Action/Track/Sample/ActionTrackSampleFromSelection.cpp
	$(CPP) -c Action/Track/Sample/ActionTrackSampleFromSelection.cpp -o $@ $(CXXFLAGS)

Action/Track/Synthesizer/ActionTrackDetuneSynthesizer.o : Action/Track/Synthesizer/ActionTrackDetuneSynthesizer.cpp
	$(CPP) -c Action/Track/Synthesizer/ActionTrackDetuneSynthesizer.cpp -o $@ $(CXXFLAGS)

Action/Track/Synthesizer/ActionTrackEditSynthesizer.o : Action/Track/Synthesizer/ActionTrackEditSynthesizer.cpp
	$(CPP) -c Action/Track/Synthesizer/ActionTrackEditSynthesizer.cpp -o $@ $(CXXFLAGS)

Action/Track/Synthesizer/ActionTrackSetSynthesizer.o : Action/Track/Synthesizer/ActionTrackSetSynthesizer.cpp
	$(CPP) -c Action/Track/Synthesizer/ActionTrackSetSynthesizer.cpp -o $@ $(CXXFLAGS)

Action/Track/ActionTrack__DeleteEmpty.o : Action/Track/ActionTrack__DeleteEmpty.cpp
	$(CPP) -c Action/Track/ActionTrack__DeleteEmpty.cpp -o $@ $(CXXFLAGS)

Action/Track/ActionTrackAdd.o : Action/Track/ActionTrackAdd.cpp
	$(CPP) -c Action/Track/ActionTrackAdd.cpp -o $@ $(CXXFLAGS)

Action/Track/ActionTrackDelete.o : Action/Track/ActionTrackDelete.cpp
	$(CPP) -c Action/Track/ActionTrackDelete.cpp -o $@ $(CXXFLAGS)

Action/Track/ActionTrackMove.o : Action/Track/ActionTrackMove.cpp
	$(CPP) -c Action/Track/ActionTrackMove.cpp -o $@ $(CXXFLAGS)

Action/Action.o : Action/Action.cpp
	$(CPP) -c Action/Action.cpp -o $@ $(CXXFLAGS)

Action/ActionGroup.o : Action/ActionGroup.cpp
	$(CPP) -c Action/ActionGroup.cpp -o $@ $(CXXFLAGS)

Action/ActionManager.o : Action/ActionManager.cpp
	$(CPP) -c Action/ActionManager.cpp -o $@ $(CXXFLAGS)

Action/ActionMergable.o : Action/ActionMergable.cpp
	$(CPP) -c Action/ActionMergable.cpp -o $@ $(CXXFLAGS)

Audio/AudioBuffer.o : Audio/AudioBuffer.cpp
	$(CPP) -c Audio/AudioBuffer.cpp -o $@ $(CXXFLAGS)

Audio/AudioSucker.o : Audio/AudioSucker.cpp
	$(CPP) -c Audio/AudioSucker.cpp -o $@ $(CXXFLAGS)

Audio/BufferInterpolator.o : Audio/BufferInterpolator.cpp
	$(CPP) -c Audio/BufferInterpolator.cpp -o $@ $(CXXFLAGS)

Audio/RingBuffer.o : Audio/RingBuffer.cpp
	$(CPP) -c Audio/RingBuffer.cpp -o $@ $(CXXFLAGS)

Audio/Source/AudioSource.o : Audio/Source/AudioSource.cpp
	$(CPP) -c Audio/Source/AudioSource.cpp -o $@ $(CXXFLAGS)

Audio/Source/BufferStreamer.o : Audio/Source/BufferStreamer.cpp
	$(CPP) -c Audio/Source/BufferStreamer.cpp -o $@ $(CXXFLAGS)

Audio/Source/SongRenderer.o : Audio/Source/SongRenderer.cpp
	$(CPP) -c Audio/Source/SongRenderer.cpp -o $@ $(CXXFLAGS)

Audio/Synth/DummySynthesizer.o : Audio/Synth/DummySynthesizer.cpp
	$(CPP) -c Audio/Synth/DummySynthesizer.cpp -o $@ $(CXXFLAGS)

Audio/Synth/EnvelopeADSR.o : Audio/Synth/EnvelopeADSR.cpp
	$(CPP) -c Audio/Synth/EnvelopeADSR.cpp -o $@ $(CXXFLAGS)

Audio/Synth/SampleSynthesizer.o : Audio/Synth/SampleSynthesizer.cpp
	$(CPP) -c Audio/Synth/SampleSynthesizer.cpp -o $@ $(CXXFLAGS)

Audio/Synth/Synthesizer.o : Audio/Synth/Synthesizer.cpp
	$(CPP) -c Audio/Synth/Synthesizer.cpp -o $@ $(CXXFLAGS)

Data/Curve.o : Data/Curve.cpp
	$(CPP) -c Data/Curve.cpp -o $@ $(CXXFLAGS)

Data/Data.o : Data/Data.cpp
	$(CPP) -c Data/Data.cpp -o $@ $(CXXFLAGS)

Data/Range.o : Data/Range.cpp
	$(CPP) -c Data/Range.cpp -o $@ $(CXXFLAGS)

Data/Sample.o : Data/Sample.cpp
	$(CPP) -c Data/Sample.cpp -o $@ $(CXXFLAGS)

Data/SampleRef.o : Data/SampleRef.cpp
	$(CPP) -c Data/SampleRef.cpp -o $@ $(CXXFLAGS)

Data/Song.o : Data/Song.cpp
	$(CPP) -c Data/Song.cpp -o $@ $(CXXFLAGS)

Data/SongSelection.o : Data/SongSelection.cpp
	$(CPP) -c Data/SongSelection.cpp -o $@ $(CXXFLAGS)

Data/Track.o : Data/Track.cpp
	$(CPP) -c Data/Track.cpp -o $@ $(CXXFLAGS)

Device/DeviceManager.o : Device/DeviceManager.cpp
	$(CPP) -c Device/DeviceManager.cpp -o $@ $(CXXFLAGS)

Device/Device.o : Device/Device.cpp
	$(CPP) -c Device/Device.cpp -o $@ $(CXXFLAGS)

Device/InputStreamAudio.o : Device/InputStreamAudio.cpp
	$(CPP) -c Device/InputStreamAudio.cpp -o $@ $(CXXFLAGS)

Device/InputStreamMidi.o : Device/InputStreamMidi.cpp
	$(CPP) -c Device/InputStreamMidi.cpp -o $@ $(CXXFLAGS)

Device/OutputStream.o : Device/OutputStream.cpp
	$(CPP) -c Device/OutputStream.cpp -o $@ $(CXXFLAGS)

lib/base/array.o : lib/base/array.cpp
	$(CPP) -c lib/base/array.cpp -o $@ $(CXXFLAGS)

lib/base/strings.o : lib/base/strings.cpp
	$(CPP) -c lib/base/strings.cpp -o $@ $(CXXFLAGS)

lib/file/file.o : lib/file/file.cpp
	$(CPP) -c lib/file/file.cpp -o $@ $(CXXFLAGS)

lib/file/file_op.o : lib/file/file_op.cpp
	$(CPP) -c lib/file/file_op.cpp -o $@ $(CXXFLAGS)

lib/file/msg.o : lib/file/msg.cpp
	$(CPP) -c lib/file/msg.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/Control.o : lib/hui/Controls/Control.cpp
	$(CPP) -c lib/hui/Controls/Control.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlButtonGtk.o : lib/hui/Controls/ControlButtonGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlButtonGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlCheckBoxGtk.o : lib/hui/Controls/ControlCheckBoxGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlCheckBoxGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlColorButtonGtk.o : lib/hui/Controls/ControlColorButtonGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlColorButtonGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlComboBoxGtk.o : lib/hui/Controls/ControlComboBoxGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlComboBoxGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlDrawingAreaGtk.o : lib/hui/Controls/ControlDrawingAreaGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlDrawingAreaGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlEditGtk.o : lib/hui/Controls/ControlEditGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlEditGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlExpanderGtk.o : lib/hui/Controls/ControlExpanderGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlExpanderGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlGridGtk.o : lib/hui/Controls/ControlGridGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlGridGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlGroupGtk.o : lib/hui/Controls/ControlGroupGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlGroupGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlLabelGtk.o : lib/hui/Controls/ControlLabelGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlLabelGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlListViewGtk.o : lib/hui/Controls/ControlListViewGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlListViewGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlMultilineEditGtk.o : lib/hui/Controls/ControlMultilineEditGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlMultilineEditGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlPanedGtk.o : lib/hui/Controls/ControlPanedGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlPanedGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlProgressBarGtk.o : lib/hui/Controls/ControlProgressBarGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlProgressBarGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlRadioButtonGtk.o : lib/hui/Controls/ControlRadioButtonGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlRadioButtonGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlRevealerGtk.o : lib/hui/Controls/ControlRevealerGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlRevealerGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlScrollerGtk.o : lib/hui/Controls/ControlScrollerGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlScrollerGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlSeparatorGtk.o : lib/hui/Controls/ControlSeparatorGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlSeparatorGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlSliderGtk.o : lib/hui/Controls/ControlSliderGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlSliderGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlSpinButtonGtk.o : lib/hui/Controls/ControlSpinButtonGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlSpinButtonGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlTabControlGtk.o : lib/hui/Controls/ControlTabControlGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlTabControlGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlToggleButtonGtk.o : lib/hui/Controls/ControlToggleButtonGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlToggleButtonGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ControlTreeViewGtk.o : lib/hui/Controls/ControlTreeViewGtk.cpp
	$(CPP) -c lib/hui/Controls/ControlTreeViewGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/MenuItemGtk.o : lib/hui/Controls/MenuItemGtk.cpp
	$(CPP) -c lib/hui/Controls/MenuItemGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/MenuItemSeparatorGtk.o : lib/hui/Controls/MenuItemSeparatorGtk.cpp
	$(CPP) -c lib/hui/Controls/MenuItemSeparatorGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/MenuItemSubmenuGtk.o : lib/hui/Controls/MenuItemSubmenuGtk.cpp
	$(CPP) -c lib/hui/Controls/MenuItemSubmenuGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/MenuItemToggleGtk.o : lib/hui/Controls/MenuItemToggleGtk.cpp
	$(CPP) -c lib/hui/Controls/MenuItemToggleGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ToolItemButtonGtk.o : lib/hui/Controls/ToolItemButtonGtk.cpp
	$(CPP) -c lib/hui/Controls/ToolItemButtonGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ToolItemMenuButtonGtk.o : lib/hui/Controls/ToolItemMenuButtonGtk.cpp
	$(CPP) -c lib/hui/Controls/ToolItemMenuButtonGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ToolItemSeparatorGtk.o : lib/hui/Controls/ToolItemSeparatorGtk.cpp
	$(CPP) -c lib/hui/Controls/ToolItemSeparatorGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/ToolItemToggleButtonGtk.o : lib/hui/Controls/ToolItemToggleButtonGtk.cpp
	$(CPP) -c lib/hui/Controls/ToolItemToggleButtonGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/hui.o : lib/hui/hui.cpp
	$(CPP) -c lib/hui/hui.cpp -o $@ $(CXXFLAGS)

lib/hui/common_dlg.o : lib/hui/common_dlg.cpp
	$(CPP) -c lib/hui/common_dlg.cpp -o $@ $(CXXFLAGS)

lib/hui/common_dlg_gtk.o : lib/hui/common_dlg_gtk.cpp
	$(CPP) -c lib/hui/common_dlg_gtk.cpp -o $@ $(CXXFLAGS)

lib/hui/common_dlg_win.o : lib/hui/common_dlg_win.cpp
	$(CPP) -c lib/hui/common_dlg_win.cpp -o $@ $(CXXFLAGS)

lib/hui/error.o : lib/hui/error.cpp
	$(CPP) -c lib/hui/error.cpp -o $@ $(CXXFLAGS)

lib/hui/language.o : lib/hui/language.cpp
	$(CPP) -c lib/hui/language.cpp -o $@ $(CXXFLAGS)

lib/hui/main_gtk.o : lib/hui/main_gtk.cpp
	$(CPP) -c lib/hui/main_gtk.cpp -o $@ $(CXXFLAGS)

lib/hui/main_win.o : lib/hui/main_win.cpp
	$(CPP) -c lib/hui/main_win.cpp -o $@ $(CXXFLAGS)

lib/hui/utility.o : lib/hui/utility.cpp
	$(CPP) -c lib/hui/utility.cpp -o $@ $(CXXFLAGS)

lib/hui/window_control.o : lib/hui/window_control.cpp
	$(CPP) -c lib/hui/window_control.cpp -o $@ $(CXXFLAGS)

lib/hui/window_control_gtk.o : lib/hui/window_control_gtk.cpp
	$(CPP) -c lib/hui/window_control_gtk.cpp -o $@ $(CXXFLAGS)

lib/hui/window_control_win.o : lib/hui/window_control_win.cpp
	$(CPP) -c lib/hui/window_control_win.cpp -o $@ $(CXXFLAGS)

lib/hui/Callback.o : lib/hui/Callback.cpp
	$(CPP) -c lib/hui/Callback.cpp -o $@ $(CXXFLAGS)

lib/hui/Clipboard.o : lib/hui/Clipboard.cpp
	$(CPP) -c lib/hui/Clipboard.cpp -o $@ $(CXXFLAGS)

lib/hui/Config.o : lib/hui/Config.cpp
	$(CPP) -c lib/hui/Config.cpp -o $@ $(CXXFLAGS)

lib/hui/Event.o : lib/hui/Event.cpp
	$(CPP) -c lib/hui/Event.cpp -o $@ $(CXXFLAGS)

lib/hui/Menu.o : lib/hui/Menu.cpp
	$(CPP) -c lib/hui/Menu.cpp -o $@ $(CXXFLAGS)

lib/hui/MenuGtk.o : lib/hui/MenuGtk.cpp
	$(CPP) -c lib/hui/MenuGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/MenuWin.o : lib/hui/MenuWin.cpp
	$(CPP) -c lib/hui/MenuWin.cpp -o $@ $(CXXFLAGS)

lib/hui/PainterGtk.o : lib/hui/PainterGtk.cpp
	$(CPP) -c lib/hui/PainterGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Panel.o : lib/hui/Panel.cpp
	$(CPP) -c lib/hui/Panel.cpp -o $@ $(CXXFLAGS)

lib/hui/Resource.o : lib/hui/Resource.cpp
	$(CPP) -c lib/hui/Resource.cpp -o $@ $(CXXFLAGS)

lib/hui/Timer.o : lib/hui/Timer.cpp
	$(CPP) -c lib/hui/Timer.cpp -o $@ $(CXXFLAGS)

lib/hui/Toolbar.o : lib/hui/Toolbar.cpp
	$(CPP) -c lib/hui/Toolbar.cpp -o $@ $(CXXFLAGS)

lib/hui/ToolbarGtk.o : lib/hui/ToolbarGtk.cpp
	$(CPP) -c lib/hui/ToolbarGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/ToolbarWin.o : lib/hui/ToolbarWin.cpp
	$(CPP) -c lib/hui/ToolbarWin.cpp -o $@ $(CXXFLAGS)

lib/hui/Window.o : lib/hui/Window.cpp
	$(CPP) -c lib/hui/Window.cpp -o $@ $(CXXFLAGS)

lib/hui/WindowGtk.o : lib/hui/WindowGtk.cpp
	$(CPP) -c lib/hui/WindowGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/WindowWin.o : lib/hui/WindowWin.cpp
	$(CPP) -c lib/hui/WindowWin.cpp -o $@ $(CXXFLAGS)

lib/image/color.o : lib/image/color.cpp
	$(CPP) -c lib/image/color.cpp -o $@ $(CXXFLAGS)

lib/image/image.o : lib/image/image.cpp
	$(CPP) -c lib/image/image.cpp -o $@ $(CXXFLAGS)

lib/image/image_bmp.o : lib/image/image_bmp.cpp
	$(CPP) -c lib/image/image_bmp.cpp -o $@ $(CXXFLAGS)

lib/image/image_jpg.o : lib/image/image_jpg.cpp
	$(CPP) -c lib/image/image_jpg.cpp -o $@ $(CXXFLAGS)

lib/image/image_png.o : lib/image/image_png.cpp
	$(CPP) -c lib/image/image_png.cpp -o $@ $(CXXFLAGS)

lib/image/image_tga.o : lib/image/image_tga.cpp
	$(CPP) -c lib/image/image_tga.cpp -o $@ $(CXXFLAGS)

lib/image/ImagePainter.o : lib/image/ImagePainter.cpp
	$(CPP) -c lib/image/ImagePainter.cpp -o $@ $(CXXFLAGS)

lib/kaba/asm/asm.o : lib/kaba/asm/asm.cpp
	$(CPP) -c lib/kaba/asm/asm.cpp -o $@ $(CXXFLAGS)

lib/kaba/compiler/compiler.o : lib/kaba/compiler/compiler.cpp
	$(CPP) -c lib/kaba/compiler/compiler.cpp -o $@ $(CXXFLAGS)

lib/kaba/compiler/serializer.o : lib/kaba/compiler/serializer.cpp
	$(CPP) -c lib/kaba/compiler/serializer.cpp -o $@ $(CXXFLAGS)

lib/kaba/compiler/serializer_amd64.o : lib/kaba/compiler/serializer_amd64.cpp
	$(CPP) -c lib/kaba/compiler/serializer_amd64.cpp -o $@ $(CXXFLAGS)

lib/kaba/compiler/serializer_arm.o : lib/kaba/compiler/serializer_arm.cpp
	$(CPP) -c lib/kaba/compiler/serializer_arm.cpp -o $@ $(CXXFLAGS)

lib/kaba/compiler/serializer_x86.o : lib/kaba/compiler/serializer_x86.cpp
	$(CPP) -c lib/kaba/compiler/serializer_x86.cpp -o $@ $(CXXFLAGS)

lib/kaba/lib/lib.o : lib/kaba/lib/lib.cpp
	$(CPP) -c lib/kaba/lib/lib.cpp -o $@ $(CXXFLAGS)

lib/kaba/lib/lib_file.o : lib/kaba/lib/lib_file.cpp
	$(CPP) -c lib/kaba/lib/lib_file.cpp -o $@ $(CXXFLAGS)

lib/kaba/lib/lib_hui.o : lib/kaba/lib/lib_hui.cpp
	$(CPP) -c lib/kaba/lib/lib_hui.cpp -o $@ $(CXXFLAGS)

lib/kaba/lib/lib_image.o : lib/kaba/lib/lib_image.cpp
	$(CPP) -c lib/kaba/lib/lib_image.cpp -o $@ $(CXXFLAGS)

lib/kaba/lib/lib_math.o : lib/kaba/lib/lib_math.cpp
	$(CPP) -c lib/kaba/lib/lib_math.cpp -o $@ $(CXXFLAGS)

lib/kaba/lib/lib_net.o : lib/kaba/lib/lib_net.cpp
	$(CPP) -c lib/kaba/lib/lib_net.cpp -o $@ $(CXXFLAGS)

lib/kaba/lib/lib_nix.o : lib/kaba/lib/lib_nix.cpp
	$(CPP) -c lib/kaba/lib/lib_nix.cpp -o $@ $(CXXFLAGS)

lib/kaba/lib/lib_sound.o : lib/kaba/lib/lib_sound.cpp
	$(CPP) -c lib/kaba/lib/lib_sound.cpp -o $@ $(CXXFLAGS)

lib/kaba/lib/lib_threads.o : lib/kaba/lib/lib_threads.cpp
	$(CPP) -c lib/kaba/lib/lib_threads.cpp -o $@ $(CXXFLAGS)

lib/kaba/lib/lib_x.o : lib/kaba/lib/lib_x.cpp
	$(CPP) -c lib/kaba/lib/lib_x.cpp -o $@ $(CXXFLAGS)

lib/kaba/syntax/class.o : lib/kaba/syntax/class.cpp
	$(CPP) -c lib/kaba/syntax/class.cpp -o $@ $(CXXFLAGS)

lib/kaba/syntax/implicit.o : lib/kaba/syntax/implicit.cpp
	$(CPP) -c lib/kaba/syntax/implicit.cpp -o $@ $(CXXFLAGS)

lib/kaba/syntax/lexical.o : lib/kaba/syntax/lexical.cpp
	$(CPP) -c lib/kaba/syntax/lexical.cpp -o $@ $(CXXFLAGS)

lib/kaba/syntax/parser.o : lib/kaba/syntax/parser.cpp
	$(CPP) -c lib/kaba/syntax/parser.cpp -o $@ $(CXXFLAGS)

lib/kaba/syntax/precompiler.o : lib/kaba/syntax/precompiler.cpp
	$(CPP) -c lib/kaba/syntax/precompiler.cpp -o $@ $(CXXFLAGS)

lib/kaba/syntax/preprocessor.o : lib/kaba/syntax/preprocessor.cpp
	$(CPP) -c lib/kaba/syntax/preprocessor.cpp -o $@ $(CXXFLAGS)

lib/kaba/syntax/syntax_tree.o : lib/kaba/syntax/syntax_tree.cpp
	$(CPP) -c lib/kaba/syntax/syntax_tree.cpp -o $@ $(CXXFLAGS)

lib/kaba/kaba.o : lib/kaba/kaba.cpp
	$(CPP) -c lib/kaba/kaba.cpp -o $@ $(CXXFLAGS)

lib/math/complex.o : lib/math/complex.cpp
	$(CPP) -c lib/math/complex.cpp -o $@ $(CXXFLAGS)

lib/math/interpolation.o : lib/math/interpolation.cpp
	$(CPP) -c lib/math/interpolation.cpp -o $@ $(CXXFLAGS)

lib/math/math.o : lib/math/math.cpp
	$(CPP) -c lib/math/math.cpp -o $@ $(CXXFLAGS)

lib/math/matrix.o : lib/math/matrix.cpp
	$(CPP) -c lib/math/matrix.cpp -o $@ $(CXXFLAGS)

lib/math/matrix3.o : lib/math/matrix3.cpp
	$(CPP) -c lib/math/matrix3.cpp -o $@ $(CXXFLAGS)

lib/math/plane.o : lib/math/plane.cpp
	$(CPP) -c lib/math/plane.cpp -o $@ $(CXXFLAGS)

lib/math/quaternion.o : lib/math/quaternion.cpp
	$(CPP) -c lib/math/quaternion.cpp -o $@ $(CXXFLAGS)

lib/math/random.o : lib/math/random.cpp
	$(CPP) -c lib/math/random.cpp -o $@ $(CXXFLAGS)

lib/math/ray.o : lib/math/ray.cpp
	$(CPP) -c lib/math/ray.cpp -o $@ $(CXXFLAGS)

lib/math/rect.o : lib/math/rect.cpp
	$(CPP) -c lib/math/rect.cpp -o $@ $(CXXFLAGS)

lib/math/vector.o : lib/math/vector.cpp
	$(CPP) -c lib/math/vector.cpp -o $@ $(CXXFLAGS)

lib/net/net.o : lib/net/net.cpp
	$(CPP) -c lib/net/net.cpp -o $@ $(CXXFLAGS)

lib/threads/Mutex.o : lib/threads/Mutex.cpp
	$(CPP) -c lib/threads/Mutex.cpp -o $@ $(CXXFLAGS)

lib/threads/Thread.o : lib/threads/Thread.cpp
	$(CPP) -c lib/threads/Thread.cpp -o $@ $(CXXFLAGS)

lib/threads/ThreadedWork.o : lib/threads/ThreadedWork.cpp
	$(CPP) -c lib/threads/ThreadedWork.cpp -o $@ $(CXXFLAGS)

lib/xfile/chunked.o : lib/xfile/chunked.cpp
	$(CPP) -c lib/xfile/chunked.cpp -o $@ $(CXXFLAGS)

Midi/Clef.o : Midi/Clef.cpp
	$(CPP) -c Midi/Clef.cpp -o $@ $(CXXFLAGS)

Midi/Instrument.o : Midi/Instrument.cpp
	$(CPP) -c Midi/Instrument.cpp -o $@ $(CXXFLAGS)

Midi/MidiData.o : Midi/MidiData.cpp
	$(CPP) -c Midi/MidiData.cpp -o $@ $(CXXFLAGS)

Midi/MidiEvent.o : Midi/MidiEvent.cpp
	$(CPP) -c Midi/MidiEvent.cpp -o $@ $(CXXFLAGS)

Midi/MidiEventStreamer.o : Midi/MidiEventStreamer.cpp
	$(CPP) -c Midi/MidiEventStreamer.cpp -o $@ $(CXXFLAGS)

Midi/MidiNote.o : Midi/MidiNote.cpp
	$(CPP) -c Midi/MidiNote.cpp -o $@ $(CXXFLAGS)

Midi/MidiSource.o : Midi/MidiSource.cpp
	$(CPP) -c Midi/MidiSource.cpp -o $@ $(CXXFLAGS)

Midi/Scale.o : Midi/Scale.cpp
	$(CPP) -c Midi/Scale.cpp -o $@ $(CXXFLAGS)

Plugins/Configurable.o : Plugins/Configurable.cpp
	$(CPP) -c Plugins/Configurable.cpp -o $@ $(CXXFLAGS)

Plugins/AutoConfigPanel.o : Plugins/AutoConfigPanel.cpp
	$(CPP) -c Plugins/AutoConfigPanel.cpp -o $@ $(CXXFLAGS)

Plugins/ConfigPanel.o : Plugins/ConfigPanel.cpp
	$(CPP) -c Plugins/ConfigPanel.cpp -o $@ $(CXXFLAGS)

Plugins/Effect.o : Plugins/Effect.cpp
	$(CPP) -c Plugins/Effect.cpp -o $@ $(CXXFLAGS)

Plugins/EffectManager.o : Plugins/EffectManager.cpp
	$(CPP) -c Plugins/EffectManager.cpp -o $@ $(CXXFLAGS)

Plugins/ExtendedAudioBuffer.o : Plugins/ExtendedAudioBuffer.cpp
	$(CPP) -c Plugins/ExtendedAudioBuffer.cpp -o $@ $(CXXFLAGS)

Plugins/FastFourierTransform.o : Plugins/FastFourierTransform.cpp
	$(CPP) -c Plugins/FastFourierTransform.cpp -o $@ $(CXXFLAGS)

Plugins/FavoriteManager.o : Plugins/FavoriteManager.cpp
	$(CPP) -c Plugins/FavoriteManager.cpp -o $@ $(CXXFLAGS)

Plugins/MidiEffect.o : Plugins/MidiEffect.cpp
	$(CPP) -c Plugins/MidiEffect.cpp -o $@ $(CXXFLAGS)

Plugins/Plugin.o : Plugins/Plugin.cpp
	$(CPP) -c Plugins/Plugin.cpp -o $@ $(CXXFLAGS)

Plugins/SongPlugin.o : Plugins/SongPlugin.cpp
	$(CPP) -c Plugins/SongPlugin.cpp -o $@ $(CXXFLAGS)

Plugins/TsunamiPlugin.o : Plugins/TsunamiPlugin.cpp
	$(CPP) -c Plugins/TsunamiPlugin.cpp -o $@ $(CXXFLAGS)

Plugins/PluginManager.o : Plugins/PluginManager.cpp
	$(CPP) -c Plugins/PluginManager.cpp -o $@ $(CXXFLAGS)

Rhythm/Bar.o : Rhythm/Bar.cpp
	$(CPP) -c Rhythm/Bar.cpp -o $@ $(CXXFLAGS)

Rhythm/BarCollection.o : Rhythm/BarCollection.cpp
	$(CPP) -c Rhythm/BarCollection.cpp -o $@ $(CXXFLAGS)

Rhythm/BarStreamer.o : Rhythm/BarStreamer.cpp
	$(CPP) -c Rhythm/BarStreamer.cpp -o $@ $(CXXFLAGS)

Rhythm/Beat.o : Rhythm/Beat.cpp
	$(CPP) -c Rhythm/Beat.cpp -o $@ $(CXXFLAGS)

Rhythm/BeatSource.o : Rhythm/BeatSource.cpp
	$(CPP) -c Rhythm/BeatSource.cpp -o $@ $(CXXFLAGS)

Storage/Dialog/RawConfigDialog.o : Storage/Dialog/RawConfigDialog.cpp
	$(CPP) -c Storage/Dialog/RawConfigDialog.cpp -o $@ $(CXXFLAGS)

Storage/Format/Format.o : Storage/Format/Format.cpp
	$(CPP) -c Storage/Format/Format.cpp -o $@ $(CXXFLAGS)

Storage/Format/FormatFlac.o : Storage/Format/FormatFlac.cpp
	$(CPP) -c Storage/Format/FormatFlac.cpp -o $@ $(CXXFLAGS)

Storage/Format/FormatGuitarPro.o : Storage/Format/FormatGuitarPro.cpp
	$(CPP) -c Storage/Format/FormatGuitarPro.cpp -o $@ $(CXXFLAGS)

Storage/Format/FormatMidi.o : Storage/Format/FormatMidi.cpp
	$(CPP) -c Storage/Format/FormatMidi.cpp -o $@ $(CXXFLAGS)

Storage/Format/FormatM4a.o : Storage/Format/FormatM4a.cpp
	$(CPP) -c Storage/Format/FormatM4a.cpp -o $@ $(CXXFLAGS)

Storage/Format/FormatMp3.o : Storage/Format/FormatMp3.cpp
	$(CPP) -c Storage/Format/FormatMp3.cpp -o $@ $(CXXFLAGS)

Storage/Format/FormatNami.o : Storage/Format/FormatNami.cpp
	$(CPP) -c Storage/Format/FormatNami.cpp -o $@ $(CXXFLAGS)

Storage/Format/FormatOgg.o : Storage/Format/FormatOgg.cpp
	$(CPP) -c Storage/Format/FormatOgg.cpp -o $@ $(CXXFLAGS)

Storage/Format/FormatRaw.o : Storage/Format/FormatRaw.cpp
	$(CPP) -c Storage/Format/FormatRaw.cpp -o $@ $(CXXFLAGS)

Storage/Format/FormatSoundFont2.o : Storage/Format/FormatSoundFont2.cpp
	$(CPP) -c Storage/Format/FormatSoundFont2.cpp -o $@ $(CXXFLAGS)

Storage/Format/FormatWave.o : Storage/Format/FormatWave.cpp
	$(CPP) -c Storage/Format/FormatWave.cpp -o $@ $(CXXFLAGS)

Storage/Storage.o : Storage/Storage.cpp
	$(CPP) -c Storage/Storage.cpp -o $@ $(CXXFLAGS)

Storage/StorageOperationData.o : Storage/StorageOperationData.cpp
	$(CPP) -c Storage/StorageOperationData.cpp -o $@ $(CXXFLAGS)

Stuff/Clipboard.o : Stuff/Clipboard.cpp
	$(CPP) -c Stuff/Clipboard.cpp -o $@ $(CXXFLAGS)

Stuff/Log.o : Stuff/Log.cpp
	$(CPP) -c Stuff/Log.cpp -o $@ $(CXXFLAGS)

Stuff/Observable.o : Stuff/Observable.cpp
	$(CPP) -c Stuff/Observable.cpp -o $@ $(CXXFLAGS)

Stuff/PerformanceMonitor.o : Stuff/PerformanceMonitor.cpp
	$(CPP) -c Stuff/PerformanceMonitor.cpp -o $@ $(CXXFLAGS)

View/BottomBar/BottomBar.o : View/BottomBar/BottomBar.cpp
	$(CPP) -c View/BottomBar/BottomBar.cpp -o $@ $(CXXFLAGS)

View/BottomBar/DeviceConsole.o : View/BottomBar/DeviceConsole.cpp
	$(CPP) -c View/BottomBar/DeviceConsole.cpp -o $@ $(CXXFLAGS)

View/BottomBar/LogConsole.o : View/BottomBar/LogConsole.cpp
	$(CPP) -c View/BottomBar/LogConsole.cpp -o $@ $(CXXFLAGS)

View/BottomBar/MiniBar.o : View/BottomBar/MiniBar.cpp
	$(CPP) -c View/BottomBar/MiniBar.cpp -o $@ $(CXXFLAGS)

View/BottomBar/MixingConsole.o : View/BottomBar/MixingConsole.cpp
	$(CPP) -c View/BottomBar/MixingConsole.cpp -o $@ $(CXXFLAGS)

View/Dialog/BarAddDialog.o : View/Dialog/BarAddDialog.cpp
	$(CPP) -c View/Dialog/BarAddDialog.cpp -o $@ $(CXXFLAGS)

View/Dialog/BarEditDialog.o : View/Dialog/BarEditDialog.cpp
	$(CPP) -c View/Dialog/BarEditDialog.cpp -o $@ $(CXXFLAGS)

View/Dialog/ConfigurableSelectorDialog.o : View/Dialog/ConfigurableSelectorDialog.cpp
	$(CPP) -c View/Dialog/ConfigurableSelectorDialog.cpp -o $@ $(CXXFLAGS)

View/Dialog/DetuneSynthesizerDialog.o : View/Dialog/DetuneSynthesizerDialog.cpp
	$(CPP) -c View/Dialog/DetuneSynthesizerDialog.cpp -o $@ $(CXXFLAGS)

View/Dialog/MarkerDialog.o : View/Dialog/MarkerDialog.cpp
	$(CPP) -c View/Dialog/MarkerDialog.cpp -o $@ $(CXXFLAGS)

View/Dialog/NewDialog.o : View/Dialog/NewDialog.cpp
	$(CPP) -c View/Dialog/NewDialog.cpp -o $@ $(CXXFLAGS)

View/Dialog/PauseAddDialog.o : View/Dialog/PauseAddDialog.cpp
	$(CPP) -c View/Dialog/PauseAddDialog.cpp -o $@ $(CXXFLAGS)

View/Dialog/PauseEditDialog.o : View/Dialog/PauseEditDialog.cpp
	$(CPP) -c View/Dialog/PauseEditDialog.cpp -o $@ $(CXXFLAGS)

View/Dialog/RecordDialog.o : View/Dialog/RecordDialog.cpp
	$(CPP) -c View/Dialog/RecordDialog.cpp -o $@ $(CXXFLAGS)

View/Dialog/SampleScaleDialog.o : View/Dialog/SampleScaleDialog.cpp
	$(CPP) -c View/Dialog/SampleScaleDialog.cpp -o $@ $(CXXFLAGS)

View/Dialog/SettingsDialog.o : View/Dialog/SettingsDialog.cpp
	$(CPP) -c View/Dialog/SettingsDialog.cpp -o $@ $(CXXFLAGS)

View/Dialog/TuningDialog.o : View/Dialog/TuningDialog.cpp
	$(CPP) -c View/Dialog/TuningDialog.cpp -o $@ $(CXXFLAGS)

View/Helper/PeakMeter.o : View/Helper/PeakMeter.cpp
	$(CPP) -c View/Helper/PeakMeter.cpp -o $@ $(CXXFLAGS)

View/Helper/Progress.o : View/Helper/Progress.cpp
	$(CPP) -c View/Helper/Progress.cpp -o $@ $(CXXFLAGS)

View/Helper/Slider.o : View/Helper/Slider.cpp
	$(CPP) -c View/Helper/Slider.cpp -o $@ $(CXXFLAGS)

View/Mode/ViewMode.o : View/Mode/ViewMode.cpp
	$(CPP) -c View/Mode/ViewMode.cpp -o $@ $(CXXFLAGS)

View/Mode/ViewModeCapture.o : View/Mode/ViewModeCapture.cpp
	$(CPP) -c View/Mode/ViewModeCapture.cpp -o $@ $(CXXFLAGS)

View/Mode/ViewModeCurve.o : View/Mode/ViewModeCurve.cpp
	$(CPP) -c View/Mode/ViewModeCurve.cpp -o $@ $(CXXFLAGS)

View/Mode/ViewModeDefault.o : View/Mode/ViewModeDefault.cpp
	$(CPP) -c View/Mode/ViewModeDefault.cpp -o $@ $(CXXFLAGS)

View/Mode/ViewModeMidi.o : View/Mode/ViewModeMidi.cpp
	$(CPP) -c View/Mode/ViewModeMidi.cpp -o $@ $(CXXFLAGS)

View/Mode/ViewModeScaleBars.o : View/Mode/ViewModeScaleBars.cpp
	$(CPP) -c View/Mode/ViewModeScaleBars.cpp -o $@ $(CXXFLAGS)

View/SideBar/CaptureConsole.o : View/SideBar/CaptureConsole.cpp
	$(CPP) -c View/SideBar/CaptureConsole.cpp -o $@ $(CXXFLAGS)

View/SideBar/CaptureConsoleModes/CaptureConsoleMode.o : View/SideBar/CaptureConsoleModes/CaptureConsoleMode.cpp
	$(CPP) -c View/SideBar/CaptureConsoleModes/CaptureConsoleMode.cpp -o $@ $(CXXFLAGS)

View/SideBar/CaptureConsoleModes/CaptureConsoleModeAudio.o : View/SideBar/CaptureConsoleModes/CaptureConsoleModeAudio.cpp
	$(CPP) -c View/SideBar/CaptureConsoleModes/CaptureConsoleModeAudio.cpp -o $@ $(CXXFLAGS)

View/SideBar/CaptureConsoleModes/CaptureConsoleModeMidi.o : View/SideBar/CaptureConsoleModes/CaptureConsoleModeMidi.cpp
	$(CPP) -c View/SideBar/CaptureConsoleModes/CaptureConsoleModeMidi.cpp -o $@ $(CXXFLAGS)

View/SideBar/CaptureConsoleModes/CaptureConsoleModeMulti.o : View/SideBar/CaptureConsoleModes/CaptureConsoleModeMulti.cpp
	$(CPP) -c View/SideBar/CaptureConsoleModes/CaptureConsoleModeMulti.cpp -o $@ $(CXXFLAGS)

View/SideBar/CurveConsole.o : View/SideBar/CurveConsole.cpp
	$(CPP) -c View/SideBar/CurveConsole.cpp -o $@ $(CXXFLAGS)

View/SideBar/FxConsole.o : View/SideBar/FxConsole.cpp
	$(CPP) -c View/SideBar/FxConsole.cpp -o $@ $(CXXFLAGS)

View/SideBar/LayerConsole.o : View/SideBar/LayerConsole.cpp
	$(CPP) -c View/SideBar/LayerConsole.cpp -o $@ $(CXXFLAGS)

View/SideBar/MidiEditorConsole.o : View/SideBar/MidiEditorConsole.cpp
	$(CPP) -c View/SideBar/MidiEditorConsole.cpp -o $@ $(CXXFLAGS)

View/SideBar/MidiFxConsole.o : View/SideBar/MidiFxConsole.cpp
	$(CPP) -c View/SideBar/MidiFxConsole.cpp -o $@ $(CXXFLAGS)

View/SideBar/SampleManagerConsole.o : View/SideBar/SampleManagerConsole.cpp
	$(CPP) -c View/SideBar/SampleManagerConsole.cpp -o $@ $(CXXFLAGS)

View/SideBar/SampleRefConsole.o : View/SideBar/SampleRefConsole.cpp
	$(CPP) -c View/SideBar/SampleRefConsole.cpp -o $@ $(CXXFLAGS)

View/SideBar/SideBar.o : View/SideBar/SideBar.cpp
	$(CPP) -c View/SideBar/SideBar.cpp -o $@ $(CXXFLAGS)

View/SideBar/SongConsole.o : View/SideBar/SongConsole.cpp
	$(CPP) -c View/SideBar/SongConsole.cpp -o $@ $(CXXFLAGS)

View/SideBar/SynthConsole.o : View/SideBar/SynthConsole.cpp
	$(CPP) -c View/SideBar/SynthConsole.cpp -o $@ $(CXXFLAGS)

View/SideBar/TrackConsole.o : View/SideBar/TrackConsole.cpp
	$(CPP) -c View/SideBar/TrackConsole.cpp -o $@ $(CXXFLAGS)

View/AudioView.o : View/AudioView.cpp
	$(CPP) -c View/AudioView.cpp -o $@ $(CXXFLAGS)

View/AudioViewTrack.o : View/AudioViewTrack.cpp
	$(CPP) -c View/AudioViewTrack.cpp -o $@ $(CXXFLAGS)

View/ColorScheme.o : View/ColorScheme.cpp
	$(CPP) -c View/ColorScheme.cpp -o $@ $(CXXFLAGS)

View/Selection.o : View/Selection.cpp
	$(CPP) -c View/Selection.cpp -o $@ $(CXXFLAGS)

View/TrackHeightManager.o : View/TrackHeightManager.cpp
	$(CPP) -c View/TrackHeightManager.cpp -o $@ $(CXXFLAGS)

View/ViewPort.o : View/ViewPort.cpp
	$(CPP) -c View/ViewPort.cpp -o $@ $(CXXFLAGS)

Tsunami.o : Tsunami.cpp
	$(CPP) -c Tsunami.cpp -o $@ $(CXXFLAGS)

test : ../Test/ringbuffer

../Test/ringbuffer : all ../Test/src/ringbuffer.cpp
	g++ ../Test/src/ringbuffer.cpp $(OBJ) -o ../Test/ringbuffer $(CXXFLAGSLINKING)

run-test : test
	../Test/ringbuffer

clean :
	rm -f $(BIN)
	find . -name "*.o" -exec rm -f {} \;
	find . -name "*.a" -exec rm -f {} \;

install : all
	mkdir -p /usr/share/tsunami
	mkdir -p /usr/share/tsunami/Data
	cp -f ../Data/hui_resources.txt    /usr/share/tsunami/Data
	cp -f ../Data/icon.*               /usr/share/tsunami/Data
	cp -f ../Data/*.tga                /usr/share/tsunami/Data
	cp -f ../Data/license.txt          /usr/share/tsunami/Data
	cp -f ../Data/license_small.txt    /usr/share/tsunami/Data
	cp -f ../Data/favorites_demo.txt   /usr/share/tsunami/Data
	mkdir -p /usr/share/tsunami/Plugins
	cp -rf ../Plugins/*.kaba           /usr/share/tsunami/Plugins
	cp -rf ../Plugins/Buffer           /usr/share/tsunami/Plugins
	cp -rf ../Plugins/All              /usr/share/tsunami/Plugins
	cp -rf ../Plugins/Midi             /usr/share/tsunami/Plugins
	cp -rf ../Plugins/Independent      /usr/share/tsunami/Plugins
	cp -rf ../Plugins/Synthesizer      /usr/share/tsunami/Plugins
	cp -rf ../Lessons/                 /usr/share/tsunami/
	cp -rf ../Data/Drumsets            /usr/share/tsunami/Data
	xdg-desktop-menu install ../Data/michisoft-tsunami.desktop
	xdg-mime install ../Data/michisoft-nami.xml
	xdg-icon-resource install --context mimetypes --size  32 ../Data/Icons/32.png  application-michisoft-nami --noupdate
	xdg-icon-resource install --context mimetypes --size  48 ../Data/Icons/48.png  application-michisoft-nami --noupdate
	xdg-icon-resource install --context mimetypes --size  64 ../Data/Icons/64.png  application-michisoft-nami --noupdate
	xdg-icon-resource install --context mimetypes --size 128 ../Data/Icons/128.png application-michisoft-nami
	#xdg-icon-resource install --context mimetypes ../Data/Icons/tsunami.svg application-michisoft-nami
	cp $(BIN) /usr/bin

