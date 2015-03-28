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

#include "../flvparser.h"

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
    //switch (audio->_header._soundFormat)
    //{
    //case LinearPCM:
    //    break;
    //default:
    //    break;
    //}
}

void PrintVideoTag(FLVTag* tag, int size, uint32_t preSize, AVCPacket::AVCPacketHeader* AVCHeader, uint8_t vp8Byte)
{
    std::cout << "Video Tag detected!" << std::endl;
    VideoTag* video = static_cast<VideoTag*>(tag->_data);
    // the code below user can define its own code
    //switch (video->_header._codecID)
    //{
    //default:
    //    break;
    //}
    //switch (video->_header._frameType)
    //{
    //default:
    //    break;
    //}
}

void PrintScriptTag(FLVTag* tag, int size, uint32_t preSize)
{
    ScriptKVDataParser KVParser(tag, size);
    KVParser.Parse();
}

int main(int argc, const char* argv[])
{
    int retCode = 0;
    if (argc != 2)
    {
        std::cerr << "[Usage]: flvparser inputfile" << std::endl;
        return 1;
    }
    try
    {
        FLVParser parser(argv[1],
                         &PrintFLVHeader,
                         &PrintVideoTag,
                         &PrintAudioTag,
                         &PrintScriptTag);
        if (parser.Parse())
        {
            std::cout << "[ok]: parse flv file successfully" << std::endl;
        }
        else
        {
            std::cerr << "[failed]: flv parsing failed" << std::endl;
            retCode = 1;
        }
    }
    catch (char const*)
    {
        std::cerr << "FLVParser init failed!" << std::endl;
        retCode = 1;
    }
    return retCode;
}