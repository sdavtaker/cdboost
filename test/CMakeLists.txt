file(GLOB TEST_SOURCES *_test.cpp)

add_executable(cdboost_tests main-test.cpp ${TEST_SOURCES})
target_link_libraries(cdboost_tests PRIVATE cdboost Boost::unit_test_framework)
target_compile_definitions(cdboost_tests PRIVATE BOOST_TEST_DYN_LINK)

add_test(NAME cdboost_tests COMMAND cdboost_tests)
