include_directories(
	.
)

SET(DIR_LIB_SRCS
    flvparser.cpp
)

add_library(FLVParserAPI ${DIR_LIB_SRCS})
