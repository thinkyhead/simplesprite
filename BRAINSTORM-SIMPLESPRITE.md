# SimpleSprite — Phase 2 Brainstorming Prompt (API Completeness + Extensions)

> Dispatch-ready prompt. Launch AFTER the API-doc subagent finishes and
> `~/wiki/simplesprite/gaps-and-extensions.md` exists. The doc pass is PHASE 1
> of the user's "document → brainstorm" workflow; this is PHASE 2.

## Context

SimpleSprite is an OpenGL 1.x fixed-function 2D sprite engine (C++17, static lib
`libSimpleSprite.a`) built on SDL3 (ported 2026-07-18 from SDL 1.2). It currently
backs two games (DeepSpace, SolarFire). The user now wants to generalize it into a
platform for "cool OpenGL simulations, games, interfaces, and whatever else" — an
**immense 2D dynamic canvas** with many possible consumers.

The engine is NOT yet a general-purpose canvas: it was purpose-built for two
real-time action games. Before layering language bindings / agent endpoints on top,
we need to know which existing APIs are incomplete, thin, or missing, and what
engine changes are prerequisites for the target use-cases.

## Orientation (read FIRST)

- Engine source: `~/Projects/SDL/simplesprite/source/` (headers in `source/headers/*.h`,
  impls in `source/*.cpp`). `AGENTS.md` in that dir is a module map but may be stale
  post-SDL3/modernization — verify against live headers.
- **Seed doc (THE input to this phase):** `~/wiki/simplesprite/gaps-and-extensions.md`
  — the verified list of incomplete/stub/TODO classes & methods from the doc pass,
  plus the doc subagent's own `💡 EXTENSION IDEA` notes. Read it fully.
- Companion API pages: `~/wiki/simplesprite/*.md` (one per class/subsystem).
- Port/build state: `[[sdl-games-revival]]` → `~/wiki/entities/sdl-games-revival.md`.
- Constraints to respect in ALL recommendations:
  - SDL3 + macOS (Apple Silicon & Intel), C++17. Keep the SDL-agnostic bridge
    (`source/headers/SS_sdl3.h` + `sdl3_compat/` shims) — game/binding code must
    NOT `#include <SDL.h>` directly.
  - OpenGL 1.x is the ONLY renderer today (deprecation risk on modern macOS — see
    `sdl-games-revival.md` "Known Issues"). Any GPU-modernization is a separate
    later phase; do not assume it.
  - Engine API should stay accessible (no heavy template metaprogramming).
  - Headless note: SDL3 dummy driver HANGS (no display/audio) — a server/agent mode
    needs a real or offscreen GL context strategy.

## User's stated extension vision (seed hypotheses — EVALUATE, don't just accept)

1. **Python API sub-project** — expose SimpleSprite to Python (a binding layer over
   the engine). Likely nanobind or pybind11 over a thin C++ wrapper; alternatively a
   C ABI + ctypes. Need to decide what the Python surface looks like (module-level
   scene API vs. wrapping `SS_*` classes) and what engine state (e.g. `SS_Game`
   owning the screen/world stack) implies for a Python-driven loop.
2. **Lua interface (optional)** — scripting/modding layer, probably sol2/sol3 or the
   lua C API, for in-game or in-app scripting without recompiling the engine.
3. **App + agent endpoint** — build the engine into an app that exposes an
   **MCP server** and/or an **HTTP/WebSocket endpoint API** so external agents can
   drive the canvas programmatically (create sprites, set transforms, step frames,
   capture render-to-image, etc.).
4. **"Immense 2D dynamic canvas"** — the unifying use-case: many sprites / particles
   / agents moving and interacting, used for simulations, games, data viz, interactive
   art, HUDs, educational demos. Performance and spatial structure matter at scale.

## What to produce

A structured brainstorm that answers:

1. **API completeness audit vs. the canvas goal.** Using `gaps-and-extensions.md` and
   the live headers, assess which incomplete/thin APIs actually BLOCK the target
   use-cases (vs. which are inert game-specific cruft we can leave or prune). Call out
   any gaps the doc pass MISSED (e.g. missing headless/offscreen render, missing
   deterministic step loop, missing programmatic scene construction, thread-safety of
   the world/collision systems, no API to read back pixels for agent vision).
2. **Prioritized roadmap** with sequencing, e.g.:
   - Close the core gaps that gate generality (headless render target? scene API?
     deterministic tick?).
   - Python binding (recommend tech + surface shape + prerequisites).
   - Lua scripting (recommend tech + where it hooks in).
   - Agent endpoint app: MCP server and/or HTTP/WS — recommend which, what tools/
     endpoints, how an agent would create/inspect/step a scene, and what engine
     changes (offscreen GL, event injection, frame-stepping) are prerequisites.
   - Note OpenGL 1.x deprecation as a flagged future risk, not this phase's work.
3. **Concrete recommendations** for each extension: specific library, API shape,
   integration point in the engine, trade-offs, and prerequisite engine work.
   Distinguish verified-need from opinion (mark opinions as such).
4. **Risk register:** OpenGL deprecation, Apple Silicon GL availability, SDL3 dummy
   hang for headless, engine global/singleton assumptions that fight embedding,
   performance at "immense" scale (collision bands, layer counts, texture/POT limits).

## Deliverables (write to the wiki)

- Create `~/wiki/simplesprite/roadmap.md` — the synthesized brainstorm: completeness
  findings, prioritized roadmap, extension recommendations (Python/Lua/MCP/HTTP),
  risk register. Proper frontmatter (type: concept; tags: sdl, simplesprite, engine,
  api, c++; sources: list the pages/headers read). Link `[[sdl-games-revival]]` and
  the relevant `ss-*.md` API pages (≥2 outbound wikilinks).
- Extend `~/wiki/simplesprite/gaps-and-extensions.md` if you discover NEW gaps not in
  the doc pass (add them under a clearly-marked `## Brainstorm-added gaps` section,
  with `⚠️ INCOMPLETE` notation + header:line refs).
- Register `roadmap.md` in `~/wiki/index.md` (add to the SimpleSprite section the doc
  subagent created) with a one-line summary, and append a single consolidated entry to
  `~/wiki/log.md`: `## [2026-07-19] create | SimpleSprite roadmap (phase 2 brainstorm)`.
- Do NOT touch `wiki/SCHEMA.md` or other sections; the doc subagent owns the tag group.

## Return

A concise summary: (1) the top 5 completeness blockers for the canvas goal, (2) the
recommended sequencing (core gaps → Python → Lua → agent endpoint), (3) the specific
tech picks for Python/Lua/MCP with one-line rationale each, (4) the biggest risks,
(5) confirmation of wiki pages created/updated. If a required input (e.g. the gaps
page) is missing, report that instead of inventing content.
