#ifndef ___INANITY_GUI_FONT_GLYPHS_HPP___
#define ___INANITY_GUI_FONT_GLYPHS_HPP___

#include "gui.hpp"
#include <vector>

BEGIN_INANITY_GUI

/// Abstract font glyphs class.
/** This class depends on Canvas. */
class FontGlyphs : public Object
{
public:
	/// Glyph info (for creating glyphs).
	/** All coordinates in pixels, (0, 0) is top-left. */
	struct GlyphInfo
	{
		/// Size of glyph on texture.
		int width, height;
		/// Coordinates of left-top corner of the glyph on texture.
		int leftTopX, leftTopY;
		/// Offset from pen point to left-top corner on canvas.
		int offsetX, offsetY;
	};

	typedef std::vector<GlyphInfo> GlyphInfos;

protected:
	GlyphInfos glyphInfos;
	/// Scale of glyphs.
	/** Glyphs may be upscaled, so we need to render them downscaled. */
	int scaleX, scaleY;

	FontGlyphs(const GlyphInfos& glyphInfos, int scaleX, int scaleY);

public:
	const GlyphInfos& GetGlyphInfos() const;
	int GetScaleX() const;
	int GetScaleY() const;
};

END_INANITY_GUI

#endif
