#ifndef ___INANITY_RESOURCE_CACHE_HPP___
#define ___INANITY_RESOURCE_CACHE_HPP___

#include "ResourceStore.hpp"
#include "Exception.hpp"
#include "String.hpp"
#include <hash_map>

BEGIN_INANITY

/// Класс кэша ресурсов.
/** Кэш ресурсов хранит коллекцию ресурсов, которые в него загружены. */
class ResourceCache : public Object
{
private:
	/// Хранилище ресурсов, из которого следует брать ресурсы.
	ptr<ResourceStore> resourceStore;
	/// Карта уже загруженных ресурсов (с кодом ресурса).
	std::hash_map<String, std::pair<ptr<Object>, int>> resources;

public:
	//конструктор
	ResourceCache(ptr<ResourceStore> fileLoader);

	//загрузить ресурс
	template <typename T>
	ptr<T> LoadResource(const String& fileName)
	{
		try
		{
			// попробовать найти ресурс в уже загруженных
			std::hash_map<String, std::pair<ptr<Object>, int> >::const_iterator it = resources.find(fileName);
			// если найден
			if(it != resources.end())
			{
				// проверить тип ресурса по коду
				if(it->second.second != T::resourceCode)
					THROW_PRIMARY_EXCEPTION("Invalid resource type");
				// вернуть ресурс
				return static_cast<T*>(&*it->second.first);
			}

			// иначе ресурс ещё не загружен
			// загрузить новый ресурс
			ptr<T> resource = resourceStore->LoadResource<T>(fileName);
			// добавить его в карту ресурсов
			resources[fileName] = std::make_pair(resource, T::resourceCode);
			// вернуть ресурс
			return resource;
		}
		catch(Exception* exception)
		{
			THROW_SECONDARY_EXCEPTION("Can't load resource \"" + fileName + "\"", exception);
		}
	}
};

END_INANITY

#endif
