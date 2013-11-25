#include "TextDrawer.hpp"
#include "Font.hpp"
#include "Texture.hpp"
#include "UniformBuffer.hpp"
#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"
#include "VertexShader.hpp"
#include "PixelShader.hpp"
#include "BlendState.hpp"
#include "SamplerState.hpp"
#include "Device.hpp"
#include "Context.hpp"
#include "ShaderCache.hpp"
#include "VertexLayout.hpp"
#include "VertexLayoutElement.hpp"
#include "AttributeLayout.hpp"
#include "AttributeLayoutSlot.hpp"
#include "AttributeLayoutElement.hpp"
#include "AttributeBinding.hpp"
#include "RenderBuffer.hpp"
#include "DepthStencilBuffer.hpp"
#include "../MemoryFile.hpp"
#include "../StringTraveler.hpp"
#include "../Exception.hpp"
#include "../inanity-shaders.hpp"

BEGIN_INANITY_GRAPHICS

using namespace Shaders;

/// Вспомогательная структура для рисователя.
struct TextDrawerHelper : public Object
{
	/// Максимальное количество символов, рисуемое за раз.
	static const int maxSymbolsCount = 64;

	ptr<VertexLayout> vl;
	ptr<VertexLayoutElement> vlePosition;
	ptr<AttributeLayout> al;
	ptr<AttributeLayoutSlot> als;
	ptr<AttributeLayoutElement> alePosition;
	ptr<Instancer> instancer;
	ptr<AttributeBinding> ab;

	Value<vec4> aCorner;

	Interpolant<vec2> iTexcoord;
	Interpolant<vec4> iColor;

	ptr<UniformGroup> ugSymbols;
	/// Координаты символов на экране.
	UniformArray<vec4> uPositions;
	/// Текстурные координаты символов.
	UniformArray<vec4> uTexcoords;
	/// Цвета символов.
	UniformArray<vec4> uColors;

	/// Текстура шрифта.
	/** Задаёт альфу для шрифта. */
	Sampler<float, 2> uFontSampler;
	/// Sampler state.
	ptr<SamplerState> ss;

	ptr<VertexBuffer> vb;
	ptr<VertexShader> vs;
	ptr<PixelShader> ps;
	ptr<BlendState> bs;

	TextDrawerHelper(ptr<Device> device, ptr<ShaderCache> shaderCache) :
		vl(NEW(VertexLayout(sizeof(vec4)))),
		vlePosition(vl->AddElement(DataTypes::_vec4, 0)),
		al(NEW(AttributeLayout())),
		als(al->AddSlot()),
		alePosition(al->AddElement(als, vlePosition)),
		instancer(NEW(Instancer(device, maxSymbolsCount, al))),
		ab(device->CreateAttributeBinding(al)),

		aCorner(alePosition),

		iTexcoord(1),
		iColor(2),

		ugSymbols(NEW(UniformGroup(0))),
		uPositions(ugSymbols->AddUniformArray<vec4>(maxSymbolsCount)),
		uTexcoords(ugSymbols->AddUniformArray<vec4>(maxSymbolsCount)),
		uColors(ugSymbols->AddUniformArray<vec4>(maxSymbolsCount)),

		uFontSampler(0)
	{
		try
		{
			ugSymbols->Finalize(device);

			// создать геометрию
			static const vec4 vertices[6] =
			{
				vec4(0, 1, 1, 0),
				vec4(1, 1, 0, 0),
				vec4(1, 0, 0, 1),
				vec4(0, 1, 1, 0),
				vec4(1, 0, 0, 1),
				vec4(0, 0, 1, 1)
			};

			vb = device->CreateStaticVertexBuffer(MemoryFile::CreateViaCopy(vertices, sizeof(vertices)), vl);

			// вершинный шейдер
			Temp<uint> tmpInstance;
			Temp<vec4> tmpPosition, tmpTexcoord, tmpColor;
			vs = shaderCache->GetVertexShader((
				tmpInstance = instancer->GetInstanceID(),

				tmpPosition = uPositions[tmpInstance],
				tmpTexcoord = uTexcoords[tmpInstance],
				tmpColor = uColors[tmpInstance],

				setPosition(newvec4(
					dot(aCorner["xz"], tmpPosition["xz"]),
					dot(aCorner["yw"], tmpPosition["yw"]),
					0, 1)),
				iTexcoord = newvec2(
					dot(aCorner["xz"], tmpTexcoord["xz"]),
					dot(aCorner["yw"], tmpTexcoord["yw"])),
				iColor = tmpColor
			));

			// пиксельный шейдер
			ps = shaderCache->GetPixelShader((
				iTexcoord,
				iColor,

				fragment(0, newvec4(iColor["xyz"], iColor["w"] * uFontSampler.Sample(iTexcoord)))
			));

			// настройки смешивания
			bs = device->CreateBlendState();
			bs->SetColor(BlendState::colorSourceSrcAlpha, BlendState::colorSourceInvSrcAlpha, BlendState::operationAdd);

			// настройки семплирования для шрифта
			ss = device->CreateSamplerState();
			ss->SetFilter(SamplerState::filterLinear, SamplerState::filterLinear, SamplerState::filterLinear);
			ss->SetWrap(SamplerState::wrapBorder, SamplerState::wrapBorder, SamplerState::wrapBorder);
			float color[] = { 0, 0, 0, 0 };
			ss->SetBorderColor(color);
		}
		catch(Exception* exception)
		{
			THROW_SECONDARY("Can't create text drawer helper", exception);
		}
	}
};

TextDrawer::TextDrawer(ptr<TextDrawerHelper> helper)
: helper(helper), queuedCharsCount(0) {}

void TextDrawer::Prepare(ptr<Context> context, int viewportWidth, int viewportHeight)
{
	this->context = context;

	// сбросить текущий шрифт и текстуру
	currentFont = 0;
	currentFontTexture = 0;

	// вычислить масштабирование
	scaleX = 2.0f / viewportWidth;
	scaleY = 2.0f / viewportHeight;
}

void TextDrawer::SetFont(ptr<Font> font)
{
	// запомнить шрифт
	currentFont = font;
	// если текстура поменялась
	ptr<Texture> texture = font->GetTexture();
	if(texture != currentFontTexture)
	{
		// сбросить очередь
		Flush();
		// запомнить новую текстуру
		currentFontTexture = texture;
	}
}

void TextDrawer::DrawSymbol(const vec4& position, const vec4& texcoord, const vec4& color)
{
	// если очередь переполнена
	if(queuedCharsCount >= TextDrawerHelper::maxSymbolsCount)
		// сбросить очередь
		Flush();

	// добавить символ в очередь
	helper->uPositions.SetValue(queuedCharsCount, position);
	helper->uTexcoords.SetValue(queuedCharsCount, texcoord);
	helper->uColors.SetValue(queuedCharsCount, color);
	++queuedCharsCount;
}

void TextDrawer::DrawTextLine(const String& text, float x, float y, const vec4& color, int alignment)
{
	Font* font = currentFont;

	//получить высоту символов
	float charHeight = font->GetCharHeight() * scaleY;

	//определить ширину строки
	float stringWidth = 0;
	{
		StringTraveler traveler(text);
		wchar_t symbol, lastSymbol = (wchar_t)-1;
		while(traveler.Next(symbol))
		{
			// если это не первый символ в строке
			if(lastSymbol != (wchar_t)-1)
				// добавить кернинг
				stringWidth += font->GetKerning(lastSymbol, symbol);
			// добавить ширину символа
			stringWidth += font->GetChar(symbol).width;
			// запомнить символ для следующей кернинг-пары
			lastSymbol = symbol;
		}
		// масштабирование
		stringWidth *= scaleX;
	}

	//определить высоту строки (пока просто - многострочный текст не поддерживается)
	float stringHeight = charHeight;

	//установить реальные x и y в соответствии с выравниванием
	//для FontAlignment::Left ничего не надо
	switch(alignment & FontAlignments::HorizontalMask)
	{
	case FontAlignments::Center:
		x -= stringWidth * 0.5f;
		break;
	case FontAlignments::Right:
		x -= stringWidth;
		break;
	}
	//для FontAlignment::Bottom ничего не надо
	switch(alignment & FontAlignments::VerticalMask)
	{
	case FontAlignments::Top:
		y -= stringHeight;
		break;
	case FontAlignments::Middle:
		y -= stringHeight * 0.5f;
		break;
	}

	//левая граница
	float left = x;

	//цикл обработки символов
	StringTraveler traveler(text);
	wchar_t symbol, lastSymbol = (wchar_t)-1;
	while(traveler.Next(symbol))
	{
		// если это не первый символ
		if(lastSymbol != (wchar_t)-1)
			// сдвинуть левую границу на кернинг
			left += font->GetKerning(lastSymbol, symbol) * scaleX;
		//получить символ
		const FontChar& fontChar = font->GetChar(symbol);
		//получить ширину символа
		float charWidth = fontChar.width * scaleX;

		// вывести символ
		DrawSymbol(
			vec4(
				left + fontChar.screenFirstUV.x * scaleX, y + fontChar.screenFirstUV.y * scaleY,
				left + fontChar.screenSecondUV.x * scaleX, y + fontChar.screenSecondUV.y * scaleY
			),
			vec4(fontChar.firstUV.x, fontChar.firstUV.y, fontChar.secondUV.x, fontChar.secondUV.y),
			color
		);

		//сдвинуть левую границу
		left += charWidth;

		// запомнить символ
		lastSymbol = symbol;
	}
}

void TextDrawer::Flush()
{
	// если очередь пуста, делать нечего
	if(!queuedCharsCount)
		return;

	Context::LetAttributeBinding lab(context, helper->ab);
	Context::LetVertexBuffer lvb(context, 0, helper->vb);
	Context::LetIndexBuffer lib(context, 0);
	Context::LetVertexShader lvs(context, helper->vs);
	Context::LetPixelShader lps(context, helper->ps);
	Context::LetBlendState lbs(context, helper->bs);
	Context::LetCullMode lcm(context, Context::cullModeNone);
	Context::LetFillMode lfm(context, Context::fillModeSolid);
	Context::LetDepthTestFunc ldtf(context, Context::depthTestFuncAlways);
	Context::LetDepthWrite ldw(context, false);
	Context::LetUniformBuffer lub(context, helper->ugSymbols);
	Context::LetSampler ls(context, helper->uFontSampler, currentFontTexture, helper->ss);

	// загрузить символы
	helper->ugSymbols->Upload(context);

	// нарисовать
	helper->instancer->Draw(context, queuedCharsCount);

	// сбросить количество символов
	queuedCharsCount = 0;
}

ptr<TextDrawer> TextDrawer::Create(ptr<Device> device, ptr<ShaderCache> shaderCache)
{
	return NEW(TextDrawer(NEW(TextDrawerHelper(device, shaderCache))));
}

END_INANITY_GRAPHICS
