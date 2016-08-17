set(GSM610_SOURCES
	add.c
	code.c
	decode.c
	gsm_create.c
	gsm_decode.c
	gsm_destroy.c
	gsm_encode.c
	gsm_option.c
	long_term.c
	lpc.c
	preprocess.c
	rpe.c
	short_term.c
	table.c
)

add_library(GSM610 OBJECT ${GSM610_SOURCES})
set_property(TARGET GSM610 PROPERTY POSITION_INDEPENDENT_CODE TRUE)
target_include_directories (GSM610 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
