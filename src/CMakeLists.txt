set(SOURCE_FILES
        main.cpp
        CmdArg.cpp)

set(LIBS scif trans4scif pthread zmq)

include_directories("/usr/local/include")

add_executable(tpt ${SOURCE_FILES})
target_link_libraries(tpt ${LIBS})
set_target_properties(tpt PROPERTIES
        COMPILE_FLAGS "-std=c++11")

#INSTALL
install(TARGETS tpt DESTINATION ../bin)


