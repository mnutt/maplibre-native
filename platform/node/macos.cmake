cmake_minimum_required(VERSION 3.19)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15")

# Override default CMake NATIVE_ARCH_ACTUAL
# https://gitlab.kitware.com/cmake/cmake/-/issues/20893
# https://stackoverflow.com/a/22689917/5531400
if(CMAKE_GENERATOR STREQUAL Xcode)
  set(CMAKE_OSX_ARCHITECTURES "$(ARCHS_STANDARD)")
endif()
set_target_properties(mbgl-core PROPERTIES XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=Debug] "YES")

set_target_properties(mbgl-core PROPERTIES XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES)

if(MLN_WITH_OPENGL)
  find_package(OpenGL REQUIRED)

  target_compile_definitions(
    mbgl-core
    PUBLIC GL_SILENCE_DEPRECATION
  )
  target_sources(
    mbgl-core
    PRIVATE
      ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/gl/headless_backend.cpp
      ${PROJECT_SOURCE_DIR}/platform/darwin/src/gl_functions.cpp ${PROJECT_SOURCE_DIR}/platform/darwin/src/headless_backend_cgl.mm
  )
  target_link_libraries(
    mbgl-core
    PRIVATE OpenGL::GL
  )
endif()

target_sources(
  mbgl-core
  PRIVATE
    ${PROJECT_SOURCE_DIR}/platform/darwin/src/async_task.cpp
    ${PROJECT_SOURCE_DIR}/platform/darwin/src/collator.mm
    ${PROJECT_SOURCE_DIR}/platform/darwin/src/http_file_source.mm
    ${PROJECT_SOURCE_DIR}/platform/darwin/src/image.mm
    ${PROJECT_SOURCE_DIR}/platform/darwin/src/local_glyph_rasterizer.mm
    ${PROJECT_SOURCE_DIR}/platform/darwin/src/logging_nslog.mm
    ${PROJECT_SOURCE_DIR}/platform/darwin/src/native_apple_interface.m
    ${PROJECT_SOURCE_DIR}/platform/darwin/src/nsthread.mm
    ${PROJECT_SOURCE_DIR}/platform/darwin/src/number_format.mm
    ${PROJECT_SOURCE_DIR}/platform/darwin/src/run_loop.cpp
    ${PROJECT_SOURCE_DIR}/platform/darwin/src/string_nsstring.mm
    ${PROJECT_SOURCE_DIR}/platform/darwin/src/timer.cpp
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/gfx/headless_backend.cpp
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/gfx/headless_frontend.cpp
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/layermanager/layer_manager.cpp
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/map/map_snapshotter.cpp
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/platform/time.cpp
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/asset_file_source.cpp
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/mbtiles_file_source.cpp
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/database_file_source.cpp
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/file_source_manager.cpp
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/file_source_request.cpp
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/local_file_request.cpp
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/local_file_source.cpp
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/main_resource_loader.cpp
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/offline.cpp
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/offline_database.cpp
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/offline_download.cpp
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/online_file_source.cpp
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/storage/sqlite3.cpp
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/text/bidi.cpp
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/compression.cpp
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/filesystem.cpp
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/monotonic_timer.cpp
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/png_writer.cpp
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/thread_local.cpp
    ${PROJECT_SOURCE_DIR}/platform/default/src/mbgl/util/utf.cpp
)

target_compile_options(mbgl-core PRIVATE -fobjc-arc)

# FIXME: Should not be needed, but now needed by node because of the headless frontend.
target_include_directories(
  mbgl-core
  PUBLIC ${PROJECT_SOURCE_DIR}/platform/default/include
)

target_include_directories(
  mbgl-core
  PRIVATE
    ${PROJECT_SOURCE_DIR}/platform/darwin/include ${PROJECT_SOURCE_DIR}/platform/darwin/src ${PROJECT_SOURCE_DIR}/platform/macos/src
)

include(${PROJECT_SOURCE_DIR}/vendor/icu.cmake)

target_link_libraries(
  mbgl-core
  PRIVATE
    "-framework AppKit"
    "-framework CoreGraphics"
    "-framework CoreLocation"
    "-framework SystemConfiguration"
    mbgl-vendor-icu
    sqlite3
    z
)

add_subdirectory(${PROJECT_SOURCE_DIR}/platform/glfw)
if(MLN_WITH_NODE)
  add_subdirectory(${PROJECT_SOURCE_DIR}/platform/node)
endif()