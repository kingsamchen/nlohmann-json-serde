
add_executable(nlohmann_serde_test)

target_sources(nlohmann_serde_test
  PRIVATE
    test_main.cpp
    stringification.h

    options_detail_test.cpp
    serde_test.cpp
)

target_link_libraries(nlohmann_serde_test
  PRIVATE
    doctest::doctest

    nlohmann_serde
)

nlohmann_json_serde_apply_common_compile_options(nlohmann_serde_test)

if(NLOHMANN_JSON_SERDE_USE_CLANG_TIDY)
  nlohmann_json_serde_apply_clang_tidy(nlohmann_serde_test)
endif()

if(NLOHMANN_JSON_SERDE_USE_SANITIZER)
  nlohmann_json_serde_apply_sanitizer(nlohmann_serde_test)
endif()

if(MSVC)
  if(NLOHMANN_JSON_SERDE_USE_MSVC_PARALLEL_BUILD)
    nlohmann_json_serde_apply_msvc_parallel_build(nlohmann_serde_test)
  endif()
  if(NLOHMANN_JSON_SERDE_USE_MSVC_STATIC_ANALYSIS)
    nlohmann_json_serde_apply_msvc_static_analysis(nlohmann_serde_test)
  endif()
endif()

get_target_property(tests_FILES nlohmann_serde_test SOURCES)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${tests_FILES})

add_test(NAME tests COMMAND nlohmann_serde_test)
