# Internal design

*might be outdated!*

* [Data behaviour](data.md)
* [Threads](threads.md)
* [Modules](modules.md)
* [Signal chains](signalchains.md)





## Actions

* keep instances (don't delete/new)





## GUI

* `AudioView.cur_vlayer`
  * always trying to be valid, as long as there are valid tracks/vlayers
  * determines `cur_layer`, `cur_track`







## Observer

* `InputStreamAudio` sends in main thread
* `InputStreamMidi` sends in main thread (well, does not send at all...)
* `OutputStream` sends in main thread but needs to be controlled from main thread



## Plugins



