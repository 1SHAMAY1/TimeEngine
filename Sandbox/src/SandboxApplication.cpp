#include <Engine.h>
#include "Layers/EditorLayer.hpp"
#include "Layers/LogoLayer.hpp"
#include "Layers/CameraLayer.hpp"

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
		TE_CORE_INFO("Logo animation finished. Adding CameraLayer after logo removal.");
		TE::CameraLayer* cameraLayer = new TE::CameraLayer();
		if (cameraLayer)
		{
			MarkLayerForAddition(cameraLayer);
		}
		else
		{
			TE_CORE_ERROR("Failed to create CameraLayer!");
		}
	}

private:
	TE::LogoLayer* m_LogoLayer = nullptr;
	TE::CameraLayer* m_CameraLayer = nullptr;
};

TE::Application* TE::CreateApplication()
{
	return new Project();
}
