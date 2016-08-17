set(G72X_SOURCES
	g721.c
	g723_16.c
	g723_24.c
	g723_40.c
	g72x.c
	g72x_test.c
)

add_library(G72x OBJECT ${G72X_SOURCES})
set_property(TARGET G72x PROPERTY POSITION_INDEPENDENT_CODE TRUE)
target_include_directories (G72x PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
