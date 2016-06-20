set(SOURCE_FILES
        main.cpp
        ScifNode.cpp)

add_executable(tpt ${SOURCE_FILES})
target_link_libraries(tpt scif)
set_target_properties(tpt PROPERTIES
        COMPILE_FLAGS "-std=c++11")

add_executable(tpt_mic ${SOURCE_FILES})
target_link_libraries(tpt_mic scif)
set_target_properties(tpt_mic PROPERTIES
        COMPILE_FLAGS "-std=c++11 -mmic" LINK_FLAGS "-mmic")

#add_executable(runner nonius_test.cpp)

#target_link_libraries(runner pthread)