# ssspm (WIP)
my silly simple(ish) package manager!
## usage:
```
ssspm in <package> # install a package
ssspm rm <package> # removes a package
ssspm up # updates the system
ssspm sync # syncs repos, good to do before updating
ssspm look <package> # looks for a package in the repo
```
## dependancys:
- a C++ compiler
- a C++ c ilbary
- [yaml-cpp](https://github.com/jbeder/yaml-cpp)
- [libgit2](https://libgit2.org/)
## todo:
- support for multiple repos
- add a command for adding repos to the config
