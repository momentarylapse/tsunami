# Project: Tsunami

BIN = ../tsunami
all : $(BIN)
OBJ = \
 Action/Song/Bar/ActionSongAddBar.o\
 Action/Song/Bar/ActionSongDeleteBar.o\
 Action/Song/Bar/ActionSongEditBar.o\
 Action/Song/Data/ActionSongChangeAllTrackVolumes.o\
 Action/Song/Data/ActionSongSetDefaultFormat.o\
 Action/Song/Data/ActionSongSetSampleRate.o\
 Action/Song/Data/ActionSongSetVolume.o\
 Action/Song/Level/ActionSongAddLevel.o\
 Action/Song/Level/ActionSongRenameLevel.o\
 Action/Song/Sample/ActionSongAddSample.o\
 Action/Song/Sample/ActionSongDeleteSample.o\
 Action/Song/Sample/ActionSongSampleEditName.o\
 Action/Song/Tag/ActionSongAddTag.o\
 Action/Song/Tag/ActionSongDeleteTag.o\
 Action/Song/Tag/ActionSongEditTag.o\
 Action/Song/ActionSongDeleteSelection.o\
 Action/Track/Buffer/ActionTrack__AbsorbBufferBox.o\
 Action/Track/Buffer/ActionTrack__AddBufferBox.o\
 Action/Track/Buffer/ActionTrack__CutBufferBox.o\
 Action/Track/Buffer/ActionTrack__DeleteBufferBox.o\
 Action/Track/Buffer/ActionTrack__GrowBufferBox.o\
 Action/Track/Buffer/ActionTrack__ShrinkBufferBox.o\
 Action/Track/Buffer/ActionTrackCreateBuffers.o\
 Action/Track/Buffer/ActionTrackEditBuffer.o\
 Action/Track/Data/ActionTrackEditMuted.o\
 Action/Track/Data/ActionTrackEditName.o\
 Action/Track/Data/ActionTrackEditPanning.o\
 Action/Track/Data/ActionTrackEditVolume.o\
 Action/Track/Effect/ActionTrackAddEffect.o\
 Action/Track/Effect/ActionTrackDeleteEffect.o\
 Action/Track/Effect/ActionTrackEditEffect.o\
 Action/Track/Effect/ActionTrackToggleEffectEnabled.o\
 Action/Track/Marker/ActionTrackAddMarker.o\
 Action/Track/Marker/ActionTrackDeleteMarker.o\
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
 Action/Track/Synthesizer/ActionTrackEditSynthesizer.o\
 Action/Track/Synthesizer/ActionTrackSetSynthesizer.o\
 Action/Track/ActionTrack__DeleteEmpty.o\
 Action/Track/ActionTrackAdd.o\
 Action/Track/ActionTrackDelete.o\
 Action/Action.o\
 Action/ActionGroup.o\
 Action/ActionManager.o\
 Action/ActionMergable.o\
 Audio/Renderer/AudioRenderer.o\
 Audio/Renderer/BufferRenderer.o\
 Audio/Renderer/SongRenderer.o\
 Audio/Renderer/SynthesizerRenderer.o\
 Audio/Synth/DummySynthesizer.o\
 Audio/Synth/EnvelopeADSR.o\
 Audio/Synth/SampleSynthesizer.o\
 Audio/Synth/Synthesizer.o\
 Audio/AudioInputAny.o\
 Audio/AudioInputAudio.o\
 Audio/AudioInputMidi.o\
 Audio/AudioOutput.o\
 Audio/AudioStream.o\
 Data/BufferBox.o\
 Data/Curve.o\
 Data/Data.o\
 Data/MidiData.o\
 Data/Range.o\
 Data/RingBuffer.o\
 Data/Rhythm.o\
 Data/Sample.o\
 Data/SampleRef.o\
 Data/Song.o\
 Data/Track.o\
 lib/base/array.o\
 lib/base/strings.o\
 lib/file/file.o\
 lib/file/file_op.o\
 lib/file/msg.o\
 lib/hui/Controls/HuiControl.o\
 lib/hui/Controls/HuiControlButtonGtk.o\
 lib/hui/Controls/HuiControlCheckBoxGtk.o\
 lib/hui/Controls/HuiControlColorButtonGtk.o\
 lib/hui/Controls/HuiControlComboBoxGtk.o\
 lib/hui/Controls/HuiControlDrawingAreaGtk.o\
 lib/hui/Controls/HuiControlEditGtk.o\
 lib/hui/Controls/HuiControlExpanderGtk.o\
 lib/hui/Controls/HuiControlGridGtk.o\
 lib/hui/Controls/HuiControlGroupGtk.o\
 lib/hui/Controls/HuiControlLabelGtk.o\
 lib/hui/Controls/HuiControlListViewGtk.o\
 lib/hui/Controls/HuiControlMultilineEditGtk.o\
 lib/hui/Controls/HuiControlPanedGtk.o\
 lib/hui/Controls/HuiControlProgressBarGtk.o\
 lib/hui/Controls/HuiControlRadioButtonGtk.o\
 lib/hui/Controls/HuiControlRevealerGtk.o\
 lib/hui/Controls/HuiControlScrollerGtk.o\
 lib/hui/Controls/HuiControlSeparatorGtk.o\
 lib/hui/Controls/HuiControlSliderGtk.o\
 lib/hui/Controls/HuiControlSpinButtonGtk.o\
 lib/hui/Controls/HuiControlTabControlGtk.o\
 lib/hui/Controls/HuiControlToggleButtonGtk.o\
 lib/hui/Controls/HuiControlTreeViewGtk.o\
 lib/hui/Controls/HuiMenuItemGtk.o\
 lib/hui/Controls/HuiMenuItemSeparatorGtk.o\
 lib/hui/Controls/HuiMenuItemSubmenuGtk.o\
 lib/hui/Controls/HuiMenuItemToggleGtk.o\
 lib/hui/Controls/HuiToolItemButtonGtk.o\
 lib/hui/Controls/HuiToolItemMenuButtonGtk.o\
 lib/hui/Controls/HuiToolItemSeparatorGtk.o\
 lib/hui/Controls/HuiToolItemToggleButtonGtk.o\
 lib/hui/hui.o\
 lib/hui/hui_clipboard.o\
 lib/hui/hui_common_dlg.o\
 lib/hui/hui_common_dlg_gtk.o\
 lib/hui/hui_common_dlg_win.o\
 lib/hui/hui_error.o\
 lib/hui/hui_input.o\
 lib/hui/hui_language.o\
 lib/hui/hui_main_gtk.o\
 lib/hui/hui_main_win.o\
 lib/hui/hui_resource.o\
 lib/hui/hui_utility.o\
 lib/hui/hui_window_control.o\
 lib/hui/hui_window_control_gtk.o\
 lib/hui/hui_window_control_win.o\
 lib/hui/HuiApplication.o\
 lib/hui/HuiConfig.o\
 lib/hui/HuiMenu.o\
 lib/hui/HuiMenuGtk.o\
 lib/hui/HuiMenuWin.o\
 lib/hui/HuiPainterGtk.o\
 lib/hui/HuiPanel.o\
 lib/hui/HuiTimer.o\
 lib/hui/HuiToolbar.o\
 lib/hui/HuiToolbarGtk.o\
 lib/hui/HuiToolbarWin.o\
 lib/hui/HuiWindow.o\
 lib/hui/HuiWindowGtk.o\
 lib/hui/HuiWindowWin.o\
 lib/image/color.o\
 lib/image/image.o\
 lib/image/image_bmp.o\
 lib/image/image_jpg.o\
 lib/image/image_png.o\
 lib/image/image_tga.o\
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
 lib/script/asm/asm.o\
 lib/script/compiler/compiler.o\
 lib/script/compiler/serializer.o\
 lib/script/compiler/serializer_amd64.o\
 lib/script/compiler/serializer_arm.o\
 lib/script/compiler/serializer_x86.o\
 lib/script/lib/script_data.o\
 lib/script/lib/script_data_file.o\
 lib/script/lib/script_data_hui.o\
 lib/script/lib/script_data_image.o\
 lib/script/lib/script_data_math.o\
 lib/script/lib/script_data_net.o\
 lib/script/lib/script_data_nix.o\
 lib/script/lib/script_data_sound.o\
 lib/script/lib/script_data_threads.o\
 lib/script/lib/script_data_x.o\
 lib/script/syntax/implicit.o\
 lib/script/syntax/lexical.o\
 lib/script/syntax/parser.o\
 lib/script/syntax/precompiler.o\
 lib/script/syntax/preprocessor.o\
 lib/script/syntax/syntax_tree.o\
 lib/script/syntax/type.o\
 lib/script/script.o\
 lib/threads/Mutex.o\
 lib/threads/Thread.o\
 lib/threads/ThreadedWork.o\
 Plugins/ConfigPanel.o\
 Plugins/Configurable.o\
 Plugins/Effect.o\
 Plugins/EffectManager.o\
 Plugins/ExtendedBufferBox.o\
 Plugins/FastFourierTransform.o\
 Plugins/FavoriteManager.o\
 Plugins/MidiEffect.o\
 Plugins/Plugin.o\
 Plugins/PluginManager.o\
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
 Stuff/Observer.o\
 View/BottomBar/BottomBar.o\
 View/BottomBar/LogDialog.o\
 View/BottomBar/MiniBar.o\
 View/BottomBar/MixingConsole.o\
 View/Dialog/BarAddDialog.o\
 View/Dialog/BarEditDialog.o\
 View/Dialog/CaptureDialog.o\
 View/Dialog/MarkerDialog.o\
 View/Dialog/NewDialog.o\
 View/Dialog/RecordDialog.o\
 View/Dialog/SettingsDialog.o\
 View/Dialog/ConfigurableSelectorDialog.o\
 View/Helper/PeakMeter.o\
 View/Helper/Progress.o\
 View/Helper/Slider.o\
 View/Mode/ViewMode.o\
 View/Mode/ViewModeBars.o\
 View/Mode/ViewModeCurve.o\
 View/Mode/ViewModeDefault.o\
 View/Mode/ViewModeMidi.o\
 View/SideBar/SideBar.o\
 View/SideBar/BarsConsole.o\
 View/SideBar/CurveConsole.o\
 View/SideBar/FxConsole.o\
 View/SideBar/LevelConsole.o\
 View/SideBar/MidiEditor.o\
 View/SideBar/MidiFxConsole.o\
 View/SideBar/SampleManager.o\
 View/SideBar/SampleRefDialog.o\
 View/SideBar/SongConsole.o\
 View/SideBar/SynthConsole.o\
 View/SideBar/TrackConsole.o\
 View/AudioView.o\
 View/AudioViewTrack.o\
 View/ColorScheme.o\
 View/Selection.o\
 View/TrackHeightManager.o\
 View/ViewPort.o\
 TsunamiWindow.o\
 Tsunami.o
CPP = g++
#GLOBALFLAGS = -O2 -g
GLOBALFLAGS = -O1 -g
LIB_DIR = lib
CXXFLAGS = `pkg-config --cflags gtk+-3.0` $(GLOBALFLAGS)
CXXFLAGSLINKING =  `pkg-config --cflags --libs gtk+-3.0` -lX11 -lz -lpulse -lasound -lfftw3f -lFLAC -logg -lvorbis -lvorbisfile -lvorbisenc $(GLOBALFLAGS)

$(BIN) : $(OBJ)
	$(CPP) $(OBJ) -o $(BIN) $(CXXFLAGSLINKING)

Action/Song/Bar/ActionSongAddBar.o : Action/Song/Bar/ActionSongAddBar.cpp
	$(CPP) -c Action/Song/Bar/ActionSongAddBar.cpp -o $@ $(CXXFLAGS)

Action/Song/Bar/ActionSongDeleteBar.o : Action/Song/Bar/ActionSongDeleteBar.cpp
	$(CPP) -c Action/Song/Bar/ActionSongDeleteBar.cpp -o $@ $(CXXFLAGS)

Action/Song/Bar/ActionSongEditBar.o : Action/Song/Bar/ActionSongEditBar.cpp
	$(CPP) -c Action/Song/Bar/ActionSongEditBar.cpp -o $@ $(CXXFLAGS)

Action/Song/Data/ActionSongChangeAllTrackVolumes.o : Action/Song/Data/ActionSongChangeAllTrackVolumes.cpp
	$(CPP) -c Action/Song/Data/ActionSongChangeAllTrackVolumes.cpp -o $@ $(CXXFLAGS)

Action/Song/Data/ActionSongSetDefaultFormat.o : Action/Song/Data/ActionSongSetDefaultFormat.cpp
	$(CPP) -c Action/Song/Data/ActionSongSetDefaultFormat.cpp -o $@ $(CXXFLAGS)

Action/Song/Data/ActionSongSetSampleRate.o : Action/Song/Data/ActionSongSetSampleRate.cpp
	$(CPP) -c Action/Song/Data/ActionSongSetSampleRate.cpp -o $@ $(CXXFLAGS)

Action/Song/Data/ActionSongSetVolume.o : Action/Song/Data/ActionSongSetVolume.cpp
	$(CPP) -c Action/Song/Data/ActionSongSetVolume.cpp -o $@ $(CXXFLAGS)

Action/Song/Level/ActionSongAddLevel.o : Action/Song/Level/ActionSongAddLevel.cpp
	$(CPP) -c Action/Song/Level/ActionSongAddLevel.cpp -o $@ $(CXXFLAGS)

Action/Song/Level/ActionSongRenameLevel.o : Action/Song/Level/ActionSongRenameLevel.cpp
	$(CPP) -c Action/Song/Level/ActionSongRenameLevel.cpp -o $@ $(CXXFLAGS)

Action/Song/Sample/ActionSongAddSample.o : Action/Song/Sample/ActionSongAddSample.cpp
	$(CPP) -c Action/Song/Sample/ActionSongAddSample.cpp -o $@ $(CXXFLAGS)

Action/Song/Sample/ActionSongDeleteSample.o : Action/Song/Sample/ActionSongDeleteSample.cpp
	$(CPP) -c Action/Song/Sample/ActionSongDeleteSample.cpp -o $@ $(CXXFLAGS)

Action/Song/Sample/ActionSongSampleEditName.o : Action/Song/Sample/ActionSongSampleEditName.cpp
	$(CPP) -c Action/Song/Sample/ActionSongSampleEditName.cpp -o $@ $(CXXFLAGS)

Action/Song/Tag/ActionSongAddTag.o : Action/Song/Tag/ActionSongAddTag.cpp
	$(CPP) -c Action/Song/Tag/ActionSongAddTag.cpp -o $@ $(CXXFLAGS)

Action/Song/Tag/ActionSongDeleteTag.o : Action/Song/Tag/ActionSongDeleteTag.cpp
	$(CPP) -c Action/Song/Tag/ActionSongDeleteTag.cpp -o $@ $(CXXFLAGS)

Action/Song/Tag/ActionSongEditTag.o : Action/Song/Tag/ActionSongEditTag.cpp
	$(CPP) -c Action/Song/Tag/ActionSongEditTag.cpp -o $@ $(CXXFLAGS)

Action/Song/ActionSongDeleteSelection.o : Action/Song/ActionSongDeleteSelection.cpp
	$(CPP) -c Action/Song/ActionSongDeleteSelection.cpp -o $@ $(CXXFLAGS)

Action/Track/Buffer/ActionTrack__AbsorbBufferBox.o : Action/Track/Buffer/ActionTrack__AbsorbBufferBox.cpp
	$(CPP) -c Action/Track/Buffer/ActionTrack__AbsorbBufferBox.cpp -o $@ $(CXXFLAGS)

Action/Track/Buffer/ActionTrack__AddBufferBox.o : Action/Track/Buffer/ActionTrack__AddBufferBox.cpp
	$(CPP) -c Action/Track/Buffer/ActionTrack__AddBufferBox.cpp -o $@ $(CXXFLAGS)

Action/Track/Buffer/ActionTrack__CutBufferBox.o : Action/Track/Buffer/ActionTrack__CutBufferBox.cpp
	$(CPP) -c Action/Track/Buffer/ActionTrack__CutBufferBox.cpp -o $@ $(CXXFLAGS)

Action/Track/Buffer/ActionTrack__DeleteBufferBox.o : Action/Track/Buffer/ActionTrack__DeleteBufferBox.cpp
	$(CPP) -c Action/Track/Buffer/ActionTrack__DeleteBufferBox.cpp -o $@ $(CXXFLAGS)

Action/Track/Buffer/ActionTrack__GrowBufferBox.o : Action/Track/Buffer/ActionTrack__GrowBufferBox.cpp
	$(CPP) -c Action/Track/Buffer/ActionTrack__GrowBufferBox.cpp -o $@ $(CXXFLAGS)

Action/Track/Buffer/ActionTrack__ShrinkBufferBox.o : Action/Track/Buffer/ActionTrack__ShrinkBufferBox.cpp
	$(CPP) -c Action/Track/Buffer/ActionTrack__ShrinkBufferBox.cpp -o $@ $(CXXFLAGS)

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

Action/Action.o : Action/Action.cpp
	$(CPP) -c Action/Action.cpp -o $@ $(CXXFLAGS)

Action/ActionGroup.o : Action/ActionGroup.cpp
	$(CPP) -c Action/ActionGroup.cpp -o $@ $(CXXFLAGS)

Action/ActionManager.o : Action/ActionManager.cpp
	$(CPP) -c Action/ActionManager.cpp -o $@ $(CXXFLAGS)

Action/ActionMergable.o : Action/ActionMergable.cpp
	$(CPP) -c Action/ActionMergable.cpp -o $@ $(CXXFLAGS)

Audio/Renderer/SynthesizerRenderer.o : Audio/Renderer/SynthesizerRenderer.cpp
	$(CPP) -c Audio/Renderer/SynthesizerRenderer.cpp -o $@ $(CXXFLAGS)

Audio/Renderer/AudioRenderer.o : Audio/Renderer/AudioRenderer.cpp
	$(CPP) -c Audio/Renderer/AudioRenderer.cpp -o $@ $(CXXFLAGS)

Audio/Renderer/BufferRenderer.o : Audio/Renderer/BufferRenderer.cpp
	$(CPP) -c Audio/Renderer/BufferRenderer.cpp -o $@ $(CXXFLAGS)

Audio/Renderer/SongRenderer.o : Audio/Renderer/SongRenderer.cpp
	$(CPP) -c Audio/Renderer/SongRenderer.cpp -o $@ $(CXXFLAGS)

Audio/Synth/DummySynthesizer.o : Audio/Synth/DummySynthesizer.cpp
	$(CPP) -c Audio/Synth/DummySynthesizer.cpp -o $@ $(CXXFLAGS)

Audio/Synth/EnvelopeADSR.o : Audio/Synth/EnvelopeADSR.cpp
	$(CPP) -c Audio/Synth/EnvelopeADSR.cpp -o $@ $(CXXFLAGS)

Audio/Synth/SampleSynthesizer.o : Audio/Synth/SampleSynthesizer.cpp
	$(CPP) -c Audio/Synth/SampleSynthesizer.cpp -o $@ $(CXXFLAGS)

Audio/Synth/Synthesizer.o : Audio/Synth/Synthesizer.cpp
	$(CPP) -c Audio/Synth/Synthesizer.cpp -o $@ $(CXXFLAGS)

Audio/AudioInputAny.o : Audio/AudioInputAny.cpp
	$(CPP) -c Audio/AudioInputAny.cpp -o $@ $(CXXFLAGS)

Audio/AudioInputAudio.o : Audio/AudioInputAudio.cpp
	$(CPP) -c Audio/AudioInputAudio.cpp -o $@ $(CXXFLAGS)

Audio/AudioInputMidi.o : Audio/AudioInputMidi.cpp
	$(CPP) -c Audio/AudioInputMidi.cpp -o $@ $(CXXFLAGS)

Audio/AudioOutput.o : Audio/AudioOutput.cpp
	$(CPP) -c Audio/AudioOutput.cpp -o $@ $(CXXFLAGS)

Audio/AudioStream.o : Audio/AudioStream.cpp
	$(CPP) -c Audio/AudioStream.cpp -o $@ $(CXXFLAGS)

Data/BufferBox.o : Data/BufferBox.cpp
	$(CPP) -c Data/BufferBox.cpp -o $@ $(CXXFLAGS)

Data/Curve.o : Data/Curve.cpp
	$(CPP) -c Data/Curve.cpp -o $@ $(CXXFLAGS)

Data/Data.o : Data/Data.cpp
	$(CPP) -c Data/Data.cpp -o $@ $(CXXFLAGS)

Data/MidiData.o : Data/MidiData.cpp
	$(CPP) -c Data/MidiData.cpp -o $@ $(CXXFLAGS)

Data/Range.o : Data/Range.cpp
	$(CPP) -c Data/Range.cpp -o $@ $(CXXFLAGS)

Data/RingBuffer.o : Data/RingBuffer.cpp
	$(CPP) -c Data/RingBuffer.cpp -o $@ $(CXXFLAGS)

Data/Rhythm.o : Data/Rhythm.cpp
	$(CPP) -c Data/Rhythm.cpp -o $@ $(CXXFLAGS)

Data/Sample.o : Data/Sample.cpp
	$(CPP) -c Data/Sample.cpp -o $@ $(CXXFLAGS)

Data/SampleRef.o : Data/SampleRef.cpp
	$(CPP) -c Data/SampleRef.cpp -o $@ $(CXXFLAGS)

Data/Song.o : Data/Song.cpp
	$(CPP) -c Data/Song.cpp -o $@ $(CXXFLAGS)

Data/Track.o : Data/Track.cpp
	$(CPP) -c Data/Track.cpp -o $@ $(CXXFLAGS)

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

lib/hui/Controls/HuiControl.o : lib/hui/Controls/HuiControl.cpp
	$(CPP) -c lib/hui/Controls/HuiControl.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/HuiControlButtonGtk.o : lib/hui/Controls/HuiControlButtonGtk.cpp
	$(CPP) -c lib/hui/Controls/HuiControlButtonGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/HuiControlCheckBoxGtk.o : lib/hui/Controls/HuiControlCheckBoxGtk.cpp
	$(CPP) -c lib/hui/Controls/HuiControlCheckBoxGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/HuiControlColorButtonGtk.o : lib/hui/Controls/HuiControlColorButtonGtk.cpp
	$(CPP) -c lib/hui/Controls/HuiControlColorButtonGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/HuiControlComboBoxGtk.o : lib/hui/Controls/HuiControlComboBoxGtk.cpp
	$(CPP) -c lib/hui/Controls/HuiControlComboBoxGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/HuiControlDrawingAreaGtk.o : lib/hui/Controls/HuiControlDrawingAreaGtk.cpp
	$(CPP) -c lib/hui/Controls/HuiControlDrawingAreaGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/HuiControlEditGtk.o : lib/hui/Controls/HuiControlEditGtk.cpp
	$(CPP) -c lib/hui/Controls/HuiControlEditGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/HuiControlExpanderGtk.o : lib/hui/Controls/HuiControlExpanderGtk.cpp
	$(CPP) -c lib/hui/Controls/HuiControlExpanderGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/HuiControlGridGtk.o : lib/hui/Controls/HuiControlGridGtk.cpp
	$(CPP) -c lib/hui/Controls/HuiControlGridGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/HuiControlGroupGtk.o : lib/hui/Controls/HuiControlGroupGtk.cpp
	$(CPP) -c lib/hui/Controls/HuiControlGroupGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/HuiControlLabelGtk.o : lib/hui/Controls/HuiControlLabelGtk.cpp
	$(CPP) -c lib/hui/Controls/HuiControlLabelGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/HuiControlListViewGtk.o : lib/hui/Controls/HuiControlListViewGtk.cpp
	$(CPP) -c lib/hui/Controls/HuiControlListViewGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/HuiControlMultilineEditGtk.o : lib/hui/Controls/HuiControlMultilineEditGtk.cpp
	$(CPP) -c lib/hui/Controls/HuiControlMultilineEditGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/HuiControlPanedGtk.o : lib/hui/Controls/HuiControlPanedGtk.cpp
	$(CPP) -c lib/hui/Controls/HuiControlPanedGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/HuiControlProgressBarGtk.o : lib/hui/Controls/HuiControlProgressBarGtk.cpp
	$(CPP) -c lib/hui/Controls/HuiControlProgressBarGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/HuiControlRadioButtonGtk.o : lib/hui/Controls/HuiControlRadioButtonGtk.cpp
	$(CPP) -c lib/hui/Controls/HuiControlRadioButtonGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/HuiControlRevealerGtk.o : lib/hui/Controls/HuiControlRevealerGtk.cpp
	$(CPP) -c lib/hui/Controls/HuiControlRevealerGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/HuiControlScrollerGtk.o : lib/hui/Controls/HuiControlScrollerGtk.cpp
	$(CPP) -c lib/hui/Controls/HuiControlScrollerGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/HuiControlSeparatorGtk.o : lib/hui/Controls/HuiControlSeparatorGtk.cpp
	$(CPP) -c lib/hui/Controls/HuiControlSeparatorGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/HuiControlSliderGtk.o : lib/hui/Controls/HuiControlSliderGtk.cpp
	$(CPP) -c lib/hui/Controls/HuiControlSliderGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/HuiControlSpinButtonGtk.o : lib/hui/Controls/HuiControlSpinButtonGtk.cpp
	$(CPP) -c lib/hui/Controls/HuiControlSpinButtonGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/HuiControlTabControlGtk.o : lib/hui/Controls/HuiControlTabControlGtk.cpp
	$(CPP) -c lib/hui/Controls/HuiControlTabControlGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/HuiControlToggleButtonGtk.o : lib/hui/Controls/HuiControlToggleButtonGtk.cpp
	$(CPP) -c lib/hui/Controls/HuiControlToggleButtonGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/HuiControlTreeViewGtk.o : lib/hui/Controls/HuiControlTreeViewGtk.cpp
	$(CPP) -c lib/hui/Controls/HuiControlTreeViewGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/HuiMenuItemGtk.o : lib/hui/Controls/HuiMenuItemGtk.cpp
	$(CPP) -c lib/hui/Controls/HuiMenuItemGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/HuiMenuItemSeparatorGtk.o : lib/hui/Controls/HuiMenuItemSeparatorGtk.cpp
	$(CPP) -c lib/hui/Controls/HuiMenuItemSeparatorGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/HuiMenuItemSubmenuGtk.o : lib/hui/Controls/HuiMenuItemSubmenuGtk.cpp
	$(CPP) -c lib/hui/Controls/HuiMenuItemSubmenuGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/HuiMenuItemToggleGtk.o : lib/hui/Controls/HuiMenuItemToggleGtk.cpp
	$(CPP) -c lib/hui/Controls/HuiMenuItemToggleGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/HuiToolItemButtonGtk.o : lib/hui/Controls/HuiToolItemButtonGtk.cpp
	$(CPP) -c lib/hui/Controls/HuiToolItemButtonGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/HuiToolItemMenuButtonGtk.o : lib/hui/Controls/HuiToolItemMenuButtonGtk.cpp
	$(CPP) -c lib/hui/Controls/HuiToolItemMenuButtonGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/HuiToolItemSeparatorGtk.o : lib/hui/Controls/HuiToolItemSeparatorGtk.cpp
	$(CPP) -c lib/hui/Controls/HuiToolItemSeparatorGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/Controls/HuiToolItemToggleButtonGtk.o : lib/hui/Controls/HuiToolItemToggleButtonGtk.cpp
	$(CPP) -c lib/hui/Controls/HuiToolItemToggleButtonGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/hui.o : lib/hui/hui.cpp
	$(CPP) -c lib/hui/hui.cpp -o $@ $(CXXFLAGS)

lib/hui/hui_clipboard.o : lib/hui/hui_clipboard.cpp
	$(CPP) -c lib/hui/hui_clipboard.cpp -o $@ $(CXXFLAGS)

lib/hui/hui_common_dlg.o : lib/hui/hui_common_dlg.cpp
	$(CPP) -c lib/hui/hui_common_dlg.cpp -o $@ $(CXXFLAGS)

lib/hui/hui_common_dlg_gtk.o : lib/hui/hui_common_dlg_gtk.cpp
	$(CPP) -c lib/hui/hui_common_dlg_gtk.cpp -o $@ $(CXXFLAGS)

lib/hui/hui_common_dlg_win.o : lib/hui/hui_common_dlg_win.cpp
	$(CPP) -c lib/hui/hui_common_dlg_win.cpp -o $@ $(CXXFLAGS)

lib/hui/hui_error.o : lib/hui/hui_error.cpp
	$(CPP) -c lib/hui/hui_error.cpp -o $@ $(CXXFLAGS)

lib/hui/hui_input.o : lib/hui/hui_input.cpp
	$(CPP) -c lib/hui/hui_input.cpp -o $@ $(CXXFLAGS)

lib/hui/hui_language.o : lib/hui/hui_language.cpp
	$(CPP) -c lib/hui/hui_language.cpp -o $@ $(CXXFLAGS)

lib/hui/hui_main_gtk.o : lib/hui/hui_main_gtk.cpp
	$(CPP) -c lib/hui/hui_main_gtk.cpp -o $@ $(CXXFLAGS)

lib/hui/hui_main_win.o : lib/hui/hui_main_win.cpp
	$(CPP) -c lib/hui/hui_main_win.cpp -o $@ $(CXXFLAGS)

lib/hui/hui_resource.o : lib/hui/hui_resource.cpp
	$(CPP) -c lib/hui/hui_resource.cpp -o $@ $(CXXFLAGS)

lib/hui/hui_utility.o : lib/hui/hui_utility.cpp
	$(CPP) -c lib/hui/hui_utility.cpp -o $@ $(CXXFLAGS)

lib/hui/hui_window_control.o : lib/hui/hui_window_control.cpp
	$(CPP) -c lib/hui/hui_window_control.cpp -o $@ $(CXXFLAGS)

lib/hui/hui_window_control_gtk.o : lib/hui/hui_window_control_gtk.cpp
	$(CPP) -c lib/hui/hui_window_control_gtk.cpp -o $@ $(CXXFLAGS)

lib/hui/hui_window_control_win.o : lib/hui/hui_window_control_win.cpp
	$(CPP) -c lib/hui/hui_window_control_win.cpp -o $@ $(CXXFLAGS)

lib/hui/HuiConfig.o : lib/hui/HuiConfig.cpp
	$(CPP) -c lib/hui/HuiConfig.cpp -o $@ $(CXXFLAGS)

lib/hui/HuiMenu.o : lib/hui/HuiMenu.cpp
	$(CPP) -c lib/hui/HuiMenu.cpp -o $@ $(CXXFLAGS)

lib/hui/HuiMenuGtk.o : lib/hui/HuiMenuGtk.cpp
	$(CPP) -c lib/hui/HuiMenuGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/HuiMenuWin.o : lib/hui/HuiMenuWin.cpp
	$(CPP) -c lib/hui/HuiMenuWin.cpp -o $@ $(CXXFLAGS)

lib/hui/HuiPainterGtk.o : lib/hui/HuiPainterGtk.cpp
	$(CPP) -c lib/hui/HuiPainterGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/HuiPanel.o : lib/hui/HuiPanel.cpp
	$(CPP) -c lib/hui/HuiPanel.cpp -o $@ $(CXXFLAGS)

lib/hui/HuiTimer.o : lib/hui/HuiTimer.cpp
	$(CPP) -c lib/hui/HuiTimer.cpp -o $@ $(CXXFLAGS)

lib/hui/HuiToolbar.o : lib/hui/HuiToolbar.cpp
	$(CPP) -c lib/hui/HuiToolbar.cpp -o $@ $(CXXFLAGS)

lib/hui/HuiToolbarGtk.o : lib/hui/HuiToolbarGtk.cpp
	$(CPP) -c lib/hui/HuiToolbarGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/HuiToolbarWin.o : lib/hui/HuiToolbarWin.cpp
	$(CPP) -c lib/hui/HuiToolbarWin.cpp -o $@ $(CXXFLAGS)

lib/hui/HuiWindow.o : lib/hui/HuiWindow.cpp
	$(CPP) -c lib/hui/HuiWindow.cpp -o $@ $(CXXFLAGS)

lib/hui/HuiWindowGtk.o : lib/hui/HuiWindowGtk.cpp
	$(CPP) -c lib/hui/HuiWindowGtk.cpp -o $@ $(CXXFLAGS)

lib/hui/HuiWindowWin.o : lib/hui/HuiWindowWin.cpp
	$(CPP) -c lib/hui/HuiWindowWin.cpp -o $@ $(CXXFLAGS)

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

lib/script/asm/asm.o : lib/script/asm/asm.cpp
	$(CPP) -c lib/script/asm/asm.cpp -o $@ $(CXXFLAGS)

lib/script/compiler/compiler.o : lib/script/compiler/compiler.cpp
	$(CPP) -c lib/script/compiler/compiler.cpp -o $@ $(CXXFLAGS)

lib/script/compiler/serializer.o : lib/script/compiler/serializer.cpp
	$(CPP) -c lib/script/compiler/serializer.cpp -o $@ $(CXXFLAGS)

lib/script/compiler/serializer_amd64.o : lib/script/compiler/serializer_amd64.cpp
	$(CPP) -c lib/script/compiler/serializer_amd64.cpp -o $@ $(CXXFLAGS)

lib/script/compiler/serializer_arm.o : lib/script/compiler/serializer_arm.cpp
	$(CPP) -c lib/script/compiler/serializer_arm.cpp -o $@ $(CXXFLAGS)

lib/script/compiler/serializer_x86.o : lib/script/compiler/serializer_x86.cpp
	$(CPP) -c lib/script/compiler/serializer_x86.cpp -o $@ $(CXXFLAGS)

lib/script/lib/script_data.o : lib/script/lib/script_data.cpp
	$(CPP) -c lib/script/lib/script_data.cpp -o $@ $(CXXFLAGS)

lib/script/lib/script_data_file.o : lib/script/lib/script_data_file.cpp
	$(CPP) -c lib/script/lib/script_data_file.cpp -o $@ $(CXXFLAGS)

lib/script/lib/script_data_hui.o : lib/script/lib/script_data_hui.cpp
	$(CPP) -c lib/script/lib/script_data_hui.cpp -o $@ $(CXXFLAGS)

lib/script/lib/script_data_image.o : lib/script/lib/script_data_image.cpp
	$(CPP) -c lib/script/lib/script_data_image.cpp -o $@ $(CXXFLAGS)

lib/script/lib/script_data_math.o : lib/script/lib/script_data_math.cpp
	$(CPP) -c lib/script/lib/script_data_math.cpp -o $@ $(CXXFLAGS)

lib/script/lib/script_data_net.o : lib/script/lib/script_data_net.cpp
	$(CPP) -c lib/script/lib/script_data_net.cpp -o $@ $(CXXFLAGS)

lib/script/lib/script_data_nix.o : lib/script/lib/script_data_nix.cpp
	$(CPP) -c lib/script/lib/script_data_nix.cpp -o $@ $(CXXFLAGS)

lib/script/lib/script_data_sound.o : lib/script/lib/script_data_sound.cpp
	$(CPP) -c lib/script/lib/script_data_sound.cpp -o $@ $(CXXFLAGS)

lib/script/lib/script_data_threads.o : lib/script/lib/script_data_threads.cpp
	$(CPP) -c lib/script/lib/script_data_threads.cpp -o $@ $(CXXFLAGS)

lib/script/lib/script_data_x.o : lib/script/lib/script_data_x.cpp
	$(CPP) -c lib/script/lib/script_data_x.cpp -o $@ $(CXXFLAGS)

lib/script/syntax/implicit.o : lib/script/syntax/implicit.cpp
	$(CPP) -c lib/script/syntax/implicit.cpp -o $@ $(CXXFLAGS)

lib/script/syntax/lexical.o : lib/script/syntax/lexical.cpp
	$(CPP) -c lib/script/syntax/lexical.cpp -o $@ $(CXXFLAGS)

lib/script/syntax/parser.o : lib/script/syntax/parser.cpp
	$(CPP) -c lib/script/syntax/parser.cpp -o $@ $(CXXFLAGS)

lib/script/syntax/precompiler.o : lib/script/syntax/precompiler.cpp
	$(CPP) -c lib/script/syntax/precompiler.cpp -o $@ $(CXXFLAGS)

lib/script/syntax/preprocessor.o : lib/script/syntax/preprocessor.cpp
	$(CPP) -c lib/script/syntax/preprocessor.cpp -o $@ $(CXXFLAGS)

lib/script/syntax/syntax_tree.o : lib/script/syntax/syntax_tree.cpp
	$(CPP) -c lib/script/syntax/syntax_tree.cpp -o $@ $(CXXFLAGS)

lib/script/syntax/type.o : lib/script/syntax/type.cpp
	$(CPP) -c lib/script/syntax/type.cpp -o $@ $(CXXFLAGS)

lib/script/script.o : lib/script/script.cpp
	$(CPP) -c lib/script/script.cpp -o $@ $(CXXFLAGS)

lib/threads/Mutex.o : lib/threads/Mutex.cpp
	$(CPP) -c lib/threads/Mutex.cpp -o $@ $(CXXFLAGS)

lib/threads/Thread.o : lib/threads/Thread.cpp
	$(CPP) -c lib/threads/Thread.cpp -o $@ $(CXXFLAGS)

lib/threads/ThreadedWork.o : lib/threads/ThreadedWork.cpp
	$(CPP) -c lib/threads/ThreadedWork.cpp -o $@ $(CXXFLAGS)

Plugins/Configurable.o : Plugins/Configurable.cpp
	$(CPP) -c Plugins/Configurable.cpp -o $@ $(CXXFLAGS)

Plugins/ConfigPanel.o : Plugins/ConfigPanel.cpp
	$(CPP) -c Plugins/ConfigPanel.cpp -o $@ $(CXXFLAGS)

Plugins/Effect.o : Plugins/Effect.cpp
	$(CPP) -c Plugins/Effect.cpp -o $@ $(CXXFLAGS)

Plugins/EffectManager.o : Plugins/EffectManager.cpp
	$(CPP) -c Plugins/EffectManager.cpp -o $@ $(CXXFLAGS)

Plugins/ExtendedBufferBox.o : Plugins/ExtendedBufferBox.cpp
	$(CPP) -c Plugins/ExtendedBufferBox.cpp -o $@ $(CXXFLAGS)

Plugins/FastFourierTransform.o : Plugins/FastFourierTransform.cpp
	$(CPP) -c Plugins/FastFourierTransform.cpp -o $@ $(CXXFLAGS)

Plugins/FavoriteManager.o : Plugins/FavoriteManager.cpp
	$(CPP) -c Plugins/FavoriteManager.cpp -o $@ $(CXXFLAGS)

Plugins/MidiEffect.o : Plugins/MidiEffect.cpp
	$(CPP) -c Plugins/MidiEffect.cpp -o $@ $(CXXFLAGS)

Plugins/Plugin.o : Plugins/Plugin.cpp
	$(CPP) -c Plugins/Plugin.cpp -o $@ $(CXXFLAGS)

Plugins/PluginManager.o : Plugins/PluginManager.cpp
	$(CPP) -c Plugins/PluginManager.cpp -o $@ $(CXXFLAGS)

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

Stuff/Observer.o : Stuff/Observer.cpp
	$(CPP) -c Stuff/Observer.cpp -o $@ $(CXXFLAGS)

View/BottomBar/BottomBar.o : View/BottomBar/BottomBar.cpp
	$(CPP) -c View/BottomBar/BottomBar.cpp -o $@ $(CXXFLAGS)

View/BottomBar/LogDialog.o : View/BottomBar/LogDialog.cpp
	$(CPP) -c View/BottomBar/LogDialog.cpp -o $@ $(CXXFLAGS)

View/BottomBar/MiniBar.o : View/BottomBar/MiniBar.cpp
	$(CPP) -c View/BottomBar/MiniBar.cpp -o $@ $(CXXFLAGS)

View/BottomBar/MixingConsole.o : View/BottomBar/MixingConsole.cpp
	$(CPP) -c View/BottomBar/MixingConsole.cpp -o $@ $(CXXFLAGS)

View/Dialog/BarAddDialog.o : View/Dialog/BarAddDialog.cpp
	$(CPP) -c View/Dialog/BarAddDialog.cpp -o $@ $(CXXFLAGS)

View/Dialog/BarEditDialog.o : View/Dialog/BarEditDialog.cpp
	$(CPP) -c View/Dialog/BarEditDialog.cpp -o $@ $(CXXFLAGS)

View/Dialog/CaptureDialog.o : View/Dialog/CaptureDialog.cpp
	$(CPP) -c View/Dialog/CaptureDialog.cpp -o $@ $(CXXFLAGS)

View/Dialog/MarkerDialog.o : View/Dialog/MarkerDialog.cpp
	$(CPP) -c View/Dialog/MarkerDialog.cpp -o $@ $(CXXFLAGS)

View/Dialog/NewDialog.o : View/Dialog/NewDialog.cpp
	$(CPP) -c View/Dialog/NewDialog.cpp -o $@ $(CXXFLAGS)

View/Dialog/RecordDialog.o : View/Dialog/RecordDialog.cpp
	$(CPP) -c View/Dialog/RecordDialog.cpp -o $@ $(CXXFLAGS)

View/Dialog/SettingsDialog.o : View/Dialog/SettingsDialog.cpp
	$(CPP) -c View/Dialog/SettingsDialog.cpp -o $@ $(CXXFLAGS)

View/Dialog/ConfigurableSelectorDialog.o : View/Dialog/ConfigurableSelectorDialog.cpp
	$(CPP) -c View/Dialog/ConfigurableSelectorDialog.cpp -o $@ $(CXXFLAGS)

View/Helper/PeakMeter.o : View/Helper/PeakMeter.cpp
	$(CPP) -c View/Helper/PeakMeter.cpp -o $@ $(CXXFLAGS)

View/Helper/Progress.o : View/Helper/Progress.cpp
	$(CPP) -c View/Helper/Progress.cpp -o $@ $(CXXFLAGS)

View/Helper/Slider.o : View/Helper/Slider.cpp
	$(CPP) -c View/Helper/Slider.cpp -o $@ $(CXXFLAGS)

View/Mode/ViewMode.o : View/Mode/ViewMode.cpp
	$(CPP) -c View/Mode/ViewMode.cpp -o $@ $(CXXFLAGS)

View/Mode/ViewModeBars.o : View/Mode/ViewModeBars.cpp
	$(CPP) -c View/Mode/ViewModeBars.cpp -o $@ $(CXXFLAGS)

View/Mode/ViewModeCurve.o : View/Mode/ViewModeCurve.cpp
	$(CPP) -c View/Mode/ViewModeCurve.cpp -o $@ $(CXXFLAGS)

View/Mode/ViewModeDefault.o : View/Mode/ViewModeDefault.cpp
	$(CPP) -c View/Mode/ViewModeDefault.cpp -o $@ $(CXXFLAGS)

View/Mode/ViewModeMidi.o : View/Mode/ViewModeMidi.cpp
	$(CPP) -c View/Mode/ViewModeMidi.cpp -o $@ $(CXXFLAGS)

View/SideBar/SideBar.o : View/SideBar/SideBar.cpp
	$(CPP) -c View/SideBar/SideBar.cpp -o $@ $(CXXFLAGS)

View/SideBar/LevelConsole.o : View/SideBar/LevelConsole.cpp
	$(CPP) -c View/SideBar/LevelConsole.cpp -o $@ $(CXXFLAGS)

View/SideBar/MidiEditor.o : View/SideBar/MidiEditor.cpp
	$(CPP) -c View/SideBar/MidiEditor.cpp -o $@ $(CXXFLAGS)

View/SideBar/BarsConsole.o : View/SideBar/BarsConsole.cpp
	$(CPP) -c View/SideBar/BarsConsole.cpp -o $@ $(CXXFLAGS)

View/SideBar/CurveConsole.o : View/SideBar/CurveConsole.cpp
	$(CPP) -c View/SideBar/CurveConsole.cpp -o $@ $(CXXFLAGS)

View/SideBar/FxConsole.o : View/SideBar/FxConsole.cpp
	$(CPP) -c View/SideBar/FxConsole.cpp -o $@ $(CXXFLAGS)

View/SideBar/MidiFxConsole.o : View/SideBar/MidiFxConsole.cpp
	$(CPP) -c View/SideBar/MidiFxConsole.cpp -o $@ $(CXXFLAGS)

View/SideBar/SampleManager.o : View/SideBar/SampleManager.cpp
	$(CPP) -c View/SideBar/SampleManager.cpp -o $@ $(CXXFLAGS)

View/SideBar/SampleRefDialog.o : View/SideBar/SampleRefDialog.cpp
	$(CPP) -c View/SideBar/SampleRefDialog.cpp -o $@ $(CXXFLAGS)

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

main.o : main.cpp
	$(CPP) -c main.cpp -o $@ $(CXXFLAGS)

Tsunami.o : Tsunami.cpp
	$(CPP) -c Tsunami.cpp -o $@ $(CXXFLAGS)

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
