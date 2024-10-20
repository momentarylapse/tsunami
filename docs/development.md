# Development

## Current state

* [internal design](design/main.md) (TODO)
* [plugins](design/plugins.md)

## Forward

* [broad, long-term plans and ideas](plans/goals.md)
* [short-term detail](plans/todo.md)
* [feedback](feedback)
* [bugs](bugs.md)

## Management

We're on a rolling release model.

There are two permanent branches in the git repository:
* `devel` - the __current development state__. New features will be added here quickly. _Might be unstable!_
* `master` - the __released/stable state__. Follows the `devel` branch with a time delay for testing.

Larger features will be merged from feature branches into `devel`, small ones might be committed directly.

Our test team (i.e. me) will use the program in the field. We also have a [CI](../.github/workflows/cmake-multi-platform.yml) with some automatic tests. When found stable enough, `devel` will be merged into `master` irregularly.

Every 2-4 months there will be a larger [release on github](https://github.com/momentarylapse/tsunami/releases) with binaries. 

We've switched to a date based versioning scheme (e.g. `v2024-10`), since semantic version numbers (`1.2.3`) are scary. Versions are automatically taken from git. Tags define releases and non-release builds automatically identify the current git commit hash.
