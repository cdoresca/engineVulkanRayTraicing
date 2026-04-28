cmake_minimum_required(VERSION 4.0)

project(Engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(LIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs)

# ============================================================================
# Libraries
# ============================================================================

# GLM
add_subdirectory(${LIBS_DIR}/glm)

# Embree — auto-download prebuilt release
include(FetchContent)

set(EMBREE_VERSION "4.4.0")
set(EMBREE_URL "https://github.com/RenderKit/embree/releases/download/v${EMBREE_VERSION}/embree-${EMBREE_VERSION}.x64.windows.zip")
set(FETCHCONTENT_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/embree-download)

FetchContent_Declare(
    embree_download
    URL ${EMBREE_URL}
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

message(STATUS "Fetching Embree ${EMBREE_VERSION}...")
FetchContent_MakeAvailable(embree_download)
message(STATUS "Embree fetched at: ${embree_download_SOURCE_DIR}")

find_package(embree 4 REQUIRED
    PATHS "${embree_download_SOURCE_DIR}"
    NO_DEFAULT_PATH
)

# tinyobjloader
add_library(tiny INTERFACE)
target_include_directories(tiny INTERFACE ${LIBS_DIR}/tinyobjloader)
set(TINY_IMPL_CPP ${LIBS_DIR}/tinyobjloader/tiny_obj_loader.cpp)
set_target_properties(tiny PROPERTIES FOLDER Libs) 

# stb
file(GLOB STB_FILES ${LIBS_DIR}/stb/*)
add_library(stb INTERFACE ${STB_FILES})
target_include_directories(stb INTERFACE ${LIBS_DIR}/stb)
set_target_properties(stb PROPERTIES FOLDER Libs) 

# nlohmann
file(GLOB NLOHMANN_FILES ${LIBS_DIR}/nlohmann/*)
add_library(nlohmann INTERFACE ${NLOHMANN_FILES})
target_include_directories(nlohmann INTERFACE ${LIBS_DIR}/nlohmann)
set_target_properties(nlohmann PROPERTIES FOLDER Libs) 

# GLFW
set(GLFW_INCLUDE_DIR ${LIBS_DIR}/glfw-3.4/include/GLFW)
set(GLFW_LIBRARY_DIR ${LIBS_DIR}/glfw-3.4/lib-vc2022)
add_library(glfw STATIC IMPORTED)
set_target_properties(glfw PROPERTIES
    IMPORTED_LOCATION ${GLFW_LIBRARY_DIR}/glfw3.lib
    INTERFACE_INCLUDE_DIRECTORIES ${GLFW_INCLUDE_DIR}
)
set_target_properties(glfw PROPERTIES FOLDER Libs/GLFW3)

# Vulkan
find_package(Vulkan REQUIRED)

# ImGui
file(GLOB IMGUI_FILES ${LIBS_DIR}/imgui/*.cpp)
add_library(imgui STATIC ${IMGUI_FILES})
target_include_directories(imgui PUBLIC ${LIBS_DIR}/imgui)
target_include_directories(imgui PRIVATE 
    ${GLFW_INCLUDE_DIR}
    ${Vulkan_INCLUDE_DIRS}
)
target_link_libraries(imgui glfw ${Vulkan_LIBRARIES})
target_compile_definitions(imgui PUBLIC IMGUI_DEFINE_MATH_OPERATORS)
set_target_properties(imgui PROPERTIES FOLDER Libs) 

# ImGuizmo
add_library(imguizmo STATIC 
    ${LIBS_DIR}/imguizmo/ImGuizmo.cpp
)

target_include_directories(imguizmo PUBLIC 
    ${LIBS_DIR}/imguizmo
    ${LIBS_DIR}/imgui
)

target_link_libraries(imguizmo imgui)

set_target_properties(imguizmo PROPERTIES FOLDER Libs)

# ============================================================================
# Source Files
# ============================================================================

file(GLOB_RECURSE SOURCE_FILES
    ${SOURCE_DIR}/*.cpp 
    ${SOURCE_DIR}/*.hpp 
    ${SOURCE_DIR}/*.h
    ${SOURCE_DIR}/*.vert 
    ${SOURCE_DIR}/*.frag 
    ${SOURCE_DIR}/*.glsl 
    ${SOURCE_DIR}/*.rmiss 
    ${SOURCE_DIR}/*.rchit 
    ${SOURCE_DIR}/*.rgen 
    ${SOURCE_DIR}/*.comp 
)

foreach(SRC IN ITEMS ${SOURCE_FILES})
    get_filename_component(SRC_PATH "${SRC}" PATH)
    file(RELATIVE_PATH SRC_PATH_REL "${SOURCE_DIR}" "${SRC_PATH}")
    string(REPLACE "/" "\\" GROUP_PATH "/${SRC_PATH_REL}")
    source_group("Sources${GROUP_PATH}" FILES "${SRC}")
endforeach()
message("Getting all sources... DONE")

# ============================================================================
# Compilation shaders
# ============================================================================

set(SHADER_SCRIPT ${SOURCE_DIR}/shader/compile_shaders.bat)
set(SHADER_SRC_DIR ${SOURCE_DIR}/shader)
set(SHADER_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/shader)
set(Vulkan_GLSLC_EXECUTABLE $ENV{VULKAN_SDK}/Bin/glslc.exe)

file(GLOB SHADER_FILES 
    ${SHADER_SRC_DIR}/*.rgen 
    ${SHADER_SRC_DIR}/*.rchit 
    ${SHADER_SRC_DIR}/*.rmiss
    ${SHADER_SRC_DIR}/*.comp
)

file(MAKE_DIRECTORY ${SHADER_OUT_DIR})

set(SPV "")
foreach(SHADER ${SHADER_FILES})
    get_filename_component(NAME ${SHADER} NAME)
    list(APPEND SPV ${SHADER_OUT_DIR}/${NAME}.spv)

    get_filename_component(EXT ${SHADER} EXT)
    if(EXT STREQUAL ".rgen")
        set(STAGE "rgen")
    elseif(EXT STREQUAL ".rchit")
        set(STAGE "rchit")
    elseif(EXT STREQUAL ".rmiss")
        set(STAGE "rmiss")
    elseif(EXT STREQUAL ".comp")
        set(STAGE "comp")
    endif()

    add_custom_command(
        OUTPUT ${SHADER_OUT_DIR}/${NAME}.spv
        COMMAND "${Vulkan_GLSLC_EXECUTABLE}" --target-env=vulkan1.4 -fshader-stage=${STAGE} -I ${SHADER_SRC_DIR} ${SHADER} -o ${SHADER_OUT_DIR}/${NAME}.spv
        DEPENDS ${SHADER}
        COMMENT "Compiling shader ${NAME} as ${STAGE}"
    )
    list(APPEND SPV ${SHADER_OUT_DIR}/${NAME}.spv)
endforeach()
        
add_custom_target(shader DEPENDS ${SPV})
        
# ============================================================================
# Executable
# ============================================================================

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_sources(${PROJECT_NAME} PRIVATE ${TINY_IMPL_CPP})
add_dependencies(${PROJECT_NAME} shader)

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${SOURCE_DIR} 
    ${GLFW_INCLUDE_DIR}
    ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} imgui imguizmo tiny stb nlohmann glm glfw ${Vulkan_LIBRARIES} embree)

# Copy Embree DLLs from the downloaded location
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${embree_download_SOURCE_DIR}/bin"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/embree
    COMMAND ${CMAKE_COMMAND} -E copy
        "${embree_download_SOURCE_DIR}/bin/embree4.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/embree4.dll
    COMMAND ${CMAKE_COMMAND} -E copy
        "${embree_download_SOURCE_DIR}/bin/tbb12.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/tbb12.dll
    COMMAND ${CMAKE_COMMAND} -E copy
        "${embree_download_SOURCE_DIR}/bin/tbbmalloc.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/tbbmalloc.dll
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    FRONT_DIR="${LIBS_DIR}/imgui/font/"
    OBJ_DIR="${CMAKE_CURRENT_SOURCE_DIR}/obj/"
    SHADER_DIR="${SHADER_OUT_DIR}/"
)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})