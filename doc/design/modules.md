# Modules

## Basics

Modules are the basic building block for processing audio/midi/beat data.

Most modules are defined as plugin files and compiled into tsunami at runtime via the `kaba` JIT compiler.

Modules can be connected to form [signal chains](signalchains.md).

## Module categories

* `AudioEffect` - audio in, audio out
* `AudioSource` - audio out
* `AudioVisualizer` - audio in, audio out, but for showing graphics
* `MidiEffect` - midi in, midi out
* `MidiSource` - midi out
* `Synthesizer` - midi in, audio out
* `BeatSource` - produces beats
* Plumbing - Y-connectors, etc.
* Streams - speaker output, microphone input, midi input

Additionally, two more plugin categories:
* `SongPlugin` - executed once to affect the current song
* `TsunamiPlugin` - running in the background

## "Suck" protocoll

* modules are mostly passive and provide in and out ports
  * out port implements functionality
  * in ports only know to which out port in another module they are connected

A typical use case: `AudioSource >> AudioEffect >> OutputStream`
* `OutputStream` (speaker) is the only active part (regularly called by the audio library or the signal chain)
  * `OutputStream` provides a buffer and reads from the `AudioEffect`s out port
    * `AudioEffect` reads from the `AudioSource`s out port by forwarding the buffer
      * `AudioSource` fills the provided buffer from `AudioEffect`
    * `AudioEffect` edits the buffer
  * `OutputStream` gives the buffer to the audio library/speaker

* modules can return fewer data samples than requested
* the can choose to return `NOT_ENOUGH_DATA` instead (preferred way)
* or `END_OF_STREAM` 

## Config/panel

Modules can have a configuration state, that can be written into files. Modules might also provide a GUI panel for the user to edit the configuration.

* `ConfigPanel` - basic gui definition (easy to write in plugins)
* `ModulePanel` - wrapper with favourites (presets) and action linking...
   * subscribed to `Module`
* `Module` can have multiple panels

* gui interactions edit `Module.config` and send
```
    ConfigPanel.changed()
     -> Module.changed()
        -> Module.on_config()
        -> Module.notify(MESSAGE_CHANGED)`
```
* `ModulePanel` catches `MESSAGE_CHANGED`
   * callback... could execute Action for fx etc...
* Actions send `Module.notify(MESSAGE_CHANGED_BY_ACTION)`
   * caught by `ModulePanel`
```
         -> ConfigPanel.update() to refresh gui
```








