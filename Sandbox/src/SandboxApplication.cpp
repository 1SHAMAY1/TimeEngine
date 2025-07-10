#include <Engine.h>
#include "Layers/EditorLayer.hpp"
#include "Layers/LogoLayer.hpp"
#include "Layers/CameraLayer.hpp"
#include "Layers/ProfilingButtonLayer.hpp"
#include "Layers/EngineSettingsButtonLayer.hpp"

class Project : public TE::Application
{
public:
	Project()
	{
		auto* logoLayer = new TE::LogoLayer();
		BIND_FN_MULTI(logoLayer->LogoFinishedDelegate, this, Project::OnLogoComplete);
		PushLayer(logoLayer);
	}

	void OnLogoComplete()
	{
		TE_CORE_INFO("Logo animation finished. Adding CameraLayer and ProfilingButtonLayer after logo removal.");
		
		// Add camera layer
		auto* cameraLayer = new TE::CameraLayer();
		if (cameraLayer)
		{
			MarkLayerForAddition(cameraLayer);
		}
		else
		{
			TE_CORE_ERROR("Failed to create CameraLayer!");
		}
		
		// Add profiling button layer
		auto* profilingButtonLayer = new TE::ProfilingButtonLayer();
		if (profilingButtonLayer)
		{
			MarkLayerForAddition(profilingButtonLayer);
		}
		else
		{
			TE_CORE_ERROR("Failed to create ProfilingButtonLayer!");
		}
		
		// Add engine settings button layer
		auto* engineSettingsButtonLayer = new TE::EngineSettingsButtonLayer();
		if (engineSettingsButtonLayer)
		{
			MarkLayerForAddition(engineSettingsButtonLayer);
		}
		else
		{
			TE_CORE_ERROR("Failed to create EngineSettingsButtonLayer!");
		}
	}

private:
	// No raw layer pointers needed; engine manages layer lifetimes
};

TE::Application* TE::CreateApplication()
{
	return new Project();
}
