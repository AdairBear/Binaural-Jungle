# Binaural Jungle — Pad & String Sound-Design Brief

*Sonic identity research for the pad/string voice of an atmospheric drum & bass VST instrument.*
*Target era: 1993–1996 (with spillover into the 1997 Photek/Source Direct "technical" period).*
*Audience: DSP architects and factory preset designers.*

---

## 1. Artist / Release Deep-Dive

The pad and string voice of old-school atmospheric jungle is not a single sound. It is a cluster of recurring emotional postures — melancholy, futurism, urban-pastoral wonder, dread — built out of a shared vocabulary of samples, synths, and studio tricks. Seven tracks (spanning the core 1993–97 window) define the outer shape of that vocabulary.

### LTJ Bukem — "Horizons" (Looking Good, 1995)
The canonical "morning in the city" pad. A slow, evolving modal wash in a minor-tinged Dorian zone, with a Rhodes/DX-like electric-piano layer drifting over it. Harmonically it sits mostly on one chord, leaning on a suspended minor 9th feel and never fully resolving. The emotional palette is bittersweet, pastoral, *patient* — the breakbeat is frantic but the pad is almost ambient. Stereo field is enormous: the pad sounds like it was printed to a long plate reverb and then chorused again on the bus. The pad has almost no transient; everything is attack-removed, release-extended.

### Goldie — "Inner City Life" (Timeless, 1995)
The most iconic string sound in the genre. Lush, wide, deeply emotional string-ensemble layered under Diane Charlemagne's vocal. Rob Playford's arrangement uses a stacked-string approach (multiple sampled-string layers tuned slightly apart, plus a Juno-style PWM pad underneath) and bathes it in long decaying hall/plate reverb with tape-saturated warmth. The harmony is minor-key with classic 4→1 descending motion, minor 9 voicings, and chromatic passing chords. Emotionally: cinematic yearning, grief, and aspiration all at once. The strings are *slow* — bow-speed feel, never fast articulation.

### 4hero — "Mr Kirk's Nightmare" (1990/1993 reissue, Reinforced)
Earlier and darker than the "atmospheric" core, but the pad/string DNA is already there: a held, queasy minor-key string stab, bit-reduced through the Akai S950, with a downward chromatic bass under it. The emotional palette is urban dread — social-realist, tragic. The string tone is aggressive, filtered, and intentionally lo-fi; there is *crunch* in the sample that we will want to model. Pitch-shifted stretch artifacts are audible.

### 4hero — "Universal Love" / "Parallel Universe" era (1994)
Parallel Universe is where 4hero pivot from ragga-jungle into Sun-Ra-tinged Afrofuturism. The pads are jazz-fusion stacks — Rhodes, Mellotron-like strings, synth-brass blooms — often borrowed wholesale from 1970s Lonnie Liston Smith and Roy Ayers records. Harmony gets adventurous: maj7#11, minor 9, quartal Rhodes stacks, tritone substitutions. Emotional palette: cosmic optimism with melancholy undertow. Recording aesthetic is *warm, tape-driven, and slightly ducked* against the breakbeat.

### Photek — "Ni Ten Ichi Ryu" / "The Hidden Camera" (Modus Operandi, 1997)
The "technical" pole of the genre. Photek's pads are minimal, monochrome, almost industrial — thin string pads sitting in a cavernous reverb, often just a single sustained interval (a minor 2nd or minor 9th) rather than a full chord. Emotional palette: clinical menace, Japanese-brush-painting restraint, wide-open space. The pad is *dry at the source* and drenched on the bus — a strong contrast between arid sample and wet ambience. Very little chorus; reverb does the widening.

### Peshay — "Piano Tune" / "Vixen" (Good Looking era, 1995–96)
Peshay is Bukem's harmonic cousin but with more jazz schooling. "Piano Tune" rides a sustained Rhodes chord stab with 4ths and 5ths in the voicing, a Mellotron-ish flute/string pad floating above, and a warm detuned Juno bass. The pad's emotional register is *wistful* — late-afternoon jazz, not morning-pastoral. Harmonically this is modal (often Dorian or Aeolian) with sus4add9 colour chords.

### Omni Trio — "Renegade Snares" / "Living For The Future" (1993–95)
The rave-facing end of atmospheric jungle. Big diva-sample pads, Juno-style PWM wash, heavy tape-echo feedback throws on the top-line. The pad tends to be a single chord held under an anthemic vocal hook. Emotional palette: euphoria plus melancholy — "crying in the club" decades before the term existed.

### Alex Reece — "Pulp Fiction" (1995) / Hidden Agenda — "Is It Love" (1995)
Clean, jazz-influenced, Rhodes-forward. The pads are minimal; the Rhodes *is* the pad. Voicings are chord-stabs — sus2, min7, min9, add9 — played *legato* through a chorus and a plate. Hidden Agenda's "Is It Love" is the template for the "Rhodes chord stab" category we want as a factory preset family.

**Summary of emotional palette across the canon:** bittersweet, pastoral, urban-melancholy, cosmic-jazz, clinical-menacing, euphoric-melancholic. Harmonically: modal (Dorian / Aeolian dominant), rich extensions, 4ths/5ths stacks, avoidance of straight major-key diatonic resolution.

---

## 2. Sample Sources and Why They Matter

Jungle producers weren't designing pads from first principles — they were *recontextualising* samples from 1970s records where the pad work had already been done by session string players and jazz keyboardists. The engine's timbral center of gravity should track what those source records actually sounded like.

### Jazz-fusion (Lonnie Liston Smith, Roy Ayers, Weather Report, Herbie Hancock Mwandishi-era, Pat Metheny Group)
These records supplied the harmonic density of the genre. Lonnie Liston Smith's *Expansions* and *Cosmic Funk* are quoted across 4hero, Bukem, A Guy Called Gerald. Characteristic features: Fender Rhodes with chorus and tape-echo, Mellotron strings, Oberheim or Moog pad layers, live string sections, and wide stereo recording from early-'70s studios. The chord voicings these records contain — maj7#11, min9, quartal Rhodes stacks, suspended dominants — got sampled *with their harmonic information intact*, which is why they sound sophisticated inside a jungle track. The engine needs to be able to *emit* chords in this vocabulary, not just emulate a generic "pad."

Pat Metheny Group's *First Circle* / *Offramp* contributed the *airy*, reverbed, almost pastoral synth-string sound (Lyricon, Synclavier, Oberheim stacked strings with huge Lexicon 224 reverb) that Bukem in particular orbits. Metheny's sound is a north star for the "Bukem Atmos" category.

Herbie Hancock's Mwandishi / Sextant period added the *abstract*, microtonal, analog-synth-colour palette that shows up in 4hero and Source Direct's weirder moments.

### Soul / funk (Curtis Mayfield, Isaac Hayes, Minnie Riperton)
These contributed the *orchestral* layers — Chicago-style strings, Memphis horn pads, backing-vocal "oohs" that get sampled as choir pads. The recording aesthetic is *close-miked with a lot of plate reverb*, slightly compressed, warm 2-inch tape. When a jungle producer samples Minnie Riperton or an Isaac Hayes string passage, the pad arrives pre-mixed in that aesthetic.

### Blaxploitation soundtracks (Shaft, Superfly, Across 110th Street)
Tight string-and-wah arrangements, dramatic low-string unison lines, and the famous "cinematic descending chromatic" motif that appears throughout jungle. Harmonically: minor-key, chromatic bass motion, big tutti hits and lush sustained beds.

### Minnie Riperton / Rotary Connection
Charles Stepney's arrangements provided much of the *choral* vocabulary — wordless soprano lines that get sampled and used as pad layers. These need to be recognisable in the "Reversed Choir" preset category.

### BBC Radiophonic Workshop and library records
Sources like Delia Derbyshire, the KPM library, and DeWolfe library contributed the *uncanny* / *synthetic* pad material — early-tape synthesis, musique concrète textures, Mellotron-like flutes. These appear in Photek, Source Direct, and 4hero for their *science-fiction* character.

### Easy-listening and orchestral '70s records
Mantovani-style string ensembles, Percy Faith, Bernard Herrmann cues — sampled for lush sustained strings and reverb-soaked ambience. The recording artefacts (tape hiss, LP surface noise, RIAA-curve high-end) are part of the signature.

**DSP takeaway:** Samples arrive carrying: warm tape-saturated midrange, pre-applied plate reverb, mild LP/tape wow and flutter, slight HF rolloff, and harmonic content dominated by jazz voicings. The engine's tone stack must be able to *reproduce* these artefacts as a deliberate colour, not fight them.

---

## 3. Hardware That Shaped the Sound

### Akai S950 / S1100 / S3000XL
The definitive jungle samplers. The S950 is 12-bit with a companded 3-stage bit reduction path, producing characteristic *grit* on decay tails and quantisation noise on quiet pads. Anti-aliasing filter is gentle and audibly colours the top end (rolls off ~12–15kHz). The S950's *time-stretch* algorithm is primitive — granular crossfade at short grain sizes — and produces the famous "stretched pad" texture: a smeared, slightly metallic, rhythmically grainy sound that became an aesthetic. The S1100 is 16-bit but retains the Akai filter flavour. The S3000XL is cleaner, more "hi-fi," used more in the 1996–97 era for precise chopping.

The *aliasing and reconstruction filter* behaviour of these units matters. Pitching a sample up on an S950 produced mild aliasing that adds a gritty high-frequency edge. Pitching down produced low-bitrate stepping. Both are desirable.

### Roland Juno-60 / Juno-106
The classic "warm pad" sound. DCO (analog-modelled digital controlled oscillators), a 24dB low-pass filter with gentle resonance, and the Juno BBD-based chorus (I and II, with II being deeper and more unstable). The chorus alone is responsible for a huge part of the stereo-wide, shimmering pad sound of the era — even dry Juno oscillators through the chorus feel enormous. The chorus introduces pitch wobble at ~0.5–1Hz, bucket-brigade signal degradation, and stereo decorrelation.

### Roland JD-800
A 1991 digital synth with real-time sliders. Produced punchy, bright, *slightly digital* pads and stabs. The JD-800's strings and brass patches show up across mid-'90s D&B. Characteristic: a sharper attack than a Juno, plus built-in multi-effects (chorus, delay, reverb).

### Yamaha DX7 (and TX81Z)
FM synthesis contributed the *bell* tones, electric-piano tones (the canonical "E.PIANO 1" patch), and glassy pad layers. The DX7's cold, metallic, inharmonic top end is a specific colour that shows up as a layer under warmer analog pads. Also important: the DX7's aliased output at higher frequencies adds a characteristic "digital shimmer."

### Korg M1
The M1's piano, organ, and string patches (especially "Universe" and "Digital E.Piano") are all over early-'90s breakbeat and hardcore. The M1 has a specific PCM-rompler sound — slightly brittle, compressed dynamics, shortened sample loops — that feels "of the era."

### Ensoniq ASR-10 / EPS-16+
Heavily used by US hip-hop producers whose tracks got sampled back into jungle. Adds another layer of sample-rate-conversion crunch.

**Why limitations became virtues:** The 12-bit Akai sound, the Juno chorus noise floor, the DX7 aliasing, the M1 loop-point artefacts, and the ASR-10's pitched-down grunge all add *consistent low-frequency warmth and high-frequency grit* that sits beautifully against the breakbeat. The engine should not aim for a clean 24-bit signal path. It should aim for a signal path that reproduces these colours as switchable qualities.

---

## 4. Production Techniques

### Timestretched pads (intentional artifacting)
The single most distinctive pad-generation technique. Producers would take a short sample of a chord (from a jazz record, a library cue, or a DX7 preset) and timestretch it in an Akai S950 or a Steinberg ReCycle session to double or triple length. The result: grain artefacts, pitch wobble, phase smearing, and a characteristic "granular shimmer" on the decay. This is *the* Bukem pad technique. The engine needs a granular timestretch module whose grain size and overlap can be pushed toward the artefact zone (grain sizes of 20–60ms, envelope windowing that lets the seams show).

### Reversed attacks
Sampling a chord and reversing the head so the pad *swells in* rather than attacks cleanly. Combined with a gentle low-pass on the way in, this produces the "sucked-in pad" that starts every Bukem breakdown. The engine should support a reverse-attack envelope that precedes the normal amplitude envelope.

### Rhodes / Fender Rhodes layers
Either sampled from records or played in via a Rhodes Mark I / II / Suitcase, sometimes a DX7 E.Piano patch standing in. Always chorused, often run through a plate reverb. The Rhodes is not a pad — it's a *chord-stab voice* that lives between pad and percussion.

### Mellotron-like string ensembles
The Mellotron's loop-point artefacts and 8-second sample limit produce a specific pulsing, slightly mis-tracked ensemble feel. Sampled Mellotron tapes show up directly (4hero) or are emulated by stacking several slightly-detuned string layers and modulating their pitch slowly.

### Space Echo RE-201 tape delay
Warm, saturated, slightly pitch-unstable tape delay. Used for dub-era *throws* — feeding the pad back into a short tape delay with heavy feedback on a specific word or bar, creating a cascading echo that self-oscillates. The RE-201's spring reverb is also a distinctive colour.

### Plate reverb (EMT 140) and digital reverb (AMS RMX16, Lexicon 480L / 224)
The dominant reverb aesthetic. EMT 140 plate adds metallic shimmer and tight early reflections; AMS RMX16 adds dense, slightly-digital hall ambience ("NonLin" and "Ambience" presets are canonical); Lexicon 480L / 224 adds lush, long, cinematic tails (the Bukem/Metheny sound). Reverb times are often *huge* — 4–8 seconds — because the pad has to survive under a busy breakbeat.

### Dub-era delay throws
Inherited from King Tubby and reggae dub production. Single words or chords sent to a long delay with rising feedback, often hand-fader-ridden so the throw builds and then collapses. Photek and Source Direct use this for architectural effect; Bukem uses it as transition candy.

### Tape saturation
Master bus to 2-inch tape (or an Alesis ADAT in the lower-budget era, which has its own crunchy digital character). Produces gentle low-frequency bump, high-frequency compression, and harmonic warming.

---

## 5. Spatial Qualities

The jungle pad is a *spatial object*, not just a spectral one. Four spatial devices recur:

- **Wide stereo via detuned oscillator layers and chorus.** Juno chorus, CE-1/CE-2 pedals, or Eventide H3000 "Micro Pitch" adds pitch-decorrelated L/R channels that sound enormously wide on headphones. Correlation between L and R channels is often near zero.
- **Dub-era delay throws establishing depth.** A throw that recedes into reverb creates the illusion of physical distance — the voice moves away from the listener.
- **Heavy reverb washes creating a virtual room.** The reverb establishes an "outside" — a hall, a warehouse, a church — that the dry break punches into.
- **Dry/wet contrast between breakbeat and pad.** The break is aggressively dry, close-miked, almost in the listener's face. The pad is the opposite — distant, wet, atmospheric. This contrast is *structural* to the genre. A pad that sits in the same reverb space as the drums will feel wrong.

**DSP implication:** the binaural bus must excel at producing a large, decorrelated, reverberant space around the pad while leaving percussive material undisturbed. HRTF-based binaural placement is welcome but the primary spatial driver is *reverb width + chorus decorrelation + delay-throw depth*.

---

## 6. Harmonic Tendencies

### Modal language
Dorian and Aeolian are the two default modes. Phrygian shows up in darker material (Photek, Source Direct). Mixolydian and Lydian appear in jazzier Bukem/Peshay moments. Straight Ionian (major) is rare and almost always feels "wrong" in the genre — it reads as cheesy, rave-era, or pre-atmospheric. Factory preset demo content should default to Dorian or Aeolian.

### Voicing vocabulary
- **Minor 9** (root, b3, 5, b7, 9) — the default "lush" chord
- **Minor 11** (add 11) — the "Inner City Life" colour
- **Maj7#11** — the Lydian-maj colour, Pat Metheny / 4hero
- **Sus4add9** — the "suspended drift" colour, Bukem
- **Quartal Rhodes stacks** (4ths and 5ths stacked) — the Hidden Agenda / Peshay / Herbie Hancock colour
- **Min6/9** — darker, Isaac Hayes-flavoured
- **Dominant 7sus4** — transitional, jazz-fusion
- **Single-interval pads** — Photek, Source Direct, often a minor 2nd, perfect 5th, or tritone held as texture

### Chromatic descending lines
Inherited from cinema and jazz: a chromatic bass or top-line descending in half-steps over a held chord. Almost every canonical atmospheric track contains one.

### Avoidance of V-I
Straight dominant-to-tonic resolution feels foreign to the genre. Cadences are more often plagal, modal (i–bVII–bVI), or suspended. Factory presets should *not* be demoed over a V-I.

---

## 7. Modern Touchstones (2010s–2020s)

A second wave of producers reinterpreted the 1993–96 aesthetic. Tracking their choices sharpens the reference set.

- **Coco Bryce** (My Mine, 2019 onward) — explicitly nostalgic, uses detuned Juno pads, Amen-era breaks, very Bukem-indebted harmonic language. Production is intentionally lo-fi and tape-coloured.
- **Tim Reaper** — prolific Future Retro revivalist. Pads are simpler and more stab-oriented, but the sample palette is pure 1994.
- **Sully** — crosses old-school jungle with modern UK bass aesthetics. Uses granular timestretch heavily; pads feel fragmented and glitched in a way that extends the Akai-artefact aesthetic into 2020s tooling.
- **Calibre** — carries the Bukem atmospheric torch directly into the liquid D&B era. Lush Rhodes, live-feel drums, deep bass, long reverbs. A critical reference for what "atmospheric D&B pad" sounds like played through modern studios.
- **Marcus Intalex** (RIP) — similar role, slightly more club-oriented. Rhodes + pad + deep sub, with occasional dub-era throws.
- **Blame** — originator-and-reviver both. His 1990s work is canonical; his reissues and newer pieces are a bridge.
- **Dead Man's Chest** — explicit "1994 cosplay" project, excellent reference for *exactly* how the era sounds when emulated well in modern tooling.

**Implication for factory presets:** the engine should support both *vintage-authentic* (raw Akai-cruncher aesthetic) and *modern-liquid* (cleaner Calibre/Intalex aesthetic) flavours. Give each preset family a "grit" control that shifts between them.

---

## 8. Practical Implications for DSP and Factory Presets

### Engine capabilities (ranked by importance)

**A. Lush detuned-oscillator stacks.** Three to eight oscillator pairs, each slightly detuned (±3 to ±15 cents), with independent pan-spread. Sub-audio random pitch drift on each voice to simulate analog instability. Built-in Juno-style BBD chorus on the polyphony bus, tunable from subtle (mode I) to unstable (mode II).

**B. Granular timestretch with bit reduction.** Independent grain-size (10–200ms), grain overlap (25–400%), pitch deviation, and windowing that lets artefacts show. Post-granular bit reducer (8–16 bits, switchable companding curve to emulate Akai's 12-bit path) and a gentle switchable anti-aliasing filter modelled on the S950's reconstruction filter.

**C. Tape-saturated release tails.** The *release* is where the pad's character lives. A saturation module engaged preferentially on the release phase — asymmetric soft-clipping, low-order harmonic generation, and a mild high-frequency rolloff — produces the "drifting off into tape" feel. This is distinct from a global saturator; it should be envelope-sidechained.

**D. Plate / spring / algorithmic reverbs on the binaural bus.** At minimum: a plate model (EMT 140 flavour — metallic early reflections, dense diffusion), a spring model (RE-201 flavour), and a large-hall algorithmic model (Lexicon 480L / 224 flavour — lush, long, slightly bloomy tails). Reverb times up to 12 seconds. Pre-delay to 200ms. Wet-only output option for parallel bus usage.

**E. Reverse-attack envelope.** A reverse-ramp stage that precedes the normal ADSR, with its own length and low-pass opening curve. Essential for "swell-in" pads.

**F. Dub-era delay throw.** Mono-in, stereo-out tape-delay model with feedback, saturation, and the ability to sidechain the feedback-amount to a MIDI controller or note-trigger. Essential for throws.

**G. Binaural bus.** HRTF-based spatialisation on the reverb return, plus inter-channel decorrelation. Must not smear the dry source — only the ambience. Headphone-first, but should collapse sanely to stereo speakers.

### Factory preset categories (mandatory starter set)

| Category | Description | Key techniques |
|---|---|---|
| **Bukem Atmos** | Long evolving modal pad, Dorian/Aeolian, huge Lexicon-style reverb, gentle chorus, reversed attack, slow LFO on filter. Rhodes layer optional. Emotional: bittersweet pastoral. | Reverse attack, granular timestretch, huge hall reverb, minor 9 / sus4add9 default voicing. |
| **Goldie Strings** | Stacked string-ensemble layers, slow bow-feel, rich hall plate, tape saturation, wide stereo. Emotional: cinematic yearning. | 4+ detuned string layers, EMT plate, mild tape saturation on release, min9 / min11 voicings. |
| **Photek Science** | Thin, arid, monochrome texture-pad. Single-interval or minor-2nd cluster. Long cavernous reverb on wet bus, dry source. Emotional: clinical, menacing, spacious. | Minimal oscillators, harsh lowpass, long hall, zero chorus, dry/wet extreme contrast. |
| **Rhodes Chord Stab** | Sampled/modelled Fender Rhodes, chorused, short plate. Quartal voicings (4ths and 5ths stacked). Intended for held chords and stabs. Emotional: jazz-melancholic, Hidden Agenda / Peshay. | Rhodes-style layer, medium chorus, short plate, min7 / min9 / sus4add9 / quartal defaults. |
| **Reversed Choir** | Sampled/synthesised "ooh" voices reversed into a swell, with long plate tail. Emotional: uncanny, cinematic, Charles Stepney / Minnie Riperton–adjacent. | Reverse attack, choir sample bank or formant-shaped synth, long plate, slow LFO pan. |
| **Tape-Stretched Pad** | A granular timestretched chord, grain artefacts audible, 12-bit bit-reducer on, Akai-style reconstruction filter, long reverb. Emotional: lo-fi, nostalgic, Dead Man's Chest / Coco Bryce. | Granular engine with visible seams, bit reducer, Akai LPF, tape saturation on release. |
| **Dub Chord Stab** | Rhodes or pad stab designed for repeated triggering, with a send-able dub-delay throw built into the preset routing. Emotional: sound-system, cavernous, transitional. | Short decay, heavy tape-delay send, feedback modulation, spring-reverb tint. |

### Default harmonic demo content
- Demo presets in Dorian (D Dorian, E Dorian, F# Dorian) and Aeolian (A minor, C minor)
- Avoid straight Ionian demos
- Chord progressions should favour modal motion: i–bVII–bVI, i–iv–bVII, sustained single-chord vamps with melodic movement on top
- Include at least one demo featuring a chromatic descending line over a sustained chord

### Signal-path defaults to match the era
- Output should pass through a switchable "era" colour chain: off / Akai 12-bit / Juno chorus / tape saturation. Presets in the vintage categories default this chain **on**; modern-liquid presets default it **off**.
- Dry/wet ratio defaults toward wet — a jungle pad is rarely heard dry. 30–50% wet is a reasonable starting point for the dry pad itself, with the reverb bus adding further space on top.
- Stereo width on by default, but with a mono-compatibility check: the pad must still read as musical when summed to mono for club sound-systems.

---

## Appendix: Reference listening shortlist

A sub-playlist, short enough to load into a DSP prototyping session as reference material:

1. LTJ Bukem — *Horizons* (1995)
2. Goldie — *Inner City Life* (1995)
3. 4hero — *Universal Love* / *Parallel Universe* (1994)
4. Peshay — *Piano Tune* (1995)
5. Photek — *Ni Ten Ichi Ryu* (1997)
6. Source Direct — *Stonekiller* / *The Cult* (1996)
7. Alex Reece — *Pulp Fiction* (1995)
8. Hidden Agenda — *Is It Love* (1995)
9. Omni Trio — *Renegade Snares (Foul Play remix)* (1993)
10. Foul Play — *Open Your Mind* (1994)
11. Calibre — *Mr Right On* (modern touchstone)
12. Coco Bryce — *A Million Miles (Naphta remix)* (modern touchstone)
13. Lonnie Liston Smith — *A Chance For Peace* (source material)
14. Pat Metheny Group — *Last Train Home* (source material for Bukem aesthetic)
15. Minnie Riperton — *Les Fleurs* (source material for choir and strings)

Every preset in the factory library should be A/B-able against a track from this shortlist. If the preset can sit credibly under the Amen break of any track here, it is passing; if it reads as "generic pad VST," it is failing.
