> [!NOTE]
> This template for your C++ projects, using: CMake, clang, ninja-build (cmake generator)
>
> Make sure you have dependencies on your system and if not, you can find help below.

# Windows

1. Install [LLVM](https://github.com/llvm/llvm-project/releases). While installing make sure to set PATHs.
2. Install [CMake](https://cmake.org/download/). While installing make sure to set PATHs. 
3. Install [Ninja](https://github.com/ninja-build/ninja/releases). Just place .exe in CMake/bin folder.

# Linux

Using package manager your linux distro came with, install following packages:
```
sudo apt install clang
sudo apt install cmake
sudo apt install ninja-build
```
# Mac

This template doesn't not support Mac, you would need to modify [CMakePresets.json](CMakePresets.json) in order to support it. PRs are welcome.
