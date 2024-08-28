
add_library(nlohmann_serde INTERFACE)

target_sources(nlohmann_serde
  PRIVATE
    nlohmann_serde.h
)

target_include_directories(nlohmann_serde
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(nlohmann_serde
)

target_compile_definitions(nlohmann_serde
  INTERFACE
    $<$<BOOL:${WIN32}>:
      _UNICODE
      UNICODE
    >

    $<$<CONFIG:DEBUG>:
      _DEBUG
    >
)

target_compile_options(nlohmann_serde
  INTERFACE
    $<$<BOOL:${WIN32}>:
      /Zc:preprocessor
    >
)

get_target_property(nlohmann_serde_FILES nlohmann_serde SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${nlohmann_serde_FILES})
