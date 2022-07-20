# created by xmake

project(tsunami)
cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)
set(SOURCE 
	src/action/bar/Action__ScaleData.cpp
	src/action/bar/Action__ShiftData.cpp
	src/action/bar/ActionBar__Add.cpp
	src/action/bar/ActionBar__Delete.cpp
	src/action/bar/ActionBar__Edit.cpp
	src/action/bar/ActionBarAdd.cpp
	src/action/bar/ActionBarDelete.cpp
	src/action/bar/ActionBarEdit.cpp
	src/action/sample/ActionSampleAdd.cpp
	src/action/sample/ActionSampleDelete.cpp
	src/action/sample/ActionSampleEditName.cpp
	src/action/sample/ActionSampleReplaceBuffer.cpp
	src/action/song/data/ActionSongChangeAllTrackVolumes.cpp
	src/action/song/data/ActionSongSetDefaultFormat.cpp
	src/action/song/data/ActionSongSetSampleRate.cpp
	src/action/song/ActionSongDeleteSelection.cpp
	src/action/song/ActionSongMoveSelection.cpp
	src/action/tag/ActionTagAdd.cpp
	src/action/tag/ActionTagDelete.cpp
	src/action/tag/ActionTagEdit.cpp
	src/action/track/buffer/ActionTrack__AbsorbBuffer.cpp
	src/action/track/buffer/ActionTrack__AddBuffer.cpp
	src/action/track/buffer/ActionTrack__BufferSetChannels.cpp
	src/action/track/buffer/ActionTrack__DeleteBuffer.cpp
	src/action/track/buffer/ActionTrack__GrowBuffer.cpp
	src/action/track/buffer/ActionTrack__ShrinkBuffer.cpp
	src/action/track/buffer/ActionTrack__SplitBuffer.cpp
	src/action/track/buffer/ActionTrackCreateBuffers.cpp
	src/action/track/buffer/ActionTrackEditBuffer.cpp
	src/action/track/buffer/ActionTrackMoveBuffer.cpp
	src/action/track/buffer/ActionTrackSetChannels.cpp
	src/action/track/curve/ActionTrackAddCurve.cpp
	src/action/track/curve/ActionTrackCurveAddPoint.cpp
	src/action/track/curve/ActionTrackCurveDeletePoint.cpp
	src/action/track/curve/ActionTrackCurveEditPoint.cpp
	src/action/track/curve/ActionTrackDeleteCurve.cpp
	src/action/track/curve/ActionTrackEditCurve.cpp
	src/action/track/data/ActionTrackEditMuted.cpp
	src/action/track/data/ActionTrackEditName.cpp
	src/action/track/data/ActionTrackEditPanning.cpp
	src/action/track/data/ActionTrackEditVolume.cpp
	src/action/track/data/ActionTrackSetInstrument.cpp
	src/action/track/data/ActionTrackSetTarget.cpp
	src/action/track/effect/ActionTrackAddAudioEffect.cpp
	src/action/track/effect/ActionTrackDeleteEffect.cpp
	src/action/track/effect/ActionTrackEditAudioEffect.cpp
	src/action/track/effect/ActionTrackMoveAudioEffect.cpp
	src/action/track/effect/ActionTrackToggleAudioEffectEnabled.cpp
	src/action/track/layer/ActionTrackFadeAdd.cpp
	src/action/track/layer/ActionTrackFadeDelete.cpp
	src/action/track/layer/ActionTrackLayer__Delete.cpp
	src/action/track/layer/ActionTrackLayerAdd.cpp
	src/action/track/layer/ActionTrackLayerDelete.cpp
	src/action/track/layer/ActionTrackLayerMakeTrack.cpp
	src/action/track/layer/ActionTrackLayerMarkDominant.cpp
	src/action/track/layer/ActionTrackLayerMerge.cpp
	src/action/track/layer/ActionTrackLayerMove.cpp
	src/action/track/marker/ActionTrackAddMarker.cpp
	src/action/track/marker/ActionTrackDeleteMarker.cpp
	src/action/track/marker/ActionTrackEditMarker.cpp
	src/action/track/midi/ActionTrackAddMidiEffect.cpp
	src/action/track/midi/ActionTrackAddMidiNote.cpp
	src/action/track/midi/ActionTrackDeleteMidiEffect.cpp
	src/action/track/midi/ActionTrackDeleteMidiNote.cpp
	src/action/track/midi/ActionTrackEditMidiEffect.cpp
	src/action/track/midi/ActionTrackEditMidiNote.cpp
	src/action/track/midi/ActionTrackInsertMidi.cpp
	src/action/track/midi/ActionTrackMoveMidiEffect.cpp
	src/action/track/midi/ActionTrackToggleMidiEffectEnabled.cpp
	src/action/track/sample/ActionTrackAddSample.cpp
	src/action/track/sample/ActionTrackDeleteSample.cpp
	src/action/track/sample/ActionTrackEditSample.cpp
	src/action/track/sample/ActionTrackInsertSample.cpp
	src/action/track/sample/ActionTrackPasteAsSample.cpp
	src/action/track/sample/ActionTrackSampleFromSelection.cpp
	src/action/track/synthesizer/ActionTrackDetuneSynthesizer.cpp
	src/action/track/synthesizer/ActionTrackEditSynthesizer.cpp
	src/action/track/synthesizer/ActionTrackSetSynthesizer.cpp
	src/action/track/ActionTrack__DeleteEmpty.cpp
	src/action/track/ActionTrackAdd.cpp
	src/action/track/ActionTrackDelete.cpp
	src/action/track/ActionTrackMove.cpp
	src/action/Action.cpp
	src/action/ActionGroup.cpp
	src/action/ActionManager.cpp
	src/action/ActionMergable.cpp
	src/data/audio/AudioBuffer.cpp
	src/data/audio/BufferInterpolator.cpp
	src/data/audio/RingBuffer.cpp
	src/data/midi/Clef.cpp
	src/data/midi/Instrument.cpp
	src/data/midi/MidiData.cpp
	src/data/midi/MidiEvent.cpp
	src/data/midi/MidiNote.cpp
	src/data/midi/Scale.cpp
	src/data/midi/Temperament.cpp
	src/data/rhythm/Bar.cpp
	src/data/rhythm/BarCollection.cpp
	src/data/rhythm/Beat.cpp
	src/data/base.cpp
	src/data/CrossFade.cpp
	src/data/Curve.cpp
	src/data/Data.cpp
	src/data/Range.cpp
	src/data/Sample.cpp
	src/data/SampleRef.cpp
	src/data/Song.cpp
	src/data/SongSelection.cpp
	src/data/Track.cpp
	src/data/TrackLayer.cpp
	src/data/TrackMarker.cpp
	src/device/stream/AudioInput.cpp
	src/device/stream/AudioOutput.cpp
	src/device/stream/MidiInput.cpp
	src/device/Device.cpp
	src/device/DeviceManager.cpp
	src/lib/any/any.cpp
	src/lib/base/array.cpp
	src/lib/base/pointer.cpp
	src/lib/base/strings.cpp
	src/lib/doc/chunked.cpp
	src/lib/doc/config.cpp
	src/lib/doc/pdf.cpp
	src/lib/doc/xml.cpp
	src/lib/hui/Controls/Control.cpp
	src/lib/hui/Controls/ControlButtonGtk.cpp
	src/lib/hui/Controls/ControlCheckBoxGtk.cpp
	src/lib/hui/Controls/ControlColorButtonGtk.cpp
	src/lib/hui/Controls/ControlComboBoxGtk.cpp
	src/lib/hui/Controls/ControlDrawingAreaGtk.cpp
	src/lib/hui/Controls/ControlEditGtk.cpp
	src/lib/hui/Controls/ControlExpanderGtk.cpp
	src/lib/hui/Controls/ControlGridGtk.cpp
	src/lib/hui/Controls/ControlGroupGtk.cpp
	src/lib/hui/Controls/ControlHeaderBar.cpp
	src/lib/hui/Controls/ControlLabelGtk.cpp
	src/lib/hui/Controls/ControlListViewGtk.cpp
	src/lib/hui/Controls/ControlMenuButtonGtk.cpp
	src/lib/hui/Controls/ControlMultilineEditGtk.cpp
	src/lib/hui/Controls/ControlPanedGtk.cpp
	src/lib/hui/Controls/ControlProgressBarGtk.cpp
	src/lib/hui/Controls/ControlRadioButtonGtk.cpp
	src/lib/hui/Controls/ControlScrollerGtk.cpp
	src/lib/hui/Controls/ControlSeparatorGtk.cpp
	src/lib/hui/Controls/ControlSliderGtk.cpp
	src/lib/hui/Controls/ControlSpinButtonGtk.cpp
	src/lib/hui/Controls/ControlTabControlGtk.cpp
	src/lib/hui/Controls/ControlToggleButtonGtk.cpp
	src/lib/hui/Controls/ControlTreeViewGtk.cpp
	src/lib/hui/Controls/MenuItemGtk.cpp
	src/lib/hui/Controls/MenuItemSeparatorGtk.cpp
	src/lib/hui/Controls/MenuItemSeparatorWin.cpp
	src/lib/hui/Controls/MenuItemSubmenuGtk.cpp
	src/lib/hui/Controls/MenuItemSubmenuWin.cpp
	src/lib/hui/Controls/MenuItemToggleGtk.cpp
	src/lib/hui/Controls/MenuItemToggleWin.cpp
	src/lib/hui/Controls/MenuItemWin.cpp
	src/lib/hui/Controls/ToolItemButtonGtk.cpp
	src/lib/hui/Controls/ToolItemButtonWin.cpp
	src/lib/hui/Controls/ToolItemMenuButtonGtk.cpp
	src/lib/hui/Controls/ToolItemMenuButtonWin.cpp
	src/lib/hui/Controls/ToolItemSeparatorGtk.cpp
	src/lib/hui/Controls/ToolItemSeparatorWin.cpp
	src/lib/hui/Controls/ToolItemToggleButtonGtk.cpp
	src/lib/hui/Controls/ToolItemToggleButtonWin.cpp
	src/lib/hui/Application.cpp
	src/lib/hui/Callback.cpp
	src/lib/hui/Clipboard.cpp
	src/lib/hui/common_dlg.cpp
	src/lib/hui/common_dlg_gtk.cpp
	src/lib/hui/common_dlg_win.cpp
	src/lib/hui/error.cpp
	src/lib/hui/Event.cpp
	src/lib/hui/hui.cpp
	src/lib/hui/language.cpp
	src/lib/hui/main_gtk.cpp
	src/lib/hui/main_win.cpp
	src/lib/hui/Menu.cpp
	src/lib/hui/MenuGtk.cpp
	src/lib/hui/MenuWin.cpp
	src/lib/hui/PainterGtk.cpp
	src/lib/hui/Panel.cpp
	src/lib/hui/Resource.cpp
	src/lib/hui/Toolbar.cpp
	src/lib/hui/ToolbarGtk.cpp
	src/lib/hui/ToolbarWin.cpp
	src/lib/hui/utility.cpp
	src/lib/hui/Window.cpp
	src/lib/hui/window_control.cpp
	src/lib/hui/window_control_gtk.cpp
	src/lib/hui/window_control_win.cpp
	src/lib/hui/WindowGtk.cpp
	src/lib/hui/WindowWin.cpp
	src/lib/image/color.cpp
	src/lib/image/image.cpp
	src/lib/image/image_bmp.cpp
	src/lib/image/image_jpg.cpp
	src/lib/image/image_png.cpp
	src/lib/image/image_tga.cpp
	src/lib/image/ImagePainter.cpp
	src/lib/kaba/asm/asm.cpp
	src/lib/kaba/asm/asm_arm.cpp
	src/lib/kaba/asm/asm_x86.cpp
	src/lib/kaba/compiler/Backend.cpp
	src/lib/kaba/compiler/BackendAmd64.cpp
	src/lib/kaba/compiler/BackendARM.cpp
	src/lib/kaba/compiler/BackendX86.cpp
	src/lib/kaba/compiler/CommandList.cpp
	src/lib/kaba/compiler/compiler.cpp
	src/lib/kaba/compiler/mapper.cpp
	src/lib/kaba/compiler/serializer.cpp
	src/lib/kaba/compiler/SerializerX.cpp
	src/lib/kaba/compiler/SerialNode.cpp
	src/lib/kaba/dynamic/call.cpp
	src/lib/kaba/dynamic/dynamic.cpp
	src/lib/kaba/dynamic/exception.cpp
	src/lib/kaba/lib/dict.cpp
	src/lib/kaba/lib/extern.cpp
	src/lib/kaba/lib/lib.cpp
	src/lib/kaba/lib/lib_base.cpp
	src/lib/kaba/lib/lib_doc.cpp
	src/lib/kaba/lib/lib_gl.cpp
	src/lib/kaba/lib/lib_hui.cpp
	src/lib/kaba/lib/lib_image.cpp
	src/lib/kaba/lib/lib_kaba.cpp
	src/lib/kaba/lib/lib_math.cpp
	src/lib/kaba/lib/lib_net.cpp
	src/lib/kaba/lib/lib_os.cpp
	src/lib/kaba/lib/lib_threads.cpp
	src/lib/kaba/lib/lib_time.cpp
	src/lib/kaba/lib/lib_vulkan.cpp
	src/lib/kaba/parser/abstract.cpp
	src/lib/kaba/parser/Concretifier.cpp
	src/lib/kaba/parser/implicit.cpp
	src/lib/kaba/parser/import.cpp
	src/lib/kaba/parser/lexical.cpp
	src/lib/kaba/parser/macros.cpp
	src/lib/kaba/parser/Parser.cpp
	src/lib/kaba/parser/template.cpp
	src/lib/kaba/syntax/Block.cpp
	src/lib/kaba/syntax/Class.cpp
	src/lib/kaba/syntax/Constant.cpp
	src/lib/kaba/syntax/Function.cpp
	src/lib/kaba/syntax/Identifier.cpp
	src/lib/kaba/syntax/Node.cpp
	src/lib/kaba/syntax/Operator.cpp
	src/lib/kaba/syntax/preprocessor.cpp
	src/lib/kaba/syntax/Statement.cpp
	src/lib/kaba/syntax/SyntaxTree.cpp
	src/lib/kaba/syntax/Variable.cpp
	src/lib/kaba/CompilerConfiguration.cpp
	src/lib/kaba/Interpreter.cpp
	src/lib/kaba/kaba.cpp
	src/lib/math/complex.cpp
	src/lib/math/interpolation.cpp
	src/lib/math/math.cpp
	src/lib/math/matrix.cpp
	src/lib/math/matrix3.cpp
	src/lib/math/plane.cpp
	src/lib/math/quaternion.cpp
	src/lib/math/random.cpp
	src/lib/math/ray.cpp
	src/lib/math/rect.cpp
	src/lib/math/vec2.cpp
	src/lib/math/vec4.cpp
	src/lib/math/vector.cpp
	src/lib/net/BinaryBuffer.cpp
	src/lib/net/net.cpp
	src/lib/net/Socket.cpp
	src/lib/nix/nix.cpp
	src/lib/nix/nix_buffer.cpp
	src/lib/nix/nix_draw.cpp
	src/lib/nix/nix_framebuffer.cpp
	src/lib/nix/nix_light.cpp
	src/lib/nix/nix_shader.cpp
	src/lib/nix/nix_textures.cpp
	src/lib/nix/nix_vertexbuffer.cpp
	src/lib/nix/nix_view.cpp
	src/lib/os/CommandLineParser.cpp
	src/lib/os/date.cpp
	src/lib/os/file.cpp
	src/lib/os/filesystem.cpp
	src/lib/os/formatter.cpp
	src/lib/os/msg.cpp
	src/lib/os/path.cpp
	src/lib/os/time.cpp
	src/lib/threads/Mutex.cpp
	src/lib/threads/Thread.cpp
	src/lib/threads/ThreadedWork.cpp
	src/module/audio/AudioAccumulator.cpp
	src/module/audio/AudioBackup.cpp
	src/module/audio/AudioChannelSelector.cpp
	src/module/audio/AudioEffect.cpp
	src/module/audio/AudioJoiner.cpp
	src/module/audio/AudioSource.cpp
	src/module/audio/AudioSucker.cpp
	src/module/audio/AudioVisualizer.cpp
	src/module/audio/BufferStreamer.cpp
	src/module/audio/PeakMeter.cpp
	src/module/audio/PitchDetector.cpp
	src/module/audio/SongRenderer.cpp
	src/module/audio/TrackRenderer.cpp
	src/module/beats/BarStreamer.cpp
	src/module/beats/BeatMidifier.cpp
	src/module/beats/BeatSource.cpp
	src/module/midi/MidiAccumulator.cpp
	src/module/midi/MidiEffect.cpp
	src/module/midi/MidiEventStreamer.cpp
	src/module/midi/MidiJoiner.cpp
	src/module/midi/MidiPreviewSource.cpp
	src/module/midi/MidiSource.cpp
	src/module/midi/MidiSucker.cpp
	src/module/port/Port.cpp
	src/module/synthesizer/DebugSynthesizer.cpp
	src/module/synthesizer/DummySynthesizer.cpp
	src/module/synthesizer/EnvelopeADSR.cpp
	src/module/synthesizer/Synthesizer.cpp
	src/module/AutoConfigPanel.cpp
	src/module/ConfigPanel.cpp
	src/module/Module.cpp
	src/module/ModuleConfiguration.cpp
	src/module/ModuleFactory.cpp
	src/module/SignalChain.cpp
	src/plugins/ExtendedAudioBuffer.cpp
	src/plugins/FastFourierTransform.cpp
	src/plugins/Plugin.cpp
	src/plugins/PluginManager.cpp
	src/plugins/ProfileManager.cpp
	src/plugins/TsunamiPlugin.cpp
	src/storage/dialog/PdfConfigDialog.cpp
	src/storage/dialog/RawConfigDialog.cpp
	src/storage/format/Format.cpp
	src/storage/format/FormatFlac.cpp
	src/storage/format/FormatGuitarPro.cpp
	src/storage/format/FormatM4a.cpp
	src/storage/format/FormatMidi.cpp
	src/storage/format/FormatMp3.cpp
	src/storage/format/FormatNami.cpp
	src/storage/format/FormatOgg.cpp
	src/storage/format/FormatPdf.cpp
	src/storage/format/FormatRaw.cpp
	src/storage/format/FormatSoundFont2.cpp
	src/storage/format/FormatWave.cpp
	src/storage/Storage.cpp
	src/storage/StorageOperationData.cpp
	src/stuff/BackupManager.cpp
	src/stuff/Clipboard.cpp
	src/stuff/Diff.cpp
	src/stuff/ErrorHandler.cpp
	src/stuff/Log.cpp
	src/stuff/Observable.cpp
	src/stuff/PerformanceMonitor.cpp
	src/stuff/SessionManager.cpp
	src/test/TestAudioBuffer.cpp
	src/test/TestInterpolator.cpp
	src/test/TestMidiPreview.cpp
	src/test/TestMixer.cpp
	src/test/TestPlugins.cpp
	src/test/TestPointer.cpp
	src/test/TestRhythm.cpp
	src/test/TestRingBuffer.cpp
	src/test/TestSignalChain.cpp
	src/test/TestStreams.cpp
	src/test/TestThreads.cpp
	src/test/TestTrackVersion.cpp
	src/test/UnitTest.cpp
	src/view/audioview/graph/AudioViewLayer.cpp
	src/view/audioview/graph/AudioViewTrack.cpp
	src/view/audioview/graph/Background.cpp
	src/view/audioview/graph/Cursor.cpp
	src/view/audioview/graph/LayerHeader.cpp
	src/view/audioview/graph/TimeScale.cpp
	src/view/audioview/graph/TrackHeader.cpp
	src/view/audioview/graph/ViewMarker.cpp
	src/view/audioview/AudioView.cpp
	src/view/audioview/TrackHeightManager.cpp
	src/view/audioview/ViewPort.cpp
	src/view/bottombar/signaleditor/SignalEditorBackground.cpp
	src/view/bottombar/signaleditor/SignalEditorCable.cpp
	src/view/bottombar/signaleditor/SignalEditorModule.cpp
	src/view/bottombar/signaleditor/SignalEditorPort.cpp
	src/view/bottombar/signaleditor/SignalEditorTab.cpp
	src/view/bottombar/BottomBar.cpp
	src/view/bottombar/DeviceConsole.cpp
	src/view/bottombar/LogConsole.cpp
	src/view/bottombar/MiniBar.cpp
	src/view/bottombar/MixingConsole.cpp
	src/view/bottombar/PluginConsole.cpp
	src/view/bottombar/SessionConsole.cpp
	src/view/bottombar/SignalEditor.cpp
	src/view/dialog/BarAddDialog.cpp
	src/view/dialog/BarDeleteDialog.cpp
	src/view/dialog/BarEditDialog.cpp
	src/view/dialog/BufferCompressionDialog.cpp
	src/view/dialog/ChannelMapperDialog.cpp
	src/view/dialog/HelpDialog.cpp
	src/view/dialog/MarkerDialog.cpp
	src/view/dialog/ModuleSelectorDialog.cpp
	src/view/dialog/NewDialog.cpp
	src/view/dialog/PauseAddDialog.cpp
	src/view/dialog/PauseEditDialog.cpp
	src/view/dialog/QuestionDialog.cpp
	src/view/dialog/SampleScaleDialog.cpp
	src/view/dialog/SettingsDialog.cpp
	src/view/dialog/TemperamentDialog.cpp
	src/view/dialog/TimeTrackAddDialog.cpp
	src/view/dialog/TrackRoutingDialog.cpp
	src/view/dialog/TuningDialog.cpp
	src/view/helper/graph/Node.cpp
	src/view/helper/graph/SceneGraph.cpp
	src/view/helper/graph/Scrollable.cpp
	src/view/helper/graph/ScrollBar.cpp
	src/view/helper/CpuDisplay.cpp
	src/view/helper/Dial.cpp
	src/view/helper/Drawing.cpp
	src/view/helper/FxListEditor.cpp
	src/view/helper/MidiPreview.cpp
	src/view/helper/ModulePanel.cpp
	src/view/helper/PeakMeterDisplay.cpp
	src/view/helper/PeakThread.cpp
	src/view/helper/Progress.cpp
	src/view/helper/Slider.cpp
	src/view/helper/SymbolRenderer.cpp
	src/view/mode/ViewMode.cpp
	src/view/mode/ViewModeCapture.cpp
	src/view/mode/ViewModeCurve.cpp
	src/view/mode/ViewModeDefault.cpp
	src/view/mode/ViewModeEdit.cpp
	src/view/mode/ViewModeEditAudio.cpp
	src/view/mode/ViewModeEditDummy.cpp
	src/view/mode/ViewModeMidi.cpp
	src/view/mode/ViewModeScaleBars.cpp
	src/view/mode/ViewModeScaleMarker.cpp
	src/view/painter/BasicGridPainter.cpp
	src/view/painter/BufferPainter.cpp
	src/view/painter/GridPainter.cpp
	src/view/painter/MidiPainter.cpp
	src/view/painter/MultiLinePainter.cpp
	src/view/sidebar/captureconsolemodes/CaptureConsoleMode.cpp
	src/view/sidebar/captureconsolemodes/CaptureConsoleModeAudio.cpp
	src/view/sidebar/captureconsolemodes/CaptureConsoleModeMidi.cpp
	src/view/sidebar/captureconsolemodes/CaptureConsoleModeMulti.cpp
	src/view/sidebar/captureconsolemodes/CaptureTrackData.cpp
	src/view/sidebar/AudioEditorConsole.cpp
	src/view/sidebar/CaptureConsole.cpp
	src/view/sidebar/CurveConsole.cpp
	src/view/sidebar/DummyEditorConsole.cpp
	src/view/sidebar/MidiEditorConsole.cpp
	src/view/sidebar/SampleManagerConsole.cpp
	src/view/sidebar/SampleRefConsole.cpp
	src/view/sidebar/SideBar.cpp
	src/view/sidebar/SongConsole.cpp
	src/view/sidebar/TrackConsole.cpp
	src/view/ColorScheme.cpp
	src/view/HoverData.cpp
	src/view/MouseDelayPlanner.cpp
	src/Session.cpp
	src/Tsunami.cpp
	src/TsunamiWindow.cpp
)

find_package(PkgConfig REQUIRED)

# gtk3 or gtk4 mandatory
set(GTK3_OR_GTK4 "gtk3" CACHE STRING "Use gtk3 or gtk4?")
pkg_check_modules(GTK3 gtk+-3.0)
if(GTK3_FOUND)
	list(APPEND GTK3_OR_GTK4_FOUND "gtk3")
endif()
pkg_check_modules(GTK4 gtk4)
if(GTK4_FOUND)
	list(APPEND GTK3_OR_GTK4_FOUND "gtk4")
endif()
if(GTK3_OR_GTK4_FOUND)
	if(NOT ${GTK3_OR_GTK4} IN_LIST GTK3_OR_GTK4_FOUND)
		list(GET GTK3_OR_GTK4_FOUND 0 X)
		set(GTK3_OR_GTK4 ${X} CACHE STRING "choose" FORCE)
		message(WARNING "Choice for GTK3_OR_GTK4 invalid, using ${GTK3_OR_GTK4} instead")
	endif()
else()
	message(SEND_ERROR "Neither gtk3 or gtk4 found") #  or/nor..
	set(GTK3_OR_GTK4 "")
endif()
if(GTK3_OR_GTK4 STREQUAL "gtk3")
	include_directories(${GTK3_INCLUDE_DIRS})
	link_directories(${GTK3_LIBRARY_DIRS})
	list(APPEND DEPS ${GTK3_LIBRARIES})
	add_definitions(${GTK3_CFLAGS_OTHER})
	add_compile_definitions(HAS_LIB_GTK3=1)
endif()
if(GTK3_OR_GTK4 STREQUAL "gtk4")
	include_directories(${GTK4_INCLUDE_DIRS})
	link_directories(${GTK4_LIBRARY_DIRS})
	list(APPEND DEPS ${GTK4_LIBRARIES})
	add_definitions(${GTK4_CFLAGS_OTHER})
	add_compile_definitions(HAS_LIB_GTK4=1)
endif()

# zlib mandatory
pkg_check_modules(ZLIB REQUIRED zlib)
include_directories(${ZLIB_INCLUDE_DIRS})
link_directories(${ZLIB_LIBRARY_DIRS})
list(APPEND DEPS ${ZLIB_LIBRARIES})
add_definitions(${ZLIB_CFLAGS_OTHER})
add_compile_definitions(HAS_LIB_ZLIB=1)

# fftw3 mandatory
pkg_check_modules(FFTW3 REQUIRED fftw3f)
include_directories(${FFTW3_INCLUDE_DIRS})
link_directories(${FFTW3_LIBRARY_DIRS})
list(APPEND DEPS ${FFTW3_LIBRARIES})
add_definitions(${FFTW3_CFLAGS_OTHER})
add_compile_definitions(HAS_LIB_FFTW3=1)

# ogg (optional)
pkg_check_modules(OGG ogg)
pkg_check_modules(VORBIS vorbis)
pkg_check_modules(VORBISFILE vorbisfile)
pkg_check_modules(VORBISENC vorbisenc)
if(OGG_FOUND AND VORBIS_FOUND AND VORBISFILE_FOUND AND VORBISENC_FOUND)
	include_directories(${OGG_INCLUDE_DIRS})
	link_directories(${OGG_LIBRARY_DIRS})
	list(APPEND DEPS ${OGG_LIBRARIES})
	add_definitions(${OGG_CFLAGS_OTHER})
	include_directories(${VORBIS_INCLUDE_DIRS})
	link_directories(${VORBIS_LIBRARY_DIRS})
	list(APPEND DEPS ${VORBIS_LIBRARIES})
	add_definitions(${VORBIS_CFLAGS_OTHER})
	include_directories(${VORBISFILE_INCLUDE_DIRS})
	link_directories(${VORBISFILE_LIBRARY_DIRS})
	list(APPEND DEPS ${VORBISFILE_LIBRARIES})
	add_definitions(${VORBISFILE_CFLAGS_OTHER})
	include_directories(${VORBISENC_INCLUDE_DIRS})
	link_directories(${VORBISENC_LIBRARY_DIRS})
	list(APPEND DEPS ${VORBISENC_LIBRARIES})
	add_definitions(${VORBISENC_CFLAGS_OTHER})
	add_compile_definitions(HAS_LIB_OGG=1)
else()
	add_compile_definitions(HAS_LIB_OGG=0)
endif()

# flac (optional)
pkg_check_modules(FLAC flac)
if(FLAC_FOUND)
	include_directories(${FLAC_INCLUDE_DIRS})
	link_directories(${FLAC_LIBRARY_DIRS})
	list(APPEND DEPS ${FLAC_LIBRARIES})
	add_definitions(${FLAC_CFLAGS_OTHER})
	add_compile_definitions(HAS_LIB_FLAC=1)
else()
	add_compile_definitions(HAS_LIB_FLAC=0)
endif()

# alsa (optional)
pkg_check_modules(ALSA alsa)
if(ALSA_FOUND)
	include_directories(${ALSA_INCLUDE_DIRS})
	link_directories(${ALSA_LIBRARY_DIRS})
	list(APPEND DEPS ${ALSA_LIBRARIES})
	add_definitions(${ALSA_CFLAGS_OTHER})
	add_compile_definitions(HAS_LIB_ALSA=1)
else()
	add_compile_definitions(HAS_LIB_ALSA=0)
endif()

# pulseaudio (optional)
pkg_check_modules(LIBPULSE libpulse)
if(LIBPULSE_FOUND)
	include_directories(${LIBPULSE_INCLUDE_DIRS})
	link_directories(${LIBPULSE_LIBRARY_DIRS})
	list(APPEND DEPS ${LIBPULSE_LIBRARIES})
	add_definitions(${LIBPULSE_CFLAGS_OTHER})
	add_compile_definitions(HAS_LIB_PULSEAUDIO=1)
else()
	add_compile_definitions(HAS_LIB_PULSEAUDIO=0)
endif()

# portaudio (optional)
pkg_check_modules(PORTAUDIO-2.0 portaudio-2.0)
if(PORTAUDIO-2.0_FOUND)
	include_directories(${PORTAUDIO-2.0_INCLUDE_DIRS})
	link_directories(${PORTAUDIO-2.0_LIBRARY_DIRS})
	list(APPEND DEPS ${PORTAUDIO-2.0_LIBRARIES})
	add_definitions(${PORTAUDIO-2.0_CFLAGS_OTHER})
	add_compile_definitions(HAS_LIB_PORTAUDIO=1)
else()
	add_compile_definitions(HAS_LIB_PORTAUDIO=0)
endif()

# unwind (optional)
pkg_check_modules(LIBUNWIND libunwind)
if(LIBUNWIND_FOUND)
	include_directories(${LIBUNWIND_INCLUDE_DIRS})
	link_directories(${LIBUNWIND_LIBRARY_DIRS})
	list(APPEND DEPS ${LIBUNWIND_LIBRARIES})
	add_definitions(${LIBUNWIND_CFLAGS_OTHER})
	add_compile_definitions(HAS_LIB_UNWIND=1)
else()
	add_compile_definitions(HAS_LIB_UNWIND=0)
endif()

# dl...
list(APPEND DEPS ${CMAKE_DL_LIBS})
add_compile_definitions(HAS_LIB_DL=1)

# gl (optional)
pkg_check_modules(GL gl)
if(GL_FOUND)
	include_directories(${GL_INCLUDE_DIRS})
	link_directories(${GL_LIBRARY_DIRS})
	list(APPEND DEPS ${GL_LIBRARIES})
	add_definitions(${GL_CFLAGS_OTHER})
	add_compile_definitions(HAS_LIB_GL=1)
else()
	add_compile_definitions(HAS_LIB_GL=0)
endif()

add_executable(tsunami ${SOURCE})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(tsunami PRIVATE Threads::Threads ${DEPS})

add_compile_definitions(INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")

install(TARGETS tsunami DESTINATION bin)
install(DIRECTORY "static/icons" DESTINATION share/tsunami)
install(DIRECTORY "static/Midi Fonts" DESTINATION share/tsunami)
install(DIRECTORY "static/SignalChains" DESTINATION share/tsunami)
install(FILES "static/favorites_demo.txt" DESTINATION share/tsunami)
install(FILES "static/hui_resources.txt" DESTINATION share/tsunami)
install(FILES "static/icon.png" DESTINATION share/tsunami)
install(FILES "static/icon.svg" DESTINATION share/tsunami)
install(FILES "static/license.txt" DESTINATION share/tsunami)
install(FILES "static/license_small.txt" DESTINATION share/tsunami)
install(DIRECTORY "Plugins" DESTINATION share/tsunami)

set(INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
configure_file("static/michisoft-tsunami.desktop" /tmp/michisoft-tsunami.desktop NO_SOURCE_PERMISSIONS)

find_program(XDG_DESKTOP_MENU_EXECUTABLE xdg-desktop-menu)
if(NOT XDG_DESKTOP_MENU_EXECUTABLE STREQUAL "XDG_DESKTOP_MENU_EXECUTABLE_NOTFOUND")
	install(CODE "message(STATUS \"installing desktop file\")")
	install(CODE "execute_process(COMMAND ${XDG_DESKTOP_MENU_EXECUTABLE} install /tmp/michisoft-tsunami.desktop)")
endif()

find_program(XDG_MIME_EXECUTABLE xdg-mime)
if(NOT XDG_MIME_EXECUTABLE STREQUAL "XDG_MIME_EXECUTABLE_NOTFOUND")
	install(CODE "message(STATUS \"installing mime file\")")
	install(CODE "execute_process(COMMAND ${XDG_MIME_EXECUTABLE} install --novendor ${CMAKE_CURRENT_SOURCE_DIR}/static/michisoft-nami.xml)")
endif()
