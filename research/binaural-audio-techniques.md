# Binaural Audio Rendering Techniques — Technical Primer

**Scope:** Reference document informing the DSP implementation of Binaural Jungle, a binaural pad/string VST instrument built on JUCE 8 + C++20. The signal chain is:

```
per-voice mono source
    -> 3rd-order Ambisonic encoder  (16 channels)
    -> HOA early-reflections network (16 ch in / 16 ch out)
    -> HOA diffuse reverb tail       (16 ch in / 16 ch out)
    -> MagLS HRTF binaural decoder   (16 ch in / 2 ch stereo out)
```

The target is one decode stage per plugin instance, not per voice. Polyphony is cheap because voice count does not scale convolution cost — it only scales encoding cost, which is a pair of multiplies per (voice, channel) pair.

---

## 1. HRTF Fundamentals

The head-related transfer function (HRTF) is the frequency-dependent filtering a sound undergoes travelling from a point in space to each eardrum. Its time-domain twin is the head-related impulse response (HRIR). HRTF captures the following cues, which together let the brain place a sound in 3D.

### Interaural Time Difference (ITD)

Sound arriving from the side reaches one ear before the other. The maximum ITD is roughly 660 microseconds (head radius / speed of sound). ITD is the dominant cue for azimuth below about 1.5 kHz, where the wavelength is longer than the head diameter and phase is unambiguous. Above that, phase wraps and ITD becomes ambiguous; the brain then leans on ILD and envelope cues instead. ITD is robust across listeners — head size varies less than pinna geometry.

### Interaural Level Difference (ILD)

The head acoustically shadows the contralateral ear. The shadow effect is strongly frequency dependent: almost negligible below 500 Hz (wavelengths wrap around the head), severe above 2 kHz. ILD provides azimuth information in the regions where ITD breaks down. The duplex theory (Rayleigh, 1907) is this division of labour between ITD and ILD.

### Spectral Cues (Pinna Filtering)

The outer ear is a tiny reflector and resonator. Its shape imposes a direction-dependent notch-and-peak pattern on incoming sound, mostly in the 4–16 kHz range. These spectral cues are the primary mechanism for:

- **Elevation** discrimination (ITD/ILD barely change with elevation)
- **Front/back disambiguation** (ITD/ILD are identical for mirror-image front/back positions on the lateral plane)
- **Externalisation** (perceiving the source as outside the head rather than inside it)

### Front-Back Confusion and the Cone of Confusion

For any given ITD/ILD pair, there is an entire locus of points in space that produces those same values — a cone of revolution around the interaural axis. Without spectral cues the auditory system cannot resolve positions on this cone, and listeners typically hear a sound as either in front of them or behind them roughly 50/50 on generic HRTFs. Good spectral fidelity (preserved pinna notches) is what breaks this symmetry.

### Why Individualised HRTFs Matter — and Why We Ship Generic

Pinna geometry is as personal as a fingerprint. Listening through someone else's HRTF is like wearing someone else's glasses: usable but blurry. Individualised HRTFs dramatically reduce front-back confusion, improve elevation accuracy, and boost externalisation.

They are impractical for a VST product because acquiring them requires either:

1. A dedicated anechoic measurement session with microphones in the listener's ear canals, or
2. High-resolution 3D scans of the head and torso plus numerical HRTF synthesis (BEM / FM-BEM).

Neither scales. The pragmatic compromise — and what Binaural Jungle adopts — is to ship well-measured generic HRTF sets (MIT KEMAR, ARI) and let the user load their own SOFA file if they have one. This is the standard path across IEM, SPARTA, Resonance Audio, and Steam Audio.

---

## 2. SOFA File Format (AES69-2015)

SOFA (Spatially Oriented Format for Acoustics) is the AES-standardised container for HRIRs, BRIRs, directional room impulse responses, and related spatial audio data. It sits on top of netCDF-4 / HDF5.

### Structure

A SOFA file is a netCDF-4 file with a fixed set of dimensions and conventions. Core dimensions used by HRIR data:

- `M` — number of measurements (source positions)
- `R` — number of receivers (2 for binaural: left, right)
- `E` — number of emitters (1 for a point source measurement)
- `N` — number of samples per impulse response
- `I` — always 1, used for scalar variables

Core variables:

- `Data.IR` — the actual impulse responses, shape `[M, R, N]`
- `Data.SamplingRate` — Hz
- `SourcePosition` — shape `[M, 3]`, in spherical or cartesian coordinates
- `ListenerPosition`, `ListenerView`, `ListenerUp`, `ReceiverPosition` — listener geometry
- Global metadata: `GLOBAL:Conventions`, `GLOBAL:SOFAConventions`, `GLOBAL:DatabaseName`, `GLOBAL:License`, etc.

### Common Conventions

- **SimpleFreeFieldHRIR** — the workhorse convention for anechoic HRIR measurements. Used by almost every public HRTF dataset. Source positions in spherical coordinates (azimuth / elevation / radius), fixed listener at origin, two receivers.
- **SingleRoomDRIR** / **MultiSpeakerBRIR** — room impulse responses and binaural room responses. Relevant later if you want to import measured rooms as HOA early reflections.
- **GeneralFIR** — generic FIR container when none of the above fit.

For HRTF playback you only need to support SimpleFreeFieldHRIR. If a file declares any other convention, reject it or warn — mixing conventions up silently causes subtle but audible misplacement.

### Public Datasets Worth Shipping or Supporting

| Dataset | Subjects | Grid | Licence | Notes |
|---|---|---|---|---|
| **MIT KEMAR** | 1 (dummy head) | ~710 positions, 5 deg az steps, 10 deg el steps | Public domain / permissive | The original reference. Dry, anechoic, well documented. Ship this. |
| **ARI** (Austrian Academy / Acoustics Research Institute) | 200+ human subjects | ~1550 positions per subject | CC BY-SA-NC 4.0 (check per-subject; many are free for non-commercial and separately licensable) | Higher resolution than KEMAR. Pick 1–3 subjects with permissive terms for shipping; allow the rest as user-loadable. |
| **CIPIC** (UC Davis) | 45 subjects | 25 az x 50 el | Academic / non-commercial research | Heavy in the literature, but licence is awkward for a commercial plugin. User-load only. |
| **Club Fritz** | Multiple dummy heads (Neumann KU100, Brüel and Kjaer 4100, etc.) | Varies | Mostly permissive | Good for A/B testing dummies against each other. |
| **SADIE II** (York) | ~20 human subjects + KU100 + KEMAR | Dense (2114 positions) | CC BY | High-quality, densely sampled, explicit licence. Good candidate to ship as an alternative to ARI. |

Ship two sets by default (MIT KEMAR + one ARI subject), let users drop in anything else via the plugin UI.

### libmysofa C API

**libmysofa** (github.com/hoene/libmysofa) is the de facto embedded SOFA loader: pure C, ~5k LOC, BSD 3-Clause. It compiles cleanly into a JUCE plugin, handles netCDF-4 parsing internally (no external netCDF dependency), and exposes a small API centred on two helper layers.

Minimal usage for runtime HRIR lookup:

```c
int filterLen;
int err;
MYSOFA_EASY* mysofa = mysofa_open("/path/to/mit_kemar.sofa",
                                  sampleRate,
                                  &filterLen,
                                  &err);

float irLeft[filterLen];
float irRight[filterLen];
float delayLeft, delayRight;
float x = 1.0f, y = 0.0f, z = 0.0f;   // target direction, cartesian

mysofa_getfilter_float(mysofa, x, y, z,
                       irLeft, irRight,
                       &delayLeft, &delayRight);

// ... use IRs in your convolver ...

mysofa_close(mysofa);
```

The `_easy` layer handles: resampling to your target rate, normalising the coordinate convention to cartesian, building a neighbourhood lookup structure, and optionally separating broadband delay from the FIR (minimum-phase / ITD extraction). For this project we will **not** use per-direction lookup at runtime — we precompute a dense set of HRIRs for the MagLS decoder. libmysofa is only used at plugin init and when the user loads a new file.

### Licensing

BSD 3-Clause — fully compatible with closed-source commercial distribution. You must reproduce the copyright notice in plugin documentation/about box. No other restrictions.

---

## 3. Ambisonic Encoding

Ambisonics represents a sound field at a point as a sum of spherical harmonic basis functions. You can think of it as "Fourier series on the sphere": higher order = finer angular resolution.

### Order, Channel Count, and Why Third Order

For order `N`, the channel count is `(N+1)^2`:

| Order | Channels | Approx. spatial resolution |
|---|---|---|
| 0 (omni) | 1 | none |
| 1 (FOA) | 4 | ~90 deg blur |
| 2 | 9 | ~50 deg |
| 3 (TOA) | 16 | ~30 deg |
| 5 | 36 | ~18 deg |
| 7 | 64 | ~13 deg |

Third-order Ambisonic (TOA) — 16 channels — is the sweet spot for binaural rendering:

- It produces convincingly localised sources with a modern decoder (MagLS, AllRAD+).
- CPU cost of a single 16-channel convolution decode is affordable on consumer hardware.
- Higher orders yield diminishing returns once a good HRTF decoder is in place; gains above order 5 are inaudible for most content, and the cost of the decode convolution scales linearly with channel count.
- Most publicly available HOA tooling targets third order by default.

### ACN Channel Ordering

ACN (Ambisonic Channel Number) is the single-index ordering that flattens the two-index `(l, m)` spherical harmonic basis into a 1D array:

```
ACN(l, m) = l*l + l + m
```

For order 3 you get 16 channels, ACN 0..15:

| ACN | l | m | Name |
|---|---|---|---|
| 0 | 0 | 0 | W |
| 1 | 1 | -1 | Y |
| 2 | 1 | 0 | Z |
| 3 | 1 | 1 | X |
| 4..8 | 2 | -2..2 | second-order |
| 9..15 | 3 | -3..3 | third-order |

This is the AmbiX convention. The other ordering you will still see in old papers is FuMa, which only extends cleanly to third order and uses different channel weights and channel numbering. Modern tools (IEM, SPARTA, Resonance, Steam Audio) default to ACN + SN3D. Do the same.

### SN3D vs N3D Normalisation

Normalisation = the per-channel gain factor applied to the unit spherical harmonics.

- **N3D** (fully normalised) — orthonormal on the sphere. Channel RMS scales with order.
- **SN3D** (Schmidt semi-normalised) — N3D divided by `sqrt(2l+1)`. Keeps higher-order channels at similar RMS to the W channel, which is friendlier for metering, fixed-point processing, and storage.

AmbiX = **ACN + SN3D**. This is what every modern decoder expects. Internally inside a DSP pipeline some tools prefer N3D (orthonormality simplifies some algebra), but the bus between plugins should be AmbiX. Binaural Jungle produces and consumes AmbiX throughout. Convert if a third-party decoder requires N3D (scale factors are fixed per channel).

### Encoding a Point Source

To place a mono signal at direction `(azimuth, elevation)`, compute the real spherical harmonic coefficients `Y_l^m(az, el)` at that direction and multiply the input sample by each coefficient to fill the 16-channel bus.

```cpp
// Generate TOA AmbiX (ACN + SN3D) encoding coefficients.
// azimuth: radians, CCW from front in horizontal plane.
// elevation: radians, up positive.
// out: float[16]

void encodeToaAmbix(float azimuth, float elevation, float* out)
{
    const float sinA = std::sin(azimuth), cosA = std::cos(azimuth);
    const float sinE = std::sin(elevation), cosE = std::cos(elevation);
    const float sin2A = std::sin(2 * azimuth), cos2A = std::cos(2 * azimuth);
    const float sin3A = std::sin(3 * azimuth), cos3A = std::cos(3 * azimuth);

    // l = 0
    out[0]  = 1.0f;                                              // W

    // l = 1  (SN3D)
    out[1]  = sinA * cosE;                                       // Y  (m = -1)
    out[2]  = sinE;                                              // Z  (m =  0)
    out[3]  = cosA * cosE;                                       // X  (m = +1)

    // l = 2  (SN3D, cosE^2 and sinE terms)
    const float cosE2 = cosE * cosE;
    out[4]  = (std::sqrt(3.0f) / 2.0f) * sin2A * cosE2;          // V
    out[5]  = (std::sqrt(3.0f) / 2.0f) * sinA * std::sin(2 * elevation); // T
    out[6]  = 0.5f * (3.0f * sinE * sinE - 1.0f);                // R
    out[7]  = (std::sqrt(3.0f) / 2.0f) * cosA * std::sin(2 * elevation); // S
    out[8]  = (std::sqrt(3.0f) / 2.0f) * cos2A * cosE2;          // U

    // l = 3  (third-order channels 9..15) — see full formulas below
    const float cosE3 = cosE2 * cosE;
    const float sinE2 = sinE * sinE;
    out[9]  = std::sqrt(5.0f / 8.0f)  * sin3A * cosE3;
    out[10] = std::sqrt(15.0f)        * sin2A * sinE * cosE2 * 0.5f;
    out[11] = std::sqrt(3.0f / 8.0f)  * sinA  * cosE * (5.0f * sinE2 - 1.0f);
    out[12] = 0.5f * sinE * (5.0f * sinE2 - 3.0f);
    out[13] = std::sqrt(3.0f / 8.0f)  * cosA  * cosE * (5.0f * sinE2 - 1.0f);
    out[14] = std::sqrt(15.0f)        * cos2A * sinE * cosE2 * 0.5f;
    out[15] = std::sqrt(5.0f / 8.0f)  * cos3A * cosE3;
}
```

Double-check the exact SN3D constants against a reference implementation (IEM's `AllRADecoder` or SPARTA's `saf_utility_sh.c`) before trusting — there are multiple sign and axis conventions floating around and off-by-factor-of-sqrt-3 bugs are the most common mistake in this code.

Per-voice encode cost: 16 multiplies + 16 add-into-bus, once per sample. If azimuth/elevation are updated at control rate (every 64 samples), the trig ops amortise away.

### The Encoder -> Bus Model

Every voice writes into the same 16-channel shared HOA bus. This is the payoff of the architecture: downstream cost (reflections, reverb, HRTF decode) is **independent of voice count**. 64 simultaneous pad voices cost the same to decode as 1.

---

## 4. HOA Decoding to Binaural

An HOA decoder maps the 16-channel ambisonic signal to a set of output channels. For a loudspeaker array, the output channels are speaker feeds. For binaural, the output channels are left and right ear, and the decoder IS a pair of filters per ambisonic channel.

A binaural HOA decoder of order `N` is a matrix `D` of filters, shape `(2, (N+1)^2)`. Each element `D[ear][acn]` is an FIR filter. To render:

```
leftOut  = sum over acn of (ambiBus[acn] conv D[0][acn])
rightOut = sum over acn of (ambiBus[acn] conv D[1][acn])
```

For third order, that is 32 FIR filters. Each one typically 128–512 taps. This is the work partitioned convolution exists to make efficient.

The question is how to design the filters in `D`. Three main approaches.

### AllRAD (All-Round Ambisonic Decoding)

Zotter and Frank, 2012. Pick a virtual loudspeaker layout (typically a t-design like the Lebedev grid, or a near-uniform spherical distribution), design an ambisonic-to-speaker-feed matrix using pseudo-inverse of the spherical harmonic matrix, then convolve each virtual speaker's feed with that direction's HRIR and sum. Result: a binaural HOA decoder.

Strengths:
- Simple to derive, physically intuitive.
- Works well up to ~2 kHz where the linear sound-field reconstruction assumption holds.

Weaknesses:
- Above about 2 kHz the sound field cannot be reconstructed accurately at the ears with only 16 channels — wavelengths are shorter than the head and comb-filter artefacts appear.
- Produces audible colouration and slightly blurred imaging at high frequencies.

### EPAD (Energy-Preserving Ambisonic Decoding)

Zotter, Frank, Pomberger, 2012. Similar setup to AllRAD but constrains the decoder to preserve total energy across directions, trading off reconstruction accuracy for flatter timbre. Better than AllRAD for diffuse content, still limited by the virtual-loudspeaker paradigm at high frequencies.

### MagLS — Magnitude Least Squares (Current Best Practice)

Schörkhuber, Zotter et al, 2018 / refined in a 2022 paper. Recognises that at high frequencies **phase** matching across the ears is not perceptually meaningful and cannot be achieved with limited order anyway. What matters perceptually is:

- At low frequencies (below ~1.5 kHz): accurate phase (ITD) and magnitude.
- At high frequencies: accurate magnitude (spectral cues, ILD) across directions.

MagLS decodes exactly as classical least-squares up to some transition frequency (typically 1 kHz for third order), then above it solves a **magnitude-only** least-squares problem, leaving phase free to be optimised for smooth filters. A cross-fade across a small transition band glues the two regions together without audible discontinuity.

Result: visibly better externalisation, less colouration, and cleaner imaging than AllRAD/EPAD at the same order. It has become the de facto binaural HOA decoder over the last five years.

References to read before implementing:
- Zotter and Frank, *Ambisonics: A Practical 3D Audio Theory for Recording, Studio Production, Sound Reinforcement, and Virtual Reality* (Springer, 2019). Open access. Chapter 4 covers the decoder zoo; Chapter 8 is binaural.
- Schörkhuber, Zaunschirm, Höldrich, *Binaural Rendering of Ambisonic Signals via Magnitude Least Squares*, DAGA 2018. The canonical MagLS paper.
- Schörkhuber and Höldrich, *Magnitude-Least-Squares Optimisation with Smooth Filters for Binaural Ambisonic Rendering*, 2022. Adds regularisation and smoothness constraints; this is the version to target.

Reference implementations to lift from: SPARTA's `saf_hrir` and `saf_vbap` modules, and the `getBinauralAmbisonicDecoderMagLS` function in IEM's `BinauralDecoder`. Both are GPL-family — you cannot copy code verbatim into a closed-source plugin, but you can read them to validate a clean-room port.

### The Design Output

MagLS gives you a precomputed 32-filter set (for TOA). These are what the partitioned convolution engine runs every block. They only need to be regenerated when the user changes the HRTF set or the HRTF is resampled to match the host sample rate.

Design time: a few seconds in matlab/numpy. You can either do the solve at plugin init (using Eigen) or precompute offline per HRTF dataset and ship the resulting filters as a binary resource. Doing it at runtime per user HRTF is tractable but non-trivial — plan to support both.

---

## 5. Partitioned Convolution

Convolving a 16-channel signal with 32 filters of length 256–512 at block rate is not feasible with naive time-domain convolution. You need frequency-domain convolution, but a single huge FFT per block has unacceptable latency. The standard answer is **uniformly partitioned convolution** (Gardner, 1995).

### Gardner's Uniformly Partitioned Algorithm

Split the impulse response `h` of length `L` into `P = L / B` partitions of length `B`, where `B` is your audio block size. For each block of input `x`:

1. Compute FFT of the latest `2B` samples of input (the current block padded with the previous block — overlap-save), yielding `X_k`.
2. Store `X_k` in a circular buffer of the last `P` input spectra.
3. For each partition `p` in [0, P):
   - Multiply the corresponding HRIR partition spectrum `H_p` by the input spectrum `X_{k-p}`, accumulate in the frequency domain.
4. Inverse-FFT the accumulated sum, discard the first `B` samples (aliased region), keep the last `B` samples as output.

The latency is one block (`B` samples). Computational cost is roughly:

```
cost per block ~= 2 * FFT(2B) + P * complex-multiply-accumulate(B+1)
```

For `B = 64` and `L = 512` (8 partitions), each mono-in, mono-out convolution is a handful of microseconds on a modern CPU. Multiply by 32 filter instances for the binaural decoder = still well under 1 ms per block on any modern DAW host.

### Overlap-Save vs Overlap-Add

Both are valid; overlap-save is slightly cheaper because you avoid the add-accumulate in the time domain and discard instead. All the convolvers in the wild (JUCE, Steinberg HPF, IEM, SPARTA) use overlap-save internally.

### Reasonable Partition Sizes

For a binaural HOA plugin running inside a DAW, the host block size is fixed (usually 64, 128, 256, 512, or 1024). You want:

- `B` = host block size, so there is **no extra algorithmic latency** beyond the host buffer.
- Total IR length `L` = 256 for dry HRTF, up to several thousand samples when you fold early reflections into the same convolver.

If the host runs at B = 64 and L = 512, P = 8 partitions. Fine.

If the host runs at B = 1024 (cinematic mix contexts) and L = 512, you have a single partition. Also fine but you lose the elegance — it is just a standard block FFT convolution.

### Non-uniform partitioning

Gardner also proposed non-uniform partitioning: small partitions at the head of the IR (for low latency) and progressively larger partitions at the tail (for efficiency on long IRs). This is what convolution reverbs do. For a ~10-second reverb tail with low latency you cannot afford tiny partitions all the way through — you would blow CPU on the FFTs.

For this plugin we expect two convolvers:

- **Binaural decoder**: IR length ~256–512 samples, uniform partitioning = fine.
- **Early reflections** (if convolution-based): a few hundred ms, uniform partitioning is still OK at modest block sizes.

The diffuse reverb tail is better implemented as a feedback-delay-network (FDN) in the HOA domain, not as convolution — see Section 7.

### juce::dsp::Convolution

JUCE ships a drop-in uniformly partitioned convolver: `juce::dsp::Convolution`. Highlights:

- Single input channel in, single output channel out per instance. For an N-in M-out system you instantiate `N*M` convolvers (or use the multi-channel variant that routes channel-wise). For the HOA binaural decoder you want 32 instances (16 ambisonic channels x 2 ears).
- Internally uses FFTW/PFFFT — fast.
- `loadImpulseResponse()` takes a `juce::AudioBuffer<float>` or a file path. Call this once per HRTF update.
- Latency is `0` or `headSize` depending on how you construct it. Default is zero algorithmic latency beyond the block.
- Supports trim and normalise flags — **disable normalisation** for HRTF use, you want the measured amplitudes preserved.
- Thread-safe replacement via `loadImpulseResponse` from the message thread while audio thread runs.

Gotchas specific to this plugin:

- If you swap HRTF mid-audio the engine crossfades over a short window; this is fine for user action but you must not do it automatically on every parameter change.
- If the host changes block size, call `prepare()` again. JUCE handles internal repartitioning but it can briefly glitch.
- `juce::dsp::Convolution` does one IR per instance. For a 32-IR decoder you manage 32 instances yourself — or write a dedicated multi-channel convolver on top of `juce::dsp::FFT` if you want a tighter memory layout. Recommended: start with 32 JUCE instances, profile, then optimise if needed.

---

## 6. Externalisation

Play a binaural-encoded synth pad through headphones with a bare HRTF convolution and it will frequently sound **inside the listener's head** rather than out in the room. This is the single biggest "uncanny valley" problem in binaural rendering and the hardest part to get right. Solve it and the synth sells itself.

### Why Sources Fail to Externalise

Anechoic HRTF gives you direction but no distance, no room, no movement. The auditory system interprets the result as "something close and inside my head" because every acoustic cue it normally uses to externalise is missing or neutral:

- **No early reflections** — no room to place the sound in.
- **No reverberant tail** — no sense of enclosure.
- **HRTF mismatch** — generic HRTFs confuse the brain's expectations (the pinna it measured does not match yours), and the brain collapses the perceptual estimate onto a safe default: in-head.
- **Static source** — the brain uses small head movements to resolve front-back ambiguity. Without head tracking, even a perfect HRTF has no way to provide this cue.
- **Inconsistent distance cues** — if level is loud but there is no air absorption, no Doppler, no reverb, the signal is internally contradictory.

### What We Can Do in V1

1. **Early reflections in HOA**. A small shoebox-ish reflection pattern (6–20 reflections, 15–80 ms after the direct sound) placed in HOA space gives the brain a room to externalise into. This is a major externalisation lever. Can be done either as convolution with a measured/synthetic early-reflection impulse in HOA, or as a small delay network writing into ambisonic bus with per-reflection direction encoders.

2. **Diffuse reverb tail in HOA**. A late reverberation network fed by the HOA bus, outputting 16-channel diffuse reverb, then passing through the same MagLS decoder. Because the reverb lives in ambisonic space before decoding, it picks up full spatial diffuseness — there are no correlation artefacts at the ears. An HOA-domain FDN with ~16 delay lines, unitary scattering matrix, and frequency-dependent damping is the standard architecture (see SPARTA's `sparta_tvcompressor` and its reverbs, and Resonance Audio's `reverb_onset_compensator.cc` / `reverb_bank.cc`).

3. **Modest low-pass with distance**. Air absorption via a simple one-pole LPF (cutoff dropping with apparent distance) adds a cheap but convincing distance cue.

4. **Decorrelation of source width**. For pad content especially, widen each voice slightly (see Section 7). This alone adds noticeable externalisation by making the sound less "point-source-y".

### Out of Scope for V1 — But Worth Designing Around

- **Dynamic head tracking**. A tiny yaw movement immediately resolves front/back. When supported, live head tracking (via OSC, via IMU devices, via Apple's head-tracked AirPods) is the single strongest externalisation cue short of personalised HRTFs. Architect the plugin so the listener orientation is a live parameter that feeds the encoders' azimuth/elevation before they become ACN coefficients; swapping in a head-tracker input later is then a data-routing change, not a DSP rewrite.

- **HRTF personalisation**. For power users, let them drop their own SOFA file. The plugin already supports this via libmysofa; ensure MagLS decoder filter design can be re-run at runtime when a new HRTF is loaded. Offline HRTF personalisation (photo-based, etc.) is out of scope — but a "bring your own SOFA" button is not, and it is the single highest-value feature to expose.

---

## 7. Pad/String-Specific Gotchas

Binaural Jungle is not a generic binauraliser — it is a pad/string instrument. Several properties of sustained-release textural content drive specific decisions.

### Long Releases Meet Reverb: Comb Filtering

Pads sustain for seconds and releases can run 5+ seconds. When a sustained, slowly-modulating signal enters a reverb with regular modes, the tail reinforces and cancels partials of the source in a way that manifests as flangy, metallic colouration. This is worst when:

- Reverb modal density is low (small rooms, short FDN).
- Source spectrum is narrow and tonal (pad, string).
- Dry/wet ratio is high.

Mitigations:
- Use an HOA FDN with more delay lines (16 rather than 8). Higher modal density = less audible ringing.
- Frequency-dependent absorption: use broadband damping filters in each FDN feedback path that roll off high frequencies faster than low frequencies, so tonal content does not sit in a resonant cavity forever.
- Mild modulation of the delay line lengths (chorus-like, <=0.3% LFO depth) breaks up standing modes in the tail without smearing the direct sound (because modulation is only in the reverb network, not in the dry path).

### Diffuse Sources Benefit from Decorrelation

A point-source pad sounds wrong. Real strings, choirs, and ensembles are distributed and decorrelated — the left and right ears hear related but not identical signals. If you encode a pad as a single azimuth/elevation point, listeners describe it as "tight" or "small". Widen the source by encoding multiple slightly detuned/delayed/filtered copies at nearby directions.

Cheap trick: for each voice, encode the source at (az, el) **and** at (az +/- delta, el +/- delta) with short relative delays and a decorrelation filter. Three or four spatially decorrelated copies per voice at <=15 deg spread is usually enough to change "point source" into "diffuse texture".

Because the final bus is HOA, the decorrelation lives on the ambisonic bus and is preserved through reflections and reverb. This is a core reason why the HOA pipeline is a good fit for this instrument — decorrelated spatial spread is basically free.

### HOA Polyphony Is Cheap

Per voice cost:
- Synthesis (oscillators, envelopes, filters): bounded, typical few hundred multiplies per sample.
- HOA encode: 16 multiplies per sample per encoded copy. 3 copies for decorrelation = 48.

Per-bus cost (once, independent of voice count):
- HOA early reflections: a few hundred multiplies.
- HOA reverb FDN: ~16 delay reads + matrix multiply + damping filters.
- MagLS decode: 32 partitioned convolutions. This is the bulk of the work.

**Conclusion**: the expensive part of the signal chain is fixed cost. Doubling polyphony only doubles the cheap part. Compare to per-voice HRTF convolution, where doubling polyphony doubles the most expensive stage.

This is the **architectural reason to use HOA here**, not just a spatial-quality argument. Pads want many voices. The HOA pipeline makes many voices affordable. For a hypothetical mono lead synth with 1–2 voices, per-voice HRTF convolution would be a reasonable alternative; for a pad with 16+ voices it is not.

### Envelope Follower Latency

The binaural decoder adds 1 block of latency (maybe more if you use non-zero latency modes of JUCE's convolver). For a pad with soft, slow attacks this is inaudible. Keep an eye on it if you ever add a percussive mode.

### Modulation and Ambisonic Bus Update Rate

Azimuth/elevation per voice can drift over time (you will want this — a slowly moving pad voice is more engaging than a static one). Update the 16 SN3D coefficients at control rate (every 32–64 samples) with linear interpolation between updates. Updating per sample is unnecessary and burns CPU.

---

## 8. Open-Source Code to Study

None of these can be copied verbatim into a closed-source plugin without care (several are GPL/AGPL), but each is worth reading for architecture.

### IEM Plug-in Suite (AGPLv3, C++/JUCE)

https://plugins.iem.at/

The canonical JUCE ambisonic plugin pack from IEM Graz. Read:

- `StereoEncoder` and `MultiEncoder` — how to implement ACN/SN3D encoding cleanly in JUCE.
- `BinauralDecoder` — reference MagLS binaural decoder. Filter design in `decoder/BinauralDecoderMagLS.h`.
- `RoomEncoder` — early reflections in HOA.

Architecture note: uses JUCE's `juce::dsp` pipeline and `juce::dsp::Convolution`. Each plugin is a thin wrapper around a JUCE-native DSP graph. Licensing: AGPL means you cannot ship a closed plugin that links IEM code. Read for architecture only.

### SPARTA (GPLv3 wrappers around the Spatial Audio Framework, BSD-3 core)

https://github.com/leomccormack/SPARTA

SAF — the Spatial Audio Framework (also by Leo McCormack) — is BSD-3 and can be linked into commercial code. SPARTA is the suite of JUCE plugins on top of SAF and is GPL. Read:

- `saf_hrir/saf_hrir.c` — SOFA loading, HRIR preparation.
- `saf_hoa/saf_hoa.c` — all the main HOA decoders (MagLS, AllRAD, EPAD, MMD). This is the most complete HOA decoder reference library in C.
- `saf_utility_sh.c` — spherical harmonic utilities.
- `saf_sh/` — ambisonic encoding and rotation.

Architecture note: SAF is a pure-C library with JUCE wrappers. For a commercial plugin SAF is the right library to actually link against (BSD), not IEM. It also ships with its own SOFA loader and MagLS solver. Licensing: BSD-3. Compatible with commercial distribution. Strongly recommended as the core for this plugin unless you want to roll your own MagLS solve.

### Google Resonance Audio (Apache 2.0)

https://resonance-audio.github.io/resonance-audio/

Cross-platform SDK for spatial audio in games/VR. Third-order ambisonic throughout, uses its own precomputed binaural decoder filters (shipped as a header). Read:

- `resonance_audio/ambisonics/ambisonic_codec.h` — encoding and rotation.
- `resonance_audio/ambisonics/ambisonic_binaural_decoder.h` — decoding (classic virtual-loudspeaker AllRAD-ish, not MagLS).
- `resonance_audio/dsp/partitioned_fft_filter.cc` — a textbook-clean partitioned convolution implementation. Shorter and easier to read than JUCE's.

Architecture note: Apache 2.0 = compatible with commercial use. Resonance is not as state-of-the-art as SAF on the decoder side (pre-MagLS) but its partitioned convolution and ambisonic rotation code are exemplary.

### Steam Audio / Phonon (Apache 2.0)

https://github.com/ValveSoftware/steam-audio

Valve's game audio SDK. Geometry-aware, does ray tracing for reflections. Includes a third-order ambisonic binaural renderer with HRTF loading (SOFA compatible). Read:

- `core/src/core/ambisonics_binaural_effect.cpp` — binaural renderer architecture.
- `core/src/core/hrtf.cpp` and `core/src/core/hrtf_database.cpp` — HRTF handling.
- `core/src/core/reverb_effect.cpp` — how they handle reverb in an ambisonic pipeline.

Architecture note: Apache 2.0, liberal. Large codebase — expect to spend a day orienting. Valuable as a reference for a production-grade, shipping ambisonic binaural pipeline.

### Facebook/Meta Audio360 (MIT parts) and FB360 Spatial Workstation

Parts of the Facebook 360 audio toolkit were released under MIT. Read:

- Their decoder filter design tools (MagLS-adjacent for second-order).
- The 360 Spatial Workstation — useful reference for how a DAW-integrated spatial audio workflow looks end-to-end, even if you do not lift code.

Licensing: some components MIT, others more restrictive. Check per-file headers.

---

## 9. Reference Reading List

### Books

- **Zotter, F. and Frank, M.** (2019). *Ambisonics: A Practical 3D Audio Theory for Recording, Studio Production, Sound Reinforcement, and Virtual Reality.* Springer Topics in Signal Processing, vol 19. **Open access**. The single most useful book for this project. Chapters 1–3 derive the spherical harmonic maths; chapter 4 covers decoders (AllRAD, EPAD, MMD, MagLS); chapter 7 is reverberation in ambisonic domain; chapter 8 is binaural rendering specifically.

- **Begault, D.** (1994). *3-D Sound for Virtual Reality and Multimedia.* Academic Press. Older but still the best introduction to HRTF-based rendering and its perceptual caveats. Free NASA technical memo version available online.

- **Blauert, J.** (1997). *Spatial Hearing: The Psychophysics of Human Sound Localization.* MIT Press. The foundational text on human spatial hearing. Cone of confusion, pinna cues, front/back confusion — all defined here. Dense but irreplaceable for understanding **why** binaural rendering works (and why it fails).

### Papers

- **Gardner, W. G.** (1995). *Efficient convolution without input/output delay.* J. Audio Eng. Soc. 43(3), 127–136. The uniformly partitioned convolution paper. Everything downstream of this — JUCE, Steam Audio, SAF convolvers — descends from this algorithm.

- **Zotter, F. and Frank, M.** (2012). *All-Round Ambisonic Panning and Decoding.* J. Audio Eng. Soc. 60(10), 807–820. The AllRAD paper.

- **Schörkhuber, C., Zaunschirm, M., and Höldrich, R.** (2018). *Binaural Rendering of Ambisonic Signals via Magnitude Least Squares.* Proc. DAGA 2018. The canonical MagLS paper.

- **Schörkhuber, C. and Höldrich, R.** (2022). *Magnitude Least Squares Optimization for Binaural Ambisonic Rendering with Constraints on the Rendering Filters.* The 2022 refinement — adds regularisation and smoothness constraints, produces better-behaved filters. **This is the version to implement.**

- **Ben-Hur, Z. et al.** (2019). *Loudness Stability of Binaural Sound with Spherical Harmonic Representation of Sparse Head-Related Transfer Functions.* EURASIP J. Audio Speech Music Process. Useful for understanding energy preservation and why MagLS sounds flatter than AllRAD.

- **AES69-2022.** *AES standard for file exchange — Spatial acoustic data file format.* The SOFA specification. Available from aes.org. You only need to skim it — libmysofa does the hard work — but it is the authoritative reference for convention names and dimensions.

### Online Resources

- **Ambisonic Association**: ambisonic.info — historical and pedagogical reference.
- **SOFA Conventions**: sofaconventions.org — canonical list of conventions, example files, software index.
- **IEM Plug-in Suite documentation**: iem.at/~IEM/docs — the cleanest practical introduction to modern HOA workflows, even as user documentation.

---

## Appendix: Minimum Viable Build Order

For reference when planning implementation phases:

1. **HOA bus plumbing**. Voice -> 16-channel bus. Verify with a simple sine source at (0, 0): W channel should be the source, X channel should equal the source (since source is dead ahead), Y and Z should be zero, all higher-order channels should be zero too (you can derive this from the encoding formulas above).
2. **Load HRTF via libmysofa**. Verify HRIR pairs look correct for known directions (peak ITD at +/-90 deg azimuth, symmetric spectral shape at 0 deg).
3. **Build a naive direct HRTF path** (bypass HOA, per-voice convolution) as a ground-truth A/B reference. Use `juce::dsp::Convolution`. This will be slow and limited to a few voices — that is fine for A/B listening.
4. **MagLS decoder**. Solve for the 32 filters offline (python/numpy) using the HRTF set. Load the filter set into a bank of 32 `juce::dsp::Convolution` instances. Listen: dry HOA-encoded source -> MagLS decode should sound tonally and spatially similar to the naive per-voice path, within decoder order resolution.
5. **HOA early reflections** (synthetic, ~10 reflections, nearby azimuths/elevations). This is the first big externalisation win.
6. **HOA diffuse reverb FDN**. 16-delay-line, unitary mixing, damped. Second big externalisation win.
7. **Source width / decorrelation** per voice on the encoder side. Third externalisation win, especially on pads.
8. **SOFA user-load UI**. Re-solve MagLS at runtime when the user loads a new HRTF. Cache solved filters per HRTF+sample-rate pair.
9. **Head tracking hook**. Expose a listener-yaw parameter that pre-rotates all encoder directions before they hit the SH coefficient calculation. Leave the wire-in (OSC / IMU bridge) for a future release, but make sure the plumbing exists.

Each step should be A/B-able against the previous via a bypass switch. That is the only realistic way to debug spatial audio — trust the ears, instrument the code with toggles.
