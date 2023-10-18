# Development plans - short-term TODOs

## Modules

### Effects

* [x] dry/wet
* [ ] version migration?
* [ ] ranged fx
* [ ] proper pitch shifting
* [x] reverb


### Synthesizers

* Font synth
  * [x] play loops
  * [x] auto-pitch-shift
  * [x] simple envelope
  * [ ] auto-import sf2

* FM
  * [ ] nicer grid

### SignalChain

* [ ] system modules via shared pointers. Check counter before delete in SignalEditor
* [x] is_active()

### Other plugins

* [ ] improved practice looper (requires pitch shift fx)
* [x] Panel with PeakMeterDisplay


## User interface

* [ ] clef symbol rendering (own ttf file?)
* [ ] smaller midi notes when zoomed out (limited by time duration)
  * [x] more icons (midi editor, audio editor)
  * [x] audio editor...

* [ ] dialog for "locking" tracks/data
  * [x] cursor handles on layers (shift key)
  * [x] shift + click extends selection
  * [x] shift + home/end extends selection

### AudioView

* [x] move cam to selected track
* [ ] allow delayed action on focus click
* [ ] don't double draw selected buffers
* [x] delete multiple layers
* [x] delete only layer -> delete track
* [ ] auto select after delete layer
* [ ] groups
  * [x] colors
  * [ ] keep next to each other
  * [x] drag and drop group/ungroup
  * [ ] popup -> group selected
  * [x] delete master -> ungroup

### Midi editor

* [x] drag'n'drop pitch
* [x] prettier selected notes
* [x] paint mode vs select mode
* [ ] interval/chord also for key presses
* [ ] select clef
* [ ] edit instrument?
* [ ] modifiers for whole bar but only 1 pitch
* [ ] auto neutral sign
* [x] note duration rect
* [ ] note join operation
* [ ] note split operation

### Bottom bar
  * [x] always overlay button (no X button)
  * [x] devices

### Side bar

* [x] repair track sample console
* [x] synth panel as card
* [ ] fx list as grid of expandable cards?
* [x] larger module dialog

### Other

* [x] audio fx/source preview
* [x] gtk4 list reorder
* [ ] general slider ticks
* [x] no dummy menu



## Data representation

* [ ] compressed buffers
  * [x] compressed data
  * [x] store in nami files
  * [ ] keep through actions
  * [ ] load from ogg/flac files
* [ ] multiple mixing profiles?


## Feedback from Benji

* [ ] master unsolo
* [x] hide marker?
* [ ] vst!
* [ ] send/receive ASIO...

* [ ] loudness update, per song/part
* [x] solo over mute

* [ ] record, then select track?
* [ ] double click on faders -> reset
* [x] nicer routing....groups...
* [ ] mark track mixers to stay in view while scrolling
* [ ] indent grouped TrackHeaders
* [ ] plugins
  * [ ] stereo separator
  * [x] compressor
