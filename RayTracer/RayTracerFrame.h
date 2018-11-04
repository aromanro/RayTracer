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


#include <atomic>
#include <vector>

#include "Color.h"

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

	bool isFinished() const;
	void StopThreads(bool cancel = false);
	void Compute();

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

	wxDECLARE_EVENT_TABLE();
};

