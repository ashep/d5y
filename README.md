# D5Y

A minimalistic library for building ESP32-based devices.

## Installation

```shell
cd your-project
git submodule add --name d5y git@github.com:ashep/d5y.git vendor/d5y
```

Add the following to your project's `CMakeLists.txt`:

```cmake
set(EXTRA_COMPONENT_DIRS ./vendor/d5y)
```
