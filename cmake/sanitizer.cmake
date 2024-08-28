
set(NLOHMANN_JSON_SERDE_USE_SANITIZER "" CACHE STRING
  "Options are, case-insensitive: ASAN, UBSAN, TSAN. use ; to separate multiple sanitizer")
message(STATUS "NLOHMANN_JSON_SERDE_USE_SANITIZER = ${NLOHMANN_JSON_SERDE_USE_SANITIZER}")

set(NLOHMANN_JSON_SERDE_SANITIZER_COMPILE_FLAGS "")
set(NLOHMANN_JSON_SERDE_SANITIZER_LINK_FLAGS "")

if(NLOHMANN_JSON_SERDE_USE_SANITIZER)
  if(MSVC)
    foreach(SANITIZER_ORIG IN LISTS NLOHMANN_JSON_SERDE_USE_SANITIZER)
      string(TOUPPER "${SANITIZER_ORIG}" SANITIZER)

      if("${SANITIZER}" STREQUAL "ASAN")
        list(APPEND NLOHMANN_JSON_SERDE_SANITIZER_COMPILE_FLAGS "/fsanitize=address")
        list(APPEND NLOHMANN_JSON_SERDE_SANITIZER_LINK_FLAGS "/INCREMENTAL:NO")
      else()
        message(FATAL_ERROR "Unsupported sanitizer=${SANITIZER}")
      endif()
    endforeach()
  else()
    list(APPEND NLOHMANN_JSON_SERDE_SANITIZER_COMPILE_FLAGS "-fno-omit-frame-pointer")

    foreach(SANITIZER_ORIG IN LISTS NLOHMANN_JSON_SERDE_USE_SANITIZER)
      string(TOUPPER "${SANITIZER_ORIG}" SANITIZER)

      if("${SANITIZER}" STREQUAL "ASAN")
        list(APPEND NLOHMANN_JSON_SERDE_SANITIZER_COMPILE_FLAGS "-fsanitize=address")
        list(APPEND NLOHMANN_JSON_SERDE_SANITIZER_LINK_FLAGS "-fsanitize=address")
      elseif("${SANITIZER}" STREQUAL "UBSAN")
        list(APPEND NLOHMANN_JSON_SERDE_SANITIZER_COMPILE_FLAGS "-fsanitize=undefined")
        list(APPEND NLOHMANN_JSON_SERDE_SANITIZER_LINK_FLAGS "-fsanitize=undefined")
      elseif("${SANITIZER}" STREQUAL "TSAN")
        list(APPEND NLOHMANN_JSON_SERDE_SANITIZER_COMPILE_FLAGS "-fsanitize=thread")
        list(APPEND NLOHMANN_JSON_SERDE_SANITIZER_LINK_FLAGS "-fsanitize=thread")
      else()
        message(FATAL_ERROR "Unsupported sanitizer=${SANITIZER}")
      endif()
    endforeach()
  endif()
endif()

function(nlohmann_json_serde_apply_sanitizer TARGET)
  message(STATUS "Apply nlohmann_json_serde sanitizer for ${TARGET}")

  target_compile_options(${TARGET}
    PRIVATE
      ${NLOHMANN_JSON_SERDE_SANITIZER_COMPILE_FLAGS}
  )

  target_link_options(${TARGET}
    PRIVATE
      ${NLOHMANN_JSON_SERDE_SANITIZER_LINK_FLAGS}
  )
endfunction()
