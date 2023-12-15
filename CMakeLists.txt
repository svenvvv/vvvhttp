cmake_minimum_required(VERSION 3.0)

set(CMAKE_C_STANDARD 99)

project(crouter LANGUAGES C)

add_executable(crouter
    main.c
    vvvhttp/router.c
    vvvhttp/parser.c
    vvvhttp/serializer.c
    vvvhttp/util.c
    vvvhttp/response.c
)

option(USE_PCRE2 "Use PCRE2 for regex support" ON)

target_compile_options(crouter PRIVATE -fanalyzer)
target_include_directories(crouter PRIVATE include/vvvhttp)

target_compile_options(crouter PRIVATE -Wall -Wextra)

if (USE_PCRE2)
    target_compile_definitions(crouter PUBLIC VVVHTTP_ROUTE_ENABLE_REGEX)
endif ()

install(TARGETS crouter RUNTIME DESTINATION bin)
