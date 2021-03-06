#ifndef ___INANITY_DATA_BASE64_OUTPUT_STREAM_HPP___
#define ___INANITY_DATA_BASE64_OUTPUT_STREAM_HPP___

#include "data.hpp"
#include "../OutputStream.hpp"

BEGIN_INANITY

class File;

END_INANITY

BEGIN_INANITY_DATA

/// Поток, кодирующий/декодирующий поступающие данные в Base64.
class Base64OutputStream : public OutputStream
{
private:
	static const char schema[65];
	static unsigned char inverseSchema[256];

	bool encoding;
	ptr<OutputStream> outputStream;
	/// 24-битный буфер данных
	unsigned buffer;
	/// сколько ещё в буфере места, в битах
	int bufferSize;
	/// завершение кодирования
	bool flushed;

private:
	void WriteEncode(unsigned char byte);
	void ProcessEncodeBuffer();
	void WriteDecode(unsigned char byte);
	void ProcessDecodeBuffer();

public:
	Base64OutputStream(bool encoding, ptr<OutputStream> outputStream);
	~Base64OutputStream();

	void Write(const void* data, size_t size);

	void Flush();

	static ptr<File> EncodeFile(ptr<File> file);
};

END_INANITY_DATA

#endif
