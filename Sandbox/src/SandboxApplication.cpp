#include <Engine.h>
#include "Layers/EditorLayer.hpp"
#include "Layers/LogoLayer.hpp"
#include "Layers/TestTriangleLayer.hpp"

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
		TE_CORE_INFO("Logo animation finished. Adding EditorLayer after logo removal.");
		TE::EditorLayer* editorLayer = new TE::EditorLayer();
		if (editorLayer)
		{
			MarkLayerForAddition(editorLayer);
		}
		else
		{
			TE_CORE_ERROR("Failed to create EditorLayer!");
		}
	}

private:
	TE::LogoLayer* m_LogoLayer = nullptr;
	TE::EditorLayer* m_EditorLayer = nullptr;

};

TE::Application* TE::CreateApplication()
{
	return new Project();
}
