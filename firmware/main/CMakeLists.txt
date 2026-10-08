idf_component_register(
    SRCS "firmware.cpp"
    INCLUDE_DIRS "."
    REQUIRES leds
)

target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wall -Wextra
    -Wshadow -Wconversion -Wsign-conversion
    -Wold-style-cast -Wnon-virtual-dtor -Woverloaded-virtual
    -Wnull-dereference -Wdouble-promotion -Wformat=2
)
