#pragma once

#include <string>
#include <vector>

#define wxNEEDS_DECL_BEFORE_TEMPLATE

#include <wx/fileconf.h>

class Options
{
public:
	Options();
	~Options();

	void Load();
	void Save();

	void Open();
	void Close();


	// general options
	int nrThreads;
	int nrSamples;
	int recursivityStop;

	int scene;

	bool jitter;

	// 'one weekend' options
	bool depthOfField;
	bool localIllumination;
	int sky; // 0 - blue, 1 - black, 2 - skybox
	wxString skyBoxDirName;
	wxString textureFileName;


	// Cornell box
	bool mirrorOnWall;
	bool addMirrorToImportanceSampling;
	bool addSmallObjectToImportanceSampling;
	int boxContent; // 0 - two 'white' boxes, 1 - 'one metal box, one white', 2 - two 'mist' boxes, 3 - one box, one transparent sphere, 4 - load an object

	wxString objFileName;

	// for loaded model
	double scale;

	int rotateX;
	int rotateY;
	int rotateZ;

	int positionX;
	int positionY;
	int positionZ;

	int objMaterial; // 0 - original, 1 - make metal, 2 - make transparent

	// Other
	bool depthOfFieldOther;
	bool localIlluminationOther;
	int skyOther; // 0 - blue, 1 - black, 2 - skybox
	wxString skyBoxDirNameOther;
	bool floorOther;

	wxString objFileNameOther;

	// for loaded model
	double scaleOther;

	int rotateXOther;
	int rotateYOther;
	int rotateZOther;

	double positionXOther;
	double positionYOther;
	double positionZOther;

	int objMaterialOther; // 0 - original, 1 - make metal, 2 - make transparent

protected:
	wxFileConfig *m_fileconfig;
};

