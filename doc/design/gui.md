# GUI

## `AudioView`

### `SceneGraph`

* `Background` - covers everything without tracks
* `TimeScale` - the thin scale at the top
* `AudioViewTrack` - represents a track
  * `AudioViewLayer` - represents a single version/layer of a track

### Behaviour

* `AudioView.cur_vlayer`
  * always trying to be valid, as long as there are valid tracks/vlayers
  * determines `cur_layer`, `cur_track`
* ...

## `SideBar`

### `SongConsole`

### `SampleManager`

### `TrackConsole`

### `CurveConsole`

## `BottomBar`

### `MixingConsole`

### `SignalEditor`

### `LogConsole`














