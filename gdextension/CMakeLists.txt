cmake_minimum_required(VERSION 3.17)

set(LIBNAME "godot_omt")
set(GODOT_PROJECT_DIR ".." CACHE PATH "Godot project root (contains godot_omt.gdextension)")

find_package(Python3 3.4 REQUIRED)

if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/godot-cpp/src")
	message(NOTICE "godot-cpp not found — run: git submodule update --init --recursive")
endif()

add_subdirectory(godot-cpp SYSTEM)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${godot-cpp_SOURCE_DIR}/cmake")
include(GodotCPPModule)

get_target_property(GODOTCPP_SUFFIX godot::cpp GODOTCPP_SUFFIX)
get_target_property(GODOTCPP_PLATFORM godot::cpp GODOTCPP_PLATFORM)

project(godot_omt LANGUAGES CXX)

set(GODOT_BIN_PLATFORM_DIR "${GODOTCPP_PLATFORM}")
if(GODOTCPP_PLATFORM STREQUAL "linux")
	set(GODOT_BIN_PLATFORM_DIR "linux")
elseif(GODOTCPP_PLATFORM STREQUAL "macos")
	set(GODOT_BIN_PLATFORM_DIR "macos")
elseif(GODOTCPP_PLATFORM STREQUAL "windows")
	set(GODOT_BIN_PLATFORM_DIR "windows")
endif()

get_filename_component(GODOT_PROJECT_ABS_DIR "${CMAKE_SOURCE_DIR}/${GODOT_PROJECT_DIR}" ABSOLUTE)
set(GODOT_BIN_DIR "${GODOT_PROJECT_ABS_DIR}/bin/${GODOT_BIN_PLATFORM_DIR}")

add_library(${LIBNAME} SHARED
	src/omt.cpp
	src/omt.h
	src/omt_discovery.cpp
	src/omt_discovery.h
	src/omt_output.cpp
	src/omt_output.h
	src/register_types.cpp
	src/register_types.h
	src/omt_receiver.cpp
	src/omt_receiver.h
	src/omt_frame_converter.cpp
	src/omt_frame_converter.h
	src/omt_video_stream.cpp
	src/omt_video_stream.h
	src/omt_video_stream_playback.cpp
	src/omt_video_stream_playback.h
	src/omt_viewport_texture_router.cpp
	src/omt_viewport_texture_router.h
)

file(GLOB_RECURSE DOC_XML LIST_DIRECTORIES NO CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/../doc_classes/*.xml")
if(DOC_XML AND GODOTCPP_TARGET MATCHES "editor|template_debug")
	target_doc_sources(${LIBNAME} ${DOC_XML})
endif()

target_link_libraries(${LIBNAME} PRIVATE godot-cpp)
set_target_properties(${LIBNAME} PROPERTIES CXX_STANDARD 17)

# --- libomt (optional) ---
set(LIBOMT_ROOT "${CMAKE_CURRENT_LIST_DIR}/../third_party/libomt" CACHE PATH "libomt install prefix (include/ + lib/)")
option(GODOT_OMT_REQUIRE_LIBOMT "Fail configure if libomt is not found" ON)
set(LIBOMT_HEADER "${LIBOMT_ROOT}/include/libomt.h")

if(NOT EXISTS "${LIBOMT_HEADER}")
	message(STATUS "Downloading libomt.h into third_party...")
	file(DOWNLOAD
		"https://raw.githubusercontent.com/openmediatransport/libomt/master/libomt.h"
		"${LIBOMT_HEADER}"
		SHOW_PROGRESS
		TLS_VERIFY ON
	)
endif()

unset(LIBOMT_INCLUDE_DIR CACHE)
unset(LIBOMT_LINK_LIBRARY CACHE)
unset(LIBVMX_LIBRARY CACHE)

set(LIBOMT_INCLUDE_DIR "")
set(LIBOMT_LINK_LIBRARY "")
set(LIBOMT_RUNTIME_LIBRARY "")
set(LIBVMX_LIBRARY "")
set(LIBOMTNET_LIBRARY "")

if(EXISTS "${LIBOMT_ROOT}/include/libomt.h")
	set(LIBOMT_INCLUDE_DIR "${LIBOMT_ROOT}/include")
elseif(EXISTS "${LIBOMT_ROOT}/libomt.h")
	set(LIBOMT_INCLUDE_DIR "${LIBOMT_ROOT}")
endif()

set(LIBOMT_SEARCH_DIRS
	"${GODOT_BIN_DIR}"
	"${LIBOMT_ROOT}/lib"
	"${LIBOMT_ROOT}/lib64"
	"${LIBOMT_ROOT}"
)

if(GODOTCPP_PLATFORM STREQUAL "windows")
	foreach(search_dir IN LISTS LIBOMT_SEARCH_DIRS)
		if(NOT LIBOMT_LINK_LIBRARY AND EXISTS "${search_dir}/libomt.lib")
			set(LIBOMT_LINK_LIBRARY "${search_dir}/libomt.lib")
		endif()
		if(NOT LIBOMT_RUNTIME_LIBRARY AND EXISTS "${search_dir}/libomt.dll")
			set(LIBOMT_RUNTIME_LIBRARY "${search_dir}/libomt.dll")
		endif()
		if(NOT LIBVMX_LIBRARY AND EXISTS "${search_dir}/libvmx.dll")
			set(LIBVMX_LIBRARY "${search_dir}/libvmx.dll")
		endif()
		if(NOT LIBOMTNET_LIBRARY AND EXISTS "${search_dir}/libomtnet.dll")
			set(LIBOMTNET_LIBRARY "${search_dir}/libomtnet.dll")
		endif()
	endforeach()
elseif(GODOTCPP_PLATFORM STREQUAL "macos")
	foreach(search_dir IN LISTS LIBOMT_SEARCH_DIRS)
		if(NOT LIBOMT_LINK_LIBRARY AND EXISTS "${search_dir}/libomt.dylib")
			set(LIBOMT_LINK_LIBRARY "${search_dir}/libomt.dylib")
			set(LIBOMT_RUNTIME_LIBRARY "${search_dir}/libomt.dylib")
		endif()
		if(NOT LIBVMX_LIBRARY AND EXISTS "${search_dir}/libvmx.dylib")
			set(LIBVMX_LIBRARY "${search_dir}/libvmx.dylib")
		endif()
		if(NOT LIBOMTNET_LIBRARY AND EXISTS "${search_dir}/libomtnet.dll")
			set(LIBOMTNET_LIBRARY "${search_dir}/libomtnet.dll")
		endif()
	endforeach()
else()
	foreach(search_dir IN LISTS LIBOMT_SEARCH_DIRS)
		if(NOT LIBOMT_LINK_LIBRARY AND EXISTS "${search_dir}/libomt.so")
			set(LIBOMT_LINK_LIBRARY "${search_dir}/libomt.so")
			set(LIBOMT_RUNTIME_LIBRARY "${search_dir}/libomt.so")
		endif()
		if(NOT LIBVMX_LIBRARY AND EXISTS "${search_dir}/libvmx.so")
			set(LIBVMX_LIBRARY "${search_dir}/libvmx.so")
		endif()
	endforeach()
endif()

if(LIBOMT_INCLUDE_DIR AND LIBOMT_LINK_LIBRARY)
	message(STATUS "libomt link library: ${LIBOMT_LINK_LIBRARY}")
	target_include_directories(${LIBNAME} PRIVATE "${LIBOMT_INCLUDE_DIR}")
	target_link_libraries(${LIBNAME} PRIVATE "${LIBOMT_LINK_LIBRARY}")
	if(LIBVMX_LIBRARY AND NOT GODOTCPP_PLATFORM STREQUAL "windows")
		message(STATUS "libvmx: ${LIBVMX_LIBRARY}")
		target_link_libraries(${LIBNAME} PRIVATE "${LIBVMX_LIBRARY}")
	endif()
	if(UNIX AND NOT APPLE)
		target_link_libraries(${LIBNAME} PRIVATE dl pthread)
		set_target_properties(${LIBNAME} PROPERTIES BUILD_RPATH "$ORIGIN")
	endif()
else()
	if(GODOT_OMT_REQUIRE_LIBOMT)
		message(FATAL_ERROR "libomt not found for ${GODOTCPP_PLATFORM}. Expected libomt runtime/link libraries under ${GODOT_BIN_DIR} or ${LIBOMT_ROOT}.")
	endif()
	message(WARNING "libomt not found under ${LIBOMT_ROOT} — building with test-pattern stub only because GODOT_OMT_REQUIRE_LIBOMT=OFF.")
	target_compile_definitions(${LIBNAME} PRIVATE GODOT_OMT_NO_LIBOMT=1)
endif()

file(MAKE_DIRECTORY "${GODOT_BIN_DIR}")

set_target_properties(${LIBNAME}
	PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "$<1:${GODOT_BIN_DIR}>"
	LIBRARY_OUTPUT_DIRECTORY "$<1:${GODOT_BIN_DIR}>"
	PREFIX ""
	OUTPUT_NAME "${LIBNAME}${GODOTCPP_SUFFIX}"
)

add_custom_command(TARGET ${LIBNAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:${LIBNAME}>" "${GODOT_BIN_DIR}/$<TARGET_FILE_NAME:${LIBNAME}>"
	COMMENT "Copying ${LIBNAME} to ${GODOT_BIN_DIR}"
)

set(OMT_RUNTIME_LIBRARIES "")
if(LIBOMT_RUNTIME_LIBRARY)
	list(APPEND OMT_RUNTIME_LIBRARIES "${LIBOMT_RUNTIME_LIBRARY}")
endif()
if(LIBVMX_LIBRARY)
	list(APPEND OMT_RUNTIME_LIBRARIES "${LIBVMX_LIBRARY}")
endif()
if(LIBOMTNET_LIBRARY)
	list(APPEND OMT_RUNTIME_LIBRARIES "${LIBOMTNET_LIBRARY}")
endif()

if(OMT_RUNTIME_LIBRARIES)
	add_custom_command(TARGET ${LIBNAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${OMT_RUNTIME_LIBRARIES} "${GODOT_BIN_DIR}"
		COMMENT "Copying OMT runtime libraries to ${GODOT_BIN_DIR}"
	)
endif()
