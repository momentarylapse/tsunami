# Plugins

## Introduction

Plugins are written in [kaba](https://github.com/momentarylapse/kaba). Tsunami acts as a _host_ program, loading kaba files and compiling them at run-time.

The connection between tsunami and a kaba plugin goes in both directions:
* tsunami exposes some of its internals (functions and classes) to the kaba plugins
* kaba plugins can define new classes (e.g. audio effects) that can then be used inside tsunami

The API exposed by the host to the plugins ("header files") can be found in the repo in [/plugins/tsunami/](../../plugins/tsunami).

New plugins can be created by adding a new `*.kaba` file into the corresponding folder. Tsunami will look for plugins in these folders:
* `/usr/local/share/tsunami/plugins/` (see cmake prefix) when installed
  * when running from inside the repo, [/plugins/](../../plugins) will be used instead
* `~/.tsunami/plugins/`

The sub-folder corresponds to the plugin category. E.g. audio effects go into [/plugins/audio-effect/](../../plugins/audio-effect).

Plugins define classes derived from the `Module` base class. They can define a number of ports (inputs/outputs for audio/midi/beats). But it is recommended to derive from more specific classes like `AudioEffect` that come with pre-defined ports and other useful behavior. 

## Example: audio effect

Audio effects simply modify chunks of audio data:

```kaba
# import the tsunami API
use tsunami.*


# define a new plugin class "MyAudioEffect" derived from tsunami's "AudioEffect"
class MyAudioEffect extends AudioEffect

    # define a function "process" that will be called to alter a chunk of audio data
    func override process(out buf: AudioBuffer)
    
        # loop over all the samples in the first (left) channel... and do something stupid
        for mut x in buf.c[0]
            x += 0.1

```

## Example: synthesizer

Typical, simple synthesizers have 2 classes:
* the _pitch renderer_, it es responsible for producing a single, fixed pitch
  * gets instanciated each time, a new pitch is being played
  * is reused whenever the same pitch is played again
* the _synthesizer_ creates and manages the pitch renderers

```kaba
use tsunami.*


class MyRenderer extends PitchRenderer

    # gets called when tsunami needs a chunk of audio for this pitch
    func override mut render(out buf: AudioBuffer) -> bool
        for mut i=>x in buf.c[0]
            # we can use the predefined member "delta_phi"
            x = sin(i * delta_phi)
        # TODO use state to keep track of envelope and phase between chunks...
            
        # return value: keep note alive?
        return true  # forever :(   ...better use envelope > cutoff
    
    # gets called when a midi note starts
    func override on_start(volume: f32)
        # ...restart the envelope, maybe?
    
    # gets called when a midi note ends
    func override on_end()
        # ...release the envelope?


# the actual synthesizer plugin
class MySynthesizer extends Synthesizer

    # gets called 
    func override create_pitch_renderer(pitch: int) -> xfer[PitchRenderer]
        return new MyRenderer(self, pitch)
```

## Example: program extensions

These are general plugins, that can be activated by the user via the bottom bar console.

Usually they will be active as long as the user wants, interacting with the UI and data. But they can also be used for one-shot operations, ending by calling `stop()`.

They go into the `/plugins/independent/` folder.

```kaba
use tsunami.*


class MyExtension extends TsunamiPlugin

    # gets called when starting the plugin
    func override on_start()
        # ...
        
    # gets called when ending the plugin
    func override on_stop()
        # ...
    
```


## State and configuration

* _configuration_ is a special member variable, that is controlled by the user (i.e. the UI in the host program). Plugins must not change it
* _state_ are all other member variables, the plugin can freely change

```kaba
use tsunami.*


# a class containing all parameters
# deriving from "Module.Config" is important! Tsunami will use this to connect to the parameters
class MyConfig extends Module.Config

    # some example parameters/variables
    var some_float: f32
    var some_string: string
    
    # this function is called when tsunami wants to set "neutral"/initial parameters
    func override reset()
        some_float = 1.0
        some_string = ""


class MyAudioEffect extends AudioEffect
    
    # the variable containing the parameters
    var config: MyConfig
    
    # some extra variables acting as the state
    var some_state: f32
    var some_more_state: f32[]
    
    # this function is called to initialize the state, before the plugin is used or reused
    func override reset_state()
        some_state = 13.0
        some_more_state = [1,2,3]

    func override process(out buf: AudioBuffer)
        # use (read from) config
        let a = config.some_float * 2.0
        
        # use and alter the state
        some_state *= 2
```

### Automagic config UI

You define magic strings to let tsunami automatically create UI controls for your parameters:

```kaba
use tsunami.*


let AUTO_CONFIG_SOME_FLOAT = "range=0:1:0.001,scale=1000,unit=ms"
let AUTO_CONFIG_SOME_STRING = ""


class ...
```

This is not a well-designed system. You can find some more examples in [/plugins/](../../plugins).

### User defined UI

If you need full control over the UI...

Admittedly, that is complicated and beyond this document. See [/plugins/](../../plugins) for examples, there is some [API reference](https://wiki.michi.is-a-geek.org/kaba.hui/) for the UI library, and here is a "simple" template:

```kaba
use tsunami.*


# define the class of your config panel
class MyConfigPanel extends ConfigPanel
    # "ConfigPanel" already has a member referencing our effect, but we want to override the exact type
    var override c: MyAudioEffect&
    
    # panel constructor - gets called when panel is created
    func override __init__(_c: Module)
        # define UI
        from_source("
Grid ? ''
    Button button-id 'Button label'
    Edit edit-id ''")
        
        # register a ui event that is called when the button is pressed
        event("button-id", on_button_pressed)
        
    # callback when button is pressed
    func mut on_button_pressed()
        # change plugin config
        c.config.some_float = 42.0
        
        # signal that we have changed the plugin config
        changed() 
        
    # this gets called, then the config has changed
    func override update()
        # set UI state from config


class MyAudioEffect extends AudioEffect

    # this gets called, when tsunami wants a new config panel instance
    func override create_panel() -> xfer[ConfigPanel]
        return new MyConfigPanel(self)
```

