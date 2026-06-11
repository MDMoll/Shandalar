# Deck Injector

`DeckInjector.jar` is a Java Swing support tool for reading and writing
Shandalar save decks and Duel `.dck` files. It is not loaded by
`Shandalar.exe`, `Magic.exe`, or the patched Windows DLL runtime.

The historical checked-in jar was built by JDK 6-era tooling, but the supported
repo build now uses JDK 21 and intentionally emits Java 21 bytecode.

## Build

Run from the repository root:

```sh
tools/build-deck-injector.sh
```

The script compiles `src/gui/*.java` with `javac --release 21` and replaces
`DeckInjector.jar`. It accepts an optional output path:

```sh
tools/build-deck-injector.sh /private/tmp/DeckInjector.jar
```

## Verify

Run:

```sh
tools/verify-deck-injector.sh
```

The verifier checks:

| Check | Expected |
| --- | --- |
| Manifest entry point | `gui.tabbedGui` |
| Classfile major version | `65` |
| Scrubland spelling | `Scrubland` present, `Scubland` absent in compiled `allCards` |
| Card mapping probe | duel id `216` and game code `0900` map to `Scrubland` |

This verifies the support-tool jar only. It does not prove Shandalar gameplay,
CrossOver behavior, save compatibility beyond the mapping probe, or native
Windows behavior.

## Scope

The Java source under `src/gui/` is old NetBeans Swing code. Keep maintenance
focused unless the tool is deliberately revived as a first-class editor. For
now, prefer small source fixes, a reproducible JDK 21 rebuild, and targeted
verification over broad GUI modernization.
