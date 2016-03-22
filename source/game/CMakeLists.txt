
# 
# External dependencies
# 

# find_package(THIRDPARTY REQUIRED)

# 
# Executable name and options
# 

# Target name
set(target game)

# Exit here if required dependencies are not met
message(STATUS "Rune ${target}")


# 
# Sources
# 

set(sources
    imgui/imconfig.h
    imgui/imgui.cpp
    imgui/imgui.h
    imgui/imgui_draw.cpp
    imgui/imgui_internal.h
    imgui/stb_rect_pack.h
    imgui/stb_textedit.h
    imgui/stb_truetype.h
    imgui/sfml-events.h
    imgui/sfml-rendering.h

	ctti/detail/array.hpp
	ctti/detail/hash.hpp
	ctti/detail/pretty_function.hpp
	ctti/detail/string.hpp
	ctti/type_id.hpp

	engine/Time.hpp
	engine/Game.hpp
	engine/Game.cpp

    main.cpp
)


# 
# Create executable
# 

# Build executable
add_executable(${target}
    MACOSX_BUNDLE WIN32
    ${sources}
)

# Create namespaced alias
add_executable(${META_PROJECT_NAME}::${target} ALIAS ${target})


# 
# Project options
# 

set_target_properties(${target}
    PROPERTIES
    ${DEFAULT_PROJECT_OPTIONS}
    FOLDER "${IDE_FOLDER}"
)


# 
# Include directories
# 

target_include_directories(${target}
    PRIVATE
    ${DEFAULT_INCLUDE_DIRECTORIES}
    ${PROJECT_BINARY_DIR}/source/include
)


# 
# Libraries
#

target_link_libraries(${target}
    PRIVATE
    ${DEFAULT_LIBRARIES}
)


# 
# Compile definitions
# 

target_compile_definitions(${target}
    PRIVATE
    ${DEFAULT_COMPILE_DEFINITIONS}
)


# 
# Compile options
#

target_compile_options(${target}
    PRIVATE
    ${DEFAULT_COMPILE_OPTIONS}
)


# 
# Linker options
#

target_link_libraries(${target}
    PRIVATE
    ${DEFAULT_LINKER_OPTIONS}
)


# 
# Deployment
# 

# Executable
install(TARGETS ${target}
    RUNTIME DESTINATION ${INSTALL_BIN} COMPONENT runtime
    BUNDLE  DESTINATION ${INSTALL_BIN} COMPONENT runtime
)
