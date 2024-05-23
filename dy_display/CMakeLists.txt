idf_component_register(
        SRCS display.c max7219.c
        INCLUDE_DIRS include
        REQUIRES driver dy_error dy_gfx dy_max7219
)
