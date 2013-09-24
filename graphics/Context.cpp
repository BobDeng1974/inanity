#include "Context.hpp"
#include "FrameBuffer.hpp"
#include "RenderBuffer.hpp"
#include "DepthStencilBuffer.hpp"
#include "Texture.hpp"
#include "SamplerState.hpp"
#include "UniformBuffer.hpp"
#include "VertexShader.hpp"
#include "PixelShader.hpp"
#include "AttributeBinding.hpp"
#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"
#include "BlendState.hpp"

BEGIN_INANITY_GRAPHICS

Context::Context() : forceReset(true) {}

ContextState& Context::GetTargetState()
{
	return targetState;
}

void Context::Reset()
{
	forceReset = true;
}

END_INANITY_GRAPHICS
