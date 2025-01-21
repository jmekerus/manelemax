cmake_minimum_required (VERSION 3.12)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("ManeleMax")

add_executable (ManeleMax "src/main.cpp" "src/window.cpp")

target_compile_features(ManeleMax PRIVATE cxx_std_23)

target_compile_definitions(ManeleMax PRIVATE WIN32_LEAN_AND_MEAN)

set_target_properties(ManeleMax PROPERTIES WIN32_EXECUTABLE TRUE)
