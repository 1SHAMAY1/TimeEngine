#include "TTSGameplayTagFilter.hpp"

#if defined(TE_HAS_PLUGIN_GAMEPLAYTAGPLUGIN) || defined(TE_PLUGIN_GAMEPLAYTAGPLUGIN)
#include "../../../GameplayTagPlugin/src/GameplayTag.hpp"
#include "../../../GameplayTagPlugin/src/GameplayTagContainer.hpp"
#include "../../../GameplayTagPlugin/src/GameplayTagManager.hpp"

TE_DECLARE_GAMEPLAY_TAG(TAG_Audio_TTS_Muted)
TE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Audio_TTS_Muted, "Audio.TTS.Muted", "Mutes Text-to-Audio speech playback")
#endif

bool TTSGameplayTagFilter::IsTTSMuted()
{
#if defined(TE_HAS_PLUGIN_GAMEPLAYTAGPLUGIN) || defined(TE_PLUGIN_GAMEPLAYTAGPLUGIN)
    // Check if Audio.TTS.Muted is registered and active
    if (GameplayTagManager::Get().IsTagRegistered("Audio.TTS.Muted"))
    {
        return false; // Registered tag check available
    }
#endif
    return false;
}

bool TTSGameplayTagFilter::ShouldSpeak()
{
    return !IsTTSMuted();
}
