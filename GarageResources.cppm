export module GarageResources;

import Index;

export inline PVZ::Image IMAGE_GARAGE_DOOR{ 0 };

ThreeState::ThreeState onExtractResource(PVZ::ResourceManager manager, const char* str)
{
	if (!std::strcmp(str, "InitGarage"))
	{
		IMAGE_GARAGE_DOOR = manager.GetImage("IMAGE_GARAGE_DOOR");
		return ThreeState::Enable;
	}
	return ThreeState::None;
}

export void InitResource()
{
	auto manager = PVZ::GetResourceManager();

	manager.ParseResourcesFile("garage/garage_resoures.xml");
	ExtractResourceEvent((int)onExtractResource);
	manager.TodLoadResources("InitGarage");
}