# VST Plugin Development Frameworks — Evaluation for Binaural Jungle

> **Status:** Decision ratified.
> **Choice:** JUCE 8 (C++20) + `clap-juce-extensions`, CMake build, VST3 first, AU/CLAP/AAX/Standalone follow-on.
> **Scope:** Binaural synth instrument — HOA (3rd-order Ambisonics, ACN/SN3D) rendered through MagLS HRTF decode, MPE-aware voicing, SOFA-file runtime selection.
> **Author:** Thomas Adair. **Date:** 2026-04-18.

---

## 0. Decision Summary (TL;DR)

| Axis | Chosen | Why (one line) |
|---|---|---|
| Framework | **JUCE 8** | Industry default; every format in one tree; mature DSP module; real-world production record. |
| Language | **C++20** | Matches JUCE 8 minimum (requires C++17, supports C++20/23); enables `std::span`, concepts, ranges. |
| Build | **CMake (`juce_add_plugin`)** | First-class in JUCE 8; Projucer can be avoided entirely; reproducible CI. |
| CLAP | **`clap-juce-extensions`** (Urbanek et al., MIT) | Bolted onto a JUCE project with ~20 lines of CMake; no second codebase. |
| HRTF | **libmysofa** (BSD-3) | The de-facto open SOFA reader; statically linkable into a commercial binary. |
| HOA math | **Eigen** (MPL 2.0) | Weak copyleft; fine to link; dense + small matrices map perfectly to decode. |
| FFT | **JUCE FFT / pffft fallback** | NOT FFTW (GPL). JUCE FFT is good enough; `juce::dsp::Convolution` already wraps a partitioned impl. |
| Validation | **pluginval + auval + VST3 validator** in CI | All three run on every PR; GitHub Actions matrix for mac universal + Win x64. |
| License model | **JUCE Indie** ($40/mo) | Under $500k rev cap; avoids GPLv3 obligations on shipped binary. |

---

## 1. Framework Comparison Table

| Framework | Language | License | Plugin Formats | GUI Story | DSP Helpers | CI / Signing | Notable Products | Community | Learning Curve | Headline Strengths / Weaknesses |
|---|---|---|---|---|---|---|---|---|---|---|
| **JUCE 8** | C++17/20/23 | GPLv3 *or* Personal (free, watermark)/Indie ($40/mo, $500k cap)/Pro/Educational | VST3, AU, AUv3, AAX, LV2, Standalone, iOS; CLAP via `clap-juce-extensions` | Native `Component`/`Graphics`; new OpenGL + software renderer overhaul in JUCE 8; `melatonin_blur`; Foleys GuiMagic | Rich `juce::dsp` (oscillators, filters, convolution, FFT, IIR, FIR, WaveShaper) | CMake everywhere, easy GH Actions, codesign/notarise via scripts | Ableton-M4L plugins, Serum 2 (partial), FabFilter (custom), Spitfire, Output, ROLI, Krotos, Tracktion | Largest; forum + Discord active; >10k GitHub stars | Medium — well-documented | + Universal, battle-tested, supports every format. − C++ boilerplate; commercial licensing tier required for most shipping products. |
| **iPlug2** | C++17 | BSD-3 / ISC-style | VST3, VST2 (deprecated), AU, AUv3, AAX, Web (WAM), Standalone, CLAP | `IGraphics` custom renderer (Skia/NanoVG backend) | Light helpers, less batteries-included than JUCE | Self-rolled; Xcode + VS projects generated | Klevgrand, some smaller indies; oli larkin's demos | Smaller but loyal; GitHub active | Medium-high — less hand-holding | + Permissive license, no royalties, MIT-style freedom. − Fewer ready DSP building blocks, smaller ecosystem, GUI stack is non-native. |
| **nih-plug** | Rust | ISC | VST3, CLAP, Standalone (JACK/CPAL) | egui, iced, vizia, or raw wgpu — no native toolkit | Traits-based param, good audio graph; smaller DSP lib (`nih-plug` + external crates) | Cargo-native, integrates with `cargo-xtask` | Robbert van der Helm's plugins; Oxefmsynth; growing indie scene | Small, high-signal; Discord active | High — Rust FFI for audio is still rough | + Memory-safe, modern, free. − No AU/AAX. Shipping commercial on macOS requires a C++ AU wrapper. Immature vs JUCE. |
| **DPF** (DISTRHO Plugin Framework) | C++ | ISC | LV2, VST2, VST3, CLAP, AU (limited), JACK, DSSI | Dear ImGui, OpenGL, or custom | Minimal DSP; bring your own | CMake + Makefiles; Linux-first | ZamAudio, DISTRHO plugins, Airwindows ports | Small but healthy in Linux pro-audio | Medium | + Free, ISC, minimal overhead, perfect for Linux. − macOS/Windows polish thin; no first-class AAX. |
| **CLAP SDK (standalone)** | C (header-only) | MIT | CLAP only | BYO | None — protocol only | BYO | u-he, Bitwig-authored utilities; Surge XT | CLAP community (Bitwig/u-he driven), growing fast | Medium (protocol, not framework) | + Cleanest modern plugin ABI (thread-safe param system, poly voice stacking, note expression). − Format-only; must be wrapped or combined with another framework. |
| **Faust + wrapper** | Faust DSL → C++/WASM | GPLv2/MIT runtime | Anything Faust's `faust2*` scripts can produce (VST3, AU, JUCE, iPlug2, DPF, LV2) | None native; wrapper-provided | FAUST is itself a DSP toolkit | Via target wrapper | GRAME Research, HERMuTe, many academic plugins | Niche but loyal, research-heavy | Low for DSP, high for GUI | + Extremely fast DSP prototyping, auto-vectorised. − No GUI, limited control over block processing, poor for stateful synths with complex voicing. |

### Framework score (HOA/binaural synth fit)

| Criterion | Weight | JUCE 8 | iPlug2 | nih-plug | DPF | CLAP SDK | Faust |
|---|---:|---:|---:|---:|---:|---:|---:|
| All shipping formats | 20 | **10** | 9 | 5 | 7 | 2 | 6 |
| DSP helpers (convolution, FFT) | 15 | **10** | 5 | 6 | 3 | 0 | **10** |
| GUI maturity | 15 | **9** | 7 | 5 | 4 | 0 | 1 |
| License for commercial binary | 15 | 8 (paid tier) | **10** | **10** | **10** | **10** | 8 |
| Ecosystem / hiring | 10 | **10** | 6 | 4 | 3 | 5 | 3 |
| SOFA / Eigen integration ease | 10 | **9** | 7 | 6 | 6 | 7 | 4 |
| MPE support built-in | 10 | **10** | 6 | 7 | 4 | 8 | 2 |
| Production debugging tools | 5 | **9** | 6 | 5 | 4 | 3 | 3 |
| **Weighted total / 100** | | **93** | 72 | 60 | 54 | 43 | 53 |

JUCE 8 wins on every axis except raw license cost, and the $40/mo Indie fee is under a rounding error compared to the integration work saved.

---

## 2. JUCE 8 Deep Dive

### 2.1 What changed in JUCE 8 vs JUCE 7

JUCE 8 (released late 2024, through 8.0.x patch stream in 2025) is the biggest version bump since JUCE 5. Relevant deltas:

- **Unified CMake API.** `juce_add_plugin()` and `juce_add_gui_app()` are stable and preferred; the Projucer now exists primarily for legacy maintenance. The old `JUCE.cmake` include-then-subdirectory dance is gone.
- **New GUI rendering stack.** JUCE's Direct2D backend on Windows is now default; Metal backend on macOS is improved; software renderer is faster. Text rendering moved to a harfbuzz-based layout — multi-script text (CJK, RTL) finally works without manual font hacks.
- **Accessibility.** `juce::AccessibilityHandler` is now mandatory to pass iLok-era Avid certification and Windows screen-reader tests. Every custom component should publish an accessibility role.
- **Animation + easing primitives.** `juce::Animator` and `juce::ValueSmoothingTypes` gained multi-target interpolation useful for meter ballistics and source-position GUI handles.
- **ARA 2.0.** More complete for hosts that support it (Studio One 7, Logic 11, Reaper). Not relevant for a synth instrument but worth noting.
- **VST3 SDK 3.7.12+** supported; host-context menu passthrough, program-change handling, and MIDI 2.0 note expression are first-class.
- **AUv3/iOS** building cleanly from CMake. Helpful if a mobile tech demo becomes relevant.
- **`juce::dsp::Convolution`** gained better uniformly-partitioned scheduling and a thread-safe `loadImpulseResponse()` that hot-swaps without clicks. **This is the single feature most relevant to HRTF decode.**
- **C++20 support.** `std::span`, `concepts`, designated initialisers are now permitted inside JUCE-consuming code.
- **LV2 upstreamed.** Previously community-maintained, LV2 is now a first-class format in the JUCE tree.
- **Removed legacy.** VST2 SDK support is gone (you must bring your own). Carbon code on macOS is gone. 32-bit macOS is gone.

### 2.2 Licensing tiers

| Tier | Cost | Revenue cap | Output | Key obligation |
|---|---|---|---|---|
| **Personal** (free) | $0 | $0 (non-commercial) | Binary runs with a splash screen | Source-code clause of GPLv3 still applies to distributed binaries |
| **Indie** | $40/month or $480/year | **$500,000/year** company gross revenue | No splash; closed-source binary | Stay under the cap; if exceeded, upgrade at the beginning of next term |
| **Pro** | ~$130/month (list) | No cap | No splash; closed-source binary | Standard commercial redistribution |
| **Educational** | Free for accredited institutions | n/a | Closed-source binary for academic work | Non-commercial |
| **GPLv3** | Free | No cap | Full source release | Any downstream recipient receives the source |

Binaural Jungle starts under Indie. The $500k company revenue cap (not per-product; the whole entity) is the relevant ceiling. If the plugin plus any other JUCE-built product together cross the cap, we upgrade to Pro for the next term.

### 2.3 Dual-license mechanics in practice

JUCE's model is **dual-licensed source**: the JUCE framework code is available under GPLv3 *and* under the commercial (Indie/Pro) license. The commercial license explicitly grants us the right to statically link JUCE into a closed-source binary without triggering GPLv3 copyleft.

Two subtle gotchas:

1. **VST3 SDK** is Steinberg's separate dual-license (GPLv3 or Steinberg commercial agreement). For Indie tier we sign Steinberg's free online VST3 SDK License Agreement and we're fine.
2. **Third-party JUCE modules** (anything in `JUCE/modules/` or external modules) must be audited individually. `juce_core`, `juce_audio_basics`, `juce_dsp`, `juce_audio_processors` are all covered under the commercial license. Community modules (e.g. `foleys_gui_magic`) have their own licenses that must be tracked separately.

---

## 3. Plugin Format Support via JUCE

| Format | Route | License touch point | Notes |
|---|---|---|---|
| **VST3** | `juce_add_plugin(... FORMATS VST3)` — built-in | Steinberg VST3 SDK Agreement (free for Indie) | Primary initial target. |
| **AU / AUv3** | `FORMATS AU` or `AUv3` — built-in, macOS only | Apple dev account ($99/yr) for codesign + notarisation | Must pass `auval -v aumu XXXX YYYY`. |
| **AAX** | `FORMATS AAX` — requires Avid SDK (NDA + dev program enrolment, free) | PACE iLok integration for signed release builds | Dev builds run in Pro Tools Developer Mode. Signing is a paid PACE account. |
| **LV2** | `FORMATS LV2` — built-in as of JUCE 8 | LGPL parts of lilv/lv2 are dynamically linked | Good to ship on Linux. |
| **CLAP** | `clap-juce-extensions` (MIT, Tim Urbanek/Jatin Chowdhury) added as `add_subdirectory` + `clap_juce_extensions_plugin(target)` | MIT — no friction | Produces a `.clap` next to VST3. |
| **Standalone** | `FORMATS Standalone` — built-in | None | Useful for dev loop + end-user sketchpad. |

### 3.1 One CMake target, many outputs

```cmake
juce_add_plugin(BinauralJungle
    COMPANY_NAME                "YourCompany"
    PLUGIN_MANUFACTURER_CODE    Thom            # 4-char
    PLUGIN_CODE                 BjGl            # 4-char
    FORMATS                     VST3 AU Standalone
    PRODUCT_NAME                "Binaural Jungle"
    IS_SYNTH                    TRUE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    VST3_CATEGORIES             "Instrument|Synth|Stereo|Spatial"
    AU_MAIN_TYPE                "kAudioUnitType_MusicDevice"
    MICROPHONE_PERMISSION_ENABLED FALSE
)

clap_juce_extensions_plugin(TARGET BinauralJungle
    CLAP_ID "com.yourcompany.binauraljungle"
    CLAP_FEATURES instrument synthesizer stereo "spatial-audio"
)
```

Adding AAX later is a single line: `FORMATS VST3 AU AAX Standalone` plus `AAX_PATH` variable pointing at the Avid SDK.

---

## 4. JUCE DSP Helpers Relevant to Binaural Synth

`juce_dsp` is the load-bearing module for the signal path.

| Class | Role in Binaural Jungle |
|---|---|
| `juce::dsp::Oscillator<float>` | Per-voice carrier. Cheap sin/saw/square lookups; can inject custom `std::function` for wavetable oscillators. |
| `juce::dsp::LadderFilter<float>` | Moog-style ladder for character, drives the voice colour into the HOA encode. |
| `juce::dsp::StateVariableTPTFilter<float>` | TPT SVF — zero-delay feedback filter. Cleaner than `IIR::Filter`. Preferred for modulated cutoffs. |
| `juce::dsp::IIR::Filter` | Static / fixed-coefficient work: shelving for HRTF pre-emphasis, LF compensation. |
| `juce::dsp::FIR::Filter` | If we decide to pre-filter HRTFs with a minimum-phase FIR instead of full convolution per ear. |
| `juce::dsp::Convolution` | **The HRTF engine.** Uniformly partitioned convolution with background-thread IR loading. Handles partition-size selection; exposes `loadImpulseResponse(AudioBuffer, sampleRate, stereo/mono, trim, normalise)`. |
| `juce::dsp::FFT` | Order-parameterised real FFT (power-of-two sizes 32…8192). Used for offline analysis, spectrogram metering, SOFA impulse pre-processing. |
| `juce::dsp::WaveShaper` | Soft-clip stage before HOA encode to keep the ambisonic bus well-behaved. |
| `juce::AudioBuffer<float>` | Carries the 16-channel HOA signal internally. JUCE doesn't restrict channel count here; the limit is on the `BusesProperties` we declare. |
| `juce::MidiBuffer` | Note-on/off + CC + MPE channel-pressure/pitch-bend routing. |
| `juce::MPEInstrument` / `juce::MPESynthesiser` | Per-note pitch, pressure, Y (timbre), and channel-pressure expression — **the mechanism that maps MIDI → 3D source position**. Each MPE voice can own a {azimuth, elevation, distance} triple updated from the CC1/CC74/CC128 expression streams. |
| `juce::SmoothedValue<float, ValueSmoothingTypes::Linear>` | De-zipper source-position parameters before they enter the HOA encode matrix. Critical — any zipper here is audible as a ping-pong spatial artifact. |
| `juce::dsp::ProcessContextReplacing` / `NonReplacing` | Standard JUCE block-processing wrapper. Our voice-bus and HOA-bus processing chain pass these contexts. |

### 4.1 Convolution latency budget

`juce::dsp::Convolution` uses a uniformly-partitioned FFT scheme. For a 44.1 kHz HRTF of ~512 taps (~11.6 ms) and a host block size of 128 samples:

- Partition size typically matches block size → zero added latency beyond block.
- JUCE's implementation reports latency via `getLatency()` — this must be propagated into `setLatencySamples()` in the processor so the host compensates.
- For the **two convolutions per listener** (one per ear, post-decode), the amortised cost at 3rd-order HOA (16 channels) is ~32 FFTs per block. Fits in <5% CPU on M1 at 128-sample blocks.

---

## 5. HOA Pipeline Integration Details

### 5.1 Channel count and internal bus

3rd-order ACN/SN3D Ambisonics = **(N+1)² = 16 channels**. Channel indexing per ACN (Ambisonic Channel Number):

```
W (0,0) = 0
Y (1,-1), Z (1,0), X (1,1) = 1, 2, 3
V, T, R, S, U (2,-2..2) = 4..8
Q, O, M, K, L, N, P (3,-3..3) = 9..15
```

SN3D normalisation (not N3D, not FuMa). We convert on ingest if we ever take external ambisonic audio.

### 5.2 Bus configuration in the `AudioProcessor`

The plugin presents **stereo output** to the host (binaural is inherently two channels delivered to headphones). The HOA bus is purely internal — it never leaves the plugin.

```cpp
BusesProperties busLayout;
busLayout = busLayout.withOutput("Binaural", juce::AudioChannelSet::stereo(), true);
// No external ambisonic bus. HOA lives inside the voice renderer.
```

Internally, each voice allocates its own `juce::AudioBuffer<float>(16, blockSize)` for HOA storage. After summation, a single 16→2 matrix multiply (HOA decode × HRTF convolution) produces the stereo output.

An **alternate build flavour** (for post-production / surround use) can expose a 16-channel output bus by switching `BusesProperties`. VST3 supports arbitrary channel sets via `SpeakerArrangement`; declare `kAmbi3rdOrderACN` so hosts that understand it (Reaper, Nuendo, Pro Tools with relevant plugin layout) can route correctly. AU is limited here — no native ambisonic layout — so the alternate build is VST3-only.

### 5.3 MagLS decode matrix

MagLS (Magnitude Least Squares) produces a (2 × 16) complex frequency-dependent decode. Two implementation strategies:

1. **Pre-baked time-domain decode:** pre-compute `M_mls(f)` per SOFA measurement set, apply inverse FFT with linear phase reconstruction → 16 stereo HRIRs per listener (i.e. 32 IRs total). `juce::dsp::Convolution` holds each as a 2-channel IR; 16 `Convolution` instances run in parallel; their outputs sum into the stereo bus. This is the simplest and is what we ship first.
2. **Direct spherical-harmonic convolution:** 16 single-channel convolutions with pre-magled HRIRs per SH channel, then trivial sum. Same CPU bill, slightly cleaner code.

Both strategies rely on `juce::dsp::Convolution::loadImpulseResponse()` being called once at SOFA-swap time, on a non-audio thread.

### 5.4 FFT size and block-size choice

- **Host block size:** we accept 32..4096; DAWs typically use 128–512.
- **Convolution partition size:** let JUCE pick (`Convolution::Latency{blockSize}`), which maps to uniform partitions equal to the host block. This gives minimal added latency.
- **IR length per SH channel:** truncate MagLS-rendered IRs to 512 samples at 48 kHz (= 10.7 ms). More than enough for anechoic HRTFs; anything reverberant should be added as a separate late-reflection convolution stage with a longer partition.
- **Non-power-of-two blocks:** `juce::dsp::Convolution` handles them internally; no special care needed.

### 5.5 Channel layout reporting

For VST3 spatial builds, publish the ambisonic layout explicitly:

```cpp
// Inside isBusesLayoutSupported:
if (layouts.getMainOutputChannels() == 16) return true; // Ambi3 ACN
if (layouts.getMainOutputChannels() == 2)  return true; // binaural
return false;
```

Reaper reads this correctly; Ableton will always see stereo (it has no ambisonic understanding and will fall back to the first two channels if we expose 16, so the binaural build is safer for that DAW).

---

## 6. SOFA File Loading

### 6.1 libmysofa

**libmysofa** (Christian Hoene, BSD-3-Clause) is the only actively maintained, portable SOFA reader. It parses the HDF5-based `.sofa` container, extracts `Data.IR`, `Data.SamplingRate`, and `SourcePosition`, and offers a nearest-neighbour or weighted-interpolation lookup of HRIR pairs given an (azimuth, elevation, distance).

- **License:** BSD-3 — no copyleft, statically linkable into our closed-source binary without source exposure.
- **Dependencies:** bundled miniz for zlib compatibility; no runtime HDF5 dependency on end-user machines.
- **Integration:** `FetchContent_Declare(libmysofa GIT_REPOSITORY https://github.com/hoene/libmysofa.git GIT_TAG v1.3.3)` → `FetchContent_MakeAvailable(libmysofa)` → `target_link_libraries(BinauralJungle PRIVATE mysofa)`.

### 6.2 Wrapping

We do **not** wrap libmysofa into a `juce::AudioFormat`. `AudioFormat` is designed for streaming waveform data (WAV/AIFF/FLAC), and SOFA is a metadata-heavy multidimensional dataset — square peg.

Instead, a dedicated `HrtfSet` class:

```cpp
class HrtfSet {
public:
    bool loadFromFile(const juce::File& sofaFile);
    // Get HRIR pair for a given direction (nearest or interpolated)
    HrirPair lookup(float azimuthRad, float elevationRad, float distanceM) const;
    // Pre-bake the MagLS 16→2 decoder from the full measurement set
    DecodeIRs buildMagLsDecoder(int firLength) const;
private:
    MYSOFA_EASY* sofa = nullptr; // libmysofa handle
    // cached positions, IRs, sample rate
};
```

`DecodeIRs` feeds directly into 16 pre-loaded `juce::dsp::Convolution` instances.

### 6.3 License compatibility table

| Dep | License | Compatible with Indie JUCE binary? |
|---|---|---|
| JUCE 8 (Indie) | Commercial | yes |
| VST3 SDK (Steinberg free) | Commercial | yes |
| libmysofa | BSD-3 | yes |
| clap-juce-extensions | MIT | yes |
| Eigen | MPL 2.0 | yes (weak copyleft — only per-file, not whole binary) |

All good.

---

## 7. Third-Party DSP Libraries

| Library | License | Use | Decision |
|---|---|---|---|
| **Eigen** | MPL 2.0 | Dense matrix math — HOA encode/decode, MagLS solving | **YES.** Header-only, statically included. MPL only triggers at per-file level. |
| **FFTW** | GPL (commercial license exists for $5k+) | General FFT | **NO.** We are not paying $5k and we cannot link GPL into a closed binary. |
| **pffft** | BSD-like | Cross-platform FFT | **Available as fallback.** `juce::dsp::FFT` is fine for everything we currently need; keep pffft in mind if profiling pushes us. |
| **kissfft** | BSD-3 | Simple FFT | **Backup** — simpler than pffft, slightly slower. |
| **Spatial Audio Framework (SAF)** | GPLv3 | HOA encode, decode, spherical harmonics, HRIR convolution | **STUDY ONLY.** SAF is an excellent reference for algorithm correctness and exposes reference MagLS code. We read it, we don't link. |
| **IEM Plug-in Suite** | AGPLv3 | Same as SAF, mature JUCE plugins | **STUDY ONLY.** AGPL is the most viral — even network use triggers source release. Reference only for behaviour validation. |
| **juce-toys** | MIT (Jules' debugging helpers) | Parameter debugger, component browser, simd printers | **YES.** Dev-only inclusion. |
| **FRUT** | MIT (McMartin) | Projucer→CMake bootstrap utility | **NO, not needed.** JUCE 8's native CMake renders FRUT obsolete. |
| **melatonin_blur** | MIT (Sudara) | GPU-backed gaussian blur for drop shadows | **YES for GUI.** Adds <200 KB binary size, makes shadows tolerable on slow GPUs. |
| **chowdsp_utils** | GPL / commercial | General DSP utilities, meters, state-space filters | **MAYBE.** Revisit when specific need appears. License is dual; permissive under $ threshold similar to JUCE. |
| **Chowdhury DSP plugin_logger** | MIT | Opt-in crash telemetry | **YES eventually.** Useful for field diagnosis of HRTF corruption or SOFA decode bugs. |

---

## 8. Build and CI

### 8.1 CMake layout

```
BinauralJungle/
├── CMakeLists.txt           # top-level; pulls JUCE, clap-ext, libmysofa, eigen
├── cmake/                   # helper scripts (codesign, notarise, bundle)
├── src/
│   ├── PluginProcessor.{h,cpp}
│   ├── PluginEditor.{h,cpp}
│   ├── dsp/
│   │   ├── HoaEncoder.{h,cpp}
│   │   ├── MagLsDecoder.{h,cpp}
│   │   ├── HrtfSet.{h,cpp}     # wraps libmysofa
│   │   └── Voice.{h,cpp}        # MPE-aware voice
│   └── gui/
│       ├── LookAndFeelBj.{h,cpp}
│       └── SourceSphere.{h,cpp} # 3D position widget
├── resources/               # factory SOFA presets, UI assets
├── tests/                   # Catch2 or Google Test
├── .github/workflows/
│   ├── ci.yml               # build + pluginval matrix
│   └── release.yml          # signed + notarised artifacts
└── .clang-format / .clang-tidy
```

### 8.2 Top-level `CMakeLists.txt` skeleton

```cmake
cmake_minimum_required(VERSION 3.22)
project(BinauralJungle VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0")

include(FetchContent)

FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.5)
FetchContent_Declare(clap_juce_extensions GIT_REPOSITORY https://github.com/free-audio/clap-juce-extensions.git GIT_TAG main)
FetchContent_Declare(libmysofa GIT_REPOSITORY https://github.com/hoene/libmysofa.git GIT_TAG v1.3.3)
FetchContent_Declare(eigen GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git GIT_TAG 3.4.0)

FetchContent_MakeAvailable(JUCE clap_juce_extensions libmysofa eigen)

juce_add_plugin(BinauralJungle ...)                  # see Section 3.1
clap_juce_extensions_plugin(TARGET BinauralJungle    # see Section 3.1
                            CLAP_ID "com.yourcompany.binauraljungle"
                            CLAP_FEATURES instrument synthesizer stereo "spatial-audio")

target_compile_features(BinauralJungle PRIVATE cxx_std_20)
target_link_libraries(BinauralJungle PRIVATE
    juce::juce_audio_utils juce::juce_dsp
    mysofa Eigen3::Eigen)

juce_generate_juce_header(BinauralJungle)
```

### 8.3 GitHub Actions

**`ci.yml` matrix:**

| Job | OS | Compiler | Artifact | Validation |
|---|---|---|---|---|
| build-mac-universal | macos-14 | Xcode 15 | `.vst3`, `.component`, `.clap`, universal `.app` | `auval`, `pluginval --strictness-level 8`, VST3 validator |
| build-win | windows-2022 | MSVC 17 | `.vst3`, `.clap` | `pluginval --strictness-level 8`, VST3 validator |
| build-linux | ubuntu-22.04 | GCC 12 / Clang 16 | `.vst3`, `.clap`, `.so` LV2 | `pluginval --strictness-level 8` |
| lint | ubuntu-22.04 | clang-format 18 + clang-tidy 18 | - | style + analysis gates |

Key GH Actions helpers:

- `actions/setup-xcode@v1` — pin Xcode version.
- `lukka/get-cmake@latest` — fresh CMake.
- `microsoft/setup-msbuild@v2` — Windows toolchain.
- **Codesign (macOS):** import developer-id certificate from a base64 secret, `codesign --deep --options runtime --sign "Developer ID Application: ..."`, then `xcrun notarytool submit ... --wait`.
- **Codesign (Windows):** Azure Trusted Signing or SSL.com EV cert; `signtool sign /tr http://timestamp.sectigo.com /td SHA256 /fd SHA256 /a <artifact>`.

### 8.4 Validators

- **auval** (macOS, ships with Xcode): `auval -v aumu Thom BjGl`. Non-zero exit fails the build.
- **pluginval** (Tracktion, GPLv3 binary but we don't redistribute it): `pluginval --validate <plugin> --strictness-level 8 --timeout-ms 30000`. Fuzz tests parameter ranges, edit-load, process-edit, threading.
- **Steinberg VST3 Validator**: `validator.exe` shipped with the SDK. Catches preset save/load regressions, bus-layout misreporting.

---

## 9. GUI Approach Options

| Option | License | Pro | Con | Verdict |
|---|---|---|---|---|
| **JUCE native** (`Component` + `Graphics` + `LookAndFeel`) | Indie-covered | Zero extra dep, full control, native a11y | Hand-rolled layout; no hot-reload | **Primary path.** |
| **Foleys GuiMagic** | LGPLv3 (dual license; commercial tier exists) | XML-driven GUI, live preview | LGPL needs dynamic linking OR a commercial license; binary size | Skip for v1; re-evaluate if the GUI balloons. |
| **melatonin_blur** | MIT | Fast GPU blur for drop shadows | Needs GL context | **Adopt.** |
| **melatonin_inspector** | MIT | Live component tree + CSS-like tweaks in-plugin | Dev-only overhead | **Adopt for debug builds.** |
| **Rive** (via JUCE OpenGL) | MIT runtime | Vector + state-machine animations | Immature JUCE bridge, bindings are DIY | Skip for v1. |
| **Carbon / chowdhury plugin-logger** | MIT | Opt-in crash telemetry, audio-thread-safe ring buffer | Extra config to ship | Add after beta. |
| **Projucer-generated Look-and-Feel v4** | JUCE | Stock JUCE appearance | Dated | Use as a scaffolding start, then override. |

### GUI plan (v1)

- JUCE native components.
- Custom `LookAndFeelBj` for knobs, sliders, and the central 3D source-position widget.
- `melatonin_blur` for shadow passes on the source sphere and meter bezels.
- Accessibility handlers on every interactive component — mandatory for Avid AAX certification and general screen-reader support.

---

## 10. Recommendations and Day-Zero Checklist

### 10.1 Concrete setup order

1. **Create `CMakeLists.txt`** at repo root using the skeleton in 8.2.
2. **`.clang-format`** (LLVM-based, 100-col) and **`.clang-tidy`** (disable fuchsia checks, enable `modernize-*`, `cppcoreguidelines-*`, `bugprone-*`). Add a pre-commit hook that runs `clang-format -i` on staged files.
3. **Bootstrap plugin skeleton** with `juce_add_plugin()` in CMake — **do not** go through Projucer. Projucer in 2026 is a legacy editor; we don't need it.
4. **Wire `clap-juce-extensions`** in the same CMake file. Verify a built `.clap` loads in Bitwig Studio 5 before writing any DSP.
5. **Add libmysofa + Eigen via FetchContent.** Verify a smoke test program loads a reference SOFA file (e.g. KU100 from the SADIE II dataset) and prints the IR length.
6. **Stub the `HrtfSet` + `MagLsDecoder`** classes with unit tests (Catch2). Load a SOFA, bake a decoder, sanity-check IR energy.
7. **Stub the `HoaEncoder`** — given (az, el, distance), emit 16-channel encode gain vector. Unit test: energy sum of encode × decode should approximate identity for a test signal.
8. **First MPE voice** inherits `juce::MPESynthesiserVoice`. Oscillator in, panned via HOA encode, decoded + convolved to stereo. Verify with headphones + a reference azimuth sweep.
9. **CI from day zero.** Even the "empty plugin" commit should trigger pluginval and auval on a hosted runner. Bugs found at day 200 are 100× more expensive than bugs found at day 2.
10. **Set up codesign + notarisation workflows on a branch** before shipping the first beta. Do not wait until release day to discover the cert flow is broken.
11. **Licence tracking:** add `THIRD_PARTY_LICENSES.md` that enumerates JUCE, VST3 SDK, libmysofa, Eigen, clap-juce-ext, melatonin, along with their copyright notices. Ship it in the installer. This is a contractual requirement for every listed dep.

### 10.2 Explicit do-nots

- Do **not** use FFTW.
- Do **not** link Spatial Audio Framework or IEM Plug-in Suite — GPL/AGPL contamination.
- Do **not** fall back to Projucer. Our CI only knows CMake.
- Do **not** expose a 16-channel ambisonic output in the Ableton/Logic build — DAWs without ambisonic understanding will misroute and degrade the listener's experience.
- Do **not** use `juce::AudioFormat` as a SOFA wrapper — wrong abstraction.
- Do **not** defer accessibility to v2 — it's a one-time cost and gates Avid certification.

### 10.3 Milestone gates

| Milestone | Exit criteria |
|---|---|
| M1: Hello world plugin | VST3 + AU + CLAP + Standalone all load in Bitwig, Reaper, Logic, Live. pluginval + auval green. CI runs on every PR. |
| M2: HRTF-on-a-sine | A 440 Hz sine, encoded at az=90°, decoded and convolved through a KU100 SOFA, sounds like it's at az=90° through headphones. Unit tests verify energy and phase. |
| M3: MPE voice | Single MPE voice with azimuth = CC74, elevation = channel pressure, distance = pitch bend × k. Sounds correct across at least three SOFA sets. |
| M4: Poly + GUI | 16-voice polyphony, source sphere widget, preset save/load, host automation. Pluginval strictness 10. |
| M5: Beta | Codesigned, notarised installers for macOS arm64+x86_64 universal and Windows x64. AAX dev-mode build loads in Pro Tools. |
| M6: v1 Ship | AAX PACE signed, LV2 Linux build, telemetry opt-in, THIRD_PARTY_LICENSES.md shipped. |

---

## Appendix A — Why not Rust (nih-plug) for this project?

nih-plug is genuinely great for effects and simple synths targeting VST3 + CLAP only. For Binaural Jungle specifically, three blockers:

1. No AU support — on macOS, AU is required for Logic users, a meaningful fraction of the target audience.
2. AAX support is non-existent. We plan to ship AAX within 12 months.
3. The HRTF/Eigen/SOFA stack is C/C++ native. Rust FFI bindings exist but add friction; the productivity cost outweighs the safety benefit given that the audio thread is not where memory bugs typically happen (parameter code and GUI threads are, and JUCE's thread discipline is well-understood).

If a second, simpler utility plugin were being built, nih-plug would be a defensible choice.

## Appendix B — Why not iPlug2?

iPlug2 is the best free-as-in-beer JUCE alternative and has a viable commercial-without-fees story. The deciding factor is **ecosystem gravity**: the pool of JUCE developers is an order of magnitude larger, Stack Overflow + the JUCE forum + Discord together solve problems within hours, and every binaural/spatial audio reference plugin we care about (IEM, SPARTA, most u-he experiments) is JUCE-based. The $40/mo is cheap compared to the hiring and support advantage.

---

*End of document.*
