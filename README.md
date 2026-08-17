# Engine — Vulkan Ray Tracing Engine

A C++20 real-time rendering engine built on the **Vulkan** graphics API, featuring hardware-accelerated **ray tracing** (via `VK_KHR_ray_tracing_pipeline`), a **CPU path tracing backend powered by Intel Embree**, compute shaders, disney material and an in-app editor built with **Dear ImGui** / **ImGuizmo**.

> Repo: [`cdoresca/engineVulkanRayTraicing`](https://github.com/cdoresca/engineVulkanRayTraicing)

## Features

- **Vulkan ray tracing pipeline** — ray generation (`.rgen`), closest-hit (`.rchit`), and miss (`.rmiss`) shaders compiled to SPIR-V at build time
- **Compute shaders** (`.comp`) for GPU-side post-processing / auxiliary passes
- **Embree-backed CPU ray tracing**, fetched automatically as a prebuilt binary during CMake configuration
- **Scene loading** with `tinyobjloader` (`.obj` meshes) and `nlohmann/json` (scene/config description)
- **Image I/O** via `stb`
- **Math** via `GLM`
- **Windowing & input** via `GLFW`
- **In-engine UI & gizmos** via `Dear ImGui` and `ImGuizmo`

## Project structure

```
.
├── docs/      # documentation / notes
├── libs/      # third-party dependencies (glm, glfw, imgui, imguizmo, stb, nlohmann, tinyobjloader...)
├── output/    # build/render output
├── src/       # engine source (including src/shader for .rgen/.rchit/.rmiss/.comp shaders)
├── CMakeLists.txt
└── run.bat    # build (Release) and launch the engine
```

## Requirements

- **Windows** (the current build setup targets Windows: prebuilt GLFW `lib-vc2022` binaries, Embree Windows release, `.bat` scripts)
- **Visual Studio 2022** (or another VS 2022–compatible C++ toolchain)
- **CMake ≥ 4.0**
- **[Vulkan SDK](https://vulkan.lunarg.com/)**, with the `VULKAN_SDK` environment variable set (used to locate `glslc.exe` for shader compilation and the Vulkan headers/libraries)
- A GPU and driver supporting `VK_KHR_ray_tracing_pipeline` for the hardware ray tracing path
- Internet access on first configure (CMake's `FetchContent` downloads a prebuilt Embree 4.4.0 release)

## Building

```bash
git clone https://github.com/cdoresca/engineVulkanRayTraicing.git
cd engineVulkanRayTraicing

cmake -B build
cmake --build build --config Release
```

On first configure, CMake will:
1. Add the bundled libraries under `libs/` (GLM, GLFW, ImGui, ImGuizmo, stb, nlohmann, tinyobjloader)
2. Download and configure a prebuilt **Embree 4.4.0** Windows release
3. Locate the system **Vulkan SDK**
4. Compile all shaders in `src/shader` (`.rgen`, `.rchit`, `.rmiss`, `.comp`) to SPIR-V via `glslc`
5. Copy the required Embree DLLs (`embree4.dll`, `tbb12.dll`, `tbbmalloc.dll`) next to the built executable

## Running

Use the provided script, which builds in Release and launches the engine:

```bat
run.bat
```

Or manually, after building:

```bat
build\Release\Engine.exe
```

## Notes

- This project originates from an academic assignment (course project `H26`, École/Université de Sherbrooke). The repository was later mirrored to GitHub.
- Shader compilation targets `vulkan1.4` via `glslc`; make sure your installed Vulkan SDK version supports this target environment.
