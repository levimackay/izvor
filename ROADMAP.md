# Roadmap

Each phase has its own doc in `docs/phases/` broken into ~1–2 hour tasks.
Estimates assume you do the assigned reading; they are honest, not padded.
Total: roughly **70–90 hours** of focused work. At 5–7 hours a week that's a
semester — which is exactly what this is: a self-taught compilers course.

| Phase | Name | What you build | Tasks | Est. hours | Status |
|------:|------|----------------|:-----:|:----------:|--------|
| 0 | [C fundamentals](docs/phases/00-c-fundamentals.md) | The C you need: pointers, structs, malloc/free, headers, Makefiles, lldb | 10 | 15–18 | scaffolded, start here |
| 1 | [Lexer](docs/phases/01-lexer.md) | Source text → stream of tokens for arithmetic expressions | 5 | 6–8 | scaffolded |
| 2 | [Parser](docs/phases/02-parser.md) | Tokens → abstract syntax tree via recursive descent | 5 | 8–10 | doc only |
| 3 | [Tree-walking interpreter](docs/phases/03-interpreter.md) | Evaluate the AST: arithmetic + variables | 4 | 6–8 | doc only |
| 4 | [Control flow + functions](docs/phases/04-control-flow-functions.md) | `if`, `while`, user-defined functions, scopes | 6 | 10–13 | doc only |
| 5 | [Bytecode compiler](docs/phases/05-bytecode-compiler.md) | Walk the AST and emit compact bytecode instructions | 5 | 8–10 | doc only |
| 6 | [Stack-based VM](docs/phases/06-vm.md) | Execute the bytecode on your own virtual machine | 5 | 8–10 | doc only |
| 7 | [Type checking (stretch)](docs/phases/07-type-checking.md) | A static checker that rejects `1 + true` before running | 4 | 6–8 | doc only |

## Rules of the road

- **In order.** Each phase builds literally on the previous one's code.
- **A task is done when its test passes** (Phases 0–1 ship with tests; from
  Phase 2 on, writing the assert-based test yourself *is* part of each task —
  by then you'll have written ten of them and know the pattern).
- **Commit per task.** Small commits give you a history of your own learning
  and a safe point to return to when a refactor goes sideways.
- **Scaffolds arrive when you do.** Only Phases 0–1 have stub files today, on
  purpose — building the later scaffolds yourself is part of those phases.

## The big picture

```
 source text ──lexer──▶ tokens ──parser──▶ AST ──interpreter──▶ value   (phases 1–4)
                                            │
                                            └──compiler──▶ bytecode ──VM──▶ value   (phases 5–6)
                                            └──type checker──▶ ok / type error     (phase 7)
```

Phases 1–4 build a *tree-walking interpreter* — the simplest complete
language implementation. Phases 5–6 rebuild the back half the way real
languages (Python, Lua, the JVM) do it: compile once to compact bytecode,
then execute that on a fast virtual machine. Phase 7 adds the beginnings of
what languages like Swift and TypeScript do before any code runs at all.
