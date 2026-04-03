# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Build everything (uses WPILib development builds by default)
./gradlew build

# Build against the last tagged WPILib release instead of dev builds
./gradlew build -PreleaseMode

# Build for all desktop platforms (used in CI for Windows/macOS)
./gradlew build -Pbuildalldesktop

# Cross-compile for specific targets only
./gradlew build -Ponlylinuxathena   # roboRIO
./gradlew build -Ponlylinuxarm32
./gradlew build -Ponlylinuxarm64

# Install the roboRIO cross-compiler toolchain
./gradlew installRoboRIOToolchain

# Run tests (Java + native; disabled when targeting non-desktop platforms)
./gradlew test

# Publish locally to build/repos/releases
./gradlew publish
```

## Architecture Overview

The project produces three distinct libraries that are published independently to Maven:

| Library | Sources | Maven artifact suffix | Notes |
|---|---|---|---|
| **TigerTuningDriver** (JNI driver) | `src/main/driver/` | `-driver` | Low-level; HAL + wpiutil only. C symbols only — all exported symbols must be listed in `src/main/driver/symbols.txt`. Cross-compiled for roboRIO, arm32, arm64. |
| **TigerTuning** (native C++) | `src/main/native/` | `-cpp` | Full wpilib access. Links against TigerTuningDriver (shared). Singleton configuration cache backed by a JSON file with SmartDashboard override support. |
| **Java** | `src/main/java/` | `-java` | Full wpilib access. Loads TigerTuningDriver via JNI at static init through `com.tigertuning.jni.TigerTuningJNI`. |

**Key constraint:** The driver library may only export C symbols (no C++ ABI). Any new exported symbol must be explicitly added to `src/main/driver/symbols.txt`.

### TigerTuning C++ API

`TigerTuning` is a singleton (`TigerTuning::GetInstance()`) that reads a JSON file at startup (default: `/home/lvuser/deploy/tigertuning.json`). Call `TigerTuning::Initialize(filePath)` before first use to override the path.

Getter methods: `GetDouble`, `GetInt`, `GetBoolean`, `GetString`, `GetDoubleArray`, `GetStringArray` — each takes a key and a default value. When the SmartDashboard key `TigerTuning/OverrideEnabled` is `true` (and no FMS is attached), getters return SmartDashboard values instead, seeding the dashboard from the JSON cache on first enable.

### JNI loading pattern

`com.tigertuning.jni.TigerTuningJNI` calls `System.loadLibrary("TigerTuningDriver")` at static initialization (controllable via `TigerTuningJNI.Helper.setExtractOnStaticLoad()`). JNI header generation is driven by `compileJava` and the `JniNativeLibrarySpec` model in `build.gradle`.

### Test suites

- `TigerTuningTest` — native C++ tests (`src/test/native/cpp/`) — links TigerTuning, wpilib, googletest
- `TigerTuningDriverTest` — driver-level tests (`src/test/driver/cpp/`) — links wpilib, googletest
- Java tests — JUnit 5 (`src/test/java/com/tigertuning/`) — the native `lib/` install directory is set on `java.library.path` so JNI loads work

## Renaming / Customizing

When adapting this template, the following must all change together:

- **Java library name**: rename the project folder (it becomes the JAR base name)
- **Native C++ library** (`TigerTuning`): `exportsConfigs` block in `build.gradle`, `components` block in `build.gradle`, `taskList` input in `publish.gradle`
- **Driver library** (`TigerTuningDriver`): `privateExportsConfigs` in `build.gradle`, `components` block in `build.gradle`, `driverTaskList` in `publish.gradle`, the `lib library:` reference in the TigerTuning component, and `System.loadLibrary(...)` in the JNI Java class
- **Maven coordinates**: `artifactGroupId`, `baseArtifactId` near the top of `publish.gradle`
- **Vendordep JSON**: `TigerTuning.json` — fill in the UUID, maven URL, and json URL; `frcYear` is currently `"2026beta"`. Template variables `${version}`, `${groupId}`, `${artifactId}` are substituted at build time by the `vendordepJson` task. Note: the UUID field intentionally contains placeholder text that causes parse failure until replaced.

## CI

GitHub Actions (`.github/workflows/ci.yml`) runs:
1. Gradle wrapper validation
2. Cross-compile builds via WPILib Docker images (Athena/roboRIO, Arm32, Arm64, Linux)
3. Native host builds on Windows and macOS
4. A combine step using `wpilibsuite/build-tools` that merges all `build/allOutputs` artifacts into a Maven repository
