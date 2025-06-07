# CC-MPC-Align

This repository contains a sample CloudCompare plugin that demonstrates how to align multiple point clouds using sequential ICP steps.

## Building

The plugin is designed to be compiled as part of CloudCompare's plugin system.
- Clone CloudCompare and place this repository under the `plugins` directory or add it as an external plugin.
- Then enable the `MultiAlignPlugin` option in CMake and build CloudCompare normally.

### Building on macOS (Apple Silicon)

1. Install the required tools via Homebrew:
   ```bash
   brew install cmake qt5 git
   ```
2. Fetch CloudCompare 2.14 sources and this repository:
   ```bash
   git clone --branch v2.14 https://github.com/CloudCompare/CloudCompare.git
   cd CloudCompare/plugins
   git clone <this repo url> CC-MPC-Align
   ```
3. Create a build directory and configure with CMake, enabling the plugin:
   ```bash
   mkdir ../build && cd ../build
   cmake -DCMAKE_PREFIX_PATH=$(brew --prefix qt5) -DPLUGIN_MULTIALIGNPLUGIN=ON ..
   ```
4. Build CloudCompare as usual:
   ```bash
   cmake --build . --target ALL_BUILD --config Release
   ```
5. The resulting CloudCompare.app will contain the plugin in `Contents/MacOS/plugins`.

### Building the plugin standalone

If you already have a compiled CloudCompare available, the plugin can be built on
its own. Set the `CLOUDCOMPARE_DIR` environment variable to point at your
CloudCompare installation and provide Qt through `CMAKE_PREFIX_PATH`:

```bash
export CLOUDCOMPARE_DIR=/path/to/CloudCompare
export CMAKE_PREFIX_PATH=/path/to/Qt/lib/cmake
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release \
  -DCLOUDCOMPARE_DIR="$CLOUDCOMPARE_DIR" \
  -DCMAKE_PREFIX_PATH="$CMAKE_PREFIX_PATH"
cmake --build . --config Release
```

The resulting `MultiAlignPlugin.so` will be located in `build/plugins/`.

## Usage

Once compiled and loaded, select several point clouds in CloudCompare and trigger the MultiCloud Alignment action from the Plugins menu.
The first selected cloud acts as the reference and all other clouds will be aligned to it using the standard ICP algorithm.
If the dialog's **Save transformations to file** option is checked, all resulting
4x4 matrices (including the identity for the reference cloud) are written to
`alignment_transforms.txt` in your Documents folder.

## Open3D Example

The `scripts/fgr_multi_align.py` script demonstrates how to perform a fast global
registration using Open3D's FGR algorithm followed by an ICP refinement. It
aligns multiple clouds provided as `.ply` files on the command line and prints
the resulting 4x4 transformation matrices.

To run the script, make sure the [Open3D](https://github.com/isl-org/Open3D) Python
package is installed:

```bash
pip install open3d
```

```bash
python scripts/fgr_multi_align.py cloud1.ply cloud2.ply cloud3.ply
```

This script illustrates how external libraries mentioned in
`research_methods.md` can complement the basic ICP-based plugin.