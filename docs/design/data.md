# Data

## Basics

* mostly pointer arrays
  * can use addresses as UUIDs - this allows consistent selection and references stay consistent after deleting und restoring

## Ownership

* mostly single ownership
* `Sample` have shared ownership
* midi notes now also shared

## Audio data

`AudioBuffer`

* `float[]` per channel
* can contain multiple (arbitrary) channels
* buffers can be slices (references to other buffers)

* saves peak data (for smoother rendering)


## Midi data

* `MidiNote`
  * `range` - start and stop time
  * `pitch`
  * `volume`
  * meta data - string for TAB, articulation
* `MidiEvent` - same but only single time stamp (each not has 2 events: start/stop)
* `MidiNoteBuffer`
  * `(shared MidiNote)[]` - each note is allocated and owned
* `MidiEventBuffer`
  * `MidiEvent[]` - keep as values

## Beat data

...

## Song

`Song`

* multiple `Track`s
  * type audio/midi/beats
  * multiple `TrackLayer`s
    * contains multiple `AudioBuffer`s or
    * `MidiNoteBuffer`
  * `Synthesizer`
* `Tag`s
* `Sample`s
* `Bar`s - the metronome data


