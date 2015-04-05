FLVParserAPI
============

[![Build Status](https://api.travis-ci.org/XimingCheng/FLVParserAPI.png)](https://travis-ci.org/XimingCheng/FLVParserAPI)

FLVParserAPI is a set of APIs for analysis the FLV container files.

Build With CMake
----------------

```sh
git clone https://github.com/XimingCheng/FLVParserAPI.git
cd FLVParserAPI
mkdir build
cd build
cmake -G "Unix Makefiles" ..
make
```

Main features
-------------

* Support BIG/LITTILE ENDIAN machine
* FLV Header Parsing
* FLV Audio Tag Analysis
	* Audio Codec outputs
	* Audio SoundRate/SoundSize/Formats/SoundType AND AACPacketType
* FLV Video Tag Analysis
	* Video Codec outputs
	* Video FrameType/CodecID
* FLV meta script Tag Analysis
	* onMetaData Analysis
	* ECMA array Analysis
	* Strict array
	* different kinds of type Analysis

Example
-------

* Audio Information Detection
* Video Information Detection
* Script Data Information Detection

```cpp
#include "../api/flvparser.h"

using namespace flvparser;

void PrintFLVHeader(FLVHeader* header, uint32_t preSize)
{
    std::cout << "FLV file header detected!" << std::endl;
    std::cout << "Signature -> " << header->_signature[0] <<
        header->_signature[1] << header->_signature[2] << std::endl;
}

void PrintAudioTag(FLVTag* tag, int size, uint32_t preSize, uint8_t AACPacketType)
{
    std::cout << "Audio Tag detected!" << std::endl;
    AudioTag* audio = static_cast<AudioTag*>(tag->_data);
    // the code below user can define its own code
    switch (audio->_header._soundFormat)
    {
    case LinearPCM:
         // user define code
        break;
    default:
        break;
    }
}

void PrintVideoTag(FLVTag* tag, int size, uint32_t preSize, AVCPacket::AVCPacketHeader* AVCHeader, uint8_t vp8Byte)
{
    std::cout << "Video Tag detected!" << std::endl;
    VideoTag* video = static_cast<VideoTag*>(tag->_data);
    // the code below user can define its own code
    switch (video->_header._codecID)
    {
    default:
        break;
    }
    switch (video->_header._frameType)
    {
    default:
        break;
    }
}

void PrintScriptTag(FLVTag* tag, int size, uint32_t preSize)
{
    // ScriptKVDataParser is a KV data container
    ScriptKVDataParser KVParser(tag, size);
    KVParser.Parse();
}
```