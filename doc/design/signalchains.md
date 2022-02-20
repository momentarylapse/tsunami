# Signal chains

## Signals

* sucking protocol (data is always read)
  * the consumer owns the buffer and sends a reference to the producer to fill
  * the producer returns the number of samples really written
  * can return `END_OF_STREAM` ...

## Modules

### Module commands

* `RESET_STATE` - reset internal data/buffers (not affecting accum)
* `START/STOP` - "unpause/pause" mostly for streams
* `PREPARE_START` - mostly for `OutputStream`... prebuf
* `ACCUMULATION_START/STOP/RESET` - mostly for recorders


