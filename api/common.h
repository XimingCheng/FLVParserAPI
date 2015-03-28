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

#ifndef COMMON_H_
#define COMMON_H_

#include <cassert>
#include <stdint.h>
#include <iostream>

#define PARSER_LITTLEENDIAN 0
#define PARSER_BIGENDIAN    1

#ifndef PARSER_ENDIAN
    // Detect with GCC 4.6's macro, MSVC also understand the pre-defined macro
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        #define PARSER_ENDIAN PARSER_LITTLEENDIAN
    #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        #define PARSER_ENDIAN PARSER_BIGENDIAN
    #else
        #error Unknown machine endianess detected. User needs to define PARSER_ENDIAN.
    #endif // __BYTE_ORDER__
#endif // PARSER_ENDIAN

#define FLVPARSER_NAMESPACE_BEGIN namespace flvparser {
#define FLVPARSER_NAMESPACE_END   }

#endif // COMMON_H_