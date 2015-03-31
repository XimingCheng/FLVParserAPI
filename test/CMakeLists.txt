include_directories(
	../api
)

add_executable(main
	main.cpp
)

target_link_libraries(main FLVParserAPI)