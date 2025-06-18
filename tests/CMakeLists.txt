set(UCALL_UNIT_TESTS ucall_tests)

include(CTest)

file(GLOB TARGET_SRC "./tests.cpp" )

add_executable(${UCALL_UNIT_TESTS} ${TARGET_SRC})

add_test(${UCALL_UNIT_TESTS} ${UCALL_UNIT_TESTS})