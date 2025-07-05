#include <Engine.h>
#include "Layers/EditorLayer.hpp"
#include "Layers/LogoLayer.hpp"
#include "Layers/CameraLayer.hpp"
#include "Layers/ProfilingButtonLayer.hpp"

class Project : public TE::Application
{
public:
	Project()
	{
		m_LogoLayer = new TE::LogoLayer();
		BIND_FN_MULTI(m_LogoLayer->LogoFinishedDelegate, this, Project::OnLogoComplete);
		PushLayer(m_LogoLayer);
	}

	void OnLogoComplete()
	{
		TE_CORE_INFO("Logo animation finished. Adding CameraLayer and ProfilingButtonLayer after logo removal.");
		
		// Add camera layer
		TE::CameraLayer* cameraLayer = new TE::CameraLayer();
		if (cameraLayer)
		{
			MarkLayerForAddition(cameraLayer);
		}
		else
		{
			TE_CORE_ERROR("Failed to create CameraLayer!");
		}
		
		// Add profiling button layer
		TE::ProfilingButtonLayer* profilingButtonLayer = new TE::ProfilingButtonLayer();
		if (profilingButtonLayer)
		{
			MarkLayerForAddition(profilingButtonLayer);
		}
		else
		{
			TE_CORE_ERROR("Failed to create ProfilingButtonLayer!");
		}
	}

private:
	TE::LogoLayer* m_LogoLayer = nullptr;
	TE::CameraLayer* m_CameraLayer = nullptr;
	TE::ProfilingButtonLayer* m_ProfilingButtonLayer = nullptr;
};

TE::Application* TE::CreateApplication()
{
	return new Project();
}
