#include "ShaderCompiler.hpp"
#include "../ShaderSource.hpp"
#include "../../FileSystem.hpp"
#include "../../File.hpp"
#include "D3D10BlobFile.hpp"
#include "../../Exception.hpp"
#include "../../Strings.hpp"
#include "d3dx.hpp"

/// ����� ����������� ���������� ������.
/** ������������� ��������� ID3D10Include, ����������� ��� ���������� ��������,
��������� ����� �� �������� �������� �������. */
class IncludeProcessor : public Object, public ID3D10Include
{
private:
	/// �������� �������.
	ptr<FileSystem> fileSystem;
	/// ������ �������� � ������ ������ ������
	/** ���������� � ���� ���� �������, ������ ��� ������� ����� ����� ������������
	���� ���������� ������, � ��� ����� �������. */
	std::vector<ptr<File> > files;

public:
	IncludeProcessor(ptr<FileSystem> fileSystem) : fileSystem(fileSystem) {}

	/// ������� ����.
	HRESULT STDMETHODCALLTYPE Open(D3D10_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes)
	{
		//���� ���� �� ���������, ��������� (������ ��� ��������� ������ ���� �� ������������)
		if(IncludeType != D3D10_INCLUDE_LOCAL)
			return E_FAIL;

		//�������� ���� �� �������� �������
		ptr<File> file = fileSystem->TryLoadFile(pFileName);
		if(!file)
			return E_FAIL;

		//�������� ��� � ������ ��������
		files.push_back(file);
		
		//������� ��������� �� ������ �����, � ������ �����
		*ppData = file->GetData();
		*pBytes = (UINT)file->GetSize();

		return S_OK;
	}

	/// ������� ����.
	HRESULT STDMETHODCALLTYPE Close(LPCVOID pData)
	{
		//����� � ������� ����
		for(size_t i = 0; i < files.size(); ++i)
			if(files[i]->GetData() == pData)
			{
				files.erase(files.begin() + i);
				return S_OK;
			}
		//���� ���������� �����, ������, ��� ������ ������� ���������� ����
		return E_FAIL;
	}
};

ShaderCompiler::ShaderCompiler()
	:
#ifdef _DEBUG
	debug(true), optimize(false), columnMajorMatrices(false)
#else
	debug(false), optimize(true), columnMajorMatrices(false)
#endif
{
}

ShaderCompiler::ShaderCompiler(bool debug, bool optimize, bool columnMajorMatrices)
	: debug(debug), optimize(optimize), columnMajorMatrices(columnMajorMatrices)
{
}

ptr<File> ShaderCompiler::Compile(ptr<ShaderSource> shaderSource)
{
	try
	{
		//�������� ��� �������
		ptr<File> code = shaderSource->GetCode();

		//������� ���������� ���������� ������, ���� �����
		ptr<FileSystem> includesFileSystem = shaderSource->GetIncludes();
		ptr<IncludeProcessor> includeProcessor;
		if(includesFileSystem)
			includeProcessor = NEW(IncludeProcessor(includesFileSystem));

		//�������������� ������
		ID3D10Blob* shaderBlob;
		ID3D10Blob* errorsBlob;
		HRESULT result = D3DX11CompileFromMemory((char*)code->GetData(), code->GetSize(), NULL, NULL,
			includeProcessor ? includeProcessor : 0, shaderSource->GetFunctionName().c_str(),
			shaderSource->GetProfile().c_str(), D3D10_SHADER_ENABLE_STRICTNESS
			| (debug ? D3D10_SHADER_DEBUG : 0)
			| (optimize ? D3D10_SHADER_OPTIMIZATION_LEVEL3 : (D3D10_SHADER_OPTIMIZATION_LEVEL0 | D3D10_SHADER_SKIP_OPTIMIZATION))
			| (columnMajorMatrices ? D3D10_SHADER_PACK_MATRIX_COLUMN_MAJOR : D3D10_SHADER_PACK_MATRIX_ROW_MAJOR)
			, 0,NULL, &shaderBlob, &errorsBlob, NULL);

		//��������� blob'� � ������ ��� ����������� ��������
		ptr<D3D10BlobFile> shaderFile = NEW(D3D10BlobFile(shaderBlob));
		ptr<D3D10BlobFile> errorsFile = NEW(D3D10BlobFile(errorsBlob));

		//���� ������
		if(FAILED(result))
			THROW_PRIMARY_EXCEPTION("Compile error(s): " + Strings::File2String(errorsFile));

		//������� ���� � ��������
		return shaderFile;
	}
	catch(Exception* exception)
	{
		THROW_SECONDARY_EXCEPTION("Can't compile shader", exception);
	}
}
