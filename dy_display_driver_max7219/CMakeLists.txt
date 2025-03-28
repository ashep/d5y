idf_component_register(
        SRCS max7219.c driver.c
        INCLUDE_DIRS include
        REQUIRES driver dy_error dy_gfx dy_display
)
