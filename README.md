# Plasma OpenGL port (Cube World)

An OpenGL reimplementation of the **Plasma** UI/scene engine used by **Cube World**, plus a test viewer (`plasma_test.exe`) that loads retail asset databases, renders `.cub` voxel models, plays game audio, and exercises the UI stack.

The original game used Direct3D 9 (`plasma::D3D9*`). This project replaces that backend with **OpenGL 3.3** (`plasma::OpenGL*`) while keeping class names and layout close to the decompiled `plasma::` / `cube::` sources in `classes/`.

---

## What it does

`plasma_test.exe` (`app/main.cpp`) is a **model + audio viewer** (no player movement / WASD):

| Feature | Details |
|---------|---------|
| **3D preview** | Renders `.cub` voxel models as colored cubes; auto-rotating camera |
| **Model source tabs** | **Model** · **DB** · **Race** · **Items** · **Equip** — five ways to pick what to load |
| **Model list** | Scrollable list in the left panel; click a row to load |
| **Icon gallery** | Bottom panel — scrollable grid of **all PNG icons** from `data3.db` (104 in this copy) |
| **Sound playback** | 93 WAV SFX from `data2.db` via **XAudio2** (descrambled retail blobs) |
| **Music playback** | OGG tracks from `{gameRoot}/Music/*.ogg` via **stb_vorbis** + XAudio2 |
| **Retail fonts** | UI text from `resource1.dat`; decorative title from `resource2.dat` |
| **Registries** | Race / item / equip name lists extracted from decompiled retail init order |

Shaders load at runtime from `shaders/` (`ui.vert`/`ui.frag`, `model.vert`/`model.frag`).

---

## Requirements

- **MSYS2 MinGW64** (`g++`, GLFW, GLEW, XAudio2)
- Retail game data in the working directory (or path passed as `argv[1]`):

| File / folder | Required | Purpose |
|---------------|----------|---------|
| `data1.db` | Yes | `.cub` voxel models (~2857 keys) |
| `data2.db` | For audio | Scrambled WAV SFX (93 entries) |
| `data3.db` | For icons | Scrambled PNG UI icons (104 entries) |
| `resource1.dat` | For fonts | Block UI TrueType font (renamed `.ttf`) |
| `resource2.dat` | For fonts | Script/decorative TrueType font (Venice-style) |
| `Music/` | For music | OGG files on disk (`maintheme.ogg`, `greenlands.ogg`, …) — **not** in DBs |
| `data4.db` | Optional | Language XML blobs (`dict_en.xml`, `dict_de.xml`) |
| `gui.plx` | Optional | Compiled Plasma UI (partial decode in engine) |

Copy `Music/` from your Cube World install; it is not embedded in `Cube.exe` or the databases.

---

## Build and run

```bash
# From MSYS2 MinGW64 shell
cd "CW Player"
bash build.sh
./plasma_test.exe .
```

Or:

```bash
make -j$(nproc)
export PATH=/mingw64/bin:$PATH
./plasma_test.exe /path/to/cube/world/install
```

The single argument is the **game root** — the folder containing `data1.db`, `resource1.dat`, etc.

---

## Test app layout

```
┌─────────────────┬──────────────────────────────────┐
│  Side panel     │  3D model preview                │
│  (280 px)       │  "Cube World" (script font)      │
│                 │  status line (UI font)           │
│  Model|DB|Race  │                                  │
│  Items|Equip    │                                  │
│  ─ model list   │                                  │
│  ─ Sound|Music  │                                  │
│  ─ audio list   │                                  │
│  [Play selected]│                                  │
├─────────────────┴──────────────────────────────────┤
│  Icon gallery — all data3.db PNGs (scrollable)       │
└────────────────────────────────────────────────────┘
```

### Controls

| Input | Action |
|--------|--------|
| **Model / DB / Race / Items / Equip** | Switch model list source |
| Click model list row | Load and preview that entry |
| Mouse wheel on list | Scroll model list |
| **Sound / Music** | Switch audio list mode |
| Click audio row + **Play selected** | Play SFX or OGG track |
| Mouse wheel on icon gallery | Horizontal scroll through icons |

Preview rotates automatically; there is no free-camera or player control.

### Model source tabs

| Tab | Source | Count (approx.) | Notes |
|-----|--------|-----------------|-------|
| **Model** | All keys in `data1.db` | ~2857 | Single `.cub` file per entry |
| **DB** | Creature body prefixes (`*-body.cub`) | varies | Multi-part creatures (body, foot, head, hands) |
| **Race** | `WorldRaceRegistry.inc` | 154 | Retail race/creature registry names |
| **Items** | `WorldItems.inc` | 147 | Retail item registry names |
| **Equip** | `WorldEquipRegistry.inc` | 195 | Weapons, armor, accessories |

`RegistryModelResolver` maps registry names to `data1.db` keys where possible. Coverage is partial — e.g. player races (Elf/Human) have no `-body.cub` in this DB copy; many equip entries lack matching models.

---

## Project layout

```
CW Player/
  app/main.cpp              Test viewer (model + audio + UI)
  engine/                   plasma:: engine (OpenGL port)
    include/plasma/         Public headers
    src/                    Implementations
  cube/                     Cube World game logic + asset code
    include/cube/           Headers (World, Creature, audio, …)
    src/                    Implementations + .inc registry data
  shaders/                  GLSL programs
  third_party/              sqlite3, stb_image, stb_truetype, stb_vorbis
  data1.db … gui.plx        Retail assets (when copied from install)
  resource1.dat             Retail UI font (plain TTF renamed)
  resource2.dat               Retail script font (plain TTF renamed)
```

---

## Architecture

```
Display (GLFW window)
  └── OpenGLEngine
        ├── OpenGLDrawing  → UI + scene draw commands
        ├── OpenGLTexture
        └── OpenGLRenderSurface (FBO)

Widget tree (UI)              Node tree (scene)
  Side panel + bottom gallery     Shape / mesh draw path
  ListWidget, Button, …
```

Audio path:

```
data2.db ──► WavLoader (descramble) ──► XAudio2 Sound
Music/*.ogg ──► stb_vorbis ──► XAudio2 Music
```

Font path:

```
resource1.dat ──► stb_truetype atlas ──► uiFont() / PlasmaFont (UI slot)
resource2.dat ──► stb_truetype atlas ──► scriptFont() / ScriptFont
```

---

## Retail asset files

### Databases (`data*.db`)

| File | Contents | Used by viewer |
|------|----------|----------------|
| **`data1.db`** | `.cub` voxel models (`barrel.cub`, `goblin-male-body.cub`, …) | Yes — primary model source |
| **`data2.db`** | 93 scrambled WAV SFX blobs | Yes — sound list + playback |
| **`data3.db`** | 104 scrambled PNG icons / UI art | Yes — bottom icon gallery |
| **`data4.db`** | 2 XML language blobs (`dict_en.xml`, `dict_de.xml`) | Not yet |
| **`gui.plx`** | Compiled Plasma UI widget stream | Partial PLX decode only |

Blobs in `data2.db` / `data3.db` are **descrambled** at load time (`cube::descrambleBlob`).

### Font files (`resource*.dat`)

Both files in a retail install are **plain TrueType fonts renamed to `.dat`** — not custom multi-font containers (in the Steam copy inspected here).

| File | Role | Technical notes |
|------|------|-----------------|
| **`resource1.dat`** | Main block UI font | ~19 KiB, 232 glyphs, bold (700), FontForge-built; full ASCII + Latin-1 + extended Latin |
| **`resource2.dat`** | Decorative script font | ~15 KiB, 151 glyphs, medium (500); based on classic Mac **Venice** cursive; smart quotes, dashes |

Decompiled `Cube.exe` references `resource1.dat` hundreds of times via `plasma::ScalableFont` / `FontEngine`. **`resource2.dat` does not appear as a string** in the Ghidra exports in this repo (likely loaded indirectly or from UI data).

The viewer loads both via `cube::fonts::init(gameRoot)` → `plasma::initRetailFonts`. Widgets use `uiFont()`; the 3D viewport title uses `scriptFont()`.

Modders replace these by dropping a `.ttf` renamed to `resource1.dat` / `resource2.dat`.

### Music

BGM lives in **`Music/*.ogg`** on disk (e.g. `maintheme.ogg`, `greenlands.ogg`). It is **not** in `data2.db` or embedded in the executable.

---

## Registries and model resolver

Retail init order was extracted from decompiled code into include files:

| File | Entries | Purpose |
|------|---------|---------|
| `cube/src/WorldRaceRegistry.inc` | 154 | Creature/race IDs and names |
| `cube/src/WorldItems.inc` | 147 | Item registry |
| `cube/src/WorldEquipRegistry.inc` | 195 | Equipment slots (id + subId + name) |
| `cube/src/WorldRegistryInit.inc` | — | Retail `ItemRegistry` registration order |
| `cube/src/RegistryModelOverrides.inc` | generated | Name → `data1.db` model prefix map |

`tools/build_registry_model_map.py` regenerates `RegistryModelOverrides.inc`.  
`cube::RegistryModelResolver` resolves registry names to creature prefixes or equip model keys at runtime.

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
| **`ScrollButton`** | `Widget.hpp` | Directional scroll repeat button. |
| **`ScrollSlider`** | `Widget.hpp` | Track + thumb; vertical/horizontal; bound to a `ListWidget`. |
| **`ScrollBar`** | `Widget.hpp` | Composite: **list + vertical `ScrollSlider`** on the right. |
| **`MemberFunctionConnection<T>`** | `Widget.hpp` | Slot-style C++ member function binding (retail pattern stub). |

### Fonts

| Class / API | File | Purpose |
|-------------|------|---------|
| **`Font`** | `Font.hpp` | Abstract: `lineHeight()`, `drawText()`. |
| **`PlasmaFont`** | `Font.hpp` | Vector font via **stb_truetype** + retail `resource*.dat`. |
| **`ScriptFont`** | `Font.hpp` | Decorative font wrapper (`resource2.dat`). |
| **`PixelFont`** | `Font.hpp` | Bitmap atlas font (16×N glyph grid). |
| **`ScalableFont`** | `Font.hpp` | Size-scaled draw from UI or script slot. |
| **`FontEngine`** | `Font.hpp` | Named font registry (`load`, `loadPixel`, `defaultFont`, `scriptFont`). |
| **`initRetailFonts`** | `Font.hpp` | Load `resource1.dat` + `resource2.dat` from game root. |
| **`uiFont()` / `scriptFont()`** | `Font.hpp` | Global accessors used by widgets and the test app. |

### Input & drawing helpers

| Class | File | Purpose |
|--------|------|---------|
| **`UiInput`** | `UiInput.hpp` | GLFW mouse, wheel, keyboard; dispatches clicks, slider drag, `Edit` typing. |
| **`UiDraw`** | `UiDraw.hpp` | `drawSolidQuad`, `drawTexturedQuad` (screen-space NDC quads). |
| **`Keyable`** | `Keyable.hpp` | Named map of key code → `std::function` handler. |

### Attributes, PLX, filters, utilities

See headers under `engine/include/plasma/` — `Attribute.hpp`, `PlxLoader.hpp`, `Filters.hpp`, `Tessellate.hpp`, `Exception.hpp` (ported for API parity with decompile).

---

## Class reference — `cube::`

### Assets & rendering

| Class / function | File | Purpose |
|------------------|------|---------|
| **`descrambleBlob`** | `BlobDescramble.hpp` | XOR/descramble blob payload from SQLite. |
| **`CubModel`** | `Assets.hpp` | Voxel grid: RGB bytes, dimensions, load from blob. |
| **`CubMeshBuilder`** | `Assets.hpp` | Extrudes visible voxel faces into indexed mesh. |
| **`AssetDatabase`** | `Assets.hpp` | SQLite wrapper: `fetchBlob`, `listKeys` on `data*.db`. |
| **`ModelCatalog`** | `Assets.hpp` | High-level API over `data1.db` for `.cub` names and meshes. |
| **`TextureCatalog`** | `TextureCatalog.hpp` | PNG blobs from `data3.db` → GPU textures. |
| **`RegistryModelResolver`** | `RegistryModelResolver.hpp` | Maps registry names → model keys / creature prefixes. |
| **`RetailFonts`** | `RetailFonts.hpp` | Thin wrapper around `plasma::initRetailFonts`. |
| **`CubeShader` / `Camera`** | `CubeShader.hpp`, `Camera.hpp` | Scene rendering helpers. |

### Audio

| Class / function | File | Purpose |
|------------------|------|---------|
| **`WavLoader`** | `WavLoader.hpp` | Parse WAV; descramble path for DB blobs. |
| **`AudioFileLoader`** | `AudioFileLoader.hpp` | Load OGG/WAV from filesystem; `listMusicTracks()`. |
| **`audio::Sound`** | `XAudio2Engine.hpp` | One-shot SFX playback. |
| **`audio::Music`** | `XAudio2Engine.hpp` | Looping OGG music playback. |
| **`XAudio2Engine`** | `XAudio2Engine.hpp` | Engine init + DB load for `data2.db`. |

### Game logic (ported from decompile)

Headers are grouped in `cube/include/cube/All.hpp`. Compiled into the library but **not all wired into the test viewer**:

| Area | Key classes | Status |
|------|-------------|--------|
| **World / terrain** | `World`, `Zone`, `Region`, `Terrain`, `Chunk`, `ChunkBuffer`, `Dungeon`, `House` | Ported |
| **Entities** | `Creature`, `Controller`, `GameController`, `Sprite`, `SpriteManager` | Ported |
| **Behaviors** | `WalkPathBehavior`, `LookAtPlayerBehavior`, `SpawnLocationBehavior`, `CombatBehavior`, … | Ported |
| **Items** | `ItemRegistry` + `.inc` registries | Ported + populated |
| **UI widgets** | `StartMenuWidget`, `ChatWidget`, `InventoryWidget`, `CharacterStyleWidget`, … | Headers + partial stubs |
| **Quest / text** | `QuestText`, `QuestTextNode`, `Speech` | Partial |
| **World info** | `WorldInfo`, `WorldMap` | Partial (threaded loop incomplete) |
| **Action poses** | `ActionConfig`, `AnimPose`, `PartKind` | Used by viewer for idle pose |

---

## Decompiled reference (`classes/`)

The `classes/` folder contains **Ghidra exports** of the original D3D9 Plasma engine and Cube game code (`plasma__Button.cpp`, `plasma__ScalableFont.cpp`, `cube__World.cpp`, etc.). These files are **not compiled** into `plasma_test.exe`; they are reference for reverse-engineering parity.

Retail-only pieces not fully ported to OpenGL:

- Full `plasma::ScalableFont` font cache (retail GDI / file-container path)
- `plasma::D3D9RenderSurface` → replaced by `OpenGLRenderSurface`
- Full PLX widget attribute binding (~560-byte `Button` layout)
- Complete `gui.plx` runtime widget tree

---

## Implementation status

| Area | Status |
|------|--------|
| `.cub` voxel rendering + auto-rotate preview | Working |
| Model tabs (Model / DB / Race / Items / Equip) | Working |
| Registry lists + model resolver | Working (partial model coverage) |
| OpenGL UI quads + retail fonts | Working |
| Icon gallery (all `data3.db` PNGs) | Working |
| SFX playback (`data2.db` + XAudio2) | Working |
| Music playback (`Music/*.ogg` + stb_vorbis) | Working (requires copied `Music/` folder) |
| Multi-part creature loading (`-body`, `-foot`, …) | Working |
| `ItemRegistry` + retail `.inc` data | Working |
| World / terrain / behavior classes | Ported, not used by viewer |
| `gui.plx` compiled stream | Partial decode only |
| Full retail widget fidelity | In progress |
| `data4.db` language XML | Not used |
| Retail `ScalableFont` file-container loader | Replaced by stb + plain TTF `.dat` files |

### Known gaps

- Many registry entries have no matching `.cub` in `data1.db` (player races, some equip).
- Star rating icons (`star1.png`–`star4.png`) may be missing from some `data3.db` copies.
- `resource2.dat` load path not traced in decompiled strings; viewer loads it by filename convention.
- Music is not bundled — copy `Music/` from your install.

---

## License

Game assets (`data*.db`, `gui.plx`, `resource*.dat`, `Music/`) are **Cube World retail data** — use only if you own the game. Engine port code in `engine/`, `cube/`, and `app/` is written for this project; third-party: sqlite3, stb, GLFW, GLEW.
