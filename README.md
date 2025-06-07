# CC-MPC-Align

This repository contains a sample CloudCompare plugin that demonstrates how to align multiple point clouds using sequential ICP steps.

## Building

The plugin is designed to be compiled as part of CloudCompare's plugin system.
Clone CloudCompare and place this repository under the `plugins` directory or add it as an external plugin. Then enable the `MultiAlignPlugin` option in CMake and build CloudCompare normally.

## Usage

Once compiled and loaded, select several point clouds in CloudCompare and trigger the **MultiCloud Alignment** action from the Plugins menu. The first selected cloud acts as the reference and all other clouds will be aligned to it using the standard ICP algorithm.
