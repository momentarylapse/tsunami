* fx dry/wet
* clef symbol rendering (own ttf file?)
* smaller midi notes when zoomed out (limited by time duration)
J more icons (midi editor, audio editor)
J audio editor...

* dialog for "locking" tracks/data
J cursor handles on layers (shift key)
J shift + click extends selection
J shift + home/end extends selection




J repair midi files
* fx version migration?

* Font synth
	j play loops
	j auto-pitch-shift
	j simple envelope
	* auto-import sf2

* view
J	move cam to selected track
	allow delayed action on focus click
	don't double draw selected buffers
J	delete multiple layers
J	delete only layer -> delete track
	auto select after delete layer

* render
J	note duration rect


system modules via shared pointers. Check counter before delete in SignalEditor


SignalChain
J	is_active()


PeakMeter
J	Panel with PeakMeterDisplay


compressed buffers
J	compressed data
J	nami
	keep through actions
	load from ogg/flac files


groups
J	colors
	keep next to each other
J	drag and drop group/ungroup
	popup -> group selected
J	delete master -> ungroup


bugs
J	render AudioBuffer in current layer
J	graph tracks/layers
J	SignalEditor Module hover
	EQ undo "string ended unexpectedly"



Benji
	master unsolo
X	hide marker?
	vst!
	send/receive ASIO...

	loudness update, per song/part
J	solo over mute

	record, then select track?
X	double click on faders -> reset
J	nicer routing....groups...
	mark track mixers to stay in view while scrolling
	indent grouped TrackHeaders


	plugins
		stereo separator
		compressor


midi editor
	drag'n'drop pitch
	prettier selected notes
	paint mode vs select mode
	interval/chord also for key presses
	select clef
	edit instrument?
	modifiers for whole bar but only 1 pitch
	auto neutral sign

bottom bar
	always overlay button (no X button)
	devices


== bugs ==

~ pulseaudio playback "out of data" ...argh
J Equalizer plugin illegal memory access
