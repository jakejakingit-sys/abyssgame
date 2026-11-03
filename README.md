# ABYSS — TI-84 Plus CE-T exploration-platformer project

This is a native C / CEdev project for the TI-84 Plus CE family. It is a playable **non-combat adaptation** of the supplied ABYSS concept: the sword-fighting parts are replaced by guardian-avoidance trials, rune interactions, and environmental puzzles. There are no weapon attacks.

## What's in this build

- A reusable side-scrolling platform engine with acceleration, gravity, jumping, ladders, one-way platforms, camera follow, hazards, moving guardians, and a phase-step traversal ability.
- All 20 requested level names, grouped into the four requested worlds. Level layouts are generated into one reusable compact tile map, with different palette themes per world.
- Four major guardian trials (levels 5, 10, 15, and 20). Each uses a moving guardian hazard and three runes that must be activated before the exit opens. Other trial rooms contain smaller guardian hazards.
- A waystation with PLAY, UPGRADES, COLLECTION, and STATS screens.
- Three permanent upgrade categories: Armor, Rune Reach, and Maximum HP. Rune Reach replaces the requested sword-damage upgrade as a non-combat progression stat.
- Chests, hidden caches, coins, permanent relic records, HP regeneration, death/retry, and a save AppVar named `ABYSSSV`.
- Progression abilities are traversal / exploration tools: Echo, Wall Grip, Phase Step, Ward, Long Echo, Glide, and Gate Key.

## Controls

| Calculator key | Action |
|---|---|
| LEFT / RIGHT | Move |
| UP / DOWN | Climb ladders; DOWN also activates Glide once unlocked |
| 2ND | Jump |
| UP (press) or ENTER | Interact with caches, runes, and exits |
| ALPHA | Phase Step once unlocked |
| CLEAR | Pause / return to the waystation |
| ON | Exit to the calculator OS |

The first five levels are designed to be accessible with the starting movement set. Phase Step unlocks with progression. The Ward ability adds a short protection window to the Phase Step action after its unlock.

## Build with GitHub Actions (no local CEdev install)

This repository includes `.github/workflows/build.yml`. To build in GitHub:

1. Create a new GitHub repository.
2. Upload the **contents of this project folder** to the repository root (so `.github/workflows/build.yml`, `Makefile`, `src/`, and `include/` are at the top level).
3. Open the repository's **Actions** tab. The `Build ABYSS for TI-84 Plus CE` workflow starts on a push; you can also select it and choose **Run workflow**.
4. When the run finishes successfully, open that run and download the `ABYSS-8XP` artifact. It contains `ABYSS.8xp`.

The workflow uses the `gavinhsmith/setup-ti84-ce-toolchain` action with CEdev v14.2, runs the host-side tests, runs `make`, checks that `bin/ABYSS.8xp` exists, and uploads that file. The workflow has been added to the source package, but it has **not yet been executed on your GitHub repository**; the first cloud run is the actual CEdev compile check.

## Build

1. Install the CEdev toolchain for your computer and open its terminal environment.
2. Install the matching CE Libraries runtime on the calculator. The GraphX, keypadc, and fileioc routines are dynamically linked, so a matching `clibs.8xg` group file (or its individual AppVars) is required.
3. From this project directory, run:

   ```sh
   make
   ```

4. The expected output is `bin/ABYSS.8xp`.
5. Transfer `ABYSS.8xp` to the calculator and launch it through the native-program launcher enabled by your arTIfiCE / AsmHook2 setup.

Use a CEdev release compatible with the CE Libraries version you install. Do not mix library versions: LibLoad enforces version compatibility.

## arTIfiCE compatibility note

The arTIfiCE project currently documents v3 for CE OS 5.8.5 and v2 for supported versions through OS 5.8.4; OS 5.4.0 and earlier do not require arTIfiCE for native programs. Confirm the exact OS and calculator model against the arTIfiCE compatibility page before transferring files. The TI-84 Plus CE-T Python Edition is listed as supported.

## Host-side checks

Run `./tests/run_tests.sh` on a computer with a C compiler to check source syntax plus save-checksum, chest persistence, guardian-trial rune gates, and level-unlock logic. These tests use small host stand-ins for CE Libraries; they do not emulate the calculator, verify frame rate, or replace a CEdev build.

## Memory strategy

- `World.map` is the only active level tile buffer: `80 x 15 = 1,200` bytes.
- There are no heap allocations and no full-screen RAM framebuffer.
- Rendering uses GraphX's visible-screen target (`gfx_SetDrawScreen`) directly. The project deliberately never calls `gfx_SetDrawBuffer`, `gfx_BlitBuffer`, or a full-screen sprite allocation.
- Level geometry is regenerated at level load; no inactive tilemaps are kept in RAM.
- The save record is a small fixed-size AppVar. Reads can occur directly from archive; writes open the AppVar in RAM, write it, then archive it through fileioc. The AppVar is left in RAM and a warning is displayed if archive space is insufficient.
- The CEdev link map is emitted to `bin/ABYSS.map`. Use it to inspect the actual BSS / stack layout for the exact toolchain build. The project has **not** been run through CEdev or on physical hardware in this environment, so a measured total-RAM claim would be premature.

## Save data

`ABYSSSV` stores a versioned, checksummed record containing completed-level bits, the unlocked level, coins, upgrade levels, ability bits, relics, treasure and secret records, and statistics. A checksum/version mismatch resets to a clean save rather than trusting damaged data.

## Important scope / verification status

This is a substantial playable engine foundation with a complete 20-stage progression table, but it is **not yet a fully content-authored commercial-quality 20-level campaign**. The layouts are deterministic procedural templates rather than 20 individually hand-built dungeons; boss rooms are non-combat rune trials; and the art is drawn from compact GraphX primitives rather than hand-authored sprite sheets. It has not been hardware-tested or performance-profiled. The next development pass should playtest collision and frame pacing on a real CE-T, inspect `ABYSS.map`, and then hand-author more distinctive room layouts and sprite animation frames within the verified RAM budget.

## References

- CEdev getting started / build flow: https://ce-programming.github.io/toolchain/nightly/static/getting-started.html
- CEdev GraphX documentation: https://ce-programming.github.io/toolchain/libraries/graphx.html
- CEdev file-I/O documentation: https://ce-programming.github.io/toolchain/libraries/fileioc.html
- CE Libraries releases: https://github.com/CE-Programming/libraries/releases
- arTIfiCE compatibility and installation: https://yvantt.github.io/arTIfiCE/
