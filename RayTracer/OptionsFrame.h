#pragma once

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

#include <wx/propdlg.h>


#include "Options.h"

class OptionsFrame : public wxPropertySheetDialog
{
	wxDECLARE_CLASS(OptionsFrame);
public:
	OptionsFrame(const Options& opt, const wxString& title, wxWindow* parent = NULL);

	Options options;
	
protected:

	virtual bool TransferDataFromWindow() override;

	void OnClose(wxCloseEvent& event);
	void OnSkyboxChoose(wxCommandEvent& event);
	void OnSkyboxClear(wxCommandEvent& event);
	void OnSphereTexChoose(wxCommandEvent& event);
	void OnSphereTexClear(wxCommandEvent& event);
	void OnObjectChoose(wxCommandEvent& event);
	void OnObjectClear(wxCommandEvent& event);
	void OnChoiceContent(wxCommandEvent& event);
	void OnSkyboxChooseOther(wxCommandEvent& event);
	void OnSkyboxClearOther(wxCommandEvent& event);
	void OnObjectChooseOther(wxCommandEvent& event);
	void OnObjectClearOther(wxCommandEvent& event);


	wxPanel* CreateGeneralSettingsPage(wxBookCtrlBase* parent);
	wxPanel* CreateInOneWeekendSettingsPage(wxBookCtrlBase* parent);
	wxPanel* CreateCornellBoxSettingsPage(wxBookCtrlBase* parent);
	wxPanel* CreateOtherSettingsPage(wxBookCtrlBase* parent);

	wxDECLARE_EVENT_TABLE();
};

