#include "Core/PreRequisites.h"
#include "Core/Audio/TEAudioClipAsset.hpp"
#include "Core/Log.h"

TEAudioClipAsset::TEAudioClipAsset()
{
    m_Handle = 0;
}

TEAudioClipAsset::TEAudioClipAsset(const TEString &filePath)
    : m_SourceFilePath(filePath)
{
    LoadFromFile(filePath);
}

TEAudioClipAsset::~TEAudioClipAsset()
{
    if (m_ClipHandle != InvalidAudioClip)
    {
        AudioEngine::UnloadClip(m_ClipHandle);
        m_ClipHandle = InvalidAudioClip;
    }
}

TERef<Asset> TEAudioClipAsset::Clone() const
{
    auto copy = CreateRef<TEAudioClipAsset>();
    copy->m_Name = m_Name;
    copy->m_SourceFilePath = m_SourceFilePath;
    copy->m_Duration = m_Duration;
    copy->m_SampleRate = m_SampleRate;
    copy->m_Channels = m_Channels;
    return copy;
}

bool TEAudioClipAsset::LoadFromFile(const TEString &path)
{
    m_SourceFilePath = path;
    m_ClipHandle = AudioEngine::LoadClip(path);
    if (m_ClipHandle != InvalidAudioClip)
    {
        m_Name = path;
        TE_CORE_INFO("[TEAudioClipAsset] Successfully loaded audio clip from '{0}'", path.c_str());
        return true;
    }
    return false;
}

void TEAudioClipAsset::OnContentBrowserCreate(const TEString &path)
{
    LoadFromFile(path);
}
