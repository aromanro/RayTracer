#include "RayTracerApp.h"

#include <ctime>

wxIMPLEMENT_APP(RayTracerApp);


bool RayTracerApp::OnInit()
{
	if (!wxApp::OnInit())
		return false;

	
	std::srand(unsigned(std::time(0)));

	wxInitAllImageHandlers();

	options.Load();

	frame = new RayTracerFrame("RayTracer", wxPoint(50, 50), wxSize(1024, 800));
	frame->Show(true);

	return true;
}

int RayTracerApp::OnExit()
{
	return wxApp::OnExit();
}