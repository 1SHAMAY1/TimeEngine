#include <Engine.h>

class Project : public TE::Application
{

public:
	Project()
	{
	}
	~Project()
	{
	}

};



TE::Application* TE::CreateApplication()
{
	return new Project();
}