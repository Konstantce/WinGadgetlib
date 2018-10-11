add_executable(win_gadget_lib gadget.cpp utils.cpp test.cpp)

target_include_directories(win_gadget_lib PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_include_directories(win_gadget_lib PUBLIC ${Boost_INCLUDE_DIRS})
target_include_directories(win_gadget_lib PUBLIC ${NTL_INCLUDE_DIR})

target_link_libraries(win_gadget_lib PUBLIC ${NTL_LIBRARY})

install(TARGETS win_gadget_lib
		RUNTIME DESTINATION bin 
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
