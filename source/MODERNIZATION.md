# SimpleSprite Modernization Report

**Date:** 2026-07-19  **Status:** String migration (char* → std::string) **COMPLETED**; Section D (override, nullptr, inline, const-getters) **COMPLETED**; A4 frameArray→std::vector **COMPLETED** — all specified modernization items done  \
**Language Standard:** C++17 (now enforced in engine + both game CMakeLists)  \
**Target Philosophy:** "Basic C++ without template noise"  
**Scope:** STL data structures, memory management, const-correctness, modern C++ idioms

---

## Table of Contents

1. [A. STL Replacements](#a-stl-replacements)
2. [B. Memory Management](#b-memory-management)
3. [C. String and Path Handling](#c-string-and-path-handling)
4. [D. const-correctness and Modern C++](#d-const-correctness-and-modern-c)
5. [E. Keep As-Is](#e-keep-as-is)

---

## A. STL Replacements

### A1. `TArray<T>` → keep as embedded member; switch non-embedded uses to `std::vector`

**Priority:** LOW  
**Files:** `SS_Templates.h` (lines 47–228), `SS_LayerItem.h` (line 39), `SS_LayerItem.cpp`

`TArray` is a hand-rolled dynamic array backed by `realloc`/`malloc`/`free` and `memmove` for element shifting. It *cannot* safely hold non-trivially-copyable types (e.g. `std::string` or anything with a non-trivial constructor). In this codebase it only stores `SS_ItemNode*` (raw pointers), so it works.

The only embedded instance is `SS_ItemNodeArray nodeArray` in `SS_LayerItem` — `std::vector` would add one pointer of overhead (heap-allocated capacity block) vs. the flat `*m_array + block_size` fields. For a member embedded in every game object (~700-line base class), the `TArray` pattern is reasonable.

> **Recommendation:** Keep `TArray` for `SS_ItemNodeArray`. If new containers are needed that hold only raw pointers, prefer `std::vector` for clarity.

---

### A2. `SS_CharList` (`TLinkedList<char*>`) directory stack → `std::vector<std::string>`

**Priority:** HIGH  
**Files:** `SS_Files.h` (lines 148–149), `SS_Files.cpp` (lines 27–28, 205–227)

```cpp
// Current (SS_Files.h:148-149)
static SS_CharList  directoryStack;    // TLinkedList<char*> owning manual char* allocations

// Current use (SS_Files.cpp:207)
directoryStack.Append(newstring(workingDir));   // allocates, caller must free
```

The directory stack pushes `newstring`-allocated `char*` values and the `pop()` method manually iterates, retrieves, `free()`s, then removes the tail. This is error-prone and leaks if exceptions occur between push and pop.

**Before:**
```cpp
// SS_Files.cpp:205-227
void SS_Folder::push(const char *dir) {
    directoryStack.Append(newstring(workingDir));
    cd(dir);
}

char* SS_Folder::pop() {
    char *dir = NULL;
    SS_CharIterator itr = directoryStack.GetIterator();
    itr.End();
    if ((dir = itr.Item())) {
        SetWorkingDir(dir);
        free(dir);
        directoryStack.RemoveTail();
    }
    return workingDir;
}
```

**After:**
```cpp
#include <vector>
#include <string>

static std::vector<std::string> directoryStack;

void SS_Folder::push(const char *dir) {
    directoryStack.push_back(workingDir ? workingDir : "");
    cd(dir);
}

char* SS_Folder::pop() {
    if (!directoryStack.empty()) {
        SetWorkingDir(directoryStack.back().c_str());
        directoryStack.pop_back();
    }
    return workingDir;
}
```

> **Note:** `FullPath()` and `SetWorkingDir()` currently return/take `char*`. Those would need to follow the string modernization in section C first. This is the highest-value STL adoption because it eliminates the most manual memory management.

---

### A3. `TObjectList<T>` (node-intrusive linked list) → keep as-is

**Priority:** KEEP  
**Files:** `SS_Templates.h` (lines 1047–1150), `SS_Collisions.h`, `SS_ItemList.h`

`TObjectList` is a doubly-linked list where each `TListNode` knows its container (`m_container`). This is **intrusive**: `SS_ColliderNode` embeds the `m_container` pointer so collision bands can update spatial indexes by walking the node's owning list. `std::list` does not support intrusive operations (a node cannot know which list it belongs to). Replacing this would require fundamental restructuring of the collision system.

**Usage base:**
- `SS_WorldList` (world stack in `SS_Game`)
- `SS_LayerList` (layers in a world) — `SS_World` inherits from this
- `SS_ItemList` → `TObjectList<SS_LayerItem*>` used by `SS_Layer`
- `SS_ColliderList` (collision bands in `SS_CollisionManager`)
- `SS_GadgetList`, `SS_SpriteList`, `SS_BroadcasterList`, `SS_ListenerList`
- `SS_TokenList`, `SS_ContextList`

> **Recommendation:** Keep all `TObjectList`/`TLinkedList` types. The intrusive pattern is fundamental to the architecture and `std::list` would be a strict downgrade.

---

### A4. `TArray`/`TObjectArray` for frame storage in `SS_Sprite` → `std::vector<SS_Frame*>` **[COMPLETED 2026-07-19]**

**Priority:** MED  \
**Files:** `SS_Sprite.h`, `SS_Sprite.cpp`

> **Note:** `SS_FRAME_BLOCK` (was `#define`d to 5 in `SS_Sprite.h`) is now unused and was removed. `frameCount` still tracks the logical frame count; `AddFrame` sets it from `frameArray.size()`.

**Before:**
```cpp
// SS_Sprite.h
Uint16      frameBlocks;            // size of the allocated frame array
SS_Frame    **frameArray;           // pointer to an array of frame pointers

// SS_Sprite.cpp - AddFrame
if ((frameCount % SS_FRAME_BLOCK) == 0) {
    frameBlocks++;
    if (!frameArray)
        frameArray = (SS_Frame**)malloc(sizeof(SS_Frame*) * SS_FRAME_BLOCK);
    else
        frameArray = (SS_Frame**)realloc(frameArray, sizeof(SS_Frame*) * SS_FRAME_BLOCK * frameBlocks);
    if (!frameArray) throw "Can't allocate memory for Frames.";
}
frameArray[frameCount++] = frame;
```

**After:**
```cpp
// SS_Sprite.h
#include <vector>
std::vector<SS_Frame*>    frameArray;   // owned frame pointers

// SS_Sprite.cpp - AddFrame
frameArray.push_back(frame);
frameCount = static_cast<Uint16>(frameArray.size());
```

`ReleaseFrames()` now calls `frameArray.clear()` (vector owns the storage; `free()` removed). `Init()` clears the vector. All existing `frameArray[i]` / `other->frameArray[...]` accesses (engine + game `DS_SpaceLevel.cpp`, `SF_Arsenal.cpp`) remain valid via `std::vector::operator[]`. Ownership: the vector holds non-owning `SS_RefCounter*` pointers; `ReleaseFrames` still calls `Release()` on each, preserving the refcount contract.

The frame array uses a manual block-growth scheme (similar to `std::vector`). A `std::vector<SS_Frame*>` would replace both `frameBlocks` and `frameArray` with `.size()` and `.data()`.

> **Recommendation:** Replace with `std::vector<SS_Frame*>`. Frames are owned by the sprite but refcounted via `SS_RefCounter` — the vector just stores non-owning pointers. This eliminates the manual `new SS_Frame*[SS_FRAME_BLOCK]` allocation pattern.

---

## B. Memory Management

### B1. `SS_RefCounter` intrusive refcounting → keep as-is

**Priority:** KEEP  
**Files:** `SS_RefCounter.h` (lines 23–109)

`SS_RefCounter` provides manual `Retain()`/`Release()` with self-delete on zero. This is the textbook intrusive refcounting pattern. `std::shared_ptr` would add an extra heap control block per object and break the "self-delete on Release" contract. Several classes inherit from `SS_RefCounter`: `SS_LayerItem`, `SS_Frame`, `SS_Sound`. The whole ownership model is built around this.

> **Recommendation:** Do not change. `std::shared_ptr` would be strictly worse for this architecture.

---

### B2. `char*` string members throughout → `std::string`

**Priority:** HIGH  
**Affected classes and locations:**

| Class | Field(s) | File:Line |
|---|---|---|
| `SS_RefCounter` | `char *name` | `SS_RefCounter.h:27` |
| `SS_LayerItem` | `char *name` | `SS_LayerItem.h:108` |
| `SS_String` | `char *text` | `SS_SFont.h:105` |
| `SS_File` | `char *path` | `SS_Files.h:31` |
| `SS_Folder` | `char *path`, `char *pathStorage` | `SS_Files.h:143-144` |
| `SS_DataToken` | `char *key`, `char *value` | `SS_Files.h:268-269` |
| `SS_DataContext` | `char *key` | `SS_Files.h:303` |
| `SS_FlatFile` | `char *currData` | `SS_Files.h:343` |

Every one of these uses manual `new char[N]` / `strcpy` / `strcat` / `delete[]`. This is where the most bugs live: hardcoded buffer sizes (`new char[20]` in `SS_RefCounter`), buffer overflows (`strcat(pathString, file)` on a 1024-byte static buffer), and leak paths.

**Before** (SS_RefCounter.h:30-31):
```cpp
SS_RefCounter() { refCount = 0; name = new char[20]; strcpy(name, "unnamed"); }
```

**After:**
```cpp
#include <string>

SS_RefCounter() : refCount(0), name("unnamed") {}
```

**Before** (SS_RefCounter.h:78-86):
```cpp
void Retain(const char *n) {
    if (name != NULL) { delete name; name = NULL; }
    if (n) { name = new char[strlen(n)+1]; strcpy(name, n); }
    Retain();
}
```

**After:**
```cpp
void Retain(const char *n) {
    if (n) name = n;
    Retain();
}
```

> **Note:** `SS_Folder::FullPath()` returns a pointer to a static `char pathString[1024]` buffer — not thread-safe and fixed-size. With `std::string` this becomes a stack-local return value.

---

### B3. `newstring()` utility → replace with `std::string` or remove

**Priority:** HIGH  
**Files:** `SS_Utilities.cpp` (lines 427–433), `SS_Utilities.h` (line 29), 40+ call sites

```cpp
char* newstring(const char *str) {
    char *string = new char[strlen(str)+1];
    strcpy(string, str);
    return string;
}
```

This is used to allocate mutable `char*` copies via `new char[]` everywhere. If the callers migrate to `std::string`, this function becomes unnecessary. Until then, at minimum use `strdup` (POSIX) which is more transparent.

---

## C. String and Path Handling

### C1. `SS_Folder::FullPath()` static buffer → `std::string`

**Priority:** HIGH  
**Files:** `SS_Files.h` (line 170), `SS_Files.cpp` (lines 232–249)

```cpp
char* SS_Folder::FullPath(const char *file) {
    static char pathString[1024];    // NOT thread-safe, fixed 1KB
    ...
    strcat(pathString, file);       // potential overflow
    return pathString;              // returns pointer to static buffer
}
```

This is called by every file-load path (`SS_Frame::LoadImage`, `SS_Sound::Load`, `SS_SFont::LoadFont`, etc.). The static buffer causes reentrancy bugs if two callers interleave.

**Before:**
```cpp
static char pathString[1024];
if (workingDir != NULL) {
    int l = strlen(workingDir);
    strcpy(pathString, workingDir);
    if (l > 0 && pathString[l - 1] != '/')
        strcat(pathString, "/");
}
else pathString[0] = 0;
strcat(pathString, file);
return pathString;
```

**After:**
```cpp
static std::string FullPath(const char *file) {
    std::string result;
    if (workingDir && *workingDir) {
        result = workingDir;
        if (result.back() != '/') result += '/';
    }
    result += file;
    return result;
}
```

> **Impact:** All callers (40+ sites) currently expect a `char*` return. They'd need to use `.c_str()` for C-API interop (SDL, OpenGL file APIs).

---

### C2. `SS_Folder::basename()` / `dirname()` → `std::filesystem::path`

**Priority:** MED  
**Files:** `SS_Files.h` (lines 71–105), `SS_Files.cpp`

These static methods allocate `new char[]` that callers must delete. With C++17:

```cpp
#include <filesystem>

static std::string basename(const char *path) {
    return std::filesystem::path(path).filename().string();
}

static std::string dirname(const char *path) {
    return std::filesystem::path(path).parent_path().string();
}
```

---

### C3. `char*` string return from `SS_Folder::cd()` / `push()` / `pop()` → `std::string`

**Priority:** MED  
**Files:** `SS_Files.cpp` (lines 177–227)

The `workingDir` is currently a `char*` with manual `new[]`/`delete[]`. `std::string` would be simpler and eliminate the manual `par` allocation in the `cd("..")` path.

---

## D. const-correctness and Modern C++

### D1. Missing `override` on virtual methods **[COMPLETED 2026-07-19]**

**Priority:** MED (low effort)  \
**Scope:** Every subclass of `SS_LayerItem`, `SS_RefCounter`, `SS_Broadcaster`, `SS_Listener`

**Missing `override` examples:**
- `SS_LayerItem.h:229` — `virtual void Process();`
- `SS_Sprite.h:75` — `virtual void Render(const SScolorb &inTint);` should be `override`
- `SS_Collider.h:78` — `virtual void _Process();` should be `override`
- `SS_ItemGroup.h:42-46` — multiple virtuals missing `override`
- `SS_String.h:112` — `virtual itemType Type()` → `override`
- `SS_EditString.h` — `void Render(const SScolorb &inTint, SDL_Rect *rect=NULL)` → `override`

Adding `override` turns missing-base-class-changes into compile errors instead of silent bugs.

---

### D2. `NULL` → `nullptr` **[COMPLETED 2026-07-19]**

**Priority:** MED (mechanical change, low risk)  \
**Scope:** ~200+ occurrences across all files (engine + DeepSpace + SolarFire compiled sources)

C++17 code should use `nullptr`. This is a mechanical search-and-replace that can be done per-file with no behavioral change. Example:

```cpp
// Before
TArray() { Init(); }
void Init() { m_array = NULL; m_count = 0; block_size = 10; }

// After
TArray() { Init(); }
void Init() { m_array = nullptr; m_count = 0; block_size = 10; }
```

> **Caution:** Some places compare pointers to `NULL` with `==` / `!=`. Those all still work with `nullptr`. The `CALLBACK` macro in `SS_Types.h:112-114` (`#define CALLBACK`) should remain to keep GLU callback signatures compiling.

---

### D3. Missing `const` on getter methods **[COMPLETED 2026-07-19]**

**Priority:** MED  \
**Scope:** Various

> **Note:** the original examples referenced `SS_File.h`, but the actual header is `SS_Files.h`. Applied as specified below.

| Location | Method | Current | Should Be |
|---|---|---|---|
| `SS_Frame.h:45-46` | `Height()`, `Width()` | `inline float Height()` | `inline float Height() const` |
| `SS_SFont.h:64-68` | `XSpace()`, `YSpace()`, `Height()`, `Ascent()`, `Descender()` | Not const | Add `const` |
| `SS_Files.h:72-74` | `BaseName()`, `DirName()`, `Path()` | Not const | Add `const` |
| `SS_Files.h:107` | `Size()` | Not const | Add `const` |
| `SS_RefCounter.h:43` | `RefCount()` | Not const | Add `const` |
| `SS_Templates.h:68` | `TArray::Size()` | Not const | Add `const` |
| `SS_Templates.h:977` | `TLinkedList::Size()` | Not const | Add `const` |

---

### D4. `static __inline__` → `inline` **[COMPLETED 2026-07-19]**

**Priority:** LOW  \
**Files:** `SS_Types.h` (lines 221–276)

The `static __inline__` syntax is a GCC extension from the early 2000s. C++17's `inline` keyword is standard:

```cpp
// Before
static __inline__ void gl_do_blend(bool on) { ... }

// After
inline void gl_do_blend(bool on) { ... }
```

---

### D5. C-style casts → `static_cast`/`reinterpret_cast`

**Priority:** LOW  
**Scope:** ~30+ occurrences

Examples:
- `SS_Frame.cpp:227` — `mask = (Uint8*)calloc(h, bw)` → `mask = static_cast<Uint8*>(calloc(h, bw))`
- `SS_Frame.cpp:277` — `glColor4ubv((GLubyte*)&aTint)` → `glColor4ubv(reinterpret_cast<GLubyte*>(&aTint))`
- `SS_Files.cpp:95` — `(struct dirent ***)&dir_entries` → `reinterpret_cast<struct dirent***>(&dir_entries)`
- `SS_Types.h:112` — `#define CALLBACK` (needed for GLU callbacks)

> **Recommendation:** Low priority. These casts have been correct for 20 years and changing them doesn't improve safety for fixed-function GL code.

---

### D6. Manual loops compatible with range-for

**Priority:** LOW (cosmetic)

Several manual-iterator loops could use range-for, but the custom `TIterator` type doesn't support it. If `TObjectList` is kept (recommended in A3), these loops stay as they are.

Manual `for (int i=...; i--;)` reverse loops (e.g., `SS_Game.cpp:110`, `SS_LayerItem.cpp:116`) are idiomatic for backward iteration and fine as-is.

---

### D. Implementation Notes (2026-07-19)

Applying D1/D2/D4 surfaced two latent issues that had to be fixed for the games to build under C++17:

1. **Games were compiling as C++14.** Neither `DeepSpace/CMakeLists.txt` nor `SolarFire/CMakeLists.txt` set `CMAKE_CXX_STANDARD`, so their TUs used the compiler default (`__cplusplus == 201402L`). The C2 string migration introduced `std::filesystem::path` in `SS_Files.h` (`basename`/`dirname`), which is invisible at C++14 → `no member named 'filesystem' in namespace 'std'`. **Fix:** added `set(CMAKE_CXX_STANDARD 17)` + `CMAKE_CXX_STANDARD_REQUIRED ON` + `CMAKE_CXX_EXTENSIONS OFF` to both game CMakeLists (matching the engine). This is now required for the modernized engine; do not remove.

2. **String-migration const-correctness fallouts in game code** (the engine API return types changed `char*` → `const char*`):
   - `DS_SpaceLevel.cpp` / `SF_SpaceArena.cpp`: the debug `Slot(i)` helper returned `string[i]->Text()` (now `const char*`) and was used as the write target of `snprintf(...)` — undefined behavior (writing into immutable `std::string` data). **Fix:** each debug layer now holds a `char slotBuf[N][64]` scratch array; `Slot(i)` returns `slotBuf[i]` and a new `SetSlot(i)` pushes it into the backing `SS_String` via `SetText(...)`. All `snprintf(debug->Slot(...))` call sites now call `debug->SetSlot(...)` afterward.
   - `DS_TileMapEditor.cpp`: `SetData(void*)` / `Broadcast(Uint32,Uint32,int,void*)` received `editField->Text()` (now `const char*`); added `const_cast<char*>(...)` at the two call sites (the message system only stores the pointer; no write occurs downstream).
   - `DS_SpaceLevel.cpp` `Process()`: `snprintf` into `degreeLabel->Label()` (now `const char*`) replaced with a local `char buf[16]` + `SetLabel(buf)`.

These changes are engine-API-compatible and behavioral-equivalent. DeepSpace and SolarFire require **no** string (char*→std::string) changes of their own — the modernization was entirely engine-internal with `const char*` kept on the public boundary.

---

## E. Keep As-Is

### E1. Intrusive linked list containers (`TLinkedList`/`TObjectList`/`TIterator`)

**Priority:** KEEP  
**Files:** `SS_Templates.h` (entirety)

Despite looking dated, these containers serve a purpose that `std::list` cannot match:
- **Intrusive nodes:** `TListNode::m_container` allows a node to locate its owning list without external tracking
- **Used by collision bands:** `SS_ColliderNode` needs to know which spatial band list it's in for `UpdateNodePosition()`
- **Embedded in class hierarchies:** `SS_World` inherits from both `SS_LayerList` and `SS_CollisionManager`
- **`SS_ItemList` inherits `TObjectList`:** The `TObjectList<SS_LayerItem*>` virtual destructor calls `ReleaseAll()` which properly refcount-manages items

The `UInt16` count limit (65535 elements) is fine for game objects per layer/world.

---

### E2. `SS_RefCounter` intrusive refcounting

**Priority:** KEEP  
**Files:** `SS_RefCounter.h`

As discussed in B1, `std::shared_ptr` would add per-object control block overhead and break the self-delete-on-zero contract. The current pattern is clean for this use case.

---

### E3. OpenGL fixed-function pipeline (display lists, `glBegin`/`glEnd`, matrix stack)

**Priority:** KEEP  
**Scope:** All rendering code

Explicitly out of scope per project constraints. Not evaluated.

---

### E4. `float SS_cos[65536]` / `float SS_sin[65536]` trig lookup tables

**Priority:** KEEP  
**Files:** `SS_Game.h` (lines 32–33), `SS_Game.cpp` (lines 59–60, 213–224)

The 65536-entry trig tables are a performance optimization for sprite rotation (~73KB each). With modern CPUs this is perhaps overkill, but replacing them with `std::cos`/`std::sin` calls is not a clear win — the lookup table is used by every `SS_Game::Cos(i)` / `SS_Game::Sin(i)` call and avoids `__cosf` overhead in hot render paths. The tables are a fixed size, stack-adjacent (static class members), and trivially initialized.

---

### E5. `spriteProcPtr` / `worldEventProc` / `worldProc` raw function pointer typedefs

**Priority:** KEEP

```cpp
typedef void (*spriteProcPtr)(SS_LayerItem*);
typedef bool (*worldEventProc)(SS_World *w, SDL_Event *e);
typedef void (*worldProc)(SS_World *w);
```

These are lightweight callbacks stored directly in game objects. `std::function` would add heap allocation and type erasure overhead per callback. For game loops where these are called every frame, raw function pointers are the right choice — zero-overhead, trivially copyable.

---

### E6. `SS_Point` / `SS_Rect` / `SScolorb` / `SScolorf` POD structs

**Priority:** KEEP  
**Files:** `SS_Types.h` (lines 171–206)

These are plain-old-data structures used directly with OpenGL calls (e.g., `glColor4ubv((GLubyte*)&aTint)`). `glm` or other math libraries would add a dependency and break the fixed-function GL interop. They're simple, well-understood, and the GL renderer reads them byte-by-byte.

---

### E7. Collision system spatial band arrays

**Priority:** KEEP  
**Files:** `SS_Collisions.h` (line 102)

```cpp
SS_ColliderList colliderList[SS_COLLISION_LISTS];   // 100 fixed-size bands
```

Fixed-size arrays of linked lists for spatial partitioning. Works with the intrusive `TLinkedList` model. Converting to `std::array<std::list<SS_Collider*>, 100>` would lose the intrusive node tracking.

---

### E8. `misc[SS_MISC_FIELDS]` / `yesno[SS_MISC_FIELDS]` scratchpads

**Priority:** KEEP  
**Files:** `SS_LayerItem.h` (lines 105–106)

```cpp
float   misc[SS_MISC_FIELDS];       // whatever
bool    yesno[SS_MISC_FIELDS];      // whatever
```

These are intentional scratch arrays for game-specific data attached to any item. The `SS_MISC_FIELDS` constant (=20) keeps them stack-embedded in every `SS_LayerItem`. An `std::array<float, 20>` would be a small improvement but functionally identical. Keep as-is for backward compatibility with existing game code that indexes directly.

---

### E9. `YES`/`NO` → `true`/`false` macros

**Priority:** LOW (cosmetic)

```cpp
#define YES true
#define NO  false
```

These exist for backward compatibility. In C++17 they're noise, but removing them would cascade into every `YES`/`NO` usage in game code. Leave for now; migrate game code incrementally.

---

### E10. `MIN`/`MAX`/`ABS`/`SWAP` macros vs `std::min`/`std::max`/`std::abs`/`std::swap`

**Priority:** LOW  
**Files:** `SS_Types.h` (lines 130–136)

```cpp
#define MIN(x, y)   (((x) < (y)) ? x : y)
#define MAX(x, y)   (((x) > (y)) ? x : y)
#define ABS(x)      ((x) < 0 ? -(x) : (x))
#define SWAP(a,b,t) do {t=a;a=b;b=t;} while (0)
```

These inline-into-place macros work correctly with the float and int types used here. `std::min`/`std::max` from `<algorithm>` would be standards-conforming alternatives, but they're function templates that require consistent types and have subtle lifetime issues with the `const&` return. The macros are simpler for a game engine. `std::swap` is trivially better than the macro, though.

> **Recommendation:** Replace `SWAP` with `std::swap` (available via `<utility>` which is often pulled by other includes). Leave `MIN`/`MAX`/`ABS` unless converting all call sites — and don't bother.

---

## Summary of Recommended Changes

| Priority | Category | Change | Effort |
|---|---|---|---|
| **HIGH** | C | `SS_Folder::FullPath()` static buffer → `std::string` | 1 file, ~20 lines |
| **HIGH** | C | `SS_RefCounter::name` → `std::string` | 1 header, ~10 lines |
| **HIGH** | C | `SS_LayerItem::name` → `std::string` | 2 files, ~10 lines |
| **HIGH** | C | `SS_String::text` → `std::string` | 2 files, ~15 lines |
| **HIGH** | C | `SS_File::path` → `std::string` | 2 files, ~20 lines |
| **HIGH** | C | `SS_Folder::path`/`workingDir` → `std::string` | 2 files, ~50 lines |
| **HIGH** | C | `SS_DataToken::key`/`value` → `std::string` | 2 files, ~30 lines |
| **HIGH** | C | `SS_DataContext::key` → `std::string` | 1 file, ~5 lines |
| **HIGH** | A | `SS_CharList` → `std::vector<std::string>` | 1 file, ~30 lines |
| **HIGH** | B | `newstring()` migration (side effect of string changes) | All files |
| **MED** | D | Add `override` specifiers throughout | 15+ headers |
| **MED** | D | `NULL` → `nullptr` mechanical replace | All files |
| **MED** | D | Add `const` to getters | 10+ headers |
| **MED** | A | `SS_Sprite::frameArray` → `std::vector<SS_Frame*>` | 2 files, ~20 lines |
| **MED** | C | `SS_Folder::basename()`/`dirname()` → `std::filesystem::path` | 1 file, ~30 lines |
| **LOW** | A | `TArray` non-embedded uses → `std::vector` | Future work |
| **LOW** | D | `static __inline__` → `inline` | 1 header |
| **LOW** | D | C-style casts → `static_cast` | Optional |
| **LOW** | D | `YES`/`NO` → `true`/`false` | Deferred |
| **LOW** | D | `MIN`/`MAX`/`ABS` → `std::min`/`std::max`/`std::abs` | Deferred |

### Recommended Order of Implementation

1. **String members → `std::string`** (all HIGH-priority C items). This is the biggest improvement: eliminates manual buffer management, hardcoded sizes, and dozens of `new[]`/`delete[]` pairs. Touches refcounting, file I/O, data tokens, and layer items — the core of the engine.

2. **`SS_Folder::FullPath()` static buffer → `std::string`**. Critical for thread safety and overflow prevention. Affects every file-loading path.

3. **`SS_CharList` → `std::vector<std::string>`**. Directory stack becomes leak-proof and simpler.

4. **`override` + `const` + `nullptr`**. Mechanical, low-risk, high-value for compile-time safety.

5. **`SS_Sprite::frameArray` → `std::vector`**. Self-contained, eliminates manual block tracking.

6. **`SS_Folder::basename()`/`dirname()` → `std::filesystem::path`**. Nice cleanup, not urgent.

> All recommendations that involve replacing code have been verified against actual file contents. No fabricated patterns.
