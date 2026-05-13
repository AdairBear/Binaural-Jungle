#pragma once

// Central registry of every APVTS parameter ID used by the synth.
//
// Single source of truth: the processor's createParameterLayout(), all GUI
// attachment sites, and any test that touches a parameter all reach for the
// same constants here. Anything outside this header that hard-codes a
// parameter string is a bug — the compiler can't catch typos in string IDs.
//
// IDs are deliberately stable: changing one breaks saved presets and host
// automation. Add new ones; don't rename existing ones.

namespace bjf::pid
{
    // ── Oscillator ─────────────────────────────────────────────────────────
    constexpr auto waveform     = "waveform";
    constexpr auto oscOctave    = "osc_octave";
    constexpr auto detuneCents  = "osc_detune_cents";
    constexpr auto oscPhase     = "osc_phase";
    constexpr auto stackSize    = "osc_stack_size";
    constexpr auto oscSpread    = "osc_spread";

    // ── Granular ───────────────────────────────────────────────────────────
    constexpr auto granSample   = "gran_sample";
    constexpr auto granDensity  = "gran_density";
    constexpr auto granSizeMs   = "gran_size_ms";
    constexpr auto granPitch    = "gran_pitch";
    constexpr auto granScatter  = "gran_scatter";
    constexpr auto granSpray    = "gran_spray";
    constexpr auto granMix      = "gran_mix";

    // ── Filter ─────────────────────────────────────────────────────────────
    constexpr auto filterType      = "filter_type";
    constexpr auto filterCutoff    = "filter_cutoff";
    constexpr auto filterReso      = "filter_resonance";
    constexpr auto filterEnvAmount = "filter_env_amount";
    constexpr auto filterKeyTrack  = "filter_key_track";
    constexpr auto filterDrive     = "filter_drive";

    // ── Amp envelope (DAHDSR) ──────────────────────────────────────────────
    constexpr auto envDelay    = "amp_delay";
    constexpr auto attack      = "amp_attack";
    constexpr auto envHold     = "amp_hold";
    constexpr auto decay       = "amp_decay";
    constexpr auto sustain     = "amp_sustain";
    constexpr auto release     = "amp_release";
    constexpr auto envCurve    = "amp_curve";

    // ── Spatial (existing) ─────────────────────────────────────────────────
    constexpr auto spatialAz       = "spatial_az";
    constexpr auto spatialEl       = "spatial_el";
    constexpr auto spatialSpreadAz = "spatial_spread_az";
    constexpr auto spatialSpreadEl = "spatial_spread_el";

    // ── Early reflections (existing image-source ER) ───────────────────────
    constexpr auto erRoomSize    = "er_room_size";
    constexpr auto erWallDamping = "er_wall_damping";
    constexpr auto erMix         = "er_mix";

    // ── HOA diffuse reverb (panel parameters; DSP wired in a later step) ───
    constexpr auto revRoomSize  = "rev_room_size";
    constexpr auto revDecay     = "rev_decay";
    constexpr auto revDamping   = "rev_damping";
    constexpr auto revPreDelay  = "rev_pre_delay";
    constexpr auto revDiffusion = "rev_diffusion";
    constexpr auto revWetDry    = "rev_wet_dry";

    // ── Master ─────────────────────────────────────────────────────────────
    constexpr auto gain   = "gain";
    constexpr auto pan    = "pan";
    constexpr auto voices = "master_voices";
    constexpr auto bpm    = "master_bpm";

    // Compile-time enumeration of every parameter ID. Kept inline so tests
    // can iterate without forcing a separate .cpp dependency.
    inline constexpr const char* kAll[] = {
        waveform, oscOctave, detuneCents, oscPhase, stackSize, oscSpread,
        granSample, granDensity, granSizeMs, granPitch, granScatter, granSpray, granMix,
        filterType, filterCutoff, filterReso, filterEnvAmount, filterKeyTrack, filterDrive,
        envDelay, attack, envHold, decay, sustain, release, envCurve,
        spatialAz, spatialEl, spatialSpreadAz, spatialSpreadEl,
        erRoomSize, erWallDamping, erMix,
        revRoomSize, revDecay, revDamping, revPreDelay, revDiffusion, revWetDry,
        gain, pan, voices, bpm
    };

    inline constexpr int kCount = static_cast<int> (sizeof (kAll) / sizeof (kAll[0]));
} // namespace bjf::pid
