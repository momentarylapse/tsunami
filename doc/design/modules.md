# Modules

## Config/panel

* `ConfigPanel` = basic gui definition (easy to write in plugins)
* `ModulePanel` = wrapper with favourites (presets) and action linking...
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






