cmake_minimum_required (VERSION 3.12)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Enable MSVC runtime library flags selection
cmake_policy(SET CMP0091 NEW)

project ("ManeleMax")

add_executable (ManeleMax "src/main.cpp" "res/resource.rc" "src/systray_icon.cpp" "src/volume_control.cpp" "src/system_media_properties_notifier.cpp" "src/auto_dj.cpp" "src/string_utils.cpp" "res/version.rc")

# Statically link MSVC runtime library
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

target_include_directories(ManeleMax PRIVATE "${PROJECT_SOURCE_DIR}/res")

target_compile_features(ManeleMax PRIVATE cxx_std_23)

target_compile_definitions(ManeleMax PRIVATE WIN32_LEAN_AND_MEAN)

set_target_properties(ManeleMax PROPERTIES WIN32_EXECUTABLE TRUE)
