set(SOURCE_FILES
        main.cpp
        CmdArg.cpp
        common.cpp
        ScifNode.cpp
        Trans4ScifNode.cpp
#        ZeroMQNode.cpp
        )

set(LIBS scif trans4scif pthread)

include_directories("/usr/local/include")
link_directories("/usr/local/lib64")
add_executable(tpt ${SOURCE_FILES})
target_link_libraries(tpt ${LIBS})
set_target_properties(tpt PROPERTIES
        COMPILE_FLAGS "-std=c++11")

#INSTALL
install(TARGETS tpt DESTINATION ../bin)


