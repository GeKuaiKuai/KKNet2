aux_source_directory(./ LIBNET)
ADD_LIBRARY(kkutil SHARED ${LIBNET})
ADD_LIBRARY(kkutil_static STATIC ${LIBNET})
target_link_libraries(kkutil pthread)
add_definitions("-g")
INSTALL(TARGETS kkutil kkutil_static
LIBRARY DESTINATION lib
ARCHIVE DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/kknet/util)
