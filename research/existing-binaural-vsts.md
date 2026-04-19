# Existing Binaural-Capable VST/AU Plugins — Competitive Landscape for Binaural Jungle v1

> Research document surveying the binaural plugin market to identify the competitive gap for a **binaural pad/string instrument** (emphasis: *instrument*, not effect).
> Last updated: 2026-04-18
> Author: Thomas Adair — Binaural Jungle project

---

## 1. Purpose & Scope

Binaural Jungle v1 is a **native binaural synth instrument** targeting old-school jungle/atmospheric drum & bass production (LTJ Bukem, Good Looking Records, Reinforced/4hero, Goldie/Metalheadz, Photek — approx. 1993–96 era). The first release is a pad/string instrument with a JUCE 8 + C++20 codebase and a 3rd-order Higher-Order Ambisonics (HOA) pipeline decoded via Magnitude Least Squares (MagLS) HRTF.

This survey exists to answer one question precisely: **does a comparable product already exist?** The answer, as we'll show, is *no* — everything currently shipping is either (a) a spatialiser effect plugin that processes external audio, (b) an SDK for game/VR engines, or (c) an ambisonics toolkit requiring users to build their own instrument chain. None ship as a **genre-native binaural synthesizer instrument**.

---

## 2. Product Survey

### 2.1 Dear Reality — dearVR Pro / Monitor / Music

The current gold standard for plug-and-play binaural mixing. Three tiers targeting different users:

- **dearVR Pro** — full 3D object-based spatialiser with room simulation (46 rooms), distance/doppler, ambisonics I/O up to 3rd order. Effect only. ~$349 USD.
- **dearVR Monitor** — binaural monitoring-over-headphones for stereo/5.1/7.1/Atmos mixes. Head-tracking via dearVR Motion. ~$199.
- **dearVR Music** — stripped-down producer-focused variant, 18 rooms, simpler UI. ~$149.

Uses proprietary HRTF. Formats: VST3/AU/AAX. GUI: polished, 3D room-centric. DAW integration: universal, strong Atmos workflow. Stand-out: best-in-class one-shot room presets, very fast to get a usable result.

URLs: https://www.dear-reality.com/products/dearvr-pro | https://www.dear-reality.com/products/dearvr-monitor | https://www.dear-reality.com/products/dearvr-music

### 2.2 Waves — Nx Virtual Mix Room / Abbey Road Studio 3

Monitoring-focused binaural emulators. Nx emulates a generic "ideal" mix room on headphones; Abbey Road Studio 3 emulates the specific acoustics of the Studio 3 control room. Both support head-tracking via webcam or Nx Head Tracker puck.

Proprietary Waves HRTF (non-individualised). Effect plugin, stereo/surround/immersive input. VST/AU/AAX. ~$29–99 (frequent sales). GUI: minimal, single-panel. DAW integration: universal. Stand-out: lowest-friction monitoring tool; Abbey Road branding; head-tracking.

URLs: https://www.waves.com/plugins/nx | https://www.waves.com/plugins/abbey-road-studio-3

### 2.3 IEM Plug-in Suite (Institute of Electronic Music, Graz)

The academic reference. Free, open-source (GPLv3 — note: **GPL, not AGPL**; the task brief said AGPL but IEM ships under GPLv3). ~30 modular ambisonics plugins: `StereoEncoder`, `RoomEncoder`, `BinauralDecoder`, `DirectivityShaper`, `MultiEncoder`, etc. Used heavily in academic spatial audio research.

Direct ambisonics pipeline up to 7th order. `BinauralDecoder` uses MagLS decoding against the KU100 dummy-head HRTF (and a few alternatives). VST3 + Standalone. GUI: utilitarian, engineer-facing, mostly labelled knobs + XY pads. DAW integration: REAPER is the native home (multichannel track support); works in Bitwig, Nuendo; awkward in Logic/Pro Tools due to channel-count limits.

Stand-out: **this is the tech** Binaural Jungle's DSP pipeline resembles. Not an instrument. Not a product. A construction kit.

URL: https://plugins.iem.at/

### 2.4 SPARTA Suite — binauraliser, binauraliser NF, ambiBIN

From the Aalto Acoustics Lab. Sister project to IEM, also GPLv3 open-source. Strong academic pedigree.

- **binauraliser** — direct HRTF panning (up to 128 sources), multiple HRIR sets (KU100, KEMAR, IRCAM LISTEN, CIPIC subjects, custom SOFA import).
- **binauraliser NF** — near-field binauralisation with distance-dependent HRTF interpolation.
- **ambiBIN** — dedicated ambisonics-to-binaural decoder with MagLS, time-alignment, and diffuse-field covariance constraint options.

Formats: VST3/AU/Standalone. GUI: functional, engineer-facing, spherical plotter + parameter panels. DAW integration: best in REAPER. Stand-out: **SOFA HRTF import** (Binaural Jungle should steal this), comprehensive decoder options, rigorous DSP.

URL: https://leomccormack.github.io/sparta-site/

### 2.5 Noise Makers — Ambi Head HD / Binauralizer Studio

The French boutique indie take. Pretty, musician-friendly UIs.

- **Ambi Head HD** — 1st-to-3rd order ambisonics-to-binaural decoder with head-tracking. ~$69.
- **Binauralizer Studio** — up to 16 sources placed on a 3D sphere, head-tracking, multiple HRTF sets. ~$79.

Effect plugins. Proprietary decoder + Neumann KU100 and a few alternate HRTFs. VST3/AU/AAX. GUI: friendly, accessible, sphere-centric. Stand-out: most *musician-friendly* pricing and UX in the ambisonics-decoder category; excellent value.

URL: https://www.noisemakers.fr/

### 2.6 Auburn Sounds — Panagement 2

Not strictly ambisonic — this is a **binaural panner + stereo effect** with depth/distance cues, modulation, and LFO-driven motion. Popular with creative producers. Interesting because it's the closest existing product to "binaural but musical/creative rather than technical".

Proprietary HRTF, direct panning. Effect plugin. VST/AU/LV2/AAX. ~$45 (free lite version exists). GUI: compact, creative, distinctive Auburn look. Stand-out: **musical-first framing**, LFO movement, "depth" control that isn't just reverb. Closest philosophical cousin to Binaural Jungle's "instrument-first" stance — but it's still an effect.

URL: https://www.auburnsounds.com/products/Panagement.html

### 2.7 Flux:: IRCAM HEar / IRCAM Tools Spat Revolution

The pro-audio / film / concert-hall tier. Built on decades of IRCAM research (Spat~ in Max/MSP).

- **IRCAM HEar** — individual-subject HRTF binaural renderer for headphone monitoring. Proprietary HRTFs from IRCAM LISTEN database. ~$169.
- **Spat Revolution** — full pro spatial audio engine, binaural + ambisonics + VBAP + wavefront synthesis, used in film, installations, concerts. €500+ tiered licensing.

Formats: VST3/AU/AAX + standalone server. GUI: Spat Revolution is a separate app with send/return plugins — very pro-audio, not musician-friendly. Stand-out: academic HRTF database depth, pro workflow, rich room acoustics modelling.

URLs: https://www.flux.audio/project/ircam-hear/ | https://www.flux.audio/project/spat-revolution/

### 2.8 Sound Particles — Earth / Density

Sound Particles the company ships a *standalone* particle-based spatial sound generator (hero product). Their plugin line includes:

- **Earth** — 3D distance/doppler/air-absorption effect. Free.
- **Density** — granular spatial multiplier effect (turns a mono source into a cloud of spatialised voices).
- **Air**, **Doppler**, **Panner** — smaller focused effects.

Proprietary spatial model, HRTF-based binaural output. Effect plugins. VST3/AU/AAX. Free–$99. GUI: modern, clean, cinematic. Stand-out: **Density** is the most instrument-adjacent product on the market — granular spatial voice cloud — but it still needs a sound source feeding it.

URL: https://soundparticles.com/products/plugins

### 2.9 SSA Plugins — SSA Binauralizer

Part of the New Audio Technology ecosystem (Spatial Audio Designer). Ambisonic-to-binaural decoder. Supports 1st–3rd order, multiple HRTF sets, head-tracking via OSC.

Effect plugin. VST3/AU/AAX. ~€99. GUI: technical, function-first. DAW integration: strong in Nuendo / post-production houses. Stand-out: tight integration with their larger Spatial Audio Designer post workflow.

URL: https://www.ssa-plugins.com/

### 2.10 L-Acoustics — L-ISA Studio (reference / pro tier)

Not a direct competitor — this is pro-tier live-sound spatial audio software (L-Acoustics hardware ecosystem). Binaural monitoring is one output mode. License gated to L-Acoustics partnerships and large installations. Included here as the ceiling reference for what "pro-tier spatial" looks like.

Effect/routing platform. Proprietary. GUI: professional console-style. Stand-out: designed for venues, not studios — worth watching for UI conventions but not directly competitive.

URL: https://l-isa.l-acoustics.com/

### 2.11 Google Resonance Audio (SDK)

Not a plugin. Open-source (Apache 2.0) spatial audio SDK for Unity, Unreal, FMOD, Wwise, web audio. 3rd-order ambisonics, HRTF-based binaural decoding. Google-sponsored HRTF dataset. Google effectively handed maintenance to the community in 2019 but the SDK is still widely used in VR/AR and web experiences.

Stand-out: **free, permissive licence, production-quality HRTF decode code** — Binaural Jungle's DSP team can learn from the reference implementation without GPL contamination.

URL: https://resonance-audio.github.io/resonance-audio/

### 2.12 Steam Audio / Phonon (Valve)

Not a plugin. Free (proprietary but source-available, Apache 2.0 on core). Spatial audio SDK for Unity, Unreal, FMOD, Wwise. Focus on real-time geometry-aware occlusion, reflections, reverb + HRTF binaural. Used heavily in VR game titles.

Stand-out: real-time ray-traced early reflections; best-in-class game-engine spatial reverb. Reference material for room modelling, not a consumer plugin.

URL: https://valvesoftware.github.io/steam-audio/

---

## 3. Summary Comparison Table

| Product | Binaural Tech | HRTF Library | Plugin Type | Formats | Price | Licence | GUI Style | Stand-out |
|---|---|---|---|---|---|---|---|---|
| dearVR Pro | Direct HRTF + room | Proprietary | Effect (spatialiser) | VST3/AU/AAX | $349 | Commercial | 3D room, polished | 46-room preset library |
| dearVR Monitor | Direct HRTF | Proprietary | Effect (monitor) | VST3/AU/AAX | $199 | Commercial | Single panel | Head-tracking; Atmos monitoring |
| dearVR Music | Direct HRTF + room | Proprietary | Effect (spatialiser) | VST3/AU/AAX | $149 | Commercial | Simplified 3D | Producer-friendly subset |
| Waves Nx | Direct HRTF | Proprietary (Waves) | Effect (monitor) | VST/AU/AAX | $29–99 | Commercial | Minimal | Webcam head-tracking |
| Abbey Road Studio 3 | Direct HRTF + room IR | Proprietary | Effect (monitor) | VST/AU/AAX | $29–99 | Commercial | Room photo | Real-room emulation |
| IEM Plug-in Suite | HOA + MagLS decode | KU100 + alts | Effect (modular toolkit) | VST3 + SA | Free | **GPLv3** | Engineer/utilitarian | Academic reference; modular |
| SPARTA binauraliser | Direct HRTF panner | Multi (SOFA import) | Effect (spatialiser) | VST3/AU/SA | Free | GPLv3 | Engineer/utilitarian | **SOFA import**; near-field version |
| SPARTA ambiBIN | HOA + MagLS / LS decode | KU100 + SOFA | Effect (decoder) | VST3/AU/SA | Free | GPLv3 | Engineer/utilitarian | Best-in-class decoder options |
| Noise Makers Ambi Head HD | HOA decode | KU100 + alts | Effect (decoder) | VST3/AU/AAX | $69 | Commercial | Friendly sphere UI | Musician-friendly |
| Noise Makers Binauralizer Studio | Direct HRTF (16 src) | KU100 + alts | Effect (panner) | VST3/AU/AAX | $79 | Commercial | Friendly sphere UI | Creative workflow |
| Panagement 2 | Direct HRTF + depth | Proprietary | Effect (creative pan) | VST/AU/LV2/AAX | $45 | Commercial | Compact creative | Musical-first, LFO motion |
| Flux IRCAM HEar | Direct HRTF (monitor) | IRCAM LISTEN | Effect (monitor) | VST3/AU/AAX | $169 | Commercial | Pro-audio | Individual HRTFs |
| Spat Revolution | HOA + VBAP + binaural | IRCAM + alts | Effect + server | VST3/AU/AAX + SA | €500+ | Commercial | Pro multi-window | Film/concert pedigree |
| Sound Particles Earth | Distance/HRTF | Proprietary | Effect (distance) | VST3/AU/AAX | Free | Commercial | Modern cinematic | Free; air absorption |
| Sound Particles Density | Granular spatialiser | Proprietary | Effect (granular) | VST3/AU/AAX | $99 | Commercial | Modern cinematic | **Closest to "instrument"** |
| SSA Binauralizer | HOA decode | Multi | Effect (decoder) | VST3/AU/AAX | €99 | Commercial | Technical | Nuendo post-pro home |
| L-Acoustics L-ISA | Hybrid spatial + binaural | Proprietary | Platform | Proprietary | Partner-tier | Commercial | Console-style | Pro live/install tier |
| Google Resonance | HOA + HRTF | Google dataset | **SDK** (not plugin) | Unity/Unreal/Web | Free | Apache 2.0 | N/A | Reference implementation |
| Steam Audio / Phonon | HOA + HRTF + geom | Multi (incl. SADIE) | **SDK** (not plugin) | Unity/Unreal/FMOD | Free | Apache 2.0 | N/A | Geometry-aware reflections |
| **Binaural Jungle v1 (target)** | **HOA 3rd order + MagLS decode** | **SOFA (KU100 default)** | **Instrument (synth/sampler)** | **VST3/AU** | **TBD (~$79)** | **Commercial** | **Musical / genre-themed** | **Genre-native instrument** |

---

## 4. Positioning Gap — Why No Native Binaural Synth-First Instrument Exists

Every shipping binaural plugin in the table above is either an **effect that processes audio fed into it**, a **monitoring utility** applied at the master bus, or an **SDK** that requires a game engine. None are **sound-generating instruments with binaural rendering as a native first-class part of the voice architecture.**

The closest adjacent products tell the story:

1. **Sound Particles Density** is an effect that multiplies voices — it's a spatialiser *of* a source, not a source.
2. **Panagement 2** is creative and musical but still fundamentally a pan/depth effect over incoming audio.
3. **IEM + SPARTA** give you everything needed to *build* a binaural instrument, but expect you to wire a synth → encoder → decoder chain yourself per-track. No producer will do this during a creative flow session.
4. **dearVR** is polished and fast, but its mental model is "put this sound there in this room" — post-production spatialisation, not instrument voice design.

### The gap, stated cleanly

**There is no VST/AU that answers "load this pad, press a key, hear a jungle-era binaural pad."** Everything in the market answers either "process this audio into binaural" or "simulate my mixing room over headphones."

### Why the gap persists (my take)

- **Academic vs. musical split.** The DSP expertise sits in research labs (IEM, Aalto, IRCAM) who build toolkits for peers, not instruments for producers. The musical-instrument expertise sits at Native Instruments / Spectrasonics / u-he / Arturia who don't work in ambisonics because the mastering chain for stereo music doesn't need it.
- **Target-format confusion.** Ambisonics grew out of VR/AR and immersive film — markets that reward *spatialising a static source*. Music production hasn't had a commercial reason to care, because Dolby Atmos for Music still mixes from stereo/surround stems, not from ambisonically-rendered instruments.
- **No genre hook.** A "binaural synth" with no aesthetic POV competes against Serum and Omnisphere on synthesis, where it will lose. It needs to win on *what it uniquely enables*.

### What makes Binaural Jungle defensible

1. **Instrument-first architecture.** Each voice renders directly into a B-format bus. HRTF decode happens at the voice sum, not as an afterthought. This matters because modulation of spatial position per-voice becomes musical (key-tracked width, velocity-tracked depth, LFO-tracked orbit) rather than applied globally.
2. **Genre-curated sound design.** 1993–96 Good Looking / Metalheadz / Reinforced pads and strings are a specific, recognisable palette — Rhodes-through-delay atmospheres, Korg M1 string layers, S950-era sampled orchestra hits, Juno pad chords processed through Lexicon verbs. This is a defensible content moat that no DSP-first competitor can replicate by tweaking knobs.
3. **Integrated HOA pipeline, hidden complexity.** The 3rd-order HOA + MagLS decode runs under the hood. The user sees "width", "depth", "orbit speed" — musical controls. IEM/SPARTA expose the pipeline; we hide it. dearVR hides it but doesn't generate sound. We do both.
4. **Jungle-era DSP character.** Binaural alone isn't differentiated — it's binaural *plus* bit-reduction, Akai S950 aliasing, 48k→22k→48k resampling, Lexicon-style early-reflection tails modelled on LTJ Bukem's specific reverbs. Sonic identity over spatial purity.
5. **Series positioning.** "Binaural Jungle v1 = pads/strings; v2 = breaks & atmospheres; v3 = bass." A family of instruments is a larger market position than a single tool.

### Risks to watch

- **Dear Reality could trivially add a synth engine.** They won't, because it's off-strategy for their pro-audio customer. But it's possible.
- **Spectrasonics Omnisphere** is already the "atmospheric pad with deep modulation" king. Binaural Jungle must be different-in-kind, not "Omnisphere but binaural" — the jungle-era genre curation is how we avoid that head-to-head.
- **Open-source risk.** A determined hobbyist could glue IEM + JUCE synth + SOFA HRTF into a free competitor. Mitigation: content + polish + genre fidelity are the moat, not the DSP.

---

## 5. Lessons to Steal

Concrete ideas worth borrowing from each competitor:

### DSP / pipeline
- **SOFA HRTF import** (from SPARTA) — even if default is KU100, letting users drop in their own SOFA file at minimal effort is a power-user delight and a credibility signal for audiophile customers.
- **MagLS decoder** (IEM/SPARTA) — already our plan; double down on it with optional time-alignment flag, which SPARTA exposes.
- **Near-field HRTF interpolation** (SPARTA binauraliser NF) — distance-dependent HRTF changes are what make "closeness" feel real versus just loud. Worth the DSP cost for pads that swell toward the listener.
- **Diffuse-field covariance constraint** (SPARTA ambiBIN) — avoids timbral colouration that vanilla MagLS decode can introduce. Worth validating in listening tests.

### UX / product
- **Room preset library as headline feature** (dearVR Pro) — users buy spatial plugins because they save time picking a space. Binaural Jungle should ship 20–30 carefully-named pad spaces: *"Peshay's Basement", "Good Looking Records 2AM", "Speaker Cabinet Haze"*. Preset curation is marketing.
- **Single-panel simplicity** (Panagement 2, Noise Makers) — resist the temptation to expose HOA order, decoder type, HRTF selector on the main panel. Put it in an "Advanced" flyout. First-run user sees maybe 6 macros: tone, width, depth, motion, age, space.
- **Webcam/phone head-tracking** (Waves Nx) — a free companion app for phones over OSC could differentiate Binaural Jungle for live jungle DJ sets and solo producers mixing on headphones.
- **Creative LFO motion** (Panagement 2) — "orbit speed" and "tilt" LFOs routable to spatial position per-voice. Make movement musical, key-synced, tempo-synced.
- **Free lite version** (Auburn Sounds Panagement, Sound Particles Earth) — a freely-redistributable 1-preset lite version ("Binaural Jungle Free: one M1 pad, one room") drives trial and gets it onto YouTube tutorials.

### Content / positioning
- **Genre-specific naming** (nobody does this well in spatial audio) — steal from synth-preset culture (u-he, Arturia, Output). Preset names matter as much as the sound.
- **Partner with a name** — Waves did this with Abbey Road Studio 3. A collaboration preset pack from Bukem, Photek, or 4hero would land harder than any marketing spend.
- **Series roadmap as marketing** — announce v1, v2, v3 up front. People buy into universes (Arturia V Collection, Output Arcade), not just products.

### Licensing / reference
- **Use Resonance Audio / Steam Audio as DSP reference** under Apache 2.0 rather than IEM/SPARTA under GPL — protects proprietary commercial shipping. Academic validation can come from reading IEM/SPARTA papers, not linking their code.

---

## 6. One-line Market Statement

> Binaural Jungle is the **first binaural-native synth instrument** with **genre-curated sound design** from the 1993–96 jungle/atmospheric DnB era. Every competitor is either a spatialiser effect or a DSP toolkit — nobody ships an instrument where binaural rendering is baked into the voice architecture alongside era-accurate pads, strings, and atmospheres.

---

## 7. References / URLs

- Dear Reality: https://www.dear-reality.com/
- Waves: https://www.waves.com/
- IEM Plug-in Suite: https://plugins.iem.at/
- SPARTA Suite: https://leomccormack.github.io/sparta-site/
- Noise Makers: https://www.noisemakers.fr/
- Auburn Sounds Panagement: https://www.auburnsounds.com/products/Panagement.html
- Flux / IRCAM: https://www.flux.audio/
- Sound Particles: https://soundparticles.com/
- SSA Plugins: https://www.ssa-plugins.com/
- L-Acoustics L-ISA: https://l-isa.l-acoustics.com/
- Google Resonance Audio: https://resonance-audio.github.io/resonance-audio/
- Steam Audio / Phonon: https://valvesoftware.github.io/steam-audio/
- SOFA HRTF format spec: https://www.sofaconventions.org/
