# Actions

Each editing action internally has its own class with data and functions to perform the action and to undo it.

A series of actions is kept as the undo history.

* data is not deleted, but kept in the action that "deleted" it
  * might take shared ownership


