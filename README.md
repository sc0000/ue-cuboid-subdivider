# Unreal Engine Cuboid Subdivider

## Overview

This is a tiny Ureal Engine Plugin/Blueprint Function Library to increase vertex density of cuboids along the longest two axes, for easier vertex painting of walls, floors, and ceilings.
Operation is extremely simple: Load the editor widget, set the max size for subdivision side length, select cuboids in the world, click the button. The selected cuboids will then be replaced with subdivided ones. Non-cuboids are ignored.

## Build & Run

Please note that this software has only ever been built, run, and tested on Windows 10 (64-bit).
As is, it requires Unreal Engine 5.3, as well as a current version of Microsoft Visual Studio. It should be more or less trivial to update to newer versions of Unreal Engine.

To build and run the host project, create a VS solution from the `.uproject` file: Right click -> `Generate Visual Studio project files.`
Build and run the solution.

Or just copy the plugin into another Unreal Engine project, and build it as part of that.

After building, load the widget from the editor: Right click `Plugins/CuboidSubdivider Content/WBP_CuboidSubdivider` -> `Run Editor Utility Widget`.

## Demo

[Demo](https://youtu.be/fS0Md-9VCQU)

## Note

This plugin started as part of a failed experiment while working on the [Viz2 Attention Tracking Tool for Architects](https://github.com/sc0000/viz2-attention-tracking).

