#include "RayTracerFrame.h"





//#include "OptionsFrame.h"
#include <wx/rawbmp.h>

#include <thread>


#include <stdio.h>

#include <iostream>
#include <fstream>

#include <memory>

#include "Triangle.h"

#include "ObjLoader.h"

#include "OptionsFrame.h"

#include "wx/aboutdlg.h"
#include "wx/statline.h"
#include "wx/generic/aboutdlgg.h"

#include "RayTracerApp.h"
#include "OptionsFrame.h"



wxDECLARE_APP(RayTracerApp);




#define MY_VTK_WINDOW 102

#define ID_CALCULATE 105
#define ID_SAVE 106

wxBEGIN_EVENT_TABLE(RayTracerFrame, wxFrame)
EVT_MENU(ID_CALCULATE, RayTracerFrame::OnCalculate)
EVT_UPDATE_UI(ID_CALCULATE, RayTracerFrame::OnUpdateCalculate)
EVT_MENU(ID_SAVE, RayTracerFrame::OnSave)
EVT_UPDATE_UI(ID_SAVE, RayTracerFrame::OnUpdateSave)
EVT_MENU(wxID_EXIT, RayTracerFrame::OnExit)
EVT_MENU(wxID_PREFERENCES, RayTracerFrame::OnOptions)
EVT_MENU(wxID_ABOUT, RayTracerFrame::OnAbout)
EVT_TIMER(101, RayTracerFrame::OnTimer)
EVT_PAINT(RayTracerFrame::OnPaint)
wxEND_EVENT_TABLE()


RayTracerFrame::RayTracerFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
	: wxFrame(NULL, wxID_ANY, title, pos, size),
	timer(this, 101), runningThreads(0)
{
	wxMenu *menuFile = new wxMenu;

	menuFile->Append(ID_CALCULATE, "C&alculate\tCtrl+a", "Starts computing");
	menuFile->Append(ID_SAVE, "&Save As\tCtrl+s", "Saves the image into a file");
	menuFile->Append(wxID_SEPARATOR);
	menuFile->Append(wxID_EXIT);

	wxMenu *menuView = new wxMenu;
	menuView->Append(wxID_PREFERENCES);

	wxMenu *menuHelp = new wxMenu;
	menuHelp->Append(wxID_ABOUT);

	wxMenuBar *menuBar = new wxMenuBar;
	menuBar->Append(menuFile, "&File");
	menuBar->Append(menuView, "&View");
	menuBar->Append(menuHelp, "&Help");

	SetMenuBar(menuBar);

	CreateStatusBar();
	SetStatusText("Welcome to RayTracer!");

#ifdef RECORD_MOVIE
	for (int i = 0; i < 15 * 30; ++i)
	{
		wxString fname = wxString::Format("C:\\temp\\frame%06d.jpg", i);
		if (!wxFileName::Exists(fname))
		{
			frameNo = i;
			break;
		}
	}
#endif
}


RayTracerFrame::~RayTracerFrame()
{
}


void RayTracerFrame::OnCalculate(wxCommandEvent& /*event*/)
{
	Compute();
}

void RayTracerFrame::OnUpdateCalculate(wxUpdateUIEvent& event)
{
	event.Enable(isFinished());
}


void RayTracerFrame::OnSave(wxCommandEvent& /*event*/)
{
	wxFileDialog saveFileDialog(this, _("Save File As _?"), wxEmptyString, wxEmptyString, _("PPM files (*.ppm)|*.ppm|Jpeg Files (*.jpg)|*.jpg|PNG files (*.png)|*.png|TIF files (*.tif)|*.tif|TGA files (*.tga)|*.tga"), wxFD_SAVE|wxFD_OVERWRITE_PROMPT);

	if (saveFileDialog.ShowModal() == wxID_OK)
	{
		const wxString path = saveFileDialog.GetPath();
		const wxFileName fileName(path);

		if (fileName.GetExt() == "ppm") SaveToAsPPM(path);
		else bitmap.ConvertToImage().SaveFile(path);
	}
}

void RayTracerFrame::OnUpdateSave(wxUpdateUIEvent& event)
{
	event.Enable(bitmap.IsOk());
}


bool RayTracerFrame::isFinished() const
{
	return 0 == runningThreads;
}

void RayTracerFrame::OnOptions(wxCommandEvent& /*event*/)
{
	RayTracerApp& app = wxGetApp();

	OptionsFrame optionsFrame(app.options, "Options", this);
	if (wxID_OK == optionsFrame.ShowModal())
	{
		app.options = optionsFrame.options;
		app.options.Save();
	}
}

void RayTracerFrame::Compute()
{
	if (!isFinished()) return;

	wxBeginBusyCursor();

	SetTitle("Computing - RayTracer");

	runningThreads = 1;

	timer.Start(100);

	RayTracerApp& app = wxGetApp();
	const Options options = app.options;

	std::thread([this, options]()
	{
		std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

		int nx = 1920;
		const int ny = 1080;
		const int nrThreads = options.nrThreads;
		int samples = options.nrSamples;
		const bool jitter = options.jitter;
		if (jitter)
			samples = ceil(sqrt(samples));

		std::vector<std::future<std::vector<std::vector<Color>>>> tasks(nrThreads);

		Random initialSeed;

		std::vector<Random> randomEngines;
		for (int i = 0; i < nrThreads; ++i)
			randomEngines.emplace_back(Random(static_cast<int>(initialSeed.getZeroOne() * 1E5)));

		Scene scene;
		InitScene(scene, options);

		std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
		double dif = std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count();

		Camera camera;
		InitCamera(camera, options);

		if (1 == options.scene) nx = 1080;
		
		const double distMax = 30000;


		for (int t = 0; t < nrThreads; ++t)
		{
			Random& random = randomEngines[t];
			tasks[t] = std::async(std::launch::async, [samples, jitter, nx, ny, &camera, &scene, &random, distMax]()->std::vector<std::vector<Color>>
			{
				std::vector<std::vector<Color>> results(nx);
				for (int i = 0; i < nx; ++i) results[i].resize(ny);

				if (jitter)
				{
					ComputeJitter(nx, ny, samples, distMax, random, camera, scene, results);
				}
				else
				{
					ComputeNoJitter(nx, ny, samples, distMax, random, camera, scene, results);
				}

				return std::move(results);
			});
		}

		GetResults(tasks, nx, ny, nrThreads, jitter, samples);

		std::chrono::high_resolution_clock::time_point t3 = std::chrono::high_resolution_clock::now();
		dif = std::chrono::duration_cast<std::chrono::seconds>(t3 - t2).count();

		wxString str = wxString::Format("Computation time was %d seconds", static_cast<int>(dif));
		
		SetStatusText(str);

		runningThreads = 0;
	}).detach();
}

void RayTracerFrame::InitCamera(Camera& camera, const Options& options)
{
	if (0 == options.scene)
		camera = Camera(Vector3D<double>(13., 2., 3.), Vector3D<double>(4., 1., 0.), Vector3D<double>(0., 1., 0.), 60, 1080. / 1920., options.depthOfField ? 0.1 : 0., (Vector3D<double>(13., 2., 3.) - Vector3D<double>(4, 1, 0)).Length());
	else if (1 == options.scene)
		camera = Camera(Vector3D<double>(278., 278., -800.), Vector3D<double>(278., 278., 0.), Vector3D<double>(0., 1., 0.), 40, 1., 0., 10);
	else
		camera = Camera(Vector3D<double>(13., 2., 3.), Vector3D<double>(4., 1., 0.), Vector3D<double>(0., 1., 0.), 60, 1080. / 1920., options.depthOfFieldOther ? 0.1 : 0., (Vector3D<double>(13., 2., 3.) - Vector3D<double>(4, 1, 0)).Length());
}

void RayTracerFrame::InitScene(Scene& scene, const Options& options)
{
	scene.recursivityStop = options.recursivityStop;

	if (0 == options.scene)
	{
		scene.blackSky = options.sky == 1;
		FillRandomScene(scene, options);
	}
	else if (1 == options.scene)
	{
		FillCornellScene(scene, options);
	}
	else
	{
		scene.blackSky = options.skyOther == 1;
		FillOtherScene(scene, options);
	}

	scene.ConstructBVH();
}


void RayTracerFrame::ComputeNoJitter(int nx, int ny, int samples, double distMax, Random& random, Camera& camera, Scene& scene, std::vector<std::vector<Color>>& results)
{
	for (int i = 0; i < nx; ++i)
	{
		const double originX = static_cast<double>(i);
		for (int j = 0; j < ny; ++j)
		{
			Color radiance;
			const double originY = static_cast<double>(j);

			for (int s = 0; s < samples; ++s)
			{
				const double x = (originX + random.getZeroOne()) / nx;
				const double y = (originY + random.getZeroOne()) / ny;

				radiance += scene.RayCast(camera.getRay(x, y, random), random, 0, distMax);
			}

			results[i][j] += radiance;
		}
	}
}

void RayTracerFrame::ComputeJitter(int nx, int ny, int samples, double distMax, Random& random, Camera& camera, Scene& scene, std::vector<std::vector<Color>>& results)
{
	const double scaleSamp = 1. / samples;
	const double xScale = 1. / nx;
	const double yScale = 1. / ny;

	for (int i = 0; i < nx; ++i)
	{
		const double originX = static_cast<double>(i);
		for (int j = 0; j < ny; ++j)
		{
			Color radiance;
			const double originY = static_cast<double>(j);

			for (int k = 0; k < samples; ++k)
			{
				const double posX = static_cast<double>(k) * scaleSamp;
				for (int l = 0; l < samples; ++l)
				{
					const double posY = static_cast<double>(l) * scaleSamp;

					const double x = (originX + posX + random.getZeroOne() * scaleSamp) * xScale;
					const double y = (originY + posY + random.getZeroOne() * scaleSamp) * yScale;

					radiance += scene.RayCast(camera.getRay(x, y, random), random, 0, distMax);
				}
			}


			results[i][j] += radiance;
		}
	}
}

void RayTracerFrame::GetResults(std::vector<std::future<std::vector<std::vector<Color>>>>& tasks, int nx, int ny, int nrThreads, bool jitter, int samples)
{
	std::vector<std::vector<Color>> image(nx);
	for (int i = 0; i < nx; ++i) image[i].resize(ny);

	for (auto& task : tasks)
	{
		const auto results = task.get();

		for (int j = 0; j < ny; ++j)
			for (int i = 0; i < nx; ++i)
				image[i][j] += results[i][j];
	}


	if (jitter) samples *= samples;

	const double scale = 1. / (static_cast<double>(samples) * nrThreads);

	for (int i = 0; i < nx; ++i)
		for (int j = 0; j < ny; ++j)
		{
			image[i][j] *= scale;
			image[i][j].Clamp();
		}

	result = image;
}




void RayTracerFrame::OnTimer(wxTimerEvent& WXUNUSED(event))
{
	if (isFinished())
	{
		timer.Stop();
		StopThreads();

		SetTitle("Finished - RayTracer");

		bitmap = wxBitmap(result.size(), result.size() ? result[0].size() : 0, 24);
		
		wxNativePixelData data(bitmap);
		if (!data) return;

		wxNativePixelData::Iterator p(data);
		p.Offset(data, 0, 0);


		int height = bitmap.GetHeight();
		for (int i = height - 1; i >= 0; --i)
		{
			wxNativePixelData::Iterator rowStart = p;

			for (int j = 0; j < bitmap.GetWidth(); ++j, ++p)
			{
				double r = result[j][i].r;
				double g = result[j][i].g;
				double b = result[j][i].b;

				// gamma adjust
				r = sqrt(r);
				g = sqrt(g);
				b = sqrt(b);
				
				p.Red() = int(255.99 * r);
				p.Green() = int(255.99 * g);
				p.Blue() = int(255.99 * b);
			}

			p = rowStart;
			if (0 != i) p.OffsetY(data, 1);
		}


		Refresh();

#ifdef RECORD_MOVIE
		wxString fname = wxString::Format("C:\\temp\\frame%06d.jpg", frameNo);
		bitmap.ConvertToImage().SaveFile(fname);

		// now start next frame computation
		if (++frameNo < 15 * 30)
		{
			// first adjust options for the next frame	
			RayTracerApp& app = wxGetApp();

			app.options.positionXOther -= 0.05;
			app.options.Save();

			Compute();
		}
#endif
	}
}




void RayTracerFrame::OnExit(wxCommandEvent& /*event*/)
{
	Close(true);
}

void RayTracerFrame::OnAbout(wxCommandEvent& /*event*/)
{
	wxAboutDialogInfo info;

	info.SetName("Ray Tracer");

	static const int majorVer = 1;
	static const int minorVer = 0;
	wxString verStr = wxString::Format("%d.%d", majorVer, minorVer);
	info.SetVersion(verStr,	wxString::Format("Version %s", verStr));

	info.SetDescription("   Ray Tracer Application   ");
	info.SetLicense("GNU GPL v3.0, see LICENSE file for details");

	info.AddDeveloper("Adrian Roman");

	info.SetWebSite("https://github.com/aromanro/RayTracer", "GitHub repository");


	wxAboutBox(info, this);	
}


void RayTracerFrame::StopThreads(bool /*cancel*/)
{
	if (wxIsBusy()) wxEndBusyCursor();
}

void RayTracerFrame::OnPaint(wxPaintEvent& WXUNUSED(evt))
{
	wxPaintDC dc(this);

	if (bitmap.IsOk())
		dc.DrawBitmap(bitmap, 10, 10, false);
}


void RayTracerFrame::SaveToAsPPM(const wxString& fileName)
{
	if (result.empty() || result[0].empty()) return;


	std::ofstream f(fileName.char_str());

	const int width = result.size();
	const int height = result[0].size();
	f << "P3\n" << width << " " <<  height << "\n255\n";

	for (int j = height - 1; j >= 0; --j)
		for (int i = 0; i < width; ++i)
		{
			Color totalRadiance = result[i][j];
			
			// gamma adjust
			totalRadiance.r = sqrt(totalRadiance.r);
			totalRadiance.g = sqrt(totalRadiance.g);
			totalRadiance.b = sqrt(totalRadiance.b);

			const int ir = int(255.99 * totalRadiance.r);
			const int ig = int(255.99 * totalRadiance.g);
			const int ib = int(255.99 * totalRadiance.b);

			f << ir << " " << ig << " " << ib << "\n";
		}
}
