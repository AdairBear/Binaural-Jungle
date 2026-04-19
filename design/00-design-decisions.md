# 00 — Design Decisions (Locked)

This document records decisions that are locked for v1. Each entry lists the options considered, the chosen path, and the rationale. Open follow-ups are at the bottom.

---

## 1. Series concept

**Decision:** Binaural Jungle is a **series** of instruments, each covering one sonic element of 1993–96 atmospheric jungle / drum & bass. v1 ships one instrument — a binaural pad/string synth.

**Rationale:** A focused instrument is shippable inside one build cycle and gives the series room to breathe (future releases: bass, stabs, breaks, vocal textures). Each instrument is priced standalone and bundle-able later.

---

## 2. Sound engine

**Options considered:**
- Synthesis only
- Granular only
- Sample playback (Akai-style)
- Synthesis + granular hybrid
- All three in v1

**Decision:** **Synthesis + granular hybrid in v1.** Sample-playback layer is architected into the voice system (shared allocator, envelopes, modulation matrix) but the sampler engine itself ships in v1.1.

**Rationale:** Synth + granular covers 90% of the era's pad/string vocabulary — Juno-like detuned pads, Rhodes layers, timestretched atmos, reversed string beds. Adding a sample playback engine to v1 triples DSP scope and drags in a factory-sample licensing problem. Deferring to v1.1 keeps the shipping schedule honest without closing the door.

**Follow-up:** v1.1 sample engine will use the existing voice allocator; only the sound-generation block differs.

---

## 3. Binaural rendering pipeline

**Options considered:**
- Direct per-voice HRTF convolution
- HOA 3rd-order + MagLS HRTF decode
- External binaural library (Steam Audio / SPARTA)
- Panning + binaural reverb only

**Decision:** **Per-voice encode to 3rd-order Ambisonic (16-channel) bus → HOA early reflections → HOA diffuse reverb → single MagLS HRTF decode to stereo.**

**Rationale:**
- HOA decouples voice count from HRTF convolution cost. 16 voices and 1 voice both pay the same single MagLS decode at the end — a big win for polyphonic pads.
- Early reflections and reverb in the HOA domain spatialise correctly without per-tap HRTF convolution.
- MagLS (magnitude least-squares) is current best-practice for HOA→binaural decoding and is well-documented in the literature (Schörkhuber et al 2018, Zotter & Frank).
- Direct per-voice HRTF is simpler but scales linearly with polyphony and does not spatialise reverb cleanly.
- Third-party libs (Steam Audio, SPARTA) introduce dependency and licensing risk (SPARTA is GPL — incompatible with commercial binary distribution).

---

## 4. HRTF assets

**Decision:** Ship with **MIT KEMAR** and **ARI** HRTF datasets included. Support user-imported **SOFA** files (AES69-2015) via **libmysofa** (BSD 3-Clause).

**Rationale:** KEMAR is the historical default and performs well for generic listeners. ARI offers higher spatial resolution and a choice of measurement subjects. libmysofa covers everything else the listener might bring. All three are licence-compatible with commercial distribution.

---

## 5. Plugin framework

**Options considered:**
- JUCE 8 (commercial tiers) + clap-juce-extensions
- iPlug2 (BSD/ISC)
- nih-plug (Rust, ISC)
- DPF (ISC)

**Decision:** **JUCE 8 + C++20 + clap-juce-extensions.**

**Rationale:** JUCE's DSP module, `juce::dsp::Convolution` (uniformly partitioned), MPE instrument support, mature cross-format build system, and industry-wide DAW compatibility outweigh its commercial-licence cost. `clap-juce-extensions` (Timo Urbanek, MIT) provides CLAP support without leaving the JUCE ecosystem. Alternatives (iPlug2, nih-plug, DPF) are viable but each has gaps in GUI maturity, AU support, or community size.

**Licence implication:** We adopt JUCE's GPLv3 free tier initially; we'll move to a JUCE Indie licence (currently ~$40/mo or ~$800/yr) once binaries ship commercially. See decision 7.

---

## 6. Plugin formats — v1

**Decision:** **VST3 only in v1.** AU, CLAP, and Standalone are next in line. AAX is deferred pending Avid Developer enrolment and PACE iLok signing.

**Rationale:** VST3 is the broadest-reach single format — it covers Cubase, Studio One, Reaper, FL Studio, Ableton Live, Bitwig. Shipping one format first lets us harden the core pipeline before dealing with format-specific quirks. AU is the obvious next target on macOS and costs nothing extra in JUCE's build system, so it will likely land in a v1.0.x point release.

---

## 7. Licensing

**Decision:** **Dual-licence.**
- **Source code:** GPLv3 (public GitHub repo).
- **Shipping binaries:** commercial licence sold directly.

**Rationale:** This is the u-he / Surge-XT model. It allows us to use JUCE's free GPLv3 tier while we ship, keeps the source open for community review and contribution, and preserves commercial revenue on binaries. We retain copyright to all first-party code. Third-party libraries must be GPL-compatible (MIT, BSD, Apache, LGPL are all fine; AGPL, commercial-only libs are not).

**Before first paid release:** upgrade JUCE licence to Indie if revenue justifies it, or remain on GPLv3 tier if we stay below the revenue threshold.

---

## 8. Scope lock for v1

**Decision:** **Polished, shippable, stereo binaural out only. No Atmos, no 7.1.4, no object-based bed.**

**Rationale:** Headphone binaural is where the jungle-listening experience lives anyway (home producers, late-night studio sessions). Atmos renders add significant mixing-engine complexity and target a different listening context. Revisit once v1.1 ships.

---

## Open questions for the next Q&A round

These follow-ups are not blocking the v1 architecture but will need answers before launch:

- **GUI tech.** JUCE native Components + custom `LookAndFeel`? JUCE + melatonin_blur for modern shadows? Foleys GuiMagic (LGPL) for rapid iteration? Something bespoke on top of JUCE's `OpenGLContext`?
- **Granular source material.** Do we ship factory granular samples, or is granular only applied to user-loaded samples in v1? If we ship samples — licensed jazz-fusion fragments, original field recordings, or synthesised tone-beds?
- **Preset partner.** Self-program factory presets, or collaborate with a known jungle-era producer or modern atmospheric DnB artist for authenticity?
- **Product name.** "Binaural Jungle" is the series name. The v1 instrument needs its own name (e.g. "Horizons", "Bukem's Room", "Kemet", "Modus").
- **Launch platforms.** Direct sale via our own Stripe/Paddle storefront? Plugin Boutique? Splice Plugins? Reverb LP? Some combination?
- **Pricing tier.** Pro tier ($149–$249), mid ($79–$129), or intro ($39–$59)? What's the competitive band given the unique positioning?
