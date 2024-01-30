#define wxNEEDS_DECL_BEFORE_TEMPLATE

#include <wx/statline.h>
#include <wx/valgen.h>
#include <wx/valnum.h>
#include <wx/panel.h>
#include <wx/bookctrl.h>

#include "RayTracerApp.h"
#include "OptionsFrame.h"
#include "RayTracerFrame.h"


#define ID_NRTHREADS 201
#define ID_NRSAMPLES 202
#define ID_RECURSIVE 203
#define ID_SCENE     204
#define ID_JITTER    205

#define ID_DEPTHFIELD 206
#define ID_LOCALILLUM 207
#define ID_SKY 208
#define ID_SKYBOXDIR_CHOOSE 209
#define ID_SKYBOXDIR_CLEAR 210
#define ID_SKYBOXDIR 211
#define ID_SPHERETEX_CHOOSE 212
#define ID_SPHERETEX_CLEAR 213
#define ID_SPHERETEX 214

#define ID_SKYSPHERE_CHOOSE 248
#define ID_SKYSPHERE_CLEAR 249
#define ID_SKYSPHERE 250

#define ID_MIRROR 215
#define ID_MIRROR_SAMPLING 216
#define ID_SMALLOBJ_SAMPLING 217
#define ID_CONTENT 218
#define ID_OBJECT_CHOOSE 219
#define ID_OBJECT_CLEAR 220
#define ID_OBJECT 221
#define ID_SCALE 222
#define ID_ROTATE_X 223
#define ID_ROTATE_Y 224
#define ID_ROTATE_Z 225
#define ID_POSITION_X 226
#define ID_POSITION_Y 227
#define ID_POSITION_Z 228
#define ID_OBJ_MATERIAL 229


#define ID_DEPTHFIELD_OTHER 230
#define ID_LOCALILLUM_OTHER 231
#define ID_SKY_OTHER 232
#define ID_SKYBOXDIR_CHOOSE_OTHER 233
#define ID_SKYBOXDIR_CLEAR_OTHER 234
#define ID_SKYBOXDIR_OTHER 235
#define ID_FLOOR_OTHER 236

#define ID_SKYSPHERE_CHOOSE_OTHER 251
#define ID_SKYSPHERE_CLEAR_OTHER 252
#define ID_SKYSPHERE_OTHER 253


#define ID_OBJECT_CHOOSE_OTHER 237
#define ID_OBJECT_CLEAR_OTHER 238
#define ID_OBJECT_OTHER 239
#define ID_SCALE_OTHER 240
#define ID_ROTATE_X_OTHER 241
#define ID_ROTATE_Y_OTHER 242
#define ID_ROTATE_Z_OTHER 243
#define ID_POSITION_X_OTHER 244
#define ID_POSITION_Y_OTHER 245
#define ID_POSITION_Z_OTHER 246
#define ID_OBJ_MATERIAL_OTHER 247



wxDECLARE_APP(RayTracerApp);


wxIMPLEMENT_CLASS(OptionsFrame, wxPropertySheetDialog);

wxBEGIN_EVENT_TABLE(OptionsFrame, wxPropertySheetDialog)
EVT_CLOSE(OptionsFrame::OnClose)
EVT_BUTTON(ID_SKYBOXDIR_CHOOSE, OptionsFrame::OnSkyboxChoose) 
EVT_BUTTON(ID_SKYBOXDIR_CLEAR, OptionsFrame::OnSkyboxClear) 
EVT_BUTTON(ID_SPHERETEX_CHOOSE, OptionsFrame::OnSphereTexChoose) 
EVT_BUTTON(ID_SPHERETEX_CLEAR, OptionsFrame::OnSphereTexClear) 
EVT_BUTTON(ID_SKYSPHERE_CHOOSE, OptionsFrame::OnSkySphereChoose)
EVT_BUTTON(ID_SKYSPHERE_CLEAR, OptionsFrame::OnSkySphereClear)
EVT_BUTTON(ID_OBJECT_CHOOSE, OptionsFrame::OnObjectChoose)
EVT_BUTTON(ID_OBJECT_CLEAR, OptionsFrame::OnObjectClear) 
EVT_CHOICE(ID_CONTENT, OptionsFrame::OnChoiceContent)
EVT_BUTTON(ID_SKYBOXDIR_CHOOSE_OTHER, OptionsFrame::OnSkyboxChooseOther) 
EVT_BUTTON(ID_SKYBOXDIR_CLEAR_OTHER, OptionsFrame::OnSkyboxClearOther) 
EVT_BUTTON(ID_SKYSPHERE_CHOOSE_OTHER, OptionsFrame::OnSkySphereChooseOther)
EVT_BUTTON(ID_SKYSPHERE_CLEAR_OTHER, OptionsFrame::OnSkySphereClearOther)
EVT_BUTTON(ID_OBJECT_CHOOSE_OTHER, OptionsFrame::OnObjectChooseOther)
EVT_BUTTON(ID_OBJECT_CLEAR_OTHER, OptionsFrame::OnObjectClearOther) 
wxEND_EVENT_TABLE()

OptionsFrame::OptionsFrame(const Options& opt, const wxString& title, wxWindow* parent)
{
	SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY);

	options = opt;
	Create(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize,  wxDEFAULT_DIALOG_STYLE | (int)wxPlatform::IfNot(wxOS_WINDOWS_CE, wxRESIZE_BORDER));

	CreateButtons(wxOK | wxCANCEL);

	wxBookCtrlBase* notebook = GetBookCtrl();


	wxPanel* settingsPage = CreateGeneralSettingsPage(notebook);
	notebook->AddPage(settingsPage, "Computation", true);

	settingsPage = CreateInOneWeekendSettingsPage(notebook);
	notebook->AddPage(settingsPage, "In a Weekend");

	settingsPage = CreateCornellBoxSettingsPage(notebook);
	notebook->AddPage(settingsPage, "Cornell Box");

	settingsPage = CreateOtherSettingsPage(notebook);
	notebook->AddPage(settingsPage, "Other");

	LayoutDialog();
}


bool OptionsFrame::TransferDataFromWindow()
{
	if (!wxDialog::TransferDataFromWindow()) return false;
	
	return true;
}

void OptionsFrame::OnClose(wxCloseEvent& event)
{
	event.Skip();
}



wxPanel* OptionsFrame::CreateGeneralSettingsPage(wxBookCtrlBase* parent)
{
	wxPanel* panel = new wxPanel(parent, wxID_ANY | wxGROW);


	wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *item0 = new wxBoxSizer(wxVERTICAL);

	// ***********************************************************************************

	wxBoxSizer* itemSizer = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText* label = new wxStaticText(panel, wxID_STATIC, "Threads:", wxDefaultPosition, wxSize(100, -1), wxALIGN_RIGHT);
	itemSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL, 5);

	wxString str = wxString::Format(wxT("%i"), options.nrThreads);

	wxTextCtrl* nrThreadsCtrl = new wxTextCtrl(panel, ID_NRTHREADS, str, wxDefaultPosition, wxSize(60, -1), 0);
	itemSizer->Add(nrThreadsCtrl, 0, wxALIGN_CENTER_VERTICAL, 5);

	item0->Add(itemSizer, 0, wxGROW, 0);


	
	itemSizer = new wxBoxSizer(wxHORIZONTAL);

	label = new wxStaticText(panel, wxID_STATIC, "Rays/pixel/thread:", wxDefaultPosition, wxSize(100, -1), wxALIGN_RIGHT);
	itemSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL, 5);

	str = wxString::Format(wxT("%i"), options.nrSamples);
	wxTextCtrl* nrSamplesCtrl = new wxTextCtrl(panel, ID_NRSAMPLES, str, wxDefaultPosition, wxSize(60, -1), 0);
	itemSizer->Add(nrSamplesCtrl, 0, wxALIGN_CENTER_VERTICAL, 5);

	item0->Add(itemSizer, 0, wxGROW, 0);



	itemSizer = new wxBoxSizer(wxHORIZONTAL);

	label = new wxStaticText(panel, wxID_STATIC, "Recursivity stop:", wxDefaultPosition, wxSize(100, -1), wxALIGN_RIGHT);
	itemSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL, 5);

	str = wxString::Format(wxT("%i"), options.recursivityStop);
	wxTextCtrl* nrRecursiveCtrl = new wxTextCtrl(panel, ID_RECURSIVE, str, wxDefaultPosition, wxSize(60, -1), 0);
	itemSizer->Add(nrRecursiveCtrl, 0, wxALIGN_CENTER_VERTICAL, 5);

	item0->Add(itemSizer, 0, wxGROW, 0);

	itemSizer = new wxBoxSizer(wxHORIZONTAL);

	label = new wxStaticText(panel, wxID_STATIC, "Scene:", wxDefaultPosition, wxSize(100, -1), wxALIGN_RIGHT);
	itemSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL, 5);

	const wxString strings[3] = { "In a Weekend", "Cornell Box", "Other" };

	wxChoice* sceneChoice = new wxChoice(panel, ID_SCENE, wxDefaultPosition, wxSize(100, -1), 3, strings, 0);
	sceneChoice->SetSelection(options.scene);
	itemSizer->Add(sceneChoice, 1, wxALIGN_CENTER_VERTICAL, 5);

	item0->Add(itemSizer, 0, wxGROW, 0);

	itemSizer = new wxBoxSizer(wxHORIZONTAL);

	wxCheckBox *checkBox = new wxCheckBox(panel, ID_JITTER, "Jitter (will round up rays/pixel/thread to a square number)");
	itemSizer->Add(checkBox, 0, wxALIGN_CENTER_VERTICAL);

	item0->Add(itemSizer, 0, wxGROW, 0);


	// Validators

	wxIntegerValidator<int> val1(&options.nrThreads, wxNUM_VAL_DEFAULT);
	val1.SetRange(1, 64);
	nrThreadsCtrl->SetValidator(val1);


	wxIntegerValidator<int> val2(&options.nrSamples, wxNUM_VAL_DEFAULT);
	val2.SetRange(1, 10000);
	nrSamplesCtrl->SetValidator(val2);

	wxIntegerValidator<int> val3(&options.recursivityStop, wxNUM_VAL_DEFAULT);
	val3.SetRange(1, 100);
	nrRecursiveCtrl->SetValidator(val3);

	sceneChoice->SetValidator(wxGenericValidator(&options.scene));

	checkBox->SetValidator(wxGenericValidator(&options.jitter));

	// *************************

	topSizer->Add(item0, 0, wxALL | wxGROW, 5);
	panel->SetSizerAndFit(topSizer);


	return panel;
}



wxPanel* OptionsFrame::CreateInOneWeekendSettingsPage(wxBookCtrlBase* parent)
{
	wxPanel* panel = new wxPanel(parent, wxID_ANY | wxGROW);

	wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *item0 = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer* itemSizer = new wxBoxSizer(wxHORIZONTAL);

	wxCheckBox *checkBoxDepth = new wxCheckBox(panel, ID_DEPTHFIELD, "Depth of field (non zero aperture)");
	itemSizer->Add(checkBoxDepth, 0, wxALIGN_CENTER_VERTICAL);

	item0->Add(itemSizer, 0, wxGROW, 0);

	itemSizer = new wxBoxSizer(wxHORIZONTAL);

	wxCheckBox *checkBoxLocal = new wxCheckBox(panel, ID_LOCALILLUM, "Local Illumination");
	itemSizer->Add(checkBoxLocal, 0, wxALIGN_CENTER_VERTICAL);

	item0->Add(itemSizer, 0, wxGROW, 0);

	itemSizer = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText* label = new wxStaticText(panel, wxID_STATIC, "Sky:", wxDefaultPosition, wxSize(50, -1), wxALIGN_RIGHT);
	itemSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL, 5);

	const wxString strings[] = { "Blue", "Black", "Sky Box", "Sky Sphere" };

	wxChoice* skyChoice = new wxChoice(panel, ID_SKY, wxDefaultPosition, wxSize(60, -1), sizeof(strings) / sizeof(wxString), strings, 0);
	skyChoice->SetSelection(options.sky);
	itemSizer->Add(skyChoice, 1, wxALIGN_CENTER_VERTICAL, 5);

	item0->Add(itemSizer, 0, wxGROW, 0);


	itemSizer = new wxBoxSizer(wxHORIZONTAL);

	label = new wxStaticText(panel, wxID_STATIC, "Sky Box:", wxDefaultPosition, wxSize(50, -1), wxALIGN_RIGHT);
	itemSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL, 5);

	wxButton* button = new wxButton(panel, ID_SKYBOXDIR_CHOOSE, "Choose", wxDefaultPosition, wxSize(70, -1));
	itemSizer->Add(button, 0, wxALIGN_CENTER_VERTICAL, 5);

	button = new wxButton(panel, ID_SKYBOXDIR_CLEAR, "Clear", wxDefaultPosition, wxSize(70, -1));
	itemSizer->Add(button, 0, wxALIGN_CENTER_VERTICAL, 5);


	wxTextCtrl* editSkyBox = new wxTextCtrl(panel, ID_SKYBOXDIR, options.skyBoxDirName, wxDefaultPosition, wxSize(100, -1), wxTE_READONLY | wxALIGN_LEFT);
	
	itemSizer->Add(editSkyBox, 1, wxALIGN_CENTER_VERTICAL, 5);

	item0->Add(itemSizer, 1, wxALL | wxGROW, 0);


	itemSizer = new wxBoxSizer(wxHORIZONTAL);

	label = new wxStaticText(panel, wxID_STATIC, "Sky Sphere:", wxDefaultPosition, wxSize(50, -1), wxALIGN_RIGHT);
	itemSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL, 5);

	button = new wxButton(panel, ID_SKYSPHERE_CHOOSE, "Choose", wxDefaultPosition, wxSize(70, -1));
	itemSizer->Add(button, 0,wxALIGN_CENTER_VERTICAL, 5);

	button = new wxButton(panel, ID_SKYSPHERE_CLEAR, "Clear", wxDefaultPosition, wxSize(70, -1));
	itemSizer->Add(button, 0, wxALIGN_CENTER_VERTICAL, 5);


	wxTextCtrl* editSkySphere = new wxTextCtrl(panel, ID_SKYSPHERE, options.skySphereFileName, wxDefaultPosition, wxSize(100, -1), wxTE_READONLY | wxALIGN_LEFT);

	itemSizer->Add(editSkySphere, 1, wxGROW, 5);

	item0->Add(itemSizer, 1, wxGROW, 0);



	itemSizer = new wxBoxSizer(wxHORIZONTAL);

	label = new wxStaticText(panel, wxID_STATIC, "Texture:", wxDefaultPosition, wxSize(50, -1), wxALIGN_RIGHT);
	itemSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL, 5);

	button = new wxButton(panel, ID_SPHERETEX_CHOOSE, "Choose", wxDefaultPosition, wxSize(70, -1));
	itemSizer->Add(button, 0, wxALIGN_CENTER_VERTICAL, 5);

	button = new wxButton(panel, ID_SPHERETEX_CLEAR, "Clear", wxDefaultPosition, wxSize(70, -1));
	itemSizer->Add(button, 0, wxALIGN_CENTER_VERTICAL, 5);


	wxTextCtrl* editTexture = new wxTextCtrl(panel, ID_SPHERETEX, options.textureFileName, wxDefaultPosition, wxSize(100, -1), wxTE_READONLY | wxALIGN_LEFT);
	
	itemSizer->Add(editTexture, 1, wxGROW, 5);

	item0->Add(itemSizer, 1, wxGROW, 0);

	itemSizer = new wxBoxSizer(wxHORIZONTAL);
	label = new wxStaticText(panel, wxID_STATIC, "The texture is for the big Lambertian sphere", wxDefaultPosition, wxSize(250, -1), wxALIGN_LEFT);
	itemSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL, 5);
	item0->Add(itemSizer, 1, wxALL | wxGROW, 0);

	// validators

	checkBoxDepth->SetValidator(wxGenericValidator(&options.depthOfField));
	checkBoxLocal->SetValidator(wxGenericValidator(&options.localIllumination));

	skyChoice->SetValidator(wxGenericValidator(&options.sky));

	editSkyBox->SetValidator(wxGenericValidator(&options.skyBoxDirName));
	editSkySphere->SetValidator(wxGenericValidator(&options.skySphereFileName));
	editTexture->SetValidator(wxGenericValidator(&options.textureFileName));

	topSizer->Add(item0, 0, wxALL | wxGROW, 5);
	panel->SetSizerAndFit(topSizer);

	return panel;
}



void OptionsFrame::OnSkyboxChoose(wxCommandEvent& /*event*/)
{
	TransferDataFromWindow();
	wxDirDialog dlg(this, "Choose Sky Box directory", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

	if (wxID_OK == dlg.ShowModal())
	{
		const wxString str = dlg.GetPath() + "\\";

		if ((wxFileName::Exists(str + "left.jpg") && wxFileName::Exists(str + "right.jpg") && wxFileName::Exists(str + "top.jpg") && wxFileName::Exists(str + "bottom.jpg") && wxFileName::Exists(str + "back.jpg") && wxFileName::Exists(str + "front.jpg")) ||
			(wxFileName::Exists(str + "left.png") && wxFileName::Exists(str + "right.png") && wxFileName::Exists(str + "top.png") && wxFileName::Exists(str + "bottom.png") && wxFileName::Exists(str + "back.png") && wxFileName::Exists(str + "front.png")))
		{
			options.skyBoxDirName = str;
			TransferDataToWindow();
		}			
	}
}

void OptionsFrame::OnSkyboxClear(wxCommandEvent& /*event*/)
{
	TransferDataFromWindow();
	options.skyBoxDirName.Clear();
	TransferDataToWindow();
}

void OptionsFrame::OnSkySphereChoose(wxCommandEvent& /*event*/)
{
	TransferDataFromWindow();
	wxFileDialog openFileDialog(this, _("Open File As _?"), wxEmptyString, wxEmptyString, _("Jpeg Files (*.jpg)|*.jpg|PNG files (*.png)|*.png|TIF files (*.tif)|*.tif|TGA files (*.tga)|*.tga|All files (*.*)|*.*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (openFileDialog.ShowModal() == wxID_OK)
	{
		const wxString path = openFileDialog.GetPath();
		const wxFileName fileName(path);

		if (fileName.Exists())
		{
			options.skySphereFileName = path;
			TransferDataToWindow();
		}
	}
}

void OptionsFrame::OnSkySphereClear(wxCommandEvent& /*event*/)
{
	TransferDataFromWindow();
	options.skySphereFileName.Clear();
	TransferDataToWindow();
}


void OptionsFrame::OnSphereTexChoose(wxCommandEvent& /*event*/)
{
	TransferDataFromWindow();
	wxFileDialog openFileDialog(this, _("Open File As _?"), wxEmptyString, wxEmptyString, _("Jpeg Files (*.jpg)|*.jpg|PNG files (*.png)|*.png|TIF files (*.tif)|*.tif|TGA files (*.tga)|*.tga|All files (*.*)|*.*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (openFileDialog.ShowModal() == wxID_OK)
	{
		const wxString path = openFileDialog.GetPath();
		const wxFileName fileName(path);

		if (fileName.Exists())
		{
			options.textureFileName = path;
			TransferDataToWindow();
		}
	}
}

void OptionsFrame::OnSphereTexClear(wxCommandEvent& /*event*/)
{
	TransferDataFromWindow();
	options.textureFileName.Clear();
	TransferDataToWindow();
}


wxPanel* OptionsFrame::CreateCornellBoxSettingsPage(wxBookCtrlBase* parent)
{
	wxPanel* panel = new wxPanel(parent, wxID_ANY | wxGROW);

	wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *item0 = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer* itemSizer = new wxBoxSizer(wxHORIZONTAL);

	wxCheckBox *checkBoxMirror = new wxCheckBox(panel, ID_MIRROR, "Mirror on the wall");
	itemSizer->Add(checkBoxMirror, 0, wxALIGN_CENTER_VERTICAL);
	item0->Add(itemSizer, 0, wxGROW, 0);

	itemSizer = new wxBoxSizer(wxHORIZONTAL);
	wxCheckBox *checkBoxMirrorSampling = new wxCheckBox(panel, ID_MIRROR_SAMPLING, "Importance Sampling for Mirror");
	itemSizer->Add(checkBoxMirrorSampling, 0, wxALIGN_CENTER_VERTICAL);
	item0->Add(itemSizer, 0, wxGROW, 0);

	itemSizer = new wxBoxSizer(wxHORIZONTAL);
	wxCheckBox *checkBoxObjSampling = new wxCheckBox(panel, ID_SMALLOBJ_SAMPLING, "Importance Sampling for small object (non composite only, that is, the sphere)");
	itemSizer->Add(checkBoxObjSampling, 0, wxALIGN_CENTER_VERTICAL);
	item0->Add(itemSizer, 0, wxGROW, 0);

	checkBoxObjSampling->Enable(3 == options.boxContent);


	itemSizer = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText* label = new wxStaticText(panel, wxID_STATIC, "Content:", wxDefaultPosition, wxSize(50, -1), wxALIGN_RIGHT);
	itemSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL, 5);

	const wxString strings[5] = { "Two 'white' boxes", "One metal box, one 'white'", "Two 'mist' boxes", "One 'white' box, one transparent sphere", "Object loaded from file" };

	wxChoice* contentChoice = new wxChoice(panel, ID_CONTENT, wxDefaultPosition, wxSize(60, -1), 5, strings, 0);
	contentChoice->SetSelection(options.boxContent);

	if (3 != options.boxContent) options.addSmallObjectToImportanceSampling = false;

	itemSizer->Add(contentChoice, 1, wxGROW, 5);

	item0->Add(itemSizer, 0, wxGROW, 0);


	itemSizer = new wxBoxSizer(wxHORIZONTAL);

	label = new wxStaticText(panel, wxID_STATIC, "Object:", wxDefaultPosition, wxSize(50, -1), wxALIGN_RIGHT);
	itemSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL, 5);

	wxButton* button = new wxButton(panel, ID_OBJECT_CHOOSE, "Choose", wxDefaultPosition, wxSize(70, -1));
	itemSizer->Add(button, 0, wxALIGN_CENTER_VERTICAL, 5);

	button = new wxButton(panel, ID_OBJECT_CLEAR, "Clear", wxDefaultPosition, wxSize(70, -1));
	itemSizer->Add(button, 0, wxALIGN_CENTER_VERTICAL, 5);


	wxTextCtrl* editObj = new wxTextCtrl(panel, ID_OBJECT, options.objFileName, wxDefaultPosition, wxSize(100, -1), wxTE_READONLY | wxALIGN_LEFT);
	
	itemSizer->Add(editObj, 1, wxGROW, 5);

	item0->Add(itemSizer, 0, wxGROW, 0);

	itemSizer = new wxBoxSizer(wxHORIZONTAL);
	label = new wxStaticText(panel, wxID_STATIC, "Scale:", wxDefaultPosition, wxSize(100, -1), wxALIGN_RIGHT);
	itemSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL, 5);
	
	wxString str = wxString::Format(wxT("%g"), options.scale);
	wxTextCtrl* scaleCtrl = new wxTextCtrl(panel, ID_SCALE, str, wxDefaultPosition, wxSize(100, -1), 0 );
	itemSizer->Add(scaleCtrl, 0, wxGROW, 5);
	
	item0->Add(itemSizer, 0, wxGROW, 0);


	itemSizer = new wxBoxSizer(wxHORIZONTAL);
	label = new wxStaticText(panel, wxID_STATIC, "Rotate around Ox:", wxDefaultPosition, wxSize(100, -1), wxALIGN_RIGHT);
	itemSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL, 5);
	
	str = wxString::Format(wxT("%d"), options.rotateX);
	wxTextCtrl* rotxCtrl = new wxTextCtrl(panel, ID_ROTATE_X, str, wxDefaultPosition, wxSize(60, -1), 0 );
	itemSizer->Add(rotxCtrl, 0, wxGROW, 5);
	
	label = new wxStaticText(panel, wxID_STATIC, "X Position:", wxDefaultPosition, wxSize(100, -1), wxALIGN_RIGHT);
	itemSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL, 5);

	str = wxString::Format(wxT("%d"), options.positionX);
	wxTextCtrl* posxCtrl = new wxTextCtrl(panel, ID_POSITION_X, str, wxDefaultPosition, wxSize(60, -1), 0 );
	itemSizer->Add(posxCtrl, 0, wxGROW, 5);

	item0->Add(itemSizer, 0, wxGROW, 0);



	itemSizer = new wxBoxSizer(wxHORIZONTAL);
	label = new wxStaticText(panel, wxID_STATIC, "Rotate around Oy:", wxDefaultPosition, wxSize(100, -1), wxALIGN_RIGHT);
	itemSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL, 5);
	
	str = wxString::Format(wxT("%d"), options.rotateY);
	wxTextCtrl* rotyCtrl = new wxTextCtrl(panel, ID_ROTATE_Y, str, wxDefaultPosition, wxSize(60, -1), 0 );
	itemSizer->Add(rotyCtrl, 0, wxGROW, 5);

	label = new wxStaticText(panel, wxID_STATIC, "Y Position:", wxDefaultPosition, wxSize(100, -1), wxALIGN_RIGHT);
	itemSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL, 5);

	str = wxString::Format(wxT("%d"), options.positionY);
	wxTextCtrl* posyCtrl = new wxTextCtrl(panel, ID_POSITION_Y, str, wxDefaultPosition, wxSize(60, -1), 0 );
	itemSizer->Add(posyCtrl, 0, wxGROW, 5);


	item0->Add(itemSizer, 0, wxGROW, 0);


	itemSizer = new wxBoxSizer(wxHORIZONTAL);
	label = new wxStaticText(panel, wxID_STATIC, "Rotate around Oz:", wxDefaultPosition, wxSize(100, -1), wxALIGN_RIGHT);
	itemSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL, 5);
	
	str = wxString::Format(wxT("%d"), options.rotateZ);
	wxTextCtrl* rotzCtrl = new wxTextCtrl(panel, ID_ROTATE_Z, str, wxDefaultPosition, wxSize(60, -1), 0 );
	itemSizer->Add(rotzCtrl, 0, wxGROW, 5);
	
	label = new wxStaticText(panel, wxID_STATIC, "Z Position:", wxDefaultPosition, wxSize(100, -1), wxALIGN_RIGHT);
	itemSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL, 5);

	str = wxString::Format(wxT("%d"), options.positionZ);
	wxTextCtrl* poszCtrl = new wxTextCtrl(panel, ID_POSITION_Z, str, wxDefaultPosition, wxSize(60, -1), 0 );
	itemSizer->Add(poszCtrl, 0, wxGROW, 5);

	item0->Add(itemSizer, 0, wxGROW, 0);


	itemSizer = new wxBoxSizer(wxHORIZONTAL);

	label = new wxStaticText(panel, wxID_STATIC, "Material:", wxDefaultPosition, wxSize(50, -1), wxALIGN_RIGHT);
	itemSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL, 5);

	const wxString matstrings[3] = { "Keep original", "Make metallic", "Make transparent" };

	wxChoice* matChoice = new wxChoice(panel, ID_OBJ_MATERIAL, wxDefaultPosition, wxSize(60, -1), 3, matstrings, 0);
	matChoice->SetSelection(options.objMaterial);
	itemSizer->Add(matChoice, 1, wxGROW, 5);

	item0->Add(itemSizer, 0, wxGROW, 0);

	// validators

	checkBoxMirror->SetValidator(wxGenericValidator(&options.mirrorOnWall));
	checkBoxMirrorSampling->SetValidator(wxGenericValidator(&options.addMirrorToImportanceSampling));
	checkBoxObjSampling->SetValidator(wxGenericValidator(&options.addSmallObjectToImportanceSampling));
	contentChoice->SetValidator(wxGenericValidator(&options.boxContent));
	editObj->SetValidator(wxGenericValidator(&options.objFileName));

	wxFloatingPointValidator<double> dblVal(&options.scale);
	dblVal.SetRange(0, 1000000);
	dblVal.SetPrecision(3);
	scaleCtrl->SetValidator(dblVal);


	wxIntegerValidator<int> val1(&options.rotateX, wxNUM_VAL_DEFAULT);
	val1.SetRange(-360, 360);
	rotxCtrl->SetValidator(val1);

	wxIntegerValidator<int> val2(&options.rotateY, wxNUM_VAL_DEFAULT);
	val2.SetRange(-360, 360);
	rotyCtrl->SetValidator(val2);

	wxIntegerValidator<int> val3(&options.rotateZ, wxNUM_VAL_DEFAULT);
	val3.SetRange(-360, 360);
	rotzCtrl->SetValidator(val3);

	wxIntegerValidator<int> v1(&options.positionX, wxNUM_VAL_DEFAULT);
	v1.SetRange(0, 555);
	posxCtrl->SetValidator(v1);

	wxIntegerValidator<int> v2(&options.positionY, wxNUM_VAL_DEFAULT);
	v2.SetRange(0, 555);
	posyCtrl->SetValidator(v2);

	wxIntegerValidator<int> v3(&options.positionZ, wxNUM_VAL_DEFAULT);
	v3.SetRange(0, 555);
	poszCtrl->SetValidator(v3);

	matChoice->SetValidator(wxGenericValidator(&options.objMaterial));


	topSizer->Add(item0, 0, wxALL | wxGROW, 5);
	panel->SetSizerAndFit(topSizer);

	return panel;
}


void OptionsFrame::OnChoiceContent(wxCommandEvent& /*event*/)
{
	TransferDataFromWindow();

	wxCheckBox* check = (wxCheckBox*)FindWindow(ID_SMALLOBJ_SAMPLING);
	if (3 == options.boxContent)
	{
		if (check) check->Enable();
	}
	else
	{
		if (check) check->Enable(false);
		options.addSmallObjectToImportanceSampling = false;
	}

	TransferDataToWindow();
}



void OptionsFrame::OnObjectChoose(wxCommandEvent& /*event*/)
{
	TransferDataFromWindow();
	wxFileDialog openFileDialog(this, _("Open File As _?"), wxEmptyString, wxEmptyString, _("Obj Files (*.obj)|*.obj"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (openFileDialog.ShowModal() == wxID_OK)
	{
		const wxString path = openFileDialog.GetPath();
		const wxFileName fileName(path);

		if (fileName.Exists())
		{
			options.objFileName = path;
			TransferDataToWindow();
		}
	}
}

void OptionsFrame::OnObjectClear(wxCommandEvent& /*event*/)
{
	TransferDataFromWindow();
	options.objFileName.Clear();
	TransferDataToWindow();
}


wxPanel* OptionsFrame::CreateOtherSettingsPage(wxBookCtrlBase* parent)
{
	wxPanel* panel = new wxPanel(parent, wxID_ANY | wxGROW);

	wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *item0 = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer* itemSizer = new wxBoxSizer(wxHORIZONTAL);

	wxCheckBox *checkBoxDepth = new wxCheckBox(panel, ID_DEPTHFIELD_OTHER, "Depth of field (non zero aperture)");
	itemSizer->Add(checkBoxDepth, 0, wxALIGN_CENTER_VERTICAL);

	item0->Add(itemSizer, 0, wxGROW, 0);

	itemSizer = new wxBoxSizer(wxHORIZONTAL);

	wxCheckBox *checkBoxLocal = new wxCheckBox(panel, ID_LOCALILLUM_OTHER, "Local Illumination");
	itemSizer->Add(checkBoxLocal, 0, wxALIGN_CENTER_VERTICAL);

	item0->Add(itemSizer, 0, wxGROW, 0);

	itemSizer = new wxBoxSizer(wxHORIZONTAL);

	wxCheckBox *checkBoxFloor = new wxCheckBox(panel, ID_FLOOR_OTHER, "Floor");
	itemSizer->Add(checkBoxFloor, 0, wxALIGN_CENTER_VERTICAL);

	item0->Add(itemSizer, 0, wxGROW, 0);

	itemSizer = new wxBoxSizer(wxHORIZONTAL);

	wxStaticText* label = new wxStaticText(panel, wxID_STATIC, "Sky:", wxDefaultPosition, wxSize(50, -1), wxALIGN_RIGHT);
	itemSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL, 5);

	const wxString strings[] = { "Blue", "Black", "Sky Box", "Sky Sphere" };

	wxChoice* skyChoice = new wxChoice(panel, ID_SKY, wxDefaultPosition, wxSize(60, -1), sizeof(strings) / sizeof(wxString), strings, 0);
	skyChoice->SetSelection(options.skyOther);
	itemSizer->Add(skyChoice, 1, wxGROW, 5);

	item0->Add(itemSizer, 0, wxGROW, 0);


	itemSizer = new wxBoxSizer(wxHORIZONTAL);

	label = new wxStaticText(panel, wxID_STATIC, "Sky Box:", wxDefaultPosition, wxSize(50, -1), wxALIGN_RIGHT);
	itemSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL, 5);

	wxButton* button = new wxButton(panel, ID_SKYBOXDIR_CHOOSE_OTHER, "Choose", wxDefaultPosition, wxSize(70, -1));
	itemSizer->Add(button, 0, wxALIGN_CENTER_VERTICAL, 5);

	button = new wxButton(panel, ID_SKYBOXDIR_CLEAR_OTHER, "Clear", wxDefaultPosition, wxSize(70, -1));
	itemSizer->Add(button, 0, wxALIGN_CENTER_VERTICAL, 5);


	wxTextCtrl* editSkyBox = new wxTextCtrl(panel, ID_SKYBOXDIR_OTHER, options.skyBoxDirNameOther, wxDefaultPosition, wxSize(100, -1), wxTE_READONLY | wxALIGN_LEFT);
	
	itemSizer->Add(editSkyBox, 1, wxGROW, 5);

	item0->Add(itemSizer, 1, wxGROW, 0);


	itemSizer = new wxBoxSizer(wxHORIZONTAL);

	label = new wxStaticText(panel, wxID_STATIC, "Sky Sphere:", wxDefaultPosition, wxSize(50, -1), wxALIGN_RIGHT);
	itemSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL, 5);

	button = new wxButton(panel, ID_SKYSPHERE_CHOOSE_OTHER, "Choose", wxDefaultPosition, wxSize(70, -1));
	itemSizer->Add(button, 0, wxALIGN_CENTER_VERTICAL, 5);

	button = new wxButton(panel, ID_SKYSPHERE_CLEAR_OTHER, "Clear", wxDefaultPosition, wxSize(70, -1));
	itemSizer->Add(button, 0, wxALIGN_CENTER_VERTICAL, 5);


	wxTextCtrl* editSkySphere = new wxTextCtrl(panel, ID_SKYSPHERE_OTHER, options.skySphereFileNameOther, wxDefaultPosition, wxSize(100, -1), wxTE_READONLY | wxALIGN_LEFT);

	itemSizer->Add(editSkySphere, 1, wxGROW, 5);

	item0->Add(itemSizer, 1, wxGROW, 0);




	itemSizer = new wxBoxSizer(wxHORIZONTAL);

	label = new wxStaticText(panel, wxID_STATIC, "Object:", wxDefaultPosition, wxSize(50, -1), wxALIGN_RIGHT);
	itemSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL, 5);

	button = new wxButton(panel, ID_OBJECT_CHOOSE_OTHER, "Choose", wxDefaultPosition, wxSize(70, -1));
	itemSizer->Add(button, 0, wxALIGN_CENTER_VERTICAL, 5);

	button = new wxButton(panel, ID_OBJECT_CLEAR_OTHER, "Clear", wxDefaultPosition, wxSize(70, -1));
	itemSizer->Add(button, 0, wxALIGN_CENTER_VERTICAL, 5);


	wxTextCtrl* editObj = new wxTextCtrl(panel, ID_OBJECT_OTHER, options.objFileNameOther, wxDefaultPosition, wxSize(100, -1), wxTE_READONLY | wxALIGN_LEFT);
	
	itemSizer->Add(editObj, 1, wxGROW, 5);

	item0->Add(itemSizer, 0, wxGROW, 0);

	itemSizer = new wxBoxSizer(wxHORIZONTAL);
	label = new wxStaticText(panel, wxID_STATIC, "Scale:", wxDefaultPosition, wxSize(100, -1), wxALIGN_RIGHT);
	itemSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL, 5);
	
	wxString str = wxString::Format(wxT("%g"), options.scaleOther);
	wxTextCtrl* scaleCtrl = new wxTextCtrl(panel, ID_SCALE_OTHER, str, wxDefaultPosition, wxSize(100, -1), 0 );
	itemSizer->Add(scaleCtrl, 0, wxGROW, 5);
	
	item0->Add(itemSizer, 0, wxGROW, 0);


	itemSizer = new wxBoxSizer(wxHORIZONTAL);
	label = new wxStaticText(panel, wxID_STATIC, "Rotate around Ox:", wxDefaultPosition, wxSize(100, -1), wxALIGN_RIGHT);
	itemSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL, 5);
	
	str = wxString::Format(wxT("%d"), options.rotateXOther);
	wxTextCtrl* rotxCtrl = new wxTextCtrl(panel, ID_ROTATE_X_OTHER, str, wxDefaultPosition, wxSize(60, -1), 0 );
	itemSizer->Add(rotxCtrl, 0, wxGROW, 5);
	
	label = new wxStaticText(panel, wxID_STATIC, "X Position:", wxDefaultPosition, wxSize(100, -1), wxALIGN_RIGHT);
	itemSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL, 5);

	str = wxString::Format(wxT("%g"), options.positionXOther);
	wxTextCtrl* posxCtrl = new wxTextCtrl(panel, ID_POSITION_X_OTHER, str, wxDefaultPosition, wxSize(60, -1), 0 );
	itemSizer->Add(posxCtrl, 0, wxGROW, 5);

	item0->Add(itemSizer, 0, wxGROW, 0);



	itemSizer = new wxBoxSizer(wxHORIZONTAL);
	label = new wxStaticText(panel, wxID_STATIC, "Rotate around Oy:", wxDefaultPosition, wxSize(100, -1), wxALIGN_RIGHT);
	itemSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL, 5);
	
	str = wxString::Format(wxT("%d"), options.rotateYOther);
	wxTextCtrl* rotyCtrl = new wxTextCtrl(panel, ID_ROTATE_Y_OTHER, str, wxDefaultPosition, wxSize(60, -1), 0 );
	itemSizer->Add(rotyCtrl, 0, wxGROW, 5);

	label = new wxStaticText(panel, wxID_STATIC, "Y Position:", wxDefaultPosition, wxSize(100, -1), wxALIGN_RIGHT);
	itemSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL, 5);

	str = wxString::Format(wxT("%g"), options.positionYOther);
	wxTextCtrl* posyCtrl = new wxTextCtrl(panel, ID_POSITION_Y_OTHER, str, wxDefaultPosition, wxSize(60, -1), 0 );
	itemSizer->Add(posyCtrl, 0, wxGROW, 5);


	item0->Add(itemSizer, 0, wxGROW, 0);


	itemSizer = new wxBoxSizer(wxHORIZONTAL);
	label = new wxStaticText(panel, wxID_STATIC, "Rotate around Oz:", wxDefaultPosition, wxSize(100, -1), wxALIGN_RIGHT);
	itemSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL, 5);
	
	str = wxString::Format(wxT("%d"), options.rotateZOther);
	wxTextCtrl* rotzCtrl = new wxTextCtrl(panel, ID_ROTATE_Z_OTHER, str, wxDefaultPosition, wxSize(60, -1), 0 );
	itemSizer->Add(rotzCtrl, 0, wxGROW, 5);
	
	label = new wxStaticText(panel, wxID_STATIC, "Z Position:", wxDefaultPosition, wxSize(100, -1), wxALIGN_RIGHT);
	itemSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL, 5);

	str = wxString::Format(wxT("%g"), options.positionZOther);
	wxTextCtrl* poszCtrl = new wxTextCtrl(panel, ID_POSITION_Z_OTHER, str, wxDefaultPosition, wxSize(60, -1), 0 );
	itemSizer->Add(poszCtrl, 0, wxGROW, 5);

	item0->Add(itemSizer, 0, wxGROW, 0);


	itemSizer = new wxBoxSizer(wxHORIZONTAL);

	label = new wxStaticText(panel, wxID_STATIC, "Material:", wxDefaultPosition, wxSize(50, -1), wxALIGN_RIGHT);
	itemSizer->Add(label, 0, wxALIGN_CENTER_VERTICAL, 5);

	const wxString matstrings[3] = { "Keep original", "Make metallic", "Make transparent" };

	wxChoice* matChoice = new wxChoice(panel, ID_OBJ_MATERIAL_OTHER, wxDefaultPosition, wxSize(60, -1), 3, matstrings, 0);
	matChoice->SetSelection(options.objMaterialOther);
	itemSizer->Add(matChoice, 1, wxGROW, 5);

	item0->Add(itemSizer, 0, wxGROW, 0);

	// validators


	checkBoxDepth->SetValidator(wxGenericValidator(&options.depthOfFieldOther));
	checkBoxLocal->SetValidator(wxGenericValidator(&options.localIlluminationOther));

	skyChoice->SetValidator(wxGenericValidator(&options.skyOther));


	editSkyBox->SetValidator(wxGenericValidator(&options.skyBoxDirNameOther));
	editSkySphere->SetValidator(wxGenericValidator(&options.skySphereFileNameOther));

	checkBoxFloor->SetValidator(wxGenericValidator(&options.floorOther));

	editObj->SetValidator(wxGenericValidator(&options.objFileNameOther));

	wxFloatingPointValidator<double> dblVal(&options.scaleOther);
	dblVal.SetRange(0, 1000000);
	dblVal.SetPrecision(3);
	scaleCtrl->SetValidator(dblVal);


	wxIntegerValidator<int> val1(&options.rotateXOther, wxNUM_VAL_DEFAULT);
	val1.SetRange(-360, 360);
	rotxCtrl->SetValidator(val1);

	wxIntegerValidator<int> val2(&options.rotateYOther, wxNUM_VAL_DEFAULT);
	val2.SetRange(-360, 360);
	rotyCtrl->SetValidator(val2);

	wxIntegerValidator<int> val3(&options.rotateZOther, wxNUM_VAL_DEFAULT);
	val3.SetRange(-360, 360);
	rotzCtrl->SetValidator(val3);

	wxFloatingPointValidator<double> v1(&options.positionXOther, wxNUM_VAL_DEFAULT);
	v1.SetRange(-100, 100);
	v1.SetPrecision(2);
	posxCtrl->SetValidator(v1);

	wxFloatingPointValidator<double> v2(&options.positionYOther, wxNUM_VAL_DEFAULT);
	v2.SetRange(-100, 100);
	v2.SetPrecision(2);
	posyCtrl->SetValidator(v2);

	wxFloatingPointValidator<double> v3(&options.positionZOther, wxNUM_VAL_DEFAULT);
	v3.SetRange(-100, 100);
	v3.SetPrecision(2);
	poszCtrl->SetValidator(v3);

	matChoice->SetValidator(wxGenericValidator(&options.objMaterialOther));


	topSizer->Add(item0, 0, wxALL | wxGROW, 5);
	panel->SetSizerAndFit(topSizer);

	return panel;
}



void OptionsFrame::OnSkyboxChooseOther(wxCommandEvent& /*event*/)
{
	TransferDataFromWindow();
	wxDirDialog dlg(this, "Choose Sky Box directory", "", wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

	if (wxID_OK == dlg.ShowModal())
	{
		const wxString str = dlg.GetPath() + "\\";

		if ((wxFileName::Exists(str + "left.jpg") && wxFileName::Exists(str + "right.jpg") && wxFileName::Exists(str + "top.jpg") && wxFileName::Exists(str + "bottom.jpg") && wxFileName::Exists(str + "back.jpg") && wxFileName::Exists(str + "front.jpg")) ||
			(wxFileName::Exists(str + "left.png") && wxFileName::Exists(str + "right.png") && wxFileName::Exists(str + "top.png") && wxFileName::Exists(str + "bottom.png") && wxFileName::Exists(str + "back.png") && wxFileName::Exists(str + "front.png")))
		{
			options.skyBoxDirNameOther = str;
			TransferDataToWindow();
		}			
	}
}

void OptionsFrame::OnSkyboxClearOther(wxCommandEvent& /*event*/)
{
	TransferDataFromWindow();
	options.skyBoxDirNameOther.Clear();
	TransferDataToWindow();
}


void OptionsFrame::OnSkySphereChooseOther(wxCommandEvent& /*event*/)
{
	TransferDataFromWindow();
	wxFileDialog openFileDialog(this, _("Open File As _?"), wxEmptyString, wxEmptyString, _("Jpeg Files (*.jpg)|*.jpg|PNG files (*.png)|*.png|TIF files (*.tif)|*.tif|TGA files (*.tga)|*.tga|All files (*.*)|*.*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (openFileDialog.ShowModal() == wxID_OK)
	{
		const wxString path = openFileDialog.GetPath();
		const wxFileName fileName(path);

		if (fileName.Exists())
		{
			options.skySphereFileNameOther = path;
			TransferDataToWindow();
		}
	}
}

void OptionsFrame::OnSkySphereClearOther(wxCommandEvent& /*event*/)
{
	TransferDataFromWindow();
	options.skySphereFileNameOther.Clear();
	TransferDataToWindow();
}

void OptionsFrame::OnObjectChooseOther(wxCommandEvent& /*event*/)
{
	TransferDataFromWindow();
	wxFileDialog openFileDialog(this, _("Open File As _?"), wxEmptyString, wxEmptyString, _("Obj Files (*.obj)|*.obj"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (openFileDialog.ShowModal() == wxID_OK)
	{
		const wxString path = openFileDialog.GetPath();
		const wxFileName fileName(path);

		if (fileName.Exists())
		{
			options.objFileNameOther = path;
			TransferDataToWindow();
		}
	}
}

void OptionsFrame::OnObjectClearOther(wxCommandEvent& /*event*/)
{
	TransferDataFromWindow();
	options.objFileNameOther.Clear();
	TransferDataToWindow();
}
