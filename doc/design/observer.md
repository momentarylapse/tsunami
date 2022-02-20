# Observer

Most parts are connected via a modified observer pattern (update subscription via callbacks).

* `InputStreamAudio` sends in main thread
* `InputStreamMidi` sends in main thread (well, does not send at all...)
* `OutputStream` sends in main thread but needs to be controlled from main thread



