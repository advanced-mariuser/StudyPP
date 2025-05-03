include(AddCatch2)

file(GLOB_RECURSE BANK_SOURCES ../src/*.cpp ../src/*.h)

add_executable(
        unit_tests
        AtomicMaxTests.cpp
        ${BANK_SOURCES}
)
target_link_libraries(
        unit_tests PRIVATE Catch2::Catch2WithMain
)

target_include_directories(unit_tests PRIVATE ../src)

include(Catch)
catch_discover_tests(unit_tests)

add_custom_target(run_tests
        COMMAND unit_tests
        DEPENDS unit_tests
)