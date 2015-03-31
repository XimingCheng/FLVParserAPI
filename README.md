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