# Binaural Jungle

A series of binaural VST instruments that reconstruct the sonic palette of old-school atmospheric jungle and drum & bass (1993–96). Each release in the series focuses on one element of the era — pads, basses, breaks, stabs — and renders it through a modern HOA-to-HRTF spatial pipeline so it sits in a three-dimensional headphone field rather than a flat stereo plane.

> v1 release — **Binaural Pad/String Instrument** (working title).
> Aesthetic reference: LTJ Bukem, Goldie, 4hero, Photek, Source Direct, Peshay.

---

## What it is

A polyphonic synth + granular instrument plugin. Every voice is positioned in a 3rd-order Ambisonic sound field, folded through HOA early reflections and a HOA diffuse reverb, and decoded to stereo headphones using MagLS HRTF decoding. The result is a pad that feels externalised — outside the listener's head — rather than panned between the ears.

## Why it exists

Binaural audio on the market today is dominated by **effect plugins** (dearVR, Waves Nx, IEM Suite, SPARTA) that spatialise external sources. There is no dedicated binaural **instrument** tailored to a specific musical aesthetic. Binaural Jungle fills that gap with a synth-first design purpose-built for one era and one genre.

## Tech stack (v1 — locked)

| Layer | Choice |
|---|---|
| Framework | JUCE 8 + C++20 |
| Plugin formats | VST3 (AU/CLAP/AAX in follow-on releases) |
| CLAP support | `clap-juce-extensions` (MIT) |
| Sound engine | Synthesis + granular hybrid |
| Binaural pipeline | Per-voice HOA 3rd-order encode → HOA early reflections → HOA diffuse reverb → MagLS HRTF decode to stereo |
| HRTF sets | MIT KEMAR + ARI (shipped) + SOFA import via libmysofa |
| Build system | CMake |
| License | Dual: GPLv3 source + commercial binaries |

Sample-playback layer is architected into the voice system but deferred to v1.1.

## Repo layout

```
Binaural Jungle/
├─ README.md                  ← this file
├─ design/
│  ├─ 00-design-decisions.md  ← locked choices + rationale
│  └─ 01-architecture.md      ← signal flow, modules, threading, latency
├─ research/
│  ├─ existing-binaural-vsts.md
│  ├─ binaural-audio-techniques.md
│  ├─ jungle-pad-sound-design.md
│  └─ vst-development-frameworks.md
├─ src/
│  ├─ app/                    ← plugin processor, editor, state
│  ├─ dsp/
│  │  ├─ voice/               ← oscillators, granular engine, envelopes
│  │  ├─ spatial/             ← HOA encode, early refs, reverb, MagLS decode
│  │  └─ util/                ← buffers, math, interpolation
│  └─ gui/                    ← editor components, look-and-feel
└─ resources/
   └─ hrtf/                   ← MIT KEMAR + ARI SOFA files (shipped)
```

## Status

- [x] Design decisions locked
- [x] Architecture spec drafted
- [x] Research (competitive, technical, sound-design, framework) complete
- [ ] CMake + JUCE project scaffold
- [ ] Voice engine prototype (mono synth, no granular, no binaural)
- [ ] HOA encode + MagLS decode prototype (white noise at a fixed azimuth)
- [ ] Integrate synth + HOA pipeline
- [ ] Granular layer
- [ ] Early reflections + HOA diffuse reverb
- [ ] GUI pass
- [ ] Factory preset programming
- [ ] Codesigning + notarisation
- [ ] VST3 release candidate

## License

- **Source code**: GPLv3 (compatible with JUCE's free tier).
- **Shipping binaries**: commercial license, sold directly.

See `design/00-design-decisions.md` for the full licensing rationale.

## Links

- Repo: https://github.com/AdairBear/Binaural-Jungle
