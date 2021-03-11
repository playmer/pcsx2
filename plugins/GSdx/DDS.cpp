#include "stdafx.h"
#include <DDS.h>

DDS::DDSFile DDS::ReadDDS(const char* fileName) {
	std::fstream binaryIo;
	char* _headerData = new char[0x80];

	DDSHeader* _header;
	DDSFile _returnFile;

	binaryIo.open(fileName, std::ios::in | std::ios::binary);
	binaryIo.read(_headerData, 0x80);

	_header = reinterpret_cast<DDSHeader*>(_headerData);

	if (_header->dwMagic == 0x20534444 && 
	   (_header->ddspf.dwFlags & 0x40) == 0x40 &&
	    _header->ddspf.dwRGBBitCount == 32 &&
	   (_header->dwCaps & 0x1000) == 0x1000)
	{
		int const _len = _header->dwHeight * _header->dwWidth * 4;

		std::vector<unsigned char> _tmpData;
		std::vector<unsigned char> _tmpFix;

		_tmpData.resize(_len);

		binaryIo.read(reinterpret_cast<char*>(_tmpData.data()), _len);
		binaryIo.close();

		_returnFile.Header = *_header;
		_returnFile.Data = _tmpData;

		if (_header->ddspf.dwRBitMask == 0x00FF0000) {
			for (int i = 0; i < _len; i += 4) {
				_tmpFix.push_back(_tmpData.at(i + 2));
				_tmpFix.push_back(_tmpData.at(i + 1));
				_tmpFix.push_back(_tmpData.at(i + 0));
				_tmpFix.push_back(_tmpData.at(i + 3));
			}

			_returnFile.Data = _tmpFix;
		}

		return _returnFile;
	}

	else
		return DDSFile();
}

bool DDS::WriteDDS(const std::string& fileName, uint32 dataSize, uint32 width, uint32 height, const uint8* pixelData) {
	std::ofstream _out(fileName, std::ios::binary);

	const int _zero = 0x00;
	const uint32 _output[] = {0x7C, 0x02100F, height, width, 0x800, 0x01, 0x01};
	const uint32 _output2[] = {0x20, 0x41, 0x00, 0x20, 0xFF, 0xFF00, 0xFF0000, 0xFF000000, 0x1000};

	std::vector<unsigned char> _imgData;
	_imgData.resize(dataSize);

	memcpy(_imgData.data(), pixelData, dataSize);

	for (int i = 3; i < dataSize; i += 4)
	{
		unsigned char const _factor = 2;
		int _value = std::min(_imgData.at(i) * _factor, 255);

		_imgData.at(i) = static_cast<unsigned char>(_value);
	}

	_out << "DDS ";
	_out.write(reinterpret_cast<const char*>(&_output), 4 * 7);

	for (int o = 0; o < 11; o++)
		_out.write(reinterpret_cast<const char*>(&_zero), 4);

	_out.write(reinterpret_cast<const char*>(&_output2), 4 * 9);

	for (int o = 0; o < 4; o++)
		_out.write(reinterpret_cast<const char*>(&_zero), 4);
	
	_out.write(reinterpret_cast<const char*>(_imgData.data()), dataSize);
	_out.close();

	return true;
}
