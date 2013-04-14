#include "DxInternalInputLayoutCache.hpp"
#include "Layout.hpp"
#include "DxVertexShader.hpp"
#include "DxInternalInputLayout.hpp"
#include "DxContext.hpp"

BEGIN_INANITY_GRAPHICS

DxInternalInputLayoutCache::Key::Key(Layout* vertexLayout, DxVertexShader* vertexShader)
: vertexLayout(vertexLayout), vertexShader(vertexShader) {}

DxInternalInputLayoutCache::Key::operator size_t() const
{
	return size_t(vertexLayout) ^ size_t(vertexShader);
}

DxInternalInputLayoutCache::DxInternalInputLayoutCache(DxContext* context)
: context(context) {}

ptr<DxInternalInputLayout> DxInternalInputLayoutCache::GetInputLayout(Layout* vertexLayout, DxVertexShader* vertexShader)
{
	Key key(vertexLayout, vertexShader);
	InputLayouts::const_iterator i = inputLayouts.find(key);
	if(i != inputLayouts.end())
		return i->second;

	ptr<DxInternalInputLayout> inputLayout = context->CreateInternalInputLayout(vertexLayout, vertexShader);

	inputLayouts[key] = inputLayout;

	return inputLayout;
}

END_INANITY_GRAPHICS
