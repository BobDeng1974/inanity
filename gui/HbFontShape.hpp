#ifndef ___INANITY_GUI_HB_FONT_SHAPE_HPP___
#define ___INANITY_GUI_HB_FONT_SHAPE_HPP___

#include "FontShape.hpp"
#include "hb.hpp"
#include "../deps/icu/repo/source/common/unicode/locid.h"

BEGIN_INANITY_GUI

class FontFace;

/// Font shape for HarfBuzz text shaping.
class HbFontShape : public FontShape
{
private:
	ptr<FontFace> face;
	hb_font_t* font;
	hb_buffer_t* buffer;

public:
	HbFontShape(ptr<FontFace> face, hb_font_t* font);
	~HbFontShape();

	/// FontShape's methods.
	/** Out parameters are optional. */
	void Shape(const String& text, Script script, Graphics::vec2* outSize, std::vector<OutGlyph>* outGlyphs);

	void ShapeParagraph(const String& text, Script script, const icu::Locale& locale, float maxLineWidth, float lineHeight, std::vector<OutGlyph>* outGlyphs);
};

END_INANITY_GUI

#endif
