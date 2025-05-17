#include <TimeEngine.h>

class Project : public TimeEngine::Application
{

public:
	Project()
	{
	}
	~Project()
	{
	}

};



TimeEngine::Application* TimeEngine::CreateApplication()
{
	return new Project();
}