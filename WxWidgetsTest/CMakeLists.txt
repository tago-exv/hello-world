set(TARGET_NAME CMakeWxHelloWorld)

# set source/header files
set(${TARGET_NAME}_Source CMakeWxHelloWorld.cpp)
set(${TARGET_NAME}_Header CMakeWxHelloWorld.h)

# common path
set(PREPROCESS_BINARY_DIR   "${CMAKE_BINARY_DIR}/Samples/${TARGET_NAME}/$(Configuration)")

if(wxWidgets_FOUND)
		
	# include path
	include_directories(${TARGET_NAME})
	
	# add executable
	add_executable(${TARGET_NAME} WIN32 ${${TARGET_NAME}_Source} ${${TARGET_NAME}_Header})
	
	set_target_properties(${TARGET_NAME}  PROPERTIES FOLDER Samples)

	# link library path
	target_link_libraries(${TARGET_NAME} ${wxWidgets_LIBRARIES})
	
	configure_current_directory(${TARGET_NAME})
	
endif() #wxWidgets_FOUND