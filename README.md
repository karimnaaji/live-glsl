fragtool
========

![00](images/fragtool.gif)

build and run
-------------

cloning with submodules:

```bash
git clone --recursive git@github.com:karimnaaji/fragtool.git
git submodule foreach git pull
```

```bash
mkdir build && cd build
cmake ..
make
```

```bash
./fragtool.out fragment_shader_to_watch
```

fragment shader inputs
----------------------

Possible use of these uniforms : 
 + time : float
 + resolution : vec2
