#pragma once

#include <string>
#include <vector>

#define wxNEEDS_DECL_BEFORE_TEMPLATE

#include <wx/fileconf.h>

class Options
{
public:
	Options();

	~Options()
	{
		delete m_fileconfig;
	}

	// avoid double deletion of m_fileconfig at destruction if copied
	Options(const Options& other)
		:
		nrThreads(other.nrThreads),
		nrSamples(other.nrSamples),
		recursivityStop(other.recursivityStop),
		scene(other.scene),
		jitter(other.jitter),
		depthOfField(other.depthOfField),
		localIllumination(other.localIllumination),
		sky(other.sky),
		skyBoxDirName(other.skyBoxDirName),
		skySphereFileName(other.skySphereFileName),
		textureFileName(other.textureFileName),
		mirrorOnWall(other.mirrorOnWall),
		addMirrorToImportanceSampling(other.addMirrorToImportanceSampling),
		addSmallObjectToImportanceSampling(other.addSmallObjectToImportanceSampling),
		boxContent(other.boxContent),
		objFileName(other.objFileName),
		scale(other.scale),
		rotateX(other.rotateX),
		rotateY(other.rotateY),
		rotateZ(other.rotateZ),
		positionX(other.positionX),
		positionY(other.positionY),
		positionZ(other.positionZ),
		objMaterial(other.objMaterial),
		depthOfFieldOther(other.depthOfFieldOther),
		localIlluminationOther(other.localIlluminationOther),
		skyOther(other.skyOther),
		skyBoxDirNameOther(other.skyBoxDirNameOther),
		skySphereFileNameOther(other.skySphereFileNameOther),
		floorOther(other.floorOther),
		objFileNameOther(other.objFileNameOther),
		scaleOther(other.scaleOther),
		rotateXOther(other.rotateXOther),
		rotateYOther(other.rotateYOther),
		rotateZOther(other.rotateZOther),
		positionXOther(other.positionXOther),
		positionYOther(other.positionYOther),
		positionZOther(other.positionZOther),
		objMaterialOther(other.objMaterialOther),
		m_fileconfig(nullptr)
	{
	}

	Options& operator=(const Options& other)
	{
		nrThreads = other.nrThreads;
		nrSamples = other.nrSamples;
		recursivityStop = other.recursivityStop;
		scene = other.scene;
		jitter = other.jitter;
		depthOfField = other.depthOfField;
		localIllumination = other.localIllumination;
		sky = other.sky;
		skyBoxDirName = other.skyBoxDirName;
		skySphereFileName = other.skySphereFileName;
		textureFileName = other.textureFileName;
		mirrorOnWall = other.mirrorOnWall;
		addMirrorToImportanceSampling = other.addMirrorToImportanceSampling;
		addSmallObjectToImportanceSampling = other.addSmallObjectToImportanceSampling;
		boxContent = other.boxContent;
		objFileName = other.objFileName;
		scale = other.scale;
		rotateX = other.rotateX;
		rotateY = other.rotateY;
		rotateZ = other.rotateZ;
		positionX = other.positionX;
		positionY = other.positionY;
		positionZ = other.positionZ;
		objMaterial = other.objMaterial;
		depthOfFieldOther = other.depthOfFieldOther;
		localIlluminationOther = other.localIlluminationOther;
		skyOther = other.skyOther;
		skySphereFileNameOther = other.skySphereFileNameOther;
		skyBoxDirNameOther = other.skyBoxDirNameOther;
		floorOther = other.floorOther;
		objFileNameOther = other.objFileNameOther;
		scaleOther = other.scaleOther;
		rotateXOther = other.rotateXOther;
		rotateYOther = other.rotateYOther;
		rotateZOther = other.rotateZOther;
		positionXOther = other.positionXOther;
		positionYOther = other.positionYOther;
		positionZOther = other.positionZOther;
		objMaterialOther = other.objMaterialOther;

		m_fileconfig = nullptr;

		return *this;
	}


	void Load();
	void Save();


	// general options
	int nrThreads;
	int nrSamples;
	int recursivityStop;

	int scene;

	bool jitter;

	// 'one weekend' options
	bool depthOfField;
	bool localIllumination;
	int sky; // 0 - blue, 1 - black, 2 - skybox, 3 - skysphere
	wxString skyBoxDirName;
	wxString skySphereFileName;
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
	int skyOther; // 0 - blue, 1 - black, 2 - skybox, 3 - skysphere
	wxString skyBoxDirNameOther;
	wxString skySphereFileNameOther;
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
	void Open();
	void Close();

	wxFileConfig *m_fileconfig;
};

