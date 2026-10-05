# VST3 VocalChain+ by 29YURO — V2

Windows VST3 vocal-chain project using JUCE 8 and CMake.

Implemented DSP: input/output gain, dry/wet, local pitch detection, scale-aware pitch correction, manual pitch shift, compressor, de-esser, 3-band tonal EQ, air high shelf, saturation/distortion, stereo width, chorus, reverb and feedback delay. The UI includes vocal style presets, vocal role selection, key/scale, reference-file selection, level/pitch display and local timing feedback.

## Important technical limits
- Pitch correction is a local time-domain implementation. It does not preserve formants like specialist commercial Auto-Tune products and may sound synthetic at stronger settings.
- The plugin does not remotely control arbitrary third-party Auto-Tune VSTs. VST3 does not provide a universal host-independent mechanism for that.
- The AI/coach area is local signal analysis, not a cloud AI service. It does not invent analysis values.
- Reference-song loading currently selects and displays a reference file; automatic style transfer from copyrighted recordings is not performed.

## GitHub build
Actions -> Build Windows VST3 -> Run workflow. Download `VST3-VocalChain-Plus-Windows` from Artifacts after a green build.
