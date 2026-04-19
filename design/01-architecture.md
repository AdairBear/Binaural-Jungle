# 01 — Architecture

## Signal flow

```
                  ┌──────────────────────────────────────────────┐
MIDI / MPE ──►    │             Voice Manager (poly)              │
                  │   ┌───────────┐   ┌───────────┐   ┌────────┐  │
                  │   │  Voice 0  │   │  Voice 1  │...│ Voice N│  │
                  │   │           │   │           │   │        │  │
                  │   │ OscStack  │   │ OscStack  │   │ OscStack│ │
                  │   │ + Gran    │   │ + Gran    │   │ + Gran  │ │
                  │   │ + Filter  │   │ + Filter  │   │ + Filter│ │
                  │   │ + Env     │   │ + Env     │   │ + Env   │ │
                  │   │ + Mod     │   │ + Mod     │   │ + Mod   │ │
                  │   │           │   │           │   │        │  │
                  │   │ mono out  │   │ mono out  │   │ mono out│ │
                  │   │   + az/el │   │   + az/el │   │  + az/el│ │
                  │   └─────┬─────┘   └─────┬─────┘   └────┬───┘  │
                  └─────────┼───────────────┼──────────────┼──────┘
                            │               │              │
                            ▼               ▼              ▼
                  ┌──────────────────────────────────────────────┐
                  │       HOA Encoder  (mono → 16ch ACN/SN3D)    │
                  │       per-voice spherical-harmonic encode     │
                  └─────────────────────┬────────────────────────┘
                                        │   16-ch HOA bus
                                        ▼
                  ┌──────────────────────────────────────────────┐
                  │           HOA Early Reflections               │
                  │  (image-source model, a few discrete taps in  │
                  │   the HOA domain — encodes each reflection    │
                  │   back into ACN/SN3D)                         │
                  └─────────────────────┬────────────────────────┘
                                        │   16-ch HOA bus
                                        ▼
                  ┌──────────────────────────────────────────────┐
                  │           HOA Diffuse Reverb                  │
                  │  (FDN or Jot-style reverb driven per-channel, │
                  │   with decorrelation and frequency damping)   │
                  └─────────────────────┬────────────────────────┘
                                        │   16-ch HOA bus (dry + wet)
                                        ▼
                  ┌──────────────────────────────────────────────┐
                  │     MagLS HRTF Decoder  (16ch HOA → 2ch)     │
                  │     partitioned convolution using              │
                  │     pre-computed MagLS decoder matrices        │
                  │     (derived from loaded SOFA HRIRs)           │
                  └─────────────────────┬────────────────────────┘
                                        │   stereo (L/R)
                                        ▼
                                    Host Output
```

## Module tree

```
src/
├─ app/
│  ├─ PluginProcessor.{h,cpp}         ← juce::AudioProcessor subclass
│  ├─ PluginEditor.{h,cpp}            ← juce::AudioProcessorEditor
│  ├─ ParameterLayout.{h,cpp}         ← APVTS parameter definitions
│  └─ PresetManager.{h,cpp}           ← load/save state, factory presets
│
├─ dsp/
│  ├─ voice/
│  │  ├─ VoiceManager.{h,cpp}         ← polyphonic allocator, MPE-aware
│  │  ├─ Voice.{h,cpp}                ← single voice: osc + gran + filter + env
│  │  ├─ OscillatorStack.{h,cpp}      ← detuned multi-osc (Juno/Jup vibe)
│  │  ├─ GranularEngine.{h,cpp}       ← time-stretch, grain cloud
│  │  ├─ Filter.{h,cpp}               ← SVF / ladder, via juce::dsp
│  │  ├─ Envelope.{h,cpp}             ← ADSR / DAHDSR
│  │  └─ ModMatrix.{h,cpp}            ← LFOs, env routing, MPE sources
│  │
│  ├─ spatial/
│  │  ├─ HOAEncoder.{h,cpp}           ← mono → 16ch ACN/SN3D encode
│  │  ├─ HOACoefficients.{h,cpp}      ← azimuth/elevation → coefs (lookup + interp)
│  │  ├─ EarlyReflections.{h,cpp}     ← image-source model in HOA
│  │  ├─ DiffuseReverb.{h,cpp}        ← FDN-in-HOA reverb
│  │  ├─ MagLSDecoder.{h,cpp}         ← 16ch HOA → stereo, partitioned convolution
│  │  ├─ SOFALoader.{h,cpp}           ← libmysofa wrapper, HRIR → filters
│  │  └─ DecoderMatrix.{h,cpp}        ← offline MagLS matrix computation
│  │
│  └─ util/
│     ├─ AudioBufferRing.{h,cpp}      ← lock-free ring buffers where needed
│     ├─ SmoothedValue.{h,cpp}        ← parameter smoothing
│     ├─ FFTWrapper.{h,cpp}           ← juce::dsp::FFT helpers
│     └─ MathConstants.h              ← TAU, inv-sqrt-2, SH normalisation
│
├─ gui/
│  ├─ LookAndFeel.{h,cpp}             ← custom rendering
│  ├─ Knob.{h,cpp}                    ← rotary control
│  ├─ SpatialPanner.{h,cpp}           ← 3D position editor (az/el/dist)
│  └─ components/…                    ← page-specific panels
│
└─ resources/
   └─ hrtf/
      ├─ mit_kemar_normal.sofa
      ├─ ari_subject_003.sofa
      └─ ari_subject_021.sofa
```

## Threading model

| Thread | Responsibilities |
|---|---|
| **Audio (RT)** | `processBlock()`: voice rendering, HOA encode, early refs, HOA reverb, MagLS decode. No allocation, no locks, no file I/O. |
| **Message (GUI)** | `juce::MessageManager`. Editor rendering, user input, parameter change dispatch. |
| **Background worker** | SOFA file loading, MagLS decoder matrix computation (can take >100 ms), preset I/O. Communicates with audio thread via `juce::AbstractFifo` or `std::atomic` pointer swaps. |

Preset switching that involves a new HRTF set triggers the worker thread to build the new MagLS decoder matrix; once ready, the audio thread hot-swaps the matrix pointer at the next block boundary.

## Latency budget

Target total round-trip latency: **≤ 10 ms at 48 kHz, 128-sample host block.**

| Stage | Samples @ 48 kHz | ms |
|---|---|---|
| Host block | 128 | 2.7 |
| HOA encode (single pass) | 0 | 0 |
| Early reflections (image-source, longest delay tap ~20 ms) | delay-line tap | — (latency-neutral for direct path) |
| HOA diffuse reverb (latency-neutral, parallel with dry path) | 0 | 0 |
| MagLS partitioned convolution (first partition 128 samples) | 128 | 2.7 |
| Plugin-reported latency | **128** | **2.7** |

With a 256-sample host block, plugin-reported latency rises to ~5.3 ms. All values well within acceptable bounds for live play.

## Voice allocation

- Polyphony: target **16 voices** in v1, stretch to 32 if CPU headroom allows.
- Allocation strategy: oldest-note-stealing, with a soft release-tail priority (don't steal a voice that's still in its audible release).
- MPE aware: per-note pitch bend, pressure, and Y-slide feed the mod matrix as sources.
- Per-voice spatial position (azimuth / elevation / distance) is a modulation destination — a "position" envelope or LFO can sweep the voice around the listener.

## Parameter layout (APVTS sketch)

```
[Oscillator]
  osc_mix (0–1, per-voice balance vs granular)
  osc_detune_cents
  osc_stack_size (2–7 oscillators)
  osc_shape (saw ↔ pulse)

[Granular]
  gran_pos (source position, 0–1)
  gran_size_ms
  gran_density (grains/sec)
  gran_pitch_spread
  gran_source (user-loaded sample slot)

[Filter]
  filter_cutoff
  filter_resonance
  filter_type (SVF LP/BP/HP, ladder)

[Envelope × 2]
  amp_attack / decay / sustain / release
  mod_attack / decay / sustain / release

[Modulation]
  mod_matrix (5 free slots, src → dst → amount)
  lfo_1 / lfo_2 (rate, shape, sync)

[Spatial]
  voice_spread_azimuth (how wide voices are panned)
  voice_spread_elevation
  position_lfo_rate
  reverb_size
  reverb_decay
  reverb_mix
  hrtf_set (MIT KEMAR / ARI / user)

[Master]
  gain
  output_limiter
```

## Prototype sequence

The build order — each step is independently testable.

1. **Plugin skeleton.** `juce_add_plugin`, VST3 target, empty `processBlock`, boots in a host, passes pluginval.
2. **Mono synth voice.** One oscillator, amp envelope, MIDI in, mono out. Stereo duplicated. Smoke-test in Reaper.
3. **Polyphonic voice manager.** Oldest-note stealing, 16-voice pool, MIDI channel + MPE.
4. **Detuned oscillator stack + filter.** The classic Juno-lush pad.
5. **HOA encode + MagLS decode prototype.** Static azimuth, single HRTF set hard-coded, single voice. Goal: hear a synth voice rotate around the head.
6. **SOFA loader.** libmysofa integration, validate HRIR load, compute MagLS decoder matrix on load.
7. **Per-voice HOA encoding.** Each voice gets its own azimuth/elevation in the spatial block.
8. **HOA diffuse reverb.** FDN in the HOA domain; externalisation test on pads with long release.
9. **Granular engine.** Time-domain granular on a user-loaded sample, then wired into the voice as a parallel sound source.
10. **Early reflections.** Image-source model, a handful of taps, encoded back into HOA.
11. **GUI pass.** APVTS parameter attachments, basic panel, 3D spatial panner widget.
12. **Factory presets.** 20–40 presets covering the canonical sound-design targets (see `research/jungle-pad-sound-design.md`).
13. **CI + validation.** GitHub Actions for macOS universal, Windows x64, pluginval + auval.
14. **Codesigning + notarisation.** Apple Developer ID + Windows EV cert pipeline.
15. **Release candidate.** Beta to a small tester pool, iterate, ship.

## CI outline

- **macOS (arm64 + x86_64 universal):** Xcode toolchain, codesign, notarise, pluginval + auval.
- **Windows (x64):** MSVC toolchain, codesign, pluginval.
- **Linux (x64):** GCC toolchain, pluginval. (Not a shipping target for v1 but catches portability regressions early.)
- Trigger: push to `main`, PRs to `main`, tags `v*`.
- Artefacts: signed VST3 bundles uploaded to the GitHub release.
