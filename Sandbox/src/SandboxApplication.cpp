#include <Engine.h>
#include "Layers/EditorLayer.hpp"
#include "Layers/LogoLayer.hpp"
#include "Layers/TestTriangleLayer.hpp"

class Project : public TE::Application
{
public:
	Project()
	{
		//PushLayer(new TE::TestTriangleLayer());
		m_LogoLayer = new TE::LogoLayer();
		BIND_FN_MULTI(m_LogoLayer->LogoFinishedDelegate, this, Project::OnLogoComplete);
		PushLayer(m_LogoLayer);
	}

	void OnLogoComplete()
	{
		TE_CORE_INFO("Logo animation finished. Now pushing EditorLayer.");
		//PushLayer(new TE::EditorLayer("Editor Layer"));
	}

private:
	TE::LogoLayer* m_LogoLayer = nullptr;
};

TE::Application* TE::CreateApplication()
{
	return new Project();
}
