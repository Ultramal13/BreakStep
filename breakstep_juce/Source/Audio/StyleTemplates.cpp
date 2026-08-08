#include "StyleTemplates.h"
#include "AudioEngine.h"
#include "DrumTrack.h"

namespace BreakStepAudio
{

juce::String StyleTemplates::getStyleName(StylePreset style)
{
    switch (style)
    {
        case StylePreset::DnB_Roller:      return "DnB Roller (174 BPM)";
        case StylePreset::Jungle_Amen:     return "Jungle Amen (168 BPM)";
        case StylePreset::Dubstep_140:     return "Dubstep 140 (140 BPM)";
        case StylePreset::UK_Garage_2Step: return "UK Garage 2-Step (132 BPM)";
        case StylePreset::UK_Bass_138:     return "UK Bass / Bassline (138 BPM)";
        case StylePreset::Breakbeat_135:   return "Nu-Skool Breaks (135 BPM)";
        case StylePreset::Liquid_DnB:      return "Liquid DnB (172 BPM)";
        default:                           return "Custom";
    }
}

double StyleTemplates::getStyleBpm(StylePreset style)
{
    switch (style)
    {
        case StylePreset::DnB_Roller:      return 174.0;
        case StylePreset::Jungle_Amen:     return 168.0;
        case StylePreset::Dubstep_140:     return 140.0;
        case StylePreset::UK_Garage_2Step: return 132.0;
        case StylePreset::UK_Bass_138:     return 138.0;
        case StylePreset::Breakbeat_135:   return 135.0;
        case StylePreset::Liquid_DnB:      return 172.0;
        default:                           return 170.0;
    }
}

double StyleTemplates::getStyleSwing(StylePreset style)
{
    switch (style)
    {
        case StylePreset::UK_Garage_2Step: return 0.58;
        case StylePreset::UK_Bass_138:     return 0.25;
        case StylePreset::Breakbeat_135:   return 0.18;
        case StylePreset::Liquid_DnB:      return 0.12;
        case StylePreset::Jungle_Amen:     return 0.08;
        default:                           return 0.0;
    }
}

void StyleTemplates::applyStyle(StylePreset style, AudioEngine& engine)
{
    engine.getSequencer().setBpm(getStyleBpm(style));
    engine.getSequencer().setSwing(getStyleSwing(style));
    engine.getSliceSequencer().setBpm(getStyleBpm(style));

    switch (style)
    {
        case StylePreset::DnB_Roller:      generateDnBRoller(engine); break;
        case StylePreset::Jungle_Amen:     generateJungleAmen(engine); break;
        case StylePreset::Dubstep_140:     generateDubstep140(engine); break;
        case StylePreset::UK_Garage_2Step: generateUKGarage(engine); break;
        case StylePreset::UK_Bass_138:     generateUKBass(engine); break;
        case StylePreset::Breakbeat_135:   generateBreakbeat(engine); break;
        case StylePreset::Liquid_DnB:      generateLiquidDnB(engine); break;
        default: break;
    }

    engine.getSongTimeline().fillStandardArrangement();
}

void StyleTemplates::generateDnBRoller(AudioEngine& engine)
{
    auto& seq = engine.getSequencer();
    auto& sliceSeq = engine.getSliceSequencer();

    for (int t = 0; t < 6; ++t)
    {
        if (auto* track = engine.getTrack(t))
        {
            track->setVintageMode(VintageMode::MPC60);
            track->setCrunch(0.15f);
        }
    }

    // Pattern 0: Main Roller
    seq.clearPattern(0);
    for (int s = 0; s < 32; ++s)
    {
        if (s == 0 || s == 10 || s == 16 || s == 26) seq.setPatternStepState(0, 0, s, 2); // Kick
        if (s == 4 || s == 12 || s == 20 || s == 28) seq.setPatternStepState(0, 1, s, 2); // Snare
        if (s % 2 == 0) seq.setPatternStepState(0, 2, s, (s % 4 == 0) ? 2 : 1);           // Hat
        if (s == 6 || s == 22) seq.setPatternStepState(0, 3, s, 2);                         // Open Hat
    }

    // Pattern 1: Ghost Snare Groove
    seq.copyPattern(0, 1);
    seq.setPatternStepState(1, 1, 7, 1);
    seq.setPatternStepState(1, 1, 15, 1);
    seq.setPatternStepState(1, 1, 23, 1);
    seq.setPatternStepState(1, 1, 31, 1);

    // Pattern 2: Steppy Variation
    seq.clearPattern(2);
    for (int s = 0; s < 32; ++s)
    {
        if (s == 0 || s == 6 || s == 12 || s == 16 || s == 22 || s == 28) seq.setPatternStepState(2, 0, s, 2);
        if (s == 4 || s == 12 || s == 20 || s == 28) seq.setPatternStepState(2, 1, s, 2);
        if (s % 2 == 0) seq.setPatternStepState(2, 2, s, 1);
    }

    // Pattern 3: Snare Fill
    seq.copyPattern(0, 3);
    for (int s = 24; s < 32; ++s) seq.setPatternStepState(3, 1, s, (s % 2 == 0) ? 2 : 1);

    // Slice Patterns
    for (int p = 0; p < 8; ++p)
    {
        sliceSeq.clearPattern(p);
        for (int i = 0; i < 16; ++i)
        {
            SliceStep st;
            st.active = (i % 2 == 0) || (i == 7) || (i == 11) || (i == 15);
            st.sliceIndex = (i + p) % 16;
            st.ratchets = (i == 15 && p % 2 != 0) ? 4 : ((i == 14 && p % 2 != 0) ? 2 : 1);
            st.probability = 1.0f;
            sliceSeq.setPatternStep(p, i, st);
        }
    }
}

void StyleTemplates::generateJungleAmen(AudioEngine& engine)
{
    auto& seq = engine.getSequencer();
    auto& sliceSeq = engine.getSliceSequencer();

    for (int t = 0; t < 6; ++t)
    {
        if (auto* track = engine.getTrack(t))
        {
            track->setVintageMode(VintageMode::EPS16Plus);
            track->setCrunch(0.45f);
        }
    }

    seq.clearPattern(0);
    for (int s = 0; s < 32; ++s)
    {
        if (s == 0 || s == 6 || s == 16 || s == 22 || s == 26) seq.setPatternStepState(0, 0, s, 2);
        if (s == 4 || s == 12 || s == 19 || s == 20 || s == 28) seq.setPatternStepState(0, 1, s, 2);
        if (s % 2 == 0) seq.setPatternStepState(0, 2, s, 2);
        if (s == 3 || s == 7 || s == 11 || s == 15) seq.setPatternStepState(0, 5, s, 1);
    }
    seq.copyPattern(0, 1);
    seq.copyPattern(0, 2);
    seq.copyPattern(0, 3);

    for (int p = 0; p < 8; ++p)
    {
        sliceSeq.clearPattern(p);
        for (int i = 0; i < 16; ++i)
        {
            SliceStep st;
            st.active = true;
            st.sliceIndex = (i * 2 + p * 3) % 16;
            st.ratchets = (i == 7 || i == 15) ? 4 : ((i == 6 || i == 14) ? 2 : 1);
            st.reverse = (i == 11 && p > 1);
            st.probability = (i % 4 == 0) ? 1.0f : 0.85f;
            sliceSeq.setPatternStep(p, i, st);
        }
    }
}

void StyleTemplates::generateDubstep140(AudioEngine& engine)
{
    auto& seq = engine.getSequencer();
    auto& sliceSeq = engine.getSliceSequencer();

    for (int t = 0; t < 6; ++t)
    {
        if (auto* track = engine.getTrack(t))
        {
            track->setVintageMode(VintageMode::MPC60);
            track->setCrunch(0.2f);
        }
    }

    // Half-time Kick on 0, Snare on 8 (in 16-step bar)
    seq.clearPattern(0);
    for (int s = 0; s < 32; ++s)
    {
        if (s == 0 || s == 16 || s == 22) seq.setPatternStepState(0, 0, s, 2); // Kick
        if (s == 8 || s == 24) seq.setPatternStepState(0, 1, s, 2);             // Snare
        if (s == 8 || s == 24) seq.setPatternStepState(0, 4, s, 2);             // Clap layer
        if (s % 4 == 2) seq.setPatternStepState(0, 2, s, 1);                    // Sparse Hat
        if (s == 14 || s == 30) seq.setPatternStepState(0, 3, s, 2);            // Open Hat
    }

    seq.copyPattern(0, 1);
    seq.setPatternStepState(1, 0, 10, 2);
    seq.setPatternStepState(1, 0, 26, 2);

    for (int p = 0; p < 8; ++p)
    {
        sliceSeq.clearPattern(p);
        for (int i = 0; i < 16; ++i)
        {
            SliceStep st;
            st.active = (i == 0 || i == 6 || i == 8 || i == 14);
            st.sliceIndex = (i + p * 2) % 16;
            st.ratchets = (i == 14) ? 2 : 1;
            st.probability = 1.0f;
            sliceSeq.setPatternStep(p, i, st);
        }
    }
}

void StyleTemplates::generateUKGarage(AudioEngine& engine)
{
    auto& seq = engine.getSequencer();
    auto& sliceSeq = engine.getSliceSequencer();

    for (int t = 0; t < 6; ++t)
    {
        if (auto* track = engine.getTrack(t))
        {
            track->setVintageMode(VintageMode::MPC3000);
            track->setCrunch(0.05f);
        }
    }

    // Classic 2-step: Kicks on 0, 6, 16, 22
    seq.clearPattern(0);
    for (int s = 0; s < 32; ++s)
    {
        if (s == 0 || s == 6 || s == 16 || s == 22) seq.setPatternStepState(0, 0, s, 2); // Kick
        if (s == 4 || s == 12 || s == 20 || s == 28) seq.setPatternStepState(0, 1, s, 2); // Snare
        if (s == 4 || s == 12 || s == 20 || s == 28) seq.setPatternStepState(0, 4, s, 1); // Clap
        if (s % 2 == 0) seq.setPatternStepState(0, 2, s, (s % 4 == 2) ? 2 : 1);           // Swung Hat
        if (s == 2 || s == 10 || s == 18 || s == 26) seq.setPatternStepState(0, 3, s, 2); // Offbeat Open Hat
    }

    seq.copyPattern(0, 1);
    seq.copyPattern(0, 2);

    for (int p = 0; p < 8; ++p)
    {
        sliceSeq.clearPattern(p);
        for (int i = 0; i < 16; ++i)
        {
            SliceStep st;
            st.active = (i == 2 || i == 6 || i == 10 || i == 14);
            st.sliceIndex = (i + p) % 16;
            st.ratchets = 1;
            st.probability = 1.0f;
            sliceSeq.setPatternStep(p, i, st);
        }
    }
}

void StyleTemplates::generateUKBass(AudioEngine& engine)
{
    auto& seq = engine.getSequencer();
    auto& sliceSeq = engine.getSliceSequencer();

    for (int t = 0; t < 6; ++t)
    {
        if (auto* track = engine.getTrack(t))
        {
            track->setVintageMode(VintageMode::EPS16Plus);
            track->setCrunch(0.35f);
        }
    }

    seq.clearPattern(0);
    for (int s = 0; s < 32; ++s)
    {
        if (s % 4 == 0 || s == 14 || s == 30) seq.setPatternStepState(0, 0, s, 2); // Driving Kicks
        if (s == 4 || s == 12 || s == 20 || s == 28) seq.setPatternStepState(0, 1, s, 2);
        if (s == 4 || s == 12 || s == 20 || s == 28) seq.setPatternStepState(0, 4, s, 2);
        if (s % 2 == 0) seq.setPatternStepState(0, 2, s, 2);
    }
    seq.copyPattern(0, 1);

    for (int p = 0; p < 8; ++p)
    {
        sliceSeq.clearPattern(p);
        for (int i = 0; i < 16; ++i)
        {
            SliceStep st;
            st.active = (i % 2 == 0) || (i == 7);
            st.sliceIndex = (i + p * 2) % 16;
            st.ratchets = (i == 15) ? 4 : 1;
            st.probability = 1.0f;
            sliceSeq.setPatternStep(p, i, st);
        }
    }
}

void StyleTemplates::generateBreakbeat(AudioEngine& engine)
{
    auto& seq = engine.getSequencer();
    auto& sliceSeq = engine.getSliceSequencer();

    for (int t = 0; t < 6; ++t)
    {
        if (auto* track = engine.getTrack(t))
        {
            track->setVintageMode(VintageMode::Clean);
            track->setCrunch(0.0f);
        }
    }

    seq.clearPattern(0);
    for (int s = 0; s < 32; ++s)
    {
        if (s == 0 || s == 6 || s == 10 || s == 16 || s == 22 || s == 26) seq.setPatternStepState(0, 0, s, 2);
        if (s == 4 || s == 12 || s == 20 || s == 28) seq.setPatternStepState(0, 1, s, 2);
        if (s % 2 == 0) seq.setPatternStepState(0, 2, s, 1);
        if (s == 2 || s == 14 || s == 18 || s == 30) seq.setPatternStepState(0, 3, s, 2);
    }
    seq.copyPattern(0, 1);

    for (int p = 0; p < 8; ++p)
    {
        sliceSeq.clearPattern(p);
        for (int i = 0; i < 16; ++i)
        {
            SliceStep st;
            st.active = (i % 2 == 0) || (i == 5) || (i == 11);
            st.sliceIndex = (i + p) % 16;
            st.ratchets = (i == 14) ? 2 : 1;
            st.probability = 1.0f;
            sliceSeq.setPatternStep(p, i, st);
        }
    }
}

void StyleTemplates::generateLiquidDnB(AudioEngine& engine)
{
    auto& seq = engine.getSequencer();
    auto& sliceSeq = engine.getSliceSequencer();

    for (int t = 0; t < 6; ++t)
    {
        if (auto* track = engine.getTrack(t))
        {
            track->setVintageMode(VintageMode::MPC3000);
            track->setCrunch(0.08f);
            track->setReverbSend(0.2f);
        }
    }

    engine.setDelayWet(0.25f);

    seq.clearPattern(0);
    for (int s = 0; s < 32; ++s)
    {
        if (s == 0 || s == 10 || s == 16 || s == 26) seq.setPatternStepState(0, 0, s, 2);
        if (s == 4 || s == 12 || s == 20 || s == 28) seq.setPatternStepState(0, 1, s, 2);
        if (s % 2 == 0) seq.setPatternStepState(0, 2, s, (s % 4 == 0) ? 2 : 1);
        if (s == 6 || s == 22) seq.setPatternStepState(0, 3, s, 2);
    }
    seq.copyPattern(0, 1);
    seq.setPatternStepState(1, 1, 7, 1);
    seq.setPatternStepState(1, 1, 23, 1);

    for (int p = 0; p < 8; ++p)
    {
        sliceSeq.clearPattern(p);
        for (int i = 0; i < 16; ++i)
        {
            SliceStep st;
            st.active = (i % 2 == 0);
            st.sliceIndex = (i + p) % 16;
            st.ratchets = 1;
            st.probability = 1.0f;
            sliceSeq.setPatternStep(p, i, st);
        }
    }
}

} // namespace BreakStepAudio
