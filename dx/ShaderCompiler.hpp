#ifndef ___INANITY_DX_SHADER_COMPILER_HPP___
#define ___INANITY_DX_SHADER_COMPILER_HPP___

#include "dx.hpp"
#include "../../String.hpp"

BEGIN_INANITY

class File;

END_INANITY

BEGIN_INANITY_GRAPHICS

class ShaderSource;

END_INANITY_GRAPHICS

BEGIN_INANITY_DX

/// ����� ����������� �������� �� ����������.
/** ������ ����� ����������� �������� ��� ������� � ���������� ������ � ����-����,
�������� ������ �������� ����������.
*/
class ShaderCompiler : public Object
{
private:
	/// �������� ����������� ������� ��������.
	bool debug;
	/// �������� �����������.
	bool optimize;
	/// ������������ column-major �������.
	bool columnMajorMatrices;

public:
	/// ������� ���������� � ����������� �� ���������.
	ShaderCompiler();
	/// ������� ���������� � ���������� �����������.
	ShaderCompiler(bool debug, bool optimize, bool columnMajorMatrices = false);

	/// �������������� ������.
	ptr<File> Compile(ptr<ShaderSource> shaderSource);
};

END_INANITY_DX

#endif
