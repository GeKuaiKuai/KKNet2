aux_source_directory(./ LIBNET)
ADD_LIBRARY(kknet SHARED ${LIBNET})
ADD_LIBRARY(kknet_static STATIC ${LIBNET})
target_link_libraries(kknet kkutil)
add_definitions("-g")
INSTALL(TARGETS kknet kknet_static
LIBRARY DESTINATION lib
ARCHIVE DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/kknet/reactor)

