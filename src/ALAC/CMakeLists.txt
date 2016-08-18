include_directories(${CMAKE_SOURCE_DIR}/src)

set(ALAC_SOURCES
	ALACBitUtilities.c
	ag_dec.c
	ag_enc.c
	alac_decoder.c
	alac_encoder.c
	dp_dec.c
	dp_enc.c
	matrix_dec.c
	matrix_enc.c
)

add_library(ALAC OBJECT ${ALAC_SOURCES})
set_property(TARGET ALAC PROPERTY POSITION_INDEPENDENT_CODE TRUE)
target_include_directories (ALAC
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)
