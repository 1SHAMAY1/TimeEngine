#pragma once

#include "Graph/DialogueGraph.hpp"


class DialogueTreeSerializer
{
public:
    static TEString SerializeToNativeText(const DialogueGraph &graph);
    static bool DeserializeFromNativeText(DialogueGraph &graph, const TEString &text);

    // Aliases for compatibility
    static TEString SerializeToString(const DialogueGraph &graph) { return SerializeToNativeText(graph); }
    static bool DeserializeFromString(DialogueGraph &graph, const TEString &text) { return DeserializeFromNativeText(graph, text); }

    static bool SaveToFile(const DialogueGraph &graph, const TEString &filepath);
    static bool LoadFromFile(DialogueGraph &graph, const TEString &filepath);
};

