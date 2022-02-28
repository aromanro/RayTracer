#include "RayTracerFrame.h"


void RayTracerFrame::FillRandomScene(Scene& scene, const Options& options)
{
	Random random;

	//auto floorTexture = std::make_shared<Materials::Lambertian>(std::dynamic_pointer_cast<Textures::Texture>(std::make_shared<Textures::CheckerTexture>(
	//	std::dynamic_pointer_cast<Textures::Texture>(std::make_shared<Textures::ColorTexture>(Color(0., 0.1, 0.3))),
	//	std::dynamic_pointer_cast<Textures::Texture>(std::make_shared<Textures::ColorTexture>(Color(0.9, 0.9, 0.9))))));

	auto floorTexture = std::make_shared<Materials::Lambertian>(std::dynamic_pointer_cast<Textures::Texture>(std::make_shared<Textures::PerlinMarbleTexture>(
		random, 512, Color(1., 1., 1.), 4.
		)));

	scene.objects.emplace_back(std::make_shared<Objects::RectangleXZ>(-100, 100, -100, 100, 0, floorTexture));


	scene.objects.emplace_back(std::make_shared<Objects::Sphere>(Vector3D<double>(0, 1, 0), 1, std::make_shared<Materials::Dielectric>(1.5)));


	std::shared_ptr<Textures::Texture> tex;

	if (options.textureFileName.size()) tex = std::dynamic_pointer_cast<Textures::Texture>(std::make_shared<Textures::ImageTexture>(std::string(options.textureFileName.c_str())));
	else tex = std::dynamic_pointer_cast<Textures::Texture>(std::make_shared<Textures::ColorTexture>(Color(0.4, 0.2, 0.1)));

	auto earth = std::make_shared<Objects::Sphere>(Vector3D<double>(0, 0, 0), 1, std::make_shared<Materials::Lambertian>(tex));
	earth->RotateAround(Vector3D<double>(0, 0, 1), M_PI);
	earth->Translate(Vector3D<double>(4, 1, 0));
	scene.objects.emplace_back(earth);


	if (options.localIllumination)
	{
		auto lightTex = std::dynamic_pointer_cast<Textures::Texture>(std::make_shared<Textures::ColorTexture>(Color(25, 25, 25)));
		auto light = std::make_shared<Objects::Sphere>(Vector3D<double>(6, 2, 2), 0.4, std::make_shared<Materials::Lambertian>(lightTex));
		scene.objects.emplace_back(light);
		scene.AddPriorityObject(light);
	}

	scene.objects.emplace_back(std::make_shared<Objects::Sphere>(Vector3D<double>(-4, 1, 0), 1, std::make_shared<Materials::Metal>(std::dynamic_pointer_cast<Textures::Texture>(std::make_shared<Textures::ColorTexture>(Color(0.7, 0.6, 0.5))))));

	SetSky(scene, options);

	FillRandomObjects(scene, options, random);
}


void RayTracerFrame::FillRandomObjects(Scene& scene, const Options& options, Random& random)
{
	for (double a = -10; a < 10; a += 1.25)
	{
		for (double b = -10; b < 10; b += 1.25)
		{
			double choose_mat = random.getZeroOne();
			double choose_obj = random.getZeroOne();
			double radius = 0.2 * random.getZeroOne() + 0.15;

			bool newBox = choose_obj >= 0.5 ? true : false;
			if (choose_obj >= 0.5) radius /= sqrt(2); // make them smaller

			Vector3D<double> center(a + 1.4 * random.getZeroOne(), radius, b + 1.4 * random.getZeroOne());

			bool intersect;

			do
			{
				intersect = false;

				for (const auto& obj : scene.objects)
				{
					const auto sphere = std::dynamic_pointer_cast<Objects::Sphere>(obj);
					const auto box = std::dynamic_pointer_cast<Objects::Box>(obj);
					const auto to = std::dynamic_pointer_cast<Transforms::TranslateAction>(obj);
					const auto ro = std::dynamic_pointer_cast<Transforms::RotateYAction>(obj);

					if (!sphere && !box && !to && !ro) continue;

					double dist = 0;
					double boxRadius = 0;

					if (sphere) dist = (sphere->getCenter() - center).Length();
					else if (box)
					{
						dist = (box->getCenter() - center).Length();
						boxRadius = box->getRadius();
					}
					else if (to)
					{
						BVH::AxisAlignedBoundingBox bbox;

						to->BoundingBox(bbox);

						auto tcenter = (bbox.max() + bbox.min()) * 0.5;

						dist = (tcenter - center).Length();
						boxRadius = (tcenter - bbox.min()).Length();
					}
					else if (ro)
					{
						BVH::AxisAlignedBoundingBox bbox;

						ro->BoundingBox(bbox);

						auto rcenter = (bbox.max() + bbox.min()) * 0.5;

						dist = (rcenter - center).Length();
						boxRadius = (rcenter - bbox.min()).Length();
					}

					if (dist < (sphere ? sphere->getRadius() + (newBox ? radius * sqrt(3) : radius) : boxRadius + (newBox ? radius * sqrt(3) : radius)) + 0.2)
					{
						intersect = true;
						break;
					}
				}

				if (intersect)
				{
					radius = 0.2 * random.getZeroOne() + 0.15;
					if (choose_obj >= 0.5) radius /= sqrt(2); // make them smaller

					center = Vector3D<double>(a + 1.4 * random.getZeroOne(), radius, b + 1.4 * random.getZeroOne());
				}
			} while (intersect);

			if (choose_mat < 0.7)
			{  // diffuse
				std::shared_ptr<Materials::Material> mat;

				if (random.getZeroOne() < 0.25)
					mat = std::make_shared<Materials::Lambertian>(std::dynamic_pointer_cast<Textures::Texture>(std::make_shared<Textures::ColorTexture>(Color(random.getZeroOne() * random.getZeroOne(), random.getZeroOne() * random.getZeroOne(), random.getZeroOne() * random.getZeroOne()))));
				else
				{
					const double exponent = 200. * random.getZeroOne();
					mat = std::make_shared<Materials::AnisotropicPhong>(exponent, exponent, std::dynamic_pointer_cast<Textures::Texture>(std::make_shared<Textures::ColorTexture>(Color(random.getZeroOne() * random.getZeroOne(), random.getZeroOne() * random.getZeroOne(), random.getZeroOne() * random.getZeroOne()))),
						std::dynamic_pointer_cast<Textures::Texture>(std::make_shared<Textures::ColorTexture>(Color(0.7, 0.7, 0.7))));
				}

				if (choose_obj < 0.5)
					scene.objects.emplace_back(std::make_shared<Objects::Sphere>(center, radius, mat));
				else
				{
					auto b = std::make_shared<Objects::Box>(Vector3D<double>(-radius, -radius, -radius), Vector3D<double>(radius, radius, radius), mat);
					const double choose_rot = random.getZeroOne() * 0.5 * M_PI;
					auto r = std::make_shared<Transforms::RotateYAction>(b, choose_rot);
					auto t = std::make_shared<Transforms::TranslateAction>(r, center);

					scene.objects.emplace_back(t);
				}
			}
			else if (choose_mat < 0.9)
			{ // metal
				auto mat = std::make_shared<Materials::Metal>(std::dynamic_pointer_cast<Textures::Texture>(std::make_shared<Textures::ColorTexture>(0.5 * Color(1. + random.getZeroOne(), 1. + random.getZeroOne(), 1. + random.getZeroOne()))));
				if (choose_obj < 0.5)
					scene.objects.emplace_back(std::make_shared<Objects::Sphere>(center, radius, mat));
				else
				{
					auto b = std::make_shared<Objects::Box>(Vector3D<double>(-radius, -radius, -radius), Vector3D<double>(radius, radius, radius), mat);
					const double choose_rot = random.getZeroOne() * 0.5 * M_PI;
					auto r = std::make_shared<Transforms::RotateYAction>(b, choose_rot);
					auto t = std::make_shared<Transforms::TranslateAction>(r, center);


					scene.objects.emplace_back(t);
				}
			}
			else
			{  // glass
				auto mat = std::make_shared<Materials::Dielectric>(1.5);

				// make some colored glass
				mat->density = random.getZeroOne() * 10;
				mat->volumeColor = Color(random.getZeroOne(), random.getZeroOne(), random.getZeroOne());

				if (choose_obj < 0.5)
					scene.objects.emplace_back(std::make_shared<Objects::Sphere>(center, radius, mat));
				else
				{
					auto b = std::make_shared<Objects::Box>(Vector3D<double>(-radius, -radius, -radius), Vector3D<double>(radius, radius, radius), mat);
					const double choose_rot = random.getZeroOne() * 0.5 * M_PI;
					auto r = std::make_shared<Transforms::RotateYAction>(b, choose_rot);
					auto t = std::make_shared<Transforms::TranslateAction>(r, center);

					scene.objects.emplace_back(t);
				}
			}
		}
	}
}


void RayTracerFrame::SetSky(Scene& scene, const Options& options)
{
	if (2 == options.sky && !options.skyBoxDirName.empty())
	{
		scene.blackSky = true;

		std::string frontFile;
		std::string backFile;
		std::string topFile;
		std::string bottomFile;
		std::string leftFile;
		std::string rightFile;

		bool exists = false;

		if (wxFileName::Exists(options.skyBoxDirName + "front.jpg"))
		{
			// jpg file names

			frontFile = options.skyBoxDirName + "front.jpg";
			backFile = options.skyBoxDirName + "back.jpg";
			topFile = options.skyBoxDirName + "top.jpg";
			bottomFile = options.skyBoxDirName + "bottom.jpg";
			leftFile = options.skyBoxDirName + "left.jpg";
			rightFile = options.skyBoxDirName + "right.jpg";

			exists = true;
		}
		else if (wxFileName::Exists(options.skyBoxDirName + "front.png"))// try png
		{
			frontFile = options.skyBoxDirName + "front.png";
			backFile = options.skyBoxDirName + "back.png";
			topFile = options.skyBoxDirName + "top.png";
			bottomFile = options.skyBoxDirName + "bottom.png";
			leftFile = options.skyBoxDirName + "left.png";
			rightFile = options.skyBoxDirName + "right.png";

			exists = true;
		}

		if (exists)
		{
			auto skybox = std::make_shared<Objects::SkyBox>(Vector3D<double>(13. - 10000, 2. - 10000, 3. - 10000), Vector3D<double>(13. + 10000, 2. + 10000, 3. + 10000));
			skybox->Load(frontFile, backFile, topFile, bottomFile, leftFile, rightFile);
			scene.sky = skybox;
		}
	}
	else if (3 == options.sky && !options.skySphereFileName.empty() && wxFileName::Exists(options.skySphereFileName))
	{
		scene.blackSky = true;

		auto skysphere = std::make_shared<Objects::SkySphere>(Vector3D<double>(0, 0, 0), 10000);
		if (skysphere->Load(std::string(options.skySphereFileName.c_str())))
		{
			scene.sky = skysphere;
		}
	}
	else if (1 == options.sky)
		scene.blackSky = true;
	else
		scene.blackSky = false;
}


void RayTracerFrame::FillCornellScene(Scene& scene, const Options& options)
{
	scene.blackSky = true;

	const auto RedMaterial = std::make_shared<Materials::Lambertian>(std::dynamic_pointer_cast<Textures::Texture>(std::make_shared<Textures::ColorTexture>(Color(0.65, 0.05, 0.05))));
	const auto WhiteMaterial = std::make_shared<Materials::Lambertian>(std::dynamic_pointer_cast<Textures::Texture>(std::make_shared<Textures::ColorTexture>(Color(0.73, 0.73, 0.73))));
	const auto GreenMaterial = std::make_shared<Materials::Lambertian>(std::dynamic_pointer_cast<Textures::Texture>(std::make_shared<Textures::ColorTexture>(Color(0.12, 0.45, 0.15))));
	const auto AluminiumMaterial = std::make_shared<Materials::Metal>(std::dynamic_pointer_cast<Textures::Texture>(std::make_shared<Textures::ColorTexture>(Color(0.8, 0.85, 0.88))));

	const auto LightMaterial = std::make_shared<Materials::Lambertian>(std::dynamic_pointer_cast<Textures::Texture>(std::make_shared<Textures::ColorTexture>(Color(15, 15, 15))));



	scene.objects.emplace_back(std::make_shared<Transforms::FlipNormal>(std::make_shared<Objects::RectangleYZ>(0, 555, 0, 555, 555, GreenMaterial))); // left wall
	scene.objects.emplace_back(std::make_shared<Objects::RectangleYZ>(0, 555, 0, 555, 0, RedMaterial)); // right wall

	// The light

	auto Light = std::make_shared<Transforms::FlipNormal>(std::make_shared<Objects::RectangleXZ>(213, 343, 227, 332, 554, LightMaterial));

	scene.AddPriorityObject(Light);
	scene.objects.emplace_back(Light);

	auto ceiling = std::make_shared<Transforms::FlipNormal>(std::make_shared<Objects::RectangleXZ>(0, 555, 0, 555, 555, WhiteMaterial));
	scene.objects.emplace_back(ceiling); // ceiling

	auto floor = std::make_shared<Objects::RectangleXZ>(0, 555, 0, 555, 0, WhiteMaterial);
	scene.objects.emplace_back(floor); // floor

	scene.objects.emplace_back(std::make_shared<Transforms::FlipNormal>(std::make_shared<Objects::RectangleXY>(0, 555, 0, 555, 555, WhiteMaterial))); // back wall

	// put a mirror on the right wall
	if (options.mirrorOnWall)
	{
		const auto MirrorMaterial = std::make_shared<Materials::Metal>(std::dynamic_pointer_cast<Textures::Texture>(std::make_shared<Textures::ColorTexture>(Color(0.98, 0.98, 0.98))));
		auto mirror = std::make_shared<Objects::RectangleYZ>(50, 443, 80, 332, 0, MirrorMaterial);
		scene.objects.emplace_back(mirror);
		if (options.addMirrorToImportanceSampling) scene.AddPriorityObject(mirror);
	}


	if (options.boxContent < 3)
	{
		const auto box1 = std::make_shared<Objects::Box>(Vector3D<double>(0, 0, 0), Vector3D<double>(165, 165, 165), WhiteMaterial);
		const auto box2 = std::make_shared<Objects::Box>(Vector3D<double>(0, 0, 0), Vector3D<double>(165, 330, 165), 1 == options.boxContent ? std::dynamic_pointer_cast<Materials::Material>(AluminiumMaterial) : WhiteMaterial);

		const auto rbox1 = std::make_shared<Transforms::RotateYAction>(box1, -18. / 180. * M_PI);
		const auto rbox2 = std::make_shared<Transforms::RotateYAction>(box2, 15. / 180. * M_PI);

		const auto trbox1 = std::make_shared<Transforms::TranslateAction>(rbox1, Vector3D<double>(130, 0, 65));
		const auto trbox2 = std::make_shared<Transforms::TranslateAction>(rbox2, Vector3D<double>(265, 0, 295));

		if (options.boxContent < 2)
		{
			scene.objects.emplace_back(trbox1);
			scene.objects.emplace_back(trbox2);

			if (options.addSmallObjectToImportanceSampling) scene.AddPriorityObject(trbox1);
		}
		else
		{
			const auto ctrbox1 = std::make_shared<Objects::ConstantMedium>(trbox1, std::make_shared<Textures::ColorTexture>(Color(1., 1., 1.)), 0.01);
			const auto ctrbox2 = std::make_shared<Objects::ConstantMedium>(trbox2, std::make_shared<Textures::ColorTexture>(Color(0., 0., 0.)), 0.01);

			scene.objects.emplace_back(ctrbox1);
			scene.objects.emplace_back(ctrbox2);

			if (options.addSmallObjectToImportanceSampling) scene.AddPriorityObject(ctrbox1);
		}
	}
	else if (3 == options.boxContent)
	{
		const auto box = std::make_shared<Objects::Box>(Vector3D<double>(0, 0, 0), Vector3D<double>(165, 330, 165), 1 == options.boxContent ? std::dynamic_pointer_cast<Materials::Material>(AluminiumMaterial) : WhiteMaterial);
		const auto rbox = std::make_shared<Transforms::RotateYAction>(box, 15. / 180. * M_PI);
		const auto trbox = std::make_shared<Transforms::TranslateAction>(rbox, Vector3D<double>(265, 0, 295));

		const auto glassSphere = std::make_shared<Objects::Sphere>(Vector3D<double>(190, 90, 190), 90, std::make_shared<Materials::Dielectric>(1.5));

		scene.objects.emplace_back(trbox);
		scene.objects.emplace_back(glassSphere);
		if (options.addSmallObjectToImportanceSampling) scene.AddPriorityObject(glassSphere);
	}
	else if (!options.objFileName.empty() && wxFileName::Exists(options.objFileName))
	{
		ObjLoader loader;
		loader.Load(std::string(options.objFileName.c_str()));

		auto object = std::make_shared<Objects::VisibleObjectComposite>();

		for (const auto& triangle : loader.triangles)
			object->objects.emplace_back(triangle);

		object->Scale(options.scale);

		const Vector3D<double> Ox(1, 0, 0);
		const Vector3D<double> Oy(0, 1, 0);
		const Vector3D<double> Oz(0, 1, 0);

		object->RotateAround(Ox, options.rotateX * M_PI / 180.);
		object->RotateAround(Oy, options.rotateY * M_PI / 180.);
		object->RotateAround(Oz, options.rotateZ * M_PI / 180.);

		object->Translate(Vector3D<double>(options.positionX, options.positionY, options.positionZ));

		if (1 == options.objMaterial)
		{
			for (auto& obj : object->objects)
				std::dynamic_pointer_cast<Objects::VisibleObjectMaterial>(obj)->SetMaterial(AluminiumMaterial);
		}
		else if (2 == options.objMaterial)
		{
			auto glassmat = std::make_shared<Materials::Dielectric>(1.5);
			glassmat->density = 0.03;
			glassmat->volumeColor = Color(1, 1, 0);
			for (auto& obj : object->objects)
				std::dynamic_pointer_cast<Objects::VisibleObjectMaterial>(obj)->SetMaterial(glassmat);
		}


		// save some values for some test objects
		//Scale(150); // monkey
		//Scale(80); // car
		//Scale(2.5); // indian

		//monkey->RotateAround(Oy, /*120.*/180 * M_PI / 180.);
		//monkey->RotateAround(Ox, 15 * M_PI / 180.);

		// indian
		//RotateAround(Ox, -90 * M_PI / 180.);
		//RotateAround(Oy, 180 * M_PI / 180.);

		//Translate(Vector3D<double>(275, 250, 300)); // monkey	
		//Translate(Vector3D<double>(275, 100, 350)); // car	
		//Translate(Vector3D<double>(100, 230, 300)); // indian

		scene.objects.emplace_back(object);
	}
}


void RayTracerFrame::FillOtherScene(Scene& scene, const Options& options)
{
	if (options.floorOther)
	{
		auto checker = std::make_shared<Materials::Lambertian>(std::dynamic_pointer_cast<Textures::Texture>(std::make_shared<Textures::CheckerTexture>(
			std::dynamic_pointer_cast<Textures::Texture>(std::make_shared<Textures::ColorTexture>(Color(0., 0.1, 0.3))),
			std::dynamic_pointer_cast<Textures::Texture>(std::make_shared<Textures::ColorTexture>(Color(0.9, 0.9, 0.9))))));
		scene.objects.emplace_back(std::make_shared<Objects::RectangleXZ>(-100, 100, -100, 100, 0, checker));
	}

	if (options.localIlluminationOther)
	{
#ifdef RECORD_MOVIE
		// move the light out of the scene and make it strong, for the effect
		auto lightTex = std::dynamic_pointer_cast<Textures::Texture>(std::make_shared<Textures::ColorTexture>(Color(30, 30, 30)));
		auto light = std::make_shared<Objects::Sphere>(Vector3D<double>(6, 3.9, 2), 0.4, std::make_shared<Materials::Lambertian>(lightTex));
#else
		auto lightTex = std::dynamic_pointer_cast<Textures::Texture>(std::make_shared<Textures::ColorTexture>(Color(25, 25, 25)));
		auto light = std::make_shared<Objects::Sphere>(Vector3D<double>(6, 2, 2), 0.4, std::make_shared<Materials::Lambertian>(lightTex));
#endif		

		scene.objects.emplace_back(light);
		scene.AddPriorityObject(light);
	}

	SetSkyOther(scene, options);

	if (!options.objFileNameOther.empty() && wxFileName::Exists(options.objFileNameOther))
	{
		ObjLoader loader;
		loader.Load(std::string(options.objFileNameOther.c_str()));

		auto object = std::make_shared<Objects::VisibleObjectComposite>();

		for (const auto& triangle : loader.triangles)
			object->objects.emplace_back(triangle);

		object->Scale(options.scaleOther);

		const Vector3D<double> Ox(1, 0, 0);
		const Vector3D<double> Oy(0, 1, 0);
		const Vector3D<double> Oz(0, 1, 0);

		object->RotateAround(Ox, options.rotateXOther * M_PI / 180.);
		object->RotateAround(Oy, options.rotateYOther * M_PI / 180.);
		object->RotateAround(Oz, options.rotateZOther * M_PI / 180.);

		object->Translate(Vector3D<double>(options.positionXOther, options.positionYOther, options.positionZOther));

		if (1 == options.objMaterialOther)
		{
			const auto AluminiumMaterial = std::make_shared<Materials::Metal>(std::dynamic_pointer_cast<Textures::Texture>(std::make_shared<Textures::ColorTexture>(Color(0.8, 0.85, 0.88))));
			for (auto& obj : object->objects)
				std::dynamic_pointer_cast<Objects::VisibleObjectMaterial>(obj)->SetMaterial(AluminiumMaterial);
		}
		else if (2 == options.objMaterialOther)
		{
			auto glassmat = std::make_shared<Materials::Dielectric>(1.5);
			glassmat->density = 0.03;
			glassmat->volumeColor = Color(1, 1, 0);
			for (auto& obj : object->objects)
				std::dynamic_pointer_cast<Objects::VisibleObjectMaterial>(obj)->SetMaterial(glassmat);
		}

		scene.objects.emplace_back(object);
	}

#ifdef RECORD_MOVIE
	ObjLoader angelLoader;
	angelLoader.Load("C:\\Work\\Blog\\Docs\\RayTracing\\Models\\AngelLucy\\Alucy.obj");

	auto glassAngel = std::make_shared<Objects::VisibleObjectComposite>();
	auto glassmat = std::make_shared<Materials::Dielectric>(1.5);

	for (const auto& triangle : angelLoader.triangles)
	{
		triangle->SetMaterial(glassmat);
		glassAngel->objects.emplace_back(triangle);
	}
	glassAngel->Scale(0.003);

	const Vector3D<double> Oy(0, 1, 0);

	glassAngel->RotateAround(Oy, (90. - options.positionXOther * 8.) * M_PI / 180.);
	glassAngel->Translate(Vector3D<double>(5, 1, -3));

	scene.objects.emplace_back(glassAngel);

	angelLoader.Load("C:\\Work\\Blog\\Docs\\RayTracing\\Models\\AngelLucy\\Alucy.obj");



	auto metallicAngel = std::make_shared<Objects::VisibleObjectComposite>();
	const auto mirrorMaterial = std::make_shared<Materials::Metal>(std::dynamic_pointer_cast<Textures::Texture>(std::make_shared<Textures::ColorTexture>(Color(0.8, 0.85, 0.88))));

	for (const auto& triangle : angelLoader.triangles)
	{
		triangle->SetMaterial(mirrorMaterial);
		metallicAngel->objects.emplace_back(triangle);
	}

	metallicAngel->Scale(0.003);

	metallicAngel->RotateAround(Oy, (220. + options.positionXOther * 8.) * M_PI / 180.);
	metallicAngel->Translate(Vector3D<double>(5, 1, 3));

	scene.objects.emplace_back(metallicAngel);
#endif
}

void RayTracerFrame::SetSkyOther(Scene& scene, const Options& options)
{
	if (2 == options.skyOther && !options.skyBoxDirNameOther.empty())
	{
		scene.blackSky = true;

		std::string frontFile;
		std::string backFile;
		std::string topFile;
		std::string bottomFile;
		std::string leftFile;
		std::string rightFile;

		bool exists = false;

		if (wxFileName::Exists(options.skyBoxDirNameOther + "front.jpg"))
		{
			// jpg file names

			frontFile = options.skyBoxDirNameOther + "front.jpg";
			backFile = options.skyBoxDirNameOther + "back.jpg";
			topFile = options.skyBoxDirNameOther + "top.jpg";
			bottomFile = options.skyBoxDirNameOther + "bottom.jpg";
			leftFile = options.skyBoxDirNameOther + "left.jpg";
			rightFile = options.skyBoxDirNameOther + "right.jpg";

			exists = true;
		}
		else if (wxFileName::Exists(options.skyBoxDirNameOther + "front.png"))// try png
		{
			frontFile = options.skyBoxDirNameOther + "front.png";
			backFile = options.skyBoxDirNameOther + "back.png";
			topFile = options.skyBoxDirNameOther + "top.png";
			bottomFile = options.skyBoxDirNameOther + "bottom.png";
			leftFile = options.skyBoxDirNameOther + "left.png";
			rightFile = options.skyBoxDirNameOther + "right.png";

			exists = true;
		}

		if (exists)
		{
			auto skybox = std::make_shared<Objects::SkyBox>(Vector3D<double>(13. - 10000, 2. - 10000, 3. - 10000), Vector3D<double>(13. + 10000, 2. + 10000, 3. + 10000));
			skybox->Load(frontFile, backFile, topFile, bottomFile, leftFile, rightFile);
			scene.sky = skybox;
		}
	}
	else if (3 == options.skyOther && !options.skySphereFileNameOther.empty() && wxFileName::Exists(options.skySphereFileNameOther))
	{
		scene.blackSky = true;
		auto skysphere = std::make_shared<Objects::SkySphere>(Vector3D<double>(0, 0, 0), 10000);
		if (skysphere->Load(std::string(options.skySphereFileNameOther.c_str())))
		{
			scene.sky = skysphere;
		}
	}
	else if (1 == options.skyOther)
		scene.blackSky = true;
	else
		scene.blackSky = false;
}
