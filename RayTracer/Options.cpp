#include "Options.h"

#include <wx/stdpaths.h> 

Options::Options()
	: nrThreads(8), nrSamples(4), recursivityStop(60), scene(0), jitter(true),
	depthOfField(false), localIllumination(true), sky(0),
	mirrorOnWall(false), addMirrorToImportanceSampling(false), addSmallObjectToImportanceSampling(false), boxContent(0),
	scale(1), rotateX(0), rotateY(0), rotateZ(0),
	positionX(275), positionY(250), positionZ(300),
	objMaterial(0),
	depthOfFieldOther(false), localIlluminationOther(true), skyOther(0), floorOther(false),
	scaleOther(1), rotateXOther(0), rotateYOther(0), rotateZOther(0),
	positionXOther(4), positionYOther(1), positionZOther(0),
	objMaterialOther(0),
	m_fileconfig(nullptr)
{
}


Options::~Options()
{
}


void Options::Open()
{
	if (m_fileconfig) return;

	wxString dir = wxStandardPaths::Get().GetConfigDir() + wxFileName::GetPathSeparator();

	if(!wxFileName::DirExists(dir))
		wxFileName::Mkdir(dir, 0777, wxPATH_MKDIR_FULL);

	wxString iniFilePath = dir + "RayTracer.ini";

	m_fileconfig = new wxFileConfig("RayTracer", wxEmptyString, iniFilePath);

	wxConfigBase::Set(m_fileconfig);
}


void Options::Close()
{
	delete m_fileconfig;
	m_fileconfig = NULL;
	wxConfigBase::Set(NULL);
}

void Options::Load()
{
	wxConfigBase *conf=wxConfigBase::Get(false);
	if (conf)
	{
		nrThreads = conf->ReadLong("/nrThreads", 8);
		nrSamples = conf->ReadLong("/nrSamples", 4);
		recursivityStop = conf->ReadLong("/recursivityStop", 60);
		scene = conf->ReadLong("/scene", 0);
		jitter = conf->ReadBool("/jitter", true);

		// 'one weekend'
		depthOfField = conf->ReadBool("/depthOfField", false);
		localIllumination = conf->ReadBool("/localIllumination", true);
		sky = conf->ReadLong("/sky", 0);
		skyBoxDirName = conf->Read("/skyBox", "");
		textureFileName = conf->Read("/textureSphere", "");

		// Cornell Box
		mirrorOnWall = conf->ReadBool("/mirrorOnWall", false);
		addMirrorToImportanceSampling = conf->ReadBool("/addMirrorToImportanceSampling", false);
		addSmallObjectToImportanceSampling = conf->ReadBool("/addSmallObjectToImportanceSampling", false);
		boxContent = conf->ReadLong("/boxContent", 0);
		objFileName = conf->Read("/objFile", "");
		scale = conf->ReadDouble("/scale", 1.);
		rotateX = conf->ReadLong("/rotateX", 0);
		rotateY = conf->ReadLong("/rotateY", 0);
		rotateZ = conf->ReadLong("/rotateZ", 0);
		positionX = conf->ReadLong("/positionX", 275);
		positionY = conf->ReadLong("/positionY", 250);
		positionZ = conf->ReadLong("/positionZ", 300);
		objMaterial = conf->ReadLong("/objMaterial", 0);

		// Other
		depthOfFieldOther = conf->ReadBool("/depthOfFieldOther", false);
		localIlluminationOther = conf->ReadBool("/localIlluminationOther", true);
		skyOther = conf->ReadLong("/skyOther", 0);
		floorOther = conf->ReadBool("/floorOther", false);
		skyBoxDirNameOther = conf->Read("/skyBoxOther", "");
		objFileNameOther = conf->Read("/objFileOther", "");
		scaleOther = conf->ReadDouble("/scaleOther", 1.);
		rotateXOther = conf->ReadLong("/rotateXOther", 0);
		rotateYOther = conf->ReadLong("/rotateYOther", 0);
		rotateZOther = conf->ReadLong("/rotateZOther", 0);
		positionXOther = conf->ReadDouble("/positionXOther", 4.);
		positionYOther = conf->ReadDouble("/positionYOther", 1.);
		positionZOther = conf->ReadDouble("/positionZOther", 0.);
		objMaterialOther = conf->ReadLong("/objMaterialOther", 0);
	}
}

void Options::Save()
{
	wxConfigBase *conf=wxConfigBase::Get(false);
	if (conf)
	{
		conf->Write("/nrThreads", static_cast<long int>(nrThreads));
		conf->Write("/nrSamples", static_cast<long int>(nrSamples));
		conf->Write("/recursivityStop", static_cast<long int>(recursivityStop));
		conf->Write("/scene", static_cast<long int>(scene));
		conf->Write("/jitter", jitter);

		// 'one weekend'
		conf->Write("/depthOfField", depthOfField);
		conf->Write("/localIllumination", localIllumination);
		conf->Write("/sky", static_cast<long int>(sky));
		conf->Write("/skyBox", skyBoxDirName);
		conf->Write("/textureSphere", textureFileName);

		// Cornell Box
		conf->Write("/mirrorOnWall", mirrorOnWall);
		conf->Write("/addMirrorToImportanceSampling", addMirrorToImportanceSampling);
		conf->Write("/addSmallObjectToImportanceSampling", addSmallObjectToImportanceSampling);
		conf->Write("/boxContent", static_cast<long int>(boxContent));
		conf->Write("/objFile", objFileName);
		conf->Write("/scale", scale);
		conf->Write("/rotateX", static_cast<long int>(rotateX));
		conf->Write("/rotateY", static_cast<long int>(rotateY));
		conf->Write("/rotateZ", static_cast<long int>(rotateZ));
		conf->Write("/positionX", static_cast<long int>(positionX));
		conf->Write("/positionY", static_cast<long int>(positionY));
		conf->Write("/positionZ", static_cast<long int>(positionZ));
		conf->Write("/objMaterial", static_cast<long int>(objMaterial));

		// Other
		conf->Write("/depthOfFieldOther", depthOfFieldOther);
		conf->Write("/localIlluminationOther", localIlluminationOther);
		conf->Write("/skyOther", static_cast<long int>(skyOther));
		conf->Write("/skyBoxOther", skyBoxDirNameOther);
		conf->Write("/floorOther", floorOther);
		conf->Write("/objFileOther", objFileNameOther);
		conf->Write("/scaleOther", scaleOther);
		conf->Write("/rotateXOther", static_cast<long int>(rotateXOther));
		conf->Write("/rotateYOther", static_cast<long int>(rotateYOther));
		conf->Write("/rotateZOther", static_cast<long int>(rotateZOther));
		conf->Write("/positionXOther", positionXOther);
		conf->Write("/positionYOther", positionYOther);
		conf->Write("/positionZOther", positionZOther);
		conf->Write("/objMaterialOther", static_cast<long int>(objMaterialOther));
	}

	if (m_fileconfig)
		m_fileconfig->Flush();
}
