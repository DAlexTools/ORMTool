
#include <iostream>
#include "App.h"


int main()
{
	Application app;

	if(app.InitializeApplication() != InitStatus::OK)
	{
		std::cerr << "Failed to initialize Application\n";
		return -1;
	}

	app.RunApplication();

	return 0;
}

