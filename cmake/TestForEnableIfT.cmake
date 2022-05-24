# --------------------------------------------------------------------------------------------
#--- TEST_FOR_ENABLE_IF_T(<variable>)
#
# Sets <variable> to TRUE or FALSE depending on whether the compiler supports std::enable_if_t
# (i.e. if it has a full implementation of C++14).
#---------------------------------------------------------------------------------------------
function(TEST_FOR_ENABLE_IF_T variable)

    try_compile(RESULT_VAR
        ${CMAKE_CURRENT_BINARY_DIR}
        ${PROJECT_SOURCE_DIR}/tools/test_compiler_supports_enable_if_t.cpp
        )

    set(${variable} ${RESULT_VAR} PARENT_SCOPE)

    message(STATUS "Test for compiler support for C++14 feature std::enable_if_t: ${RESULT_VAR}")

endfunction()
