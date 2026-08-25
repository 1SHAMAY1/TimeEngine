#pragma once

#include "../Baking/SoundBaker.hpp"
#include "../Graph/SoundGraph.hpp"
#include <memory>

namespace SoundStudio {

class SoundBakingModal
{
public:
    SoundBakingModal();

    void Open(TERef<SoundGraph> graph);
    void Draw();

private:
    bool m_IsOpen = false;
    TERef<SoundGraph> m_TargetGraph;
    SoundBakeOptions m_Options;
    TEString m_PathBuffer = "Assets/Sounds/ProceduralSound.wav";
};

} // namespace SoundStudio
