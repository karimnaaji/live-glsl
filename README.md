Live-GLSL
=========

![00](images/fragtool.gif)

build and run
-------------

this project uses _CMake_, you can download it [here](http://www.cmake.org/download/) or use an installation package manager like [homebrew](http://brew.sh/).

```bash
brew install cmake
```

clone the project with submodules:

```bash
git clone git@github.com:karimnaaji/live-glsl.git
git submodule init && git submodule update
mkdir build && cd build
cmake ..
make
```

```bash
./live-glsl.out fragment_shader_to_watch
```

example
-------

[sound experiment video](https://vimeo.com/113176634)

fragment shader inputs
----------------------

Possible use of these uniforms :
 + time : float
 + resolution : vec2
