#ifndef ___INANITY_GRAPHICS_GEOMETRY_HPP___
#define ___INANITY_GRAPHICS_GEOMETRY_HPP___

#include "graphics.hpp"
#include "../String.hpp"

BEGIN_INANITY

class File;

END_INANITY

BEGIN_INANITY_GRAPHICS

class GeometryFormat;

/// Класс геометрии.
class Geometry : public Object
{
private:
	ptr<File> vertices;
	size_t vertexStride;
	ptr<File> indices;
	size_t indexStride;
	ptr<GeometryFormat> format;

public:
	Geometry(ptr<File> vertices, size_t vertexStride, ptr<File> indices, size_t indexStride, ptr<GeometryFormat> format);

	ptr<File> GetVertices() const;
	size_t GetVertexStride() const;
	ptr<File> GetIndices() const;
	size_t GetIndexStride() const;
	ptr<GeometryFormat> GetFormat() const;
};

END_INANITY_GRAPHICS

#endif