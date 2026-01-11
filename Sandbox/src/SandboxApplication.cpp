#include <Engine.h>
#include "Layers/ProjectHubLayer.hpp"
#include "Layers/EditorLayer.hpp"
#include "Core/Project/Project.hpp"
#include <filesystem>
#include <string>

class Sandbox : public TE::Application
{
public:
	Sandbox(const std::string& startProject)
	{
		TE_CORE_INFO("Sandbox Constructor with args: '{0}'", startProject);

		if (!startProject.empty() && std::filesystem::exists(startProject))
		{
			TE_CORE_INFO("Attempting to load project: {0}", startProject);
			// Load Project Directly
			if (TE::Project::Load(startProject))
			{
				TE_CORE_INFO("Project loaded successfully. Pushing EditorLayer.");
				PushLayer(new TE::EditorLayer());
			}
			else
			{
				TE_CORE_ERROR("Failed to load project from args: {0}. Falling back to Hub.", startProject);
				PushLayer(new TE::ProjectHubLayer());
			}
		}
		else
		{
			TE_CORE_INFO("No valid project argument. Starting Project Hub.");
			// Start with the Project Hub (Launcher)
			PushLayer(new TE::ProjectHubLayer());
		}
	}
};

TE::Application* TE::CreateApplication(int argc, char** argv)
{
	std::string startProject = "";
	if (argc > 1)
	{
		startProject = argv[1];
	}

	return new Sandbox(startProject);
}
