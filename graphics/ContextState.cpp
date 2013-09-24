#include "ContextState.hpp"
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

ContextState::ContextState() :
	fillMode(fillModeSolid), cullMode(cullModeBack),
	viewportWidth(1), viewportHeight(1),
	depthTestFunc(depthTestFuncLess), depthWrite(true)
{
}

void ContextState::ResetVertexBuffers()
{
	for(int i = 0; i < vertexBufferSlotsCount; ++i)
		vertexBuffers[i] = 0;
}

void ContextState::ResetTextures()
{
	for(int i = 0; i < textureSlotsCount; ++i)
		textures[i] = 0;
}

void ContextState::ResetSamplerStates()
{
	for(int i = 0; i < textureSlotsCount; ++i)
		samplerStates[i] = 0;
}

void ContextState::ResetUniformBuffers()
{
	for(int i = 0; i < uniformBufferSlotsCount; ++i)
		uniformBuffers[i] = 0;
}

END_INANITY_GRAPHICS
