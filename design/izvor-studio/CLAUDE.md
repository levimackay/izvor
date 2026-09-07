# Izvor Studio — IDE design prototype

## What this is

`Izvor Studio.dc.html` is a high-fidelity, interactive prototype of **Izvor Studio**, the IDE
for **Izvor**, a new general-purpose programming language (source files use the `.iz`
extension). It is a single self-contained HTML file — open it directly in a browser
(`support.js` must sit next to it). No build step, no dependencies beyond two Google Fonts.

It was built as a design artifact, not production code: all state is local, all data is
fixture data. It is meant to be the visual and behavioral reference when the real IDE is
implemented against an actual compiler and language server.

## The language it represents

Izvor is designed around:

- Native performance — compiles directly to machine code, no bytecode, no JIT
- Static typing with strong inference (annotations optional almost everywhere)
- Memory safety without manual memory management (region/borrow inference, no GC)
- First-class concurrency (`spawn`, `await`, task scheduler, async lowered to state machines)
- Simple readable syntax, legible to humans and coding agents alike
- Excellent tooling as a first-class concern

Syntax conventions used in the prototype's sample code: `fn`, `async fn`, `struct`, `impl`,
`use module.{A, B}`, `let` (immutable) / `var` (mutable), `?` for error/optional propagation,
`spawn expr` to move a task onto the scheduler, `move` for explicit ownership transfer,
`Task<T>`, `Stream<T>`, `Handle<T>`, 4-space indentation, no semicolons.
Project manifest is `izvor.toml`. CLI is `izvor build --release`, `izvor run`, `izvor test`,
`izvor fmt`, `izvor debug`.

## The demo project inside the prototype

```
izvor-server/
  src/main.iz  server.iz  routes.iz  models.iz  database.iz
  tests/server_test.iz  routes_test.iz
  izvor.toml
  README.md
```

A concurrent HTTP service. Full source for every file is inlined in the logic class
(`this.src`) and is deliberately consistent across files — imports resolve, symbols
cross-reference, diagnostics point at real lines.

## Screens and features implemented

- **Welcome screen** — wordmark, "Native performance. Human simplicity.", New/Open/Clone,
  recent projects, "What's new in 1.4" panel.
- **New Project** — 7 templates, project name/location, target (Apple Silicon, x86_64,
  ARM64 Linux, WebAssembly), language version, live scaffold preview.
- **Main workspace** — activity bar (Explorer, Search, Source Control, Run & Debug,
  Extensions) with tooltips; file explorer with expand/collapse, filter, git badges,
  right-click context menu; editor tabs with dirty indicator and close buttons; breadcrumbs
  with diagnostic counts; status bar.
- **Code editor** — hand-written tokenizer for Izvor syntax highlighting, line numbers,
  current-line highlight, indentation guides, code folding, minimap with viewport + error
  marks, inline diagnostics, hover type/doc cards, autocomplete popup (Ctrl+Space),
  blinking cursor.
- **Izvor AI** (right panel) — context chips, agent plan checklist, streaming responses,
  a unified diff with Apply / Reject / Explain. **Apply actually mutates the source**
  (`await process_request` → `spawn process_request`), marks the file dirty, and navigates
  to the changed line. Context meter, model row, attachment buttons.
- **Terminal** (bottom panel) — Terminal / Build / Problems tabs, realistic `izvor build`
  and `izvor run` output appended live, clear / new / maximize / close controls.
- **Compiler view** — the 9-stage pipeline (SOURCE → LEXER → PARSER → AST → TYPE CHECKER →
  BORROW CHECKER → IR → OPTIMIZER → MACHINE CODE). Build animates stages sequentially;
  each stage shows real-looking metrics, a blurb, and representative stage output
  (tokens, AST dump, inferred signatures, region solve, SSA blocks, ARM64 asm).
- **Performance view** — 6 metric cards, live CPU sparkline, concurrent task timeline with
  handler/await segmentation per thread, playhead.
- **Problems panel** — 3 diagnostics (borrow error, avoidable allocation, concurrency hint);
  clicking navigates to the exact file and line.
- **Run & Debug** — Run/Debug/Stop, session state, breakpoints, call stack, variables,
  watch, threads; stack frames navigate to source.
- **Command palette** — Cmd/Ctrl+K, 16 commands with groups and shortcuts, arrow-key
  navigation, all commands functional.
- **Global search**, **Source Control** (branch, commit box, changes with stats),
  **Settings** (9 categories, working toggles/steppers/segmented controls),
  **Extensions** marketplace with working install buttons.
- Resizable explorer / AI / terminal panels; toast notifications; keyboard shortcuts
  (⌘K palette, ⌘B explorer, ⌘J terminal, ⌘I AI, ⌘R run, ⌘⇧B build, Esc dismiss).

## Design system

Dark-first. Color is used sparingly — hierarchy comes from typography, spacing, and
1px borders.

Surfaces (darkest to lightest): `#0a0b0d` editor canvas · `#0b0d0f` terminal ·
`#0c0e11` activity bar · `#0d0f11` panels/chrome · `#0e1114` cards · `#101317` overlays ·
`#12161a` popovers. Borders: `#15191c` (hairline), `#1b2024` (panel), `#262c32` (control),
`#2b3238` (overlay). Text: `#e6e9ec` primary · `#c6ccd2` secondary · `#8b949c` tertiary ·
`#5e666d` quaternary · `#454d54` line numbers.

Accents: primary `oklch(0.80 0.10 190)` (cyan — active states, primary buttons, cursor);
AI `oklch(0.74 0.11 300)` (violet); success `oklch(0.76 0.13 155)`; warning
`oklch(0.80 0.12 80)`; error `oklch(0.66 0.16 25)`; info `oklch(0.72 0.08 240)`.

Syntax: keyword `oklch(0.78 0.10 300)` · type `oklch(0.84 0.08 200)` · function
`oklch(0.86 0.09 100)` · string `oklch(0.80 0.09 145)` · number `oklch(0.83 0.10 70)` ·
comment `#5b646c` italic · punctuation `#7c858d` · identifier `#d3d9de`.

Type: **IBM Plex Sans** for UI (11–13.5px, 20–26px headings), **JetBrains Mono** for code,
paths, and all numerics (10.5–13px). Uppercase section labels are 10.5px with
0.09–0.11em tracking.

Metrics: row height 22px (code) / 25px (tree) / 26–30px (buttons); radii 4/5/6/7/9/12px
(small chrome → overlays); activity bar 46px; status bar 24px; tab strip 35px.
Animations are 0.1–0.24s, `cubic-bezier(0.16, 1, 0.3, 1)`; nothing longer than 0.25s
except the deliberate compiler stage cadence (230ms per stage).

## File structure

- `Izvor Studio.dc.html` — the whole prototype. Head/body scaffold, then the markup
  template, then a `class Component` logic class holding all state, fixture data, and
  computed render values. Styling is inline throughout (only `@keyframes`, fonts, and
  body resets live in a stylesheet block).
- `support.js` — the small runtime that renders the template. Required; keep it adjacent.

## If you are Claude Code picking this up

Treat this file as the **spec**, not the codebase. When building the real IDE:

1. Keep the design system above verbatim — colors, type, spacing, motion timings.
2. Keep the information architecture: activity bar → panel → editor → AI, terminal at the
   bottom, compiler and performance as full-canvas views rather than panels.
3. The compiler view is the product's signature screen. Its stage list should be driven by
   real compiler telemetry (per-stage timings, token/node/type/region counts, target info).
4. Replace fixtures with real sources: LSP for diagnostics, hovers, completion, definitions;
   the real `izvor` CLI for build/run/test output; a profiler feed for the performance view.
5. Diagnostics in the prototype are a borrow error, an avoidable allocation, and a
   concurrency hint — these three categories are intentional and should stay first-class in
   the real diagnostics UI.
6. Fonts are the only external dependency. Everything else is self-contained.

Open questions to resolve with the language author: final `.iz` grammar and keyword set,
whether `move` stays explicit, the exact compiler stage names to expose in the UI, and
whether the AI agent is local or hosted.
