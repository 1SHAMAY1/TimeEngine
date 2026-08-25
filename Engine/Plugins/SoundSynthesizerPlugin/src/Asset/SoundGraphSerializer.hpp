#pragma once

#include "../Graph/SoundGraph.hpp"
#include "Utils/TEString.hpp"
#include <memory>

namespace SoundStudio {

class SoundGraphSerializer
{
public:
    static bool Serialize(TERef<SoundGraph> graph, const TEString &filepath);
    static bool Deserialize(TERef<SoundGraph> graph, const TEString &filepath);
};

} // namespace SoundStudio
