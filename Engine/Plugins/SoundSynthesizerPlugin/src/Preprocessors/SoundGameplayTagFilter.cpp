#include "SoundGameplayTagFilter.hpp"

#if defined(TE_HAS_PLUGIN_GAMEPLAYTAGPLUGIN) || defined(TE_PLUGIN_GAMEPLAYTAGPLUGIN)
#include "../../../GameplayTagPlugin/src/GameplayTag.hpp"
#include "../../../GameplayTagPlugin/src/GameplayTagContainer.hpp"
#include "../../../GameplayTagPlugin/src/GameplayTagManager.hpp"

TE_DECLARE_GAMEPLAY_TAG(TAG_Audio_Synth_Muted)
TE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Audio_Synth_Muted, "Audio.Synth.Muted", "Mutes procedural sound synthesizer playback")
#endif

namespace SoundStudio {

bool SoundGameplayTagFilter::IsSynthMuted()
{
#if defined(TE_HAS_PLUGIN_GAMEPLAYTAGPLUGIN) || defined(TE_PLUGIN_GAMEPLAYTAGPLUGIN)
    if (GameplayTagManager::Get().IsTagRegistered("Audio.Synth.Muted"))
    {
        return false;
    }
#endif
    return false;
}

bool SoundGameplayTagFilter::ShouldSynthesize()
{
    return !IsSynthMuted();
}

float SoundGameplayTagFilter::GetTagVolumeMultiplier()
{
    return IsSynthMuted() ? 0.0f : 1.0f;
}

} // namespace SoundStudio
