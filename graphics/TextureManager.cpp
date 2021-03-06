#include "TextureManager.hpp"
#include "Device.hpp"
#include "Texture.hpp"
#include "BmpImage.hpp"
#include "PngImageLoader.hpp"
#include "TgaImageLoader.hpp"
#include "RawTextureData.hpp"
#include "../ResourceManager.ipp"
#include "../File.hpp"
#include "../Exception.hpp"

BEGIN_INANITY_GRAPHICS

TextureManager::TextureManager(ptr<FileSystem> fileSystem, ptr<Device> device, const SamplerSettings& samplerSettings)
: ResourceManager(fileSystem), device(device), samplerSettings(samplerSettings)
{
	imageLoaders["bmp"] = BmpImage::CreateLoader();
	imageLoaders["png"] = NEW(PngImageLoader());
	imageLoaders["tga"] = NEW(TgaImageLoader());
}

ptr<Texture> TextureManager::Load(const String& textureName)
{
	try
	{
		// получить расширение файла
		size_t dotPos = textureName.find_last_of('.');
		if(dotPos == std::string::npos)
			// расширения нет
			THROW("No extension in texture name");
		String extension = textureName.substr(dotPos + 1);
		// получить загрузчик
		std::unordered_map<String, ptr<ImageLoader> >::const_iterator i = imageLoaders.find(extension);
		if(i == imageLoaders.end())
			THROW("No image loader for this extension: " + extension);
		ptr<ImageLoader> imageLoader = i->second;

		// загрузить изображение
		ptr<RawTextureData> textureData = imageLoader->Load(fileSystem->LoadFile(textureName));

		// создать текстуру
		return device->CreateStaticTexture(textureData, samplerSettings);
	}
	catch(Exception* exception)
	{
		THROW_SECONDARY("Can't load texture", exception);
	}
}

END_INANITY_GRAPHICS
