#pragma once



#define _MATH_DEFINES_DEFINED

#define wxNEEDS_DECL_BEFORE_TEMPLATE

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif

#ifndef M_PI_2
#define M_PI_2     1.57079632679489661923
#endif // !M_PI_2


#ifndef M_1_PI
#define M_1_PI     0.318309886183790671538  // 1/pi
#endif 

#include <atomic>
#include <vector>
#include <future>

#include "Color.h"

#include "Random.h"

#include "Ray.h"
#include "Sphere.h"
#include "Camera.h"
#include "Material.h"
#include "Scene.h"
#include "ImageTexture.h"
#include "Perlin.h"
#include "Rectangle.h"
#include "Actions.h"

#include "Triangle.h"

#include "ObjLoader.h"

#include "Options.h"


//#define RECORD_MOVIE

class RayTracerFrame : public wxFrame
{
public:
	RayTracerFrame(const wxString& title, const wxPoint& pos, const wxSize& size);
	~RayTracerFrame();


	std::atomic_int runningThreads;

private:
	wxTimer timer;

	std::vector<std::vector<Color>> result;
	wxBitmap bitmap;

#ifdef RECORD_MOVIE
	int frameNo = 0;
#endif

	bool isFinished() const;
	void StopThreads(bool cancel = false);

	void Compute();
	
	static void InitScene(Scene& scene, const Options& options);
	static void InitCamera(Camera& camera, const Options& options);
	static void ComputeNoJitter(int nx, int ny, int samples, double distMax, Random& random, Camera& camera, Scene& scene, std::vector<std::vector<Color>>& results);
	static void ComputeJitter(int nx, int ny, int samples, double distMax, Random& random, Camera& camera, Scene& scene, std::vector<std::vector<Color>>& results);

	void GetResults(std::vector<std::future<std::vector<std::vector<Color>>>>& tasks, int nx, int ny, int nrThreads, bool jitter, int samples);

	void OnExit(wxCommandEvent& event);
	void OnOptions(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnTimer(wxTimerEvent& event);


	void OnCalculate(wxCommandEvent& event);
	void OnUpdateCalculate(wxUpdateUIEvent& event);
	void OnSave(wxCommandEvent& event);
	void OnUpdateSave(wxUpdateUIEvent& event);

	void OnPaint(wxPaintEvent& evt);

	void SaveToAsPPM(const wxString& fileName);

	static void FillRandomScene(Scene& scene, const Options& options);
	static void SetSky(Scene& scene, const Options& options);
	static void FillRandomObjects(Scene& scene, const Options& options, Random& random);
	static void AvoidIntersections(Scene& scene, Vector3D<double>& center, double& radius, double choose_obj, double a, double b, Random& random);
	static void AddObject(Scene& scene, const Options& options, Vector3D<double>& center, double radius, double choose_mat, double choose_obj, Random& random);

	static void FillCornellScene(Scene& scene, const Options& options);
	static void AddMirror(Scene& scene, const Options& options);
	static void BuildWalls(Scene& scene, const Options& options, const std::shared_ptr<Materials::Lambertian>& WhiteMaterial, const std::shared_ptr<Materials::Lambertian>& LightMaterial);
	static void SetObj(Scene& scene, const Options& options, const std::shared_ptr<Materials::Metal>& AluminiumMaterial);

	static void FillOtherScene(Scene& scene, const Options& options);
	static void SetSkyOther(Scene& scene, const Options& options);

	wxDECLARE_EVENT_TABLE();
};

