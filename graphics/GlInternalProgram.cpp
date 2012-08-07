#include "GlInternalProgram.hpp"

GlInternalProgram::GlInternalProgram(GLuint name) : name(name) {}

GlInternalProgram::~GlInternalProgram()
{
	glDeleteProgram(name);
}

GLuint GlInternalProgram::GetName() const
{
	return name;
}
