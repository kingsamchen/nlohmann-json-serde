
option(NLOHMANN_JSON_SERDE_USE_CLANG_TIDY "Enable clang-tidy on build" OFF)
message(STATUS "NLOHMANN_JSON_SERDE_USE_CLANG_TIDY = ${NLOHMANN_JSON_SERDE_USE_CLANG_TIDY}")

if(NLOHMANN_JSON_SERDE_USE_CLANG_TIDY)
  find_program(CLANG_TIDY_EXE
               NAMES clang-tidy
               DOC "Path to clang-tidy executable"
               REQUIRED)
  message(STATUS "Found clang-tidy = ${CLANG_TIDY_EXE}")
endif()

function(nlohmann_json_serde_apply_clang_tidy TARGET)
  message(STATUS "Apply nlohmann_json_serde clang-tidy for ${TARGET}")

  if(MSVC AND CMAKE_GENERATOR MATCHES "Visual Studio")
    set_target_properties(${TARGET} PROPERTIES
      VS_GLOBAL_RunCodeAnalysis true
      VS_GLOBAL_EnableClangTidyCodeAnalysis true
    )
  else()
    set(CLANG_TIDY_COMMAND
      "${CLANG_TIDY_EXE}"
      "--quiet"
      "-p" "'${CMAKE_BINARY_DIR}'"
    )
    set_target_properties(${TARGET} PROPERTIES
      CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}"
    )
  endif()
endfunction()
