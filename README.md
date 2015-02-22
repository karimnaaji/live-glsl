fragtool
========

![00](images/fragtool.gif)

build and run
-------------

cloning with submodules:

```bash
git clone git@github.com:karimnaaji/fragtool.git
git submodule init && git submodule update
```

```bash
mkdir build && cd build
cmake ..
make
```

```bash
./fragtool.out fragment_shader_to_watch [sound_path]
```

example
-------

[sound experiment video](https://vimeo.com/113176634)

fragment shader inputs
----------------------

Possible use of these uniforms :
 + time : float
 + resolution : vec2
 + uniform float spectrum[256]; // spectrum of sound
 + uniform float wave[256];     // wave of sound
