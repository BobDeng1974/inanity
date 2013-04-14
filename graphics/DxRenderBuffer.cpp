#include "DxRenderBuffer.hpp"
#include "DxTexture.hpp"

BEGIN_INANITY_GRAPHICS

DxRenderBuffer::DxRenderBuffer(ComPointer<ID3D11RenderTargetView> renderTargetView, ptr<DxTexture> texture)
: renderTargetView(renderTargetView), texture(texture)
{
}

ptr<Texture> DxRenderBuffer::GetTexture()
{
	return texture;
}

ID3D11RenderTargetView* DxRenderBuffer::GetRenderTargetViewInterface() const
{
	return renderTargetView;
}

END_INANITY_GRAPHICS
