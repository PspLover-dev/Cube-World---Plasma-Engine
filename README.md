# Plasma — OpenGL port (Cube World)

An OpenGL reimplementation of the **Plasma** UI/scene engine used by **Cube World**, plus a small test app that loads retail asset databases and renders `.cub` voxel models with a working UI.

The original game used Direct3D 9 (`plasma::D3D9*`). This project replaces that backend with **OpenGL 3.3** (`plasma::OpenGL*`) while keeping class names and layout close to the decompiled `plasma::` sources in `classes/` and `CUBE WORLD CODE/`.

---

## What it does

`plasma_test.exe` (see `app/main.cpp`) is a model viewer and UI sandbox:

- Opens **`data1.db`** — 2857+ `.cub` voxel models
- Opens **`data3.db`** — PNG textures (HUD icons, UI art)
- Loads **`gui.plx`** — compiled Plasma widget tree (partial decode)
- Renders the selected model as colored voxel cubes in 3D
- Shows a **scrollable model list**, **integer jump field**, toolbar buttons, and the **bottom HUD icon bar**

Retail fonts are **not** stored in the databases; text uses system TTF via **stb_truetype** (`PlasmaFont`).

---

## Requirements

- **MSYS2 MinGW64** (`g++`, GLFW, GLEW)
- Game data files in the working directory (or path passed as argv[1]):
  - `data1.db`, `data3.db`, `gui.plx`
  - Optional: `data2.db` (audio), `data4.db` (misc blobs) — not used by the test app yet

---

## Build and run

```bash
# From MSYS2 MinGW64 shell
cd Plasma
bash build.sh
./plasma_test.exe .
```

Or:

```bash
make -j$(nproc)
export PATH=/mingw64/bin:$PATH
./plasma_test.exe .
```

Shaders are loaded at runtime from `shaders/` (`ui.vert`/`ui.frag`, `model.vert`/`model.frag`).

---

## Project layout

```
Plasma/
  app/main.cpp          Test application
  engine/               plasma:: engine (OpenGL port)
    include/plasma/     Public headers
    src/                Implementations
  cube/                 Cube World asset + .cub format code
  shaders/              GLSL programs
  third_party/          sqlite3, stb_image, stb_truetype
  classes/              Ghidra decompile reference (not linked)
  tools/                Python helpers for PLX/DB probing
  data1.db, data3.db, gui.plx   Retail assets (not in repo)
```

---

## Controls (test app)

| Input | Action |
|--------|--------|
| Click list row | Load that model |
| Scroll slider / mouse wheel | Scroll model list |
| `#` field + Enter | Jump to model index (1-based) |
| **Prev** / **Next** / **Random** | Change model |
| **Pause** / **Spin** | Toggle auto-rotation |
| Arrow keys, Page Up/Down | Prev / next model |
| **R** | Random model |
| **Space** | Toggle rotation |

---

## Architecture

```
Display (GLFW window)
  └── OpenGLEngine
        ├── OpenGLDrawing  → UI + scene draw commands
        ├── OpenGLTexture
        └── OpenGLRenderSurface (FBO)

Widget tree (UI)          Node tree (scene) — stub path
  ScrollBar                   Shape subclasses
    ListWidget                GenericShape, MeshShape, …
    ScrollSlider
```

---

## Class reference — `plasma::`

### Core object model

| Class | File | Purpose |
|--------|------|---------|
| **`Object`** | `Object.hpp` | Base refcounted object (`addRef` / `release`). |
| **`NamedObject`** | `NamedObject.hpp` | Named engine object; factory for widgets, fonts, shapes, keyables. |
| **`ObjectManager`** | `NamedObject.hpp` | Registry of named objects on `Engine`. |

### Math

| Class / type | File | Purpose |
|--------------|------|---------|
| **`Vec2`, `Vec3`, `Vec4`, `Mat4`** | `Math.hpp` | Vectors and 4×4 matrices. |
| **`setPerspective`, `setLookAt`, `multiply`, `rotationY`, …** | `Math.hpp` | Camera and transform helpers. |

### Engine & rendering

| Class | File | Purpose |
|--------|------|---------|
| **`Engine`** | `Engine.hpp` | Abstract render API: drawing, textures, matrices, frame lifecycle. |
| **`Drawing`** | `Engine.hpp` | Collects `DrawCommand` lists (triangles, lines, textured quads). |
| **`DrawCommand`** | `Engine.hpp` | One batch: vertices, indices, layout (Scene vs UI), optional texture. |
| **`Texture`** | `Engine.hpp` | Abstract GPU texture. |
| **`RenderSurface`** | `Engine.hpp` | Off-screen color/depth target. |
| **`Display`** | `Display.cpp` | GLFW window wrapper; owns `Engine`. |
| **`OpenGLEngine`** | `OpenGLEngine.hpp` | OpenGL 3.3 implementation of `Engine`. |
| **`OpenGLDrawing`** | `OpenGLEngine.hpp` | Flushes draw commands with UI/model shader programs. |
| **`OpenGLTexture`** | `OpenGLEngine.hpp` | `GL_TEXTURE_2D` RGBA texture. |
| **`OpenGLRenderSurface`** | `OpenGLEngine.hpp` | FBO render target. |

### Scene graph

| Class | File | Purpose |
|--------|------|---------|
| **`Transformation`** | `Node.hpp` | Local TRS + parent chain → `worldMatrix()`. |
| **`Node`** | `Node.hpp` | Scene tree node with optional transform; recurses `draw()`. |

### Shapes (2D/3D scene content)

| Class | File | Purpose |
|--------|------|---------|
| **`Shape`** | `Shape.hpp` | Base drawable node: color, size, offset, visibility. |
| **`GenericShape`** | `Shape.hpp` | Arbitrary vertex/index buffer + primitive type. |
| **`MeshShape`** | `Shape.hpp` | Interleaved scene mesh (pos/normal/color). |
| **`StaticMeshShape`** | `Shape.hpp` | Mesh that builds geometry once. |
| **`SmoothMeshShape`** | `Shape.hpp` | Polygon contour → tessellated mesh (`Tessellate`). |
| **`TextShape`** | `Shape.hpp` | Text label using a `Font`. |
| **`CurveShape`** | `Shape.hpp` | Catmull-Rom spline as line strip. |
| **`Movie`** | `Shape.hpp` | Animated texture sequence (FPS-driven frame advance). |
| **`SceneVertex`** | `Shape.hpp` | Vertex: position, normal, RGB. |

### UI widgets

| Class | File | Purpose |
|--------|------|---------|
| **`Widget`** | `Widget.hpp` | Base UI element: bounds, visibility, children, hit-test, click/drag. |
| **`Button`** | `Widget.hpp` | Clickable button with label and optional icon texture. |
| **`PopUpButton`** | `Widget.hpp` | Button that toggles a popup child widget. |
| **`Edit`** | `Widget.hpp` | Text field with optional **filter** (float/integer/unsigned); `onSubmit` callback. |
| **`ListWidget`** | `Widget.hpp` | Scrollable string list; selection and `scrollOffset`. |
| **`ScrollButton`** | `Widget.hpp` | Directional scroll repeat button (retail Plasma; removed from test scrollbar). |
| **`ScrollSlider`** | `Widget.hpp` | Track + thumb; vertical/horizontal; bound to a `ListWidget`. |
| **`ScrollBar`** | `Widget.hpp` | Composite: **list + vertical `ScrollSlider`** on the right. |
| **`MemberFunctionConnection<T>`** | `Widget.hpp` | Slot-style C++ member function binding (retail pattern stub). |

### Fonts

| Class | File | Purpose |
|--------|------|---------|
| **`Font`** | `Font.hpp` | Abstract: `lineHeight()`, `drawText()`. |
| **`PlasmaFont`** | `Font.hpp` | Vector font via **stb_truetype** + system TTF (replaces retail GDI `PlasmaFont`). |
| **`PixelFont`** | `Font.hpp` | Bitmap atlas font (16×N glyph grid); retail `PixelFont` path. |
| **`ScalableFont`** | `Font.hpp` | Size-scaled TTF draw (minimal stub; retail has full `FontEngine` cache). |
| **`FontEngine`** | `Font.hpp` | Named font registry (`load`, `loadPixel`, `defaultFont`). |

### Input & drawing helpers

| Class | File | Purpose |
|--------|------|---------|
| **`UiInput`** | `UiInput.hpp` | GLFW mouse, wheel, keyboard; dispatches clicks, slider drag, `Edit` typing, `Keyable` shortcuts. |
| **`UiDraw`** | `UiDraw.hpp` | `drawSolidQuad`, `drawTexturedQuad` (screen-space NDC quads). |
| **`Keyable`** | `Keyable.hpp` | Named map of key code → `std::function` handler. |

### Attributes & animation data

| Class | File | Purpose |
|--------|------|---------|
| **`Attribute`** | `Attribute.hpp` | Polymorphic attribute base (`clone`, `typeName`). |
| **`ContinuousAttribute<T>`** | `Attribute.hpp` | Interpolatable scalar/vector (float, Vec2–Vec4, Mat4). |
| **`ContinuousArrayAttribute<T>`** | `Attribute.hpp` | Keyframe arrays. |
| **`DiscreteAttribute<T>`** | `Attribute.hpp` | Step/discrete values (int, string, wstring). |
| **Type aliases** | `Attribute.hpp` | `FloatAttribute`, `IntAttribute`, `Vec4Attribute`, etc. |

### PLX loader (GUI binary)

| Class | File | Purpose |
|--------|------|---------|
| **`PlxWidgetDef`** | `PlxLoader.hpp` | Widget type entry from PLX header. |
| **`PlxNode`** | `PlxLoader.hpp` | Widget tree node: frame, texture, children. |
| **`PlxStreamReader`** | `PlxLoader.hpp` | Binary stream decoder for compiled PLX blobs. |
| **`PlxDocument`** | `PlxLoader.hpp` | Load `gui.plx`; parse header/tree; `buildRootWidget()`. |

### Filters (Edit widgets)

| Class | File | Purpose |
|--------|------|---------|
| **`FloatFilter`** | `Filters.hpp` | Parse/validate float text. |
| **`IntegerFilter`** | `Filters.hpp` | Parse/validate signed integer text. |
| **`UnsignedFloatFilter`** | `Filters.hpp` | Non-negative float. |
| **`UnsignedIntegerFilter`** | `Filters.hpp` | Non-negative integer. |

### Geometry utilities

| Class | File | Purpose |
|--------|------|---------|
| **`Tessellate`** | `Tessellate.hpp` | Ear-clipping 2D triangulation; Catmull-Rom splines (replaces GLU). |

### Exceptions

| Class | File | Purpose |
|--------|------|---------|
| **`Exception`** | `Exception.hpp` | Base `std::runtime_error`. |
| **`InvalidFileFormatException`** | `Exception.hpp` | Bad asset/PLX format. |
| **`InvalidVersionException`** | `Exception.hpp` | Unsupported file version. |
| **`InvalidDemoLicenseException`** | `Exception.hpp` | Retail license check (ported for API parity). |

---

## Class reference — `cube::`

| Class / function | File | Purpose |
|------------------|------|---------|
| **`descrambleBlob`** | `BlobDescramble.hpp` | XOR/descramble blob payload from SQLite (retail encoding). |
| **`CubModel`** | `Assets.hpp` | Voxel grid: RGB bytes, dimensions, load from blob. |
| **`CubMeshVertex`** | `Assets.hpp` | One rendered voxel face vertex (pos, normal, color). |
| **`CubMesh`** | `Assets.hpp` | Triangle mesh built from a model. |
| **`CubMeshBuilder`** | `Assets.hpp` | Extrudes visible voxel faces into indexed mesh. |
| **`AssetDatabase`** | `Assets.hpp` | SQLite wrapper: `fetchBlob`, `listKeys` on `data*.db`. |
| **`ModelCatalog`** | `Assets.hpp` | High-level API over `data1.db` for `.cub` names and meshes. |
| **`DecodedImage`** | `TextureCatalog.hpp` | CPU-side RGBA PNG decode result. |
| **`TextureCatalog`** | `TextureCatalog.hpp` | PNG blobs from `data3.db` → GPU textures. |
| **`GuiHud`** | `GuiHud.hpp` | Bottom bar: skills, crafting, inventory, map, system, help icons. |

---

## Asset databases (retail)

| File | Contents |
|------|-----------|
| **`data1.db`** | `.cub` voxel models (keys like `barrel.cub`) |
| **`data2.db`** | Audio (not used by test app) |
| **`data3.db`** | PNG textures / HUD icons |
| **`data4.db`** | Small text/XML blobs |
| **`gui.plx`** | Compiled Plasma UI widget stream |

Fonts in retail Plasma load from **filesystem** (`fonts/`, `C:\Windows\Fonts\*.ttf`), not from these DBs.

---

## Decompiled reference (`classes/`)

The `classes/` folder contains **Ghidra exports** of the original D3D9 Plasma engine (`plasma__Button.cpp`, `plasma__ScalableFont.cpp`, etc.). These files are **not compiled** into `plasma_test.exe`; they are kept for reverse-engineering parity when porting behavior.

Notable retail-only classes not yet fully ported:

- `plasma::ScalableFont` — full font cache + `FontEngine` integration
- `plasma::D3D9RenderSurface` — replaced by `OpenGLRenderSurface`
- Full `plasma::Button` PLX attribute binding (~560-byte layout)

---

## Implementation status

| Area | Status |
|------|--------|
| `.cub` voxel rendering | Working |
| OpenGL UI quads + text (TTF) | Working |
| Model list + `ScrollSlider` | Working |
| HUD icon bar from `data3.db` | Working |
| `gui.plx` compiled stream | Partial (Seal button + children) |
| Full retail widget fidelity | In progress |
| `ScalableFont` / `FontEngine` cache | Stub only |
| Audio (`data2.db`) | Not started |

---

## License

Game assets (`data*.db`, `gui.plx`) are **Cube World retail data** — use only if you own the game. Engine port code in `engine/`, `cube/`, and `app/` is written for this project; third-party: sqlite3, stb, GLFW, GLEW.
