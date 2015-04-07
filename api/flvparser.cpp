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

#include "common.h"
#include "flvparser.h"

#include <string.h>
#include <memory>
#include <vector>

FLVPARSER_NAMESPACE_BEGIN

void DoNothingOnFLVHeader(FLVHeader*, uint32_t) {}
void DoNothingOnVideoTag(FLVTag*, int, uint32_t, AVCPacket::AVCPacketHeader*, uint8_t) {}
void DoNothingOnAudioTag(FLVTag*, int, uint32_t, uint8_t) {}
void DoNothingOnScriptTag(FLVTag*, int, uint32_t) {}

ScriptKVDataParser::ScriptKVDataParser(FLVTag* scriptTag, int size)
                : _scriptTag(scriptTag),
                  _size(size)
{

}

ScriptKVDataParser::~ScriptKVDataParser()
{
    if (_scriptTagBody)
        Free();
}

void ScriptKVDataParser::Free()
{
    // Free the parsing tree memory
    if (_scriptTagBody)
    {
        FreeData(_scriptTagBody, true);
        delete _scriptTagBody;
        _scriptTagBody = nullptr;
    }
}

void ScriptKVDataParser::FreeData(ScriptData* data, bool bKey)
{
    switch (data->_type)
    {
    case DOUBLE:
    {
        double* p = (double*)data->_data;
        delete p;
        if (bKey)
            FreeData(data->_value, false);
    }
        break;
    case BOOLEAN:
    {
        bool* p = (bool*)data->_data;
        delete p;
        if (bKey)
            FreeData(data->_value, false);
    }
        break;
    case REFERENCE:
    {
        uint16_t* p = (uint16_t*)data->_data;
        delete p;
        if (bKey)
            FreeData(data->_value, false);
    }
        break;
    case DATA_DATE:
    {
        uint64_t* p = (uint64_t*)data->_data;
        delete p;
        if (bKey)
            FreeData(data->_value, false);
    }
        break;
    case STRING:
    case LONG_STRING:
    {
        char* str = (char*)data->_data;
        delete [] str;
        if (bKey)
            FreeData(data->_value, false);
    }
        break;
    case MOVIE_CLIP:
    case NULL_DATA:
    case UNDEFINED:
    case OBJECT_END_MARKER:
        break;
    case ECMA_ARRAY:
    case STRICT_ARRAY:
    case OBJECT:
    {
        int ArrayLength = *(int*)data->_extra;
        delete &ArrayLength;
        ScriptData* dataArray = (ScriptData*)data->_data;
        for (int idx = 0; idx < ArrayLength; idx++)
        {
            FreeData(&dataArray[idx], true);
        }
        delete[] dataArray;
        delete data;
    }
        break;
    default:
        assert(0);
        break;
    }
}

bool ScriptKVDataParser::Parse()
{
    if (_scriptTag->_header._tagType == TagTypeScript)
    {
        if (_scriptTagBody)
            Free();
        int offset = 0;
        ParseScriptTagBody(offset, _scriptTagBody, true);
        return true;
    }
    return false;
}

void ScriptKVDataParser::ParseScriptTagBody(int& offset, ScriptData*& scriptTagBody, bool bKey)
{
    if (offset >= _size)
        return;
    uint8_t* data = static_cast<uint8_t*>(_scriptTag->_data);
    uint8_t type = data[offset];
    offset += sizeof(uint8_t);
    switch (type)
    {
    case DOUBLE: // double
    {
#if PARSER_ENDIAN == PARSER_LITTLEENDIAN
        uint64_t t = *(uint64_t*)(data + offset);
        uint64_t value64 = t >> 56;
        value64 |= ((t & 0xFF000000000000) >> 40);
        value64 |= ((t & 0xFF0000000000) >> 24);
        value64 |= ((t & 0xFF00000000) >> 8);
        value64 |= ((t & 0xFF000000) << 8);
        value64 |= ((t & 0xFF0000) << 24);
        value64 |= ((t & 0xFF00) << 40);
        value64 |= ((t & 0xFF) << 56);
        double value = *(double*)&value64;
#else
        double value = *(double*)(data + offset);
#endif
        offset += sizeof(double);
        scriptTagBody = new ScriptData;
        scriptTagBody->_type = type;
        scriptTagBody->_data = (void*)(new double(value));
        if (bKey)
            ParseScriptTagBody(offset, scriptTagBody->_value, false);
    }
        break;
    case BOOLEAN: // boolean
    {
        uint8_t value = *(uint8_t*)(data + offset);
        bool v = (value != 0);
        offset += sizeof(uint8_t);
        scriptTagBody = new ScriptData;
        scriptTagBody->_type = type;
        scriptTagBody->_data = (void*)(new bool(v));
        if (bKey)
            ParseScriptTagBody(offset, scriptTagBody->_value, false);
    }
        break;
    case STRING: // String
    {
#if PARSER_ENDIAN == PARSER_LITTLEENDIAN
        uint16_t t = *(uint16_t*)(data + offset);
        uint16_t stringLength = t >> 8;
        stringLength |= ((t & 0xFF) << 8);
#else
        uint16_t stringLength = *(uint16_t*)(data + offset);
#endif
        offset += sizeof(uint16_t);
        char* str = new char[stringLength + 1];
        memcpy(str, data + offset, stringLength * sizeof(char));
        offset += stringLength;
        str[stringLength] = '\0';
        scriptTagBody = new ScriptData;
        scriptTagBody->_type = type;
        scriptTagBody->_data = (void*)str;
        if (bKey)
            ParseScriptTagBody(offset, scriptTagBody->_value, false);
    }
        break;
    case OBJECT: // Object (dose not have the length)
    {
        std::vector<ScriptData*> objs;
        scriptTagBody = new ScriptData;
        scriptTagBody->_type = type;
        while (1)
        {
#if PARSER_ENDIAN == PARSER_LITTLEENDIAN
            uint16_t t = *(uint16_t*)(data + offset);
            uint16_t stringLength = t >> 8;
            stringLength |= ((t & 0xFF) << 8);
#else
            uint16_t stringLength = *(uint16_t*)(data + offset);
#endif
            offset += sizeof(uint16_t);
            char* str = new char[stringLength + 1];
            memcpy(str, data + offset, stringLength * sizeof(char));
            offset += stringLength;
            str[stringLength] = '\0';
            ScriptData* sd = new ScriptData;
            sd->_type = 2;
            sd->_data = (void*)str;
            ParseScriptTagBody(offset, sd->_value, false);
            objs.push_back(sd);

            // detect the end
            if ((data + offset)[0] == 0 &&
                (data + offset)[1] == 0 &&
                (data + offset)[2] == 9)
            {
                offset += 3;
                break;
            }
        }
        ScriptData* dataArray = new ScriptData[objs.size()];
        for (size_t idx = 0; idx < objs.size(); idx++)
        {
            dataArray[idx] = *objs[idx];
            delete objs[idx];
        }
        scriptTagBody->_data = (void*)dataArray;
        scriptTagBody->_extra = new int(objs.size());
        ParseScriptTagBody(offset, scriptTagBody->_value, true);
    }
        break;
    case MOVIE_CLIP: // MovieClip (reserved, not supported)
    case NULL_DATA: // Null
    case UNDEFINED: // Undefined
        break;
    case REFERENCE: // Reference
    {
#if PARSER_ENDIAN == PARSER_LITTLEENDIAN
        uint16_t t = *(uint16_t*)(data + offset);
        uint16_t value = t >> 8;
        value |= ((t & 0xFF) << 8);
#else
        uint16_t value = *(uint16_t*)(data + offset);
#endif
        offset += sizeof(uint16_t);
        scriptTagBody = new ScriptData;
        scriptTagBody->_type = type;
        scriptTagBody->_data = (void*)(new uint16_t(value));
        if (bKey)
            ParseScriptTagBody(offset, scriptTagBody->_value, false);
    }
        break;
    case ECMA_ARRAY: // ECMA Array
    {
#if PARSER_ENDIAN == PARSER_LITTLEENDIAN
        uint32_t t = *(uint32_t*)(data + offset);
        uint32_t ECMAArrayLength = t >> 24;
        ECMAArrayLength |= ((t & 0xFF0000) >> 8);
        ECMAArrayLength |= ((t & 0xFF00) << 8);
        ECMAArrayLength |= ((t & 0xFF) << 24);
#else
        uint32_t ECMAArrayLength = *(uint32_t*)(data + offset);
#endif
        offset += sizeof(uint32_t);
        ScriptData* dataArray = new ScriptData[ECMAArrayLength];
        scriptTagBody = new ScriptData;
        scriptTagBody->_type = type;
        scriptTagBody->_extra = new int(ECMAArrayLength);
        scriptTagBody->_data = (void*)dataArray;
        for (uint32_t idx = 0; idx < ECMAArrayLength; idx++)
        {
#if PARSER_ENDIAN == PARSER_LITTLEENDIAN
            uint16_t t = *(uint16_t*)(data + offset);
            uint16_t stringLength = t >> 8;
            stringLength |= ((t & 0xFF) << 8);
#else
            uint16_t stringLength = *(uint16_t*)(data + offset);
#endif
            offset += sizeof(uint16_t);
            char* str = new char[stringLength + 1];
            memcpy(str, data + offset, stringLength * sizeof(char));
            offset += stringLength;
            str[stringLength] = '\0';
            dataArray[idx]._type = 2;
            dataArray[idx]._data = (void*)str;
            ParseScriptTagBody(offset, dataArray[idx]._value, false);
        }
        // ObjectEndMarker
        uint8_t end0 = (data + offset)[0]; // 0
        uint8_t end1 = (data + offset)[1]; // 0
        uint8_t end2 = (data + offset)[2]; // 9
        offset += 3;
        ParseScriptTagBody(offset, scriptTagBody->_value, true);
    }
        break;
    case OBJECT_END_MARKER: // Object end marker
    {
        uint8_t end0 = (data + offset)[0]; // 0
        uint8_t end1 = (data + offset)[1]; // 0
        uint8_t end2 = (data + offset)[2]; // 9
        offset += 3;
    }
        break;
    case STRICT_ARRAY: // Strict array
    {
#if PARSER_ENDIAN == PARSER_LITTLEENDIAN
        uint32_t t = *(uint32_t*)(data + offset);
        uint32_t StrictArrayLength = t >> 24;
        StrictArrayLength |= ((t & 0xFF0000) >> 8);
        StrictArrayLength |= ((t & 0xFF00) << 8);
        StrictArrayLength |= ((t & 0xFF) << 24);
#else
        uint32_t StrictArrayLength = *(uint32_t*)(data + offset);
#endif
        offset += sizeof(uint32_t);
        std::vector<ScriptData*> dataVector(StrictArrayLength);
        ScriptData* dataArray = new ScriptData[StrictArrayLength];
        scriptTagBody = new ScriptData;
        scriptTagBody->_type = type;
        scriptTagBody->_extra = new int(StrictArrayLength);
        for (uint32_t idx = 0; idx < StrictArrayLength; idx++)
        {
            ScriptData* dataValue;
            ParseScriptTagBody(offset, dataValue, false);
            dataArray[idx] = *dataValue;
            delete dataValue;
        }
        scriptTagBody->_data = (void*)dataArray;
    }
        break;
    case DATA_DATE: // Data Date
    {
#if PARSER_ENDIAN == PARSER_LITTLEENDIAN
        uint64_t t = *(uint64_t*)(data + offset);
        uint64_t value64 = t >> 56;
        value64 |= ((t & 0xFF000000000000) >> 40);
        value64 |= ((t & 0xFF0000000000) >> 24);
        value64 |= ((t & 0xFF00000000) >> 8);
        value64 |= ((t & 0xFF000000) << 8);
        value64 |= ((t & 0xFF0000) << 24);
        value64 |= ((t & 0xFF00) << 40);
        value64 |= ((t & 0xFF) << 56);
        double dataTime = *(double*)&value64;
#else
        double dataTime = *(double*)(data + offset);
#endif
        offset += sizeof(double);
#if PARSER_ENDIAN == PARSER_LITTLEENDIAN
        uint16_t td = *(uint16_t*)(data + offset);
        uint16_t value = td >> 8;
        value |= ((td & 0xFF) << 8);
        int16_t timeOffset = *(int16_t*)&value;
#else
        int16_t timeOffset = *(int16_t*)(data + offset);
#endif
        offset += sizeof(int16_t);
        ScriptDataDate* dataDate = new ScriptDataDate;
        dataDate->_dateTime = dataTime;
        dataDate->_localDateTimeOffset = timeOffset;
        scriptTagBody = new ScriptData;
        scriptTagBody->_type = type;
        scriptTagBody->_data = (void*)dataDate;
        if (bKey)
            ParseScriptTagBody(offset, scriptTagBody->_value, false);
    }
        break;
    case LONG_STRING: // Long String
    {
#if PARSER_ENDIAN == PARSER_LITTLEENDIAN
        uint32_t t = *(uint32_t*)(data + offset);
        uint32_t stringLength = t >> 24;
        stringLength |= ((t & 0xFF0000) >> 8);
        stringLength |= ((t & 0xFF00) << 8);
        stringLength |= ((t & 0xFF) << 24);
#else
        uint32_t stringLength = *(uint32_t*)(data + offset);
#endif
        offset += sizeof(uint32_t);
        char* str = new char[stringLength + 1];
        memcpy(str, data + offset, stringLength * sizeof(char));
        offset += stringLength;
        str[stringLength] = '\0';
        scriptTagBody = new ScriptData;
        scriptTagBody->_type = type;
        scriptTagBody->_data = (void*)str;
        if (bKey)
            ParseScriptTagBody(offset, scriptTagBody->_value, false);
    }
        break;
    default:
        assert(0);
        break;
    }
}

FLVParser::FLVParser(const char* inputFile,
                     ParsingFLVHeader pH,
                     ParsingVideoTag pV,
                     ParsingAudioTag pA,
                     ParsingScriptTag pS)
                     : _pH(pH), _pV(pV), _pA(pA), _pS(pS)
{
    if (_flvFile || !inputFile)
    {
        std::cerr << "[failed]: input flv key is null or the flv handler is exist" << std::endl;
        throw "[failed]";
    }
    _flvFile = fopen(inputFile, "rb");
    if (!_flvFile)
    {
        std::cerr << "[failed]: could not open the " << inputFile <<
            " maybe the file location is invalid" << std::endl;
        throw "[failed]: throw exception";
    }
}

FLVParser::~FLVParser()
{
    if (_flvFile)
    {
        fclose(_flvFile);
        _flvFile = nullptr;
    }
}

bool FLVParser::Parse()
{
    if (_flvFile)
    {
        rewind(_flvFile);
        if (!ParseFLVHeader())
        {
            std::cout << "[failed]: parse flv header failed" << std::endl;
            return false;
        }
        while (!feof(_flvFile))
        {
            if (!ParseFLVTag())
            {
                std::cout << "[failed]: parse flv tag failed" << std::endl;
                return false;
            }
        }
        return true;
    }
    std::cerr << "[failed]: the flv handler is null" << std::endl;
    return false;
}

bool FLVParser::ParseFLVHeader()
{
    FLVHeader header;
    if (fread((void*)&header, sizeof(FLVHeader), 1, _flvFile) <= 0)
    {
        std::cerr << "[failed]: read the flv header failed" << std::endl;
        return false;
    }
    // check length
    if (header._version == 0x01 &&
        header._dataOffset[0] != 0 &&
        header._dataOffset[1] != 0 &&
        header._dataOffset[2] != 0 &&
        header._dataOffset[3] != 9)
    {
        std::cerr << "[failed]: check of the flv header length failed" << std::endl;
        return false;
    }
    // check header meta
    if (header._signature[0] != 'F' ||
        header._signature[1] != 'L' ||
        header._signature[2] != 'V')
    {
        std::cerr << "[failed]: flv header signature is not right" << std::endl;
        return false;
    }
    // get video/audio flag
    _bHasVideo = !!header._typeFlagsVideo;
    _bHasAudio = !!header._typeFlagsAudio;

    // skip first PreviousTagSize0
    uint32_t previousTagSize0 = 0;
    if (fread((void*)&previousTagSize0, sizeof(uint32_t), 1, _flvFile) <= 0)
    {
        std::cerr << "[failed]: the previousTagSize0 reads failed" << std::endl;
        return false;
    }
    if (previousTagSize0 != 0)
    {
        std::cerr << "[failed]: the previousTagSize0 != 0" << std::endl;
        return false;
    }
    _pH(&header, previousTagSize0);
    return true;
}

bool FLVParser::ParseFLVTag()
{
    FLVTag::FLVTagHeader header;
    int size = fread((void*)&header, sizeof(FLVTag::FLVTagHeader), 1, _flvFile);
    if (size < 0)
    {
        std::cerr << "[failed]: reading the flvtagheader failed" << std::endl;
        return false;
    }
    else if (size == 0)
        return true;
    if (header._tagType == 8)
    {
        return ParseAudioTag(&header);
    }
    else if (header._tagType == 9)
    {
        return ParseVideoTag(&header);
    }
    else if (header._tagType == 18)
    {
        return ParseScriptTag(&header);
    }
    else
    {
        std::cerr << "[failed]: unknown flv tag type" << std::endl;
        assert(0);
    }
    return false;
}

bool FLVParser::ParseAudioTag(const FLVTag::FLVTagHeader* header)
{
    int dataSize = 0;
#if PARSER_ENDIAN == PARSER_LITTLEENDIAN
    dataSize |= (header->_dataSize[2]);
    dataSize |= (header->_dataSize[1] << 8);
    dataSize |= (header->_dataSize[0] << 16);
#else
    dataSize |= (header->_dataSize[0]);
    dataSize |= (header->_dataSize[1] << 8);
    dataSize |= (header->_dataSize[2] << 16);
#endif

    AudioTag::AudioTagHeader audioHeader;
    if (fread((void*)&audioHeader, sizeof(audioHeader), 1, _flvFile) <= 0)
    {
        std::cerr << "[failed]: read audio header failed" << std::endl;
        return false;
    }
    dataSize -= sizeof(audioHeader);
    uint8_t AACPacketType = 0;
    if (audioHeader._soundFormat == AAC)
    {
        if (fread((void*)&AACPacketType, sizeof(AACPacketType), 1, _flvFile) <= 0)
        {
            std::cerr << "[failed]: read AACPacketType failed" << std::endl;
            return false;
        }
        dataSize -= sizeof(AACPacketType);
    }
    std::shared_ptr<uint8_t> data(new uint8_t[dataSize], std::default_delete<uint8_t[]>());
    if (dataSize > 0)
    {
        if (fread((void*)data.get(), dataSize, 1, _flvFile) <= 0)
        {
            std::cerr << "[failed]: read flv audio data failed" << std::endl;
            return false;
        }
    }
    AudioTag audioTag;
    audioTag._header = audioHeader;
    audioTag._data = data.get();
    FLVTag tag{ *header, &audioTag };
    uint32_t iPreviousTagSize = 0;
    if (fread((void*)&iPreviousTagSize, sizeof(uint32_t), 1, _flvFile) <= 0)
    {
        std::cerr << "[failed]: read the iPreviousTagSize failed" << std::endl;
        return false;
    }
#if PARSER_ENDIAN == PARSER_LITTLEENDIAN
    uint32_t t = iPreviousTagSize;
    uint32_t iPreviousTagSizeL = t >> 24;
    iPreviousTagSizeL |= ((t & 0xFF0000) >> 8);
    iPreviousTagSizeL |= ((t & 0xFF00) << 8);
    iPreviousTagSizeL |= ((t & 0xFF) << 24);
    iPreviousTagSize = iPreviousTagSizeL;
#endif
    _pA(&tag, dataSize, iPreviousTagSize, AACPacketType);
    return true;
}

bool FLVParser::ParseVideoTag(const FLVTag::FLVTagHeader* header)
{
    int dataSize = 0;
#if PARSER_ENDIAN == PARSER_LITTLEENDIAN
    dataSize |= (header->_dataSize[2]);
    dataSize |= (header->_dataSize[1] << 8);
    dataSize |= (header->_dataSize[0] << 16);
#else
    dataSize |= (header->_dataSize[0]);
    dataSize |= (header->_dataSize[1] << 8);
    dataSize |= (header->_dataSize[2] << 16);
#endif

    VideoTag::VideoTagHeader videoHeader;
    if (fread((void*)&videoHeader, sizeof(videoHeader), 1, _flvFile) <= 0)
    {
        std::cerr << "[failed]: read video header failed" << std::endl;
        return false;
    }
    dataSize -= sizeof(videoHeader);
    AVCPacket::AVCPacketHeader AVCPacketHeader;
    uint8_t vp6Byte = 0;
    if (videoHeader._codecID == AVC)
    {
        if (fread((void*)&AVCPacketHeader, sizeof(AVCPacketHeader), 1, _flvFile) <= 0)
        {
            std::cerr << "[failed]: read AVCPacketHeader failed" << std::endl;
            return false;
        }
        dataSize -= sizeof(AVCPacketHeader);
    }
    else if (videoHeader._codecID == VP6 || videoHeader._codecID == VP6WithAlpha)
    {
        if (fread((void*)&vp6Byte, sizeof(uint8_t), 1, _flvFile) <= 0)
        {
            std::cerr << "[failed]: read VP6 byte failed" << std::endl;
            return false;
        }
        dataSize -= sizeof(uint8_t);
    }
    std::shared_ptr<uint8_t> data(new uint8_t[dataSize], std::default_delete<uint8_t[]>());
    if (dataSize > 0)
    {
        if (fread((void*)data.get(), dataSize, 1, _flvFile) <= 0)
        {
            std::cerr << "[failed]: read flv video data failed" << std::endl;
            return false;
        }
    }
    VideoTag videoTag;
    videoTag._header = videoHeader;
    videoTag._data = data.get();
    FLVTag tag{ *header, &videoTag };
    uint32_t iPreviousTagSize = 0;
    if (fread((void*)&iPreviousTagSize, sizeof(uint32_t), 1, _flvFile) <= 0)
    {
        std::cerr << "[failed]: read the iPreviousTagSize failed" << std::endl;
        return false;
    }
#if PARSER_ENDIAN == PARSER_LITTLEENDIAN
    uint32_t t = iPreviousTagSize;
    uint32_t iPreviousTagSizeL = t >> 24;
    iPreviousTagSizeL |= ((t & 0xFF0000) >> 8);
    iPreviousTagSizeL |= ((t & 0xFF00) << 8);
    iPreviousTagSizeL |= ((t & 0xFF) << 24);
    iPreviousTagSize = iPreviousTagSizeL;
#endif
    _pV(&tag, dataSize, iPreviousTagSize, &AVCPacketHeader, vp6Byte);
    return true;
}

bool FLVParser::ParseScriptTag(const FLVTag::FLVTagHeader* header)
{
    int dataSize = 0;
#if PARSER_ENDIAN == PARSER_LITTLEENDIAN
    dataSize |= (header->_dataSize[2]);
    dataSize |= (header->_dataSize[1] << 8);
    dataSize |= (header->_dataSize[0] << 16);
#else
    dataSize |= (header->_dataSize[0]);
    dataSize |= (header->_dataSize[1] << 8);
    dataSize |= (header->_dataSize[2] << 16);
#endif
    // skip data
    std::shared_ptr<uint8_t> data(new uint8_t[dataSize], std::default_delete<uint8_t[]>());
    if (fread((void*)data.get(), dataSize, 1, _flvFile) <= 0)
    {
        std::cerr << "[failed]: read the flv meta data failed" << std::endl;
        return false;
    }
    FLVTag tag{ *header, data.get() };
    uint32_t iPreviousTagSize = 0;
    if (fread((void*)&iPreviousTagSize, sizeof(uint32_t), 1, _flvFile) <= 0)
    {
        std::cerr << "[failed]: read the iPreviousTagSize failed" << std::endl;
        return false;
    }
#if PARSER_ENDIAN == PARSER_LITTLEENDIAN
    uint32_t t = iPreviousTagSize;
    uint32_t iPreviousTagSizeL = t >> 24;
    iPreviousTagSizeL |= ((t & 0xFF0000) >> 8);
    iPreviousTagSizeL |= ((t & 0xFF00) << 8);
    iPreviousTagSizeL |= ((t & 0xFF) << 24);
    iPreviousTagSize = iPreviousTagSizeL;
#endif
    _pS(&tag, dataSize, iPreviousTagSize);
    return true;
}

FLVPARSER_NAMESPACE_END
