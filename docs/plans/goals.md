# Development plans (long-term)


## Windows version

Mostly working.

Remaining issues:

* `gtk` is annoying on windows
* not all utf-8 supported?!?
* installing and file associations would be nice


## MacOS version

Also mostly working on MacBooks with (aarch64) M1 chip or newer.

Remaining issues:

* (properly) distributable app bundles
* file associations
* `CoreAudio`

## Modular synthesizers

It would be cool, to simply create synthesizers by combining existing audio modules. A lot of the infrastructure already exists, but:

* currently, a lot of overhead (not sure, how problematic that really is)
* requires some additional modules for basic midi -> wave conversion
* requires support for "modules from signal chains"


## Plugin clean-up

Too many effects and plugins are obsolete.
* some should be merged or deleted
* better version migration needed
* namespaces for internal/user fx?


## UI improvements

No clear long-term goals. Too many small ideas. But it feels important.


## Just some crazy ideas

### External plugin support (LV2)?

Really not sure, how much effort that would be. The design philosophies might be too far apart.


### Replace gtk

Gtk (the user interface library) is becoming more and more painful to maintain. There is already a wrapper layer for potential future replacements. But so far, I don't like any of the alternatives.

One option would even be, to manage everything ourselves (similarly to blender). Many UI elements only use gtk as a way of drawing on the window, while the actual interactivity and layout is managed by our own code. This could be expanded, handling all UI elements ourselves. That would solve a lot of problems, but would also require a lot of desktop integration handling (global clipboard, theming, etc.).
