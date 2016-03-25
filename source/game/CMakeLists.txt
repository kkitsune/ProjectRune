
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
    engine/imgui/imconfig.h
	engine/imgui/imgui.cpp
	engine/imgui/imgui.h
	engine/imgui/imgui_draw.cpp
	engine/imgui/imgui_internal.h
	engine/imgui/stb_rect_pack.h
	engine/imgui/stb_textedit.h
	engine/imgui/stb_truetype.h
	engine/imgui/sfml-events.h
	engine/imgui/sfml-rendering.h

	engine/ctti/detail/array.hpp
	engine/ctti/detail/hash.hpp
	engine/ctti/detail/pretty_function.hpp
	engine/ctti/detail/string.hpp
	engine/ctti/type_id.hpp

	engine/selene/detail/BaseFun.h
	engine/selene/detail/Class.h
	engine/selene/detail/ClassFun.h
	engine/selene/detail/Ctor.h
	engine/selene/detail/Dtor.h
	engine/selene/detail/ExceptionHandler.h
	engine/selene/detail/ExceptionTypes.h
	engine/selene/detail/Fun.h
	engine/selene/detail/function.h
	engine/selene/detail/LuaName.h
	engine/selene/detail/LuaRef.h
	engine/selene/detail/MetatableRegistry.h
	engine/selene/detail/Obj.h
	engine/selene/detail/ObjFun.h
	engine/selene/detail/primitives.h
	engine/selene/detail/references.h
	engine/selene/detail/Registry.h
	engine/selene/detail/ResourceHandler.h
	engine/selene/detail/Selector.h
	engine/selene/detail/State.h
	engine/selene/detail/traits.h
	engine/selene/detail/Tuple.h
	engine/selene/detail/util.h
	engine/selene/selene.h

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
