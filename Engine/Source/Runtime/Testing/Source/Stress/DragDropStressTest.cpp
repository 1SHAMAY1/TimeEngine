#include "PreRequisites.h"
#include "TestHarness.hpp"
#include "TestRegistry.hpp"
#include "Editor/DragDrop/DragDropTypes.hpp"
#include "Editor/DragDrop/IDragDropHandler.hpp"
#include "Editor/DragDrop/DragDropRegistry.hpp"
#include "Editor/DragDrop/DragDropManager.hpp"
#include "Events/ApplicationEvent.h"
#include "Events/Event.h"

// -----------------------------------------------------------------------------
// Stress & Benchmark Tests
// -----------------------------------------------------------------------------

TE_STRESS_TEST_CASE(DragDropStress, PayloadEncodingBenchmark)
{
    TEArray<TEString> samplePaths;
    for (int i = 0; i < 50; ++i)
    {
        samplePaths.Add("Assets/Subfolder/Path_Number_" + TEString::FromInt(i) + "/AssetFile.tescene");
    }

    TE_BENCHMARK("Encode_Decode_50_Paths", 10000,
                 [&]()
                 {
                     for (int i = 0; i < 10000; ++i)
                     {
                         auto encoded = DragDropManager::EncodeStringArray(samplePaths);
                         auto decoded = DragDropManager::DecodeStringArray(encoded.GetData(), encoded.Size());
                         TE_CHECK_EQ(decoded.Size(), 50);
                     }
                 });
}

TE_STRESS_TEST_CASE(DragDropStress, EventSpamDispatchStress)
{
    TEArray<TEString> paths;
    paths.Add("C:/Path/To/File1.png");
    paths.Add("C:/Path/To/File2.wav");

    size_t receivedCount = 0;

    TE_BENCHMARK("WindowDropEvent_Dispatch_50000", 50000,
                 [&]()
                 {
                     for (int i = 0; i < 50000; ++i)
                     {
                         WindowDropEvent e(paths, TEVector2(100.0f, 200.0f));
                         EventDispatcher dispatcher(e);
                         dispatcher.Dispatch<WindowDropEvent>(
                             [&](WindowDropEvent &ev) -> bool
                             {
                                 receivedCount++;
                                 return true;
                             });
                     }
                 });

    TE_CHECK_EQ(receivedCount, 50000);
}
