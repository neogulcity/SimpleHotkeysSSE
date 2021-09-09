# Simple Hotkeys
This plugin made with Example plugin template for use with CommonLibSSE by [Ryan-rsm-McKenzie.](https://github.com/Ryan-rsm-McKenzie/ExamplePlugin-CommonLibSSE)

SKSE64 Plugin made for using hotkeys with EquipSet.

Currently CommonLibSSE all included in `extern`. I want to submodule it rather then included it all but, don't know how to do it with modified one.

## Requirements
* [CMake](https://cmake.org/)
	* Add this to your `PATH`
* [PowerShell](https://github.com/PowerShell/PowerShell/releases/latest)
* [Vcpkg](https://github.com/microsoft/vcpkg)
	* Add the environment variable `VCPKG_ROOT` with the value as the path to the folder containing vcpkg
* [Visual Studio Community 2019](https://visualstudio.microsoft.com/)
	* Desktop development with C++

## Register Visual Studio as a Generator
* Open `x64 Native Tools Command Prompt`
* Run `cmake`
* Close the cmd window

## Building
```
git clone https://github.com/neogulcity/SimpleHotkeysSSE
cd SimpleHotkeysSSE
cmake -B build -S .
```
