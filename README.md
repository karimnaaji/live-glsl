Live-GLSL
=========

![00](images/fragtool.gif)

build and run
-------------

this project uses _CMake_, you can download it [here](http://www.cmake.org/download/) or use an installation package manager like [homebrew](http://brew.sh/).

```bash
brew install cmake
```

To build:

```bash
cmake . -Bbuild
cmake --build build
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
