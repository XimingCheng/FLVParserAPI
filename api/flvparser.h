/**
* This file is part of FLVParser.

* FLVParser is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* FLVParser is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with FLVParser.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef FLVPARSER_H_
#define FLVPARSER_H_

#include "common.h"

#include <functional>

FLVPARSER_NAMESPACE_BEGIN

enum FLVTagType
{
    TagTypeAudio  = 8,
    TagTypeVideo  = 9,
    TagTypeScript = 18
};

enum SoundFormat
{
    LinearPCM = 0,
    ADPCM,
    MP3,
    LinearPCMLittleEndian,
    Nellymoser16kHz,
    Nellymoser8kHz,
    Nellymoser,
    ALawLogarithmicPCM,
    MuLawLogarithmicPCM,
    Reserved,
    AAC,
    Speex,
    MP38kHz,
    DeviceSpecific
};

enum SoundRate
{
    Rate5dot5kHz = 0,
    Rate11kHz,
    Rate22kHz,
    Rate44kHz
};

enum SoundSize
{
    Size8bit = 0,
    Size16bit
};

enum SoundType
{
    MonoSound = 0,
    StereoSound
};

enum AACPacketType
{
    AACSequenceHeader = 0,
    AACRaw
};

enum FrameType
{
    KeyFrame = 1,
    InterFrame,
    DisposableInterFrame,
    GeneratedKeyFrame,
    VideoInfo
};

enum CodecID
{
    SorensonH263 = 2,
    ScreenVideo,
    VP6,
    VP6WithAlpha,
    ScreenVideoV2,
    AVC
};

#pragma pack(push)
#pragma pack(1)

struct FLVHeader
{
    uint8_t     _signature[3];              //!< Signature byte "FLV"
    uint8_t     _version;                   //!< File version
    uint8_t     _typeFlagsVideo     : 1;    //!< 1 = Video tags are present
    uint8_t     _typeFlagsReserved2 : 1;    //!< Shall be 0
    uint8_t     _typeFlagsAudio     : 1;    //!< 1 = Audio tags are present
    uint8_t     _typeFlagsReserved1 : 5;    //!< Shall be 0
    uint8_t     _dataOffset[4];             //!< The length of this header in bytes
};

struct FLVTag
{
    struct FLVTagHeader
    {
        uint8_t     _tagType        : 5;    //!< Type of contents in this tag
        uint8_t     _filter         : 1;    //!< Indicates if packets are filtered
        uint8_t     _reserved       : 2;    //!< Reserved for FMS, should be 0
        uint8_t     _dataSize[3];           //!< Number of bytes after StreamID to end of tag
        uint8_t     _timestamp[3];          //!< Time in milliseconds at which the data in this tag applies
        uint8_t     _timestampExtended;     //!< This field represents the upper 8 bits
        uint8_t     _streamID[3];           //!< Always 0
    }
    _header;
    void*       _data;                      //!< (AudioTagHeader | VideoTagHeader) (EncryptionHeader FilterParams)? (data)
};

struct AudioTag
{
    struct AudioTagHeader
    {
        uint8_t     _soundType      : 1;    //!< Mono or stereo sound
        uint8_t     _soundSize      : 1;    //!< Size of each audio sample
        uint8_t     _soundRate      : 2;    //!< Sampling rate
        uint8_t     _soundFormat    : 4;    //!< Format of SoundData
    }
    _header;
    void*       _data;
};

struct VideoTag
{
    struct VideoTagHeader
    {
        uint8_t     _codecID        : 4;    //!< Codec identifier
        uint8_t     _frameType      : 4;    //!< Type of video frame
    }
    _header;
    void*       _data;
};

struct AACPacket
{
    uint8_t     _AACPacketType;             //!< AAC sequence header or AAC raw
    void*       _data;
};

struct AVCPacket
{
    struct AVCPacketHeader
    {
        uint8_t     _AVCPacketType;         //!< AVC type
        uint8_t     _compositionTime[3];    //!< Composition time offset
    }
    _header;
    void*       _data;
};
#pragma pack(pop)

// std::function bind for parsing flv data

using ParsingFLVHeader = std::function<void(FLVHeader*,
                                            uint32_t
                                            )>;

using ParsingVideoTag  = std::function<void(FLVTag*,
                                            int,
                                            uint32_t,
                                            AVCPacket::AVCPacketHeader*,
                                            uint8_t
                                            )>;

using ParsingAudioTag  = std::function<void(FLVTag*,
                                            int,
                                            uint32_t,
                                            uint8_t
                                            )>;

using ParsingScriptTag = std::function<void(FLVTag*,
                                            int,
                                            uint32_t
                                            )>;

void DoNothingOnFLVHeader(FLVHeader*, uint32_t);
void DoNothingOnVideoTag(FLVTag*, int, uint32_t, AVCPacket::AVCPacketHeader*, uint8_t);
void DoNothingOnAudioTag(FLVTag*, int, uint32_t, uint8_t);
void DoNothingOnScriptTag(FLVTag*, int, uint32_t);

enum ScriptDataType
{
    DOUBLE = 0,
    BOOLEAN,
    STRING,
    OBJECT,
    MOVIE_CLIP, // reserved, not supported
    NULL_DATA,
    UNDEFINED,
    REFERENCE,
    ECMA_ARRAY,
    OBJECT_END_MARKER,
    STRICT_ARRAY,
    DATA_DATE,
    LONG_STRING
};

struct ScriptData
{
    uint8_t             _type;
    void*               _data;
    void*               _extra {nullptr};
    ScriptData*         _value;
};

struct ScriptDataDate
{
    double              _dateTime;
    int16_t             _localDateTimeOffset;
};

class ScriptKVDataParser
{
public:
    ScriptKVDataParser(FLVTag* scriptTag, int size);
    ~ScriptKVDataParser();
    bool Parse();
    void Free();

private:

    void                ParseScriptTagBody(int& offset, ScriptData*& scriptTagBody, bool bKey);
    void                FreeData(ScriptData* data, bool bKey);

    ScriptData*         _scriptTagBody { nullptr };
    FLVTag*             _scriptTag;
    int                 _size;

    
};

class FLVParser
{
public:
    FLVParser(const char* inputFile,
              ParsingFLVHeader pH = &DoNothingOnFLVHeader,
              ParsingVideoTag pV  = &DoNothingOnVideoTag,
              ParsingAudioTag pA  = &DoNothingOnAudioTag,
              ParsingScriptTag pS = &DoNothingOnScriptTag);

    ~FLVParser();

    FLVParser(const FLVParser&)             = delete;
    FLVParser& operator= (const FLVParser&) = delete;

    bool Parse();

private:
    inline bool         ParseFLVHeader();
    inline bool         ParseFLVTag();
    inline bool         ParseAudioTag(const FLVTag::FLVTagHeader* header);
    inline bool         ParseVideoTag(const FLVTag::FLVTagHeader* header);
    inline bool         ParseScriptTag(const FLVTag::FLVTagHeader* header);

private:
    ParsingFLVHeader    _pH;
    ParsingVideoTag     _pV;
    ParsingAudioTag     _pA;
    ParsingScriptTag    _pS;

    FILE*               _flvFile    { nullptr };
    bool                _bHasVideo  { false };
    bool                _bHasAudio  { false };
};

class ScriptTagKVParser;

FLVPARSER_NAMESPACE_END

#endif // FLVPARSER_H_