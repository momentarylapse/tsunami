# Threads

## Layout

* GUI thread
  * editing (Actions)
  * Observer messages
* PeakThread
* in/out signal chains having their own threads
  * `SongRenderer` might become multi-threaded...

* Observer messages from other threads must be forwarded via `hui::run_later()`

## Locking

* `PeakThread` locks Song (write,try)
* `MidiPreview` locks self
* `ViewModeCapture.draw_post()` locks `Audio/MidiAccumulator.buf`
* `PerformanceMonitor` locks self
* FFT locks planner
* `SignalChain`...send-to-modules locks self
* `SongRenderer` locks Song (read)
* `Audio/MidiAccumulator.suck()` locks buf
* `hui::runner`... locks self
* `Actions` lock `Song` (write)
* `RingBuffer.clear()` locks buf (....)

* `SongRenderer` configuration (`allow_layers` `set_pos` etc) don't lock, instead they buffer the values and the locked `read()` will really update

## Safety

???
`Song.StructureReader`

`RingBuffer`
* safe for single consumer + single producer

### False positives

* `OutputStream.paused`
* `AudioSucker.running`
* `RingBuffer`
  * reading/writing: locks before and after, but not during mem access still safe because of these locks



## Observer

* `InputStreamAudio` sends in main thread
* `InputStreamMidi` sends in main thread (well, does not send at all...)
* `OutputStream` sends in main thread, but needs to be controlled from main thread




