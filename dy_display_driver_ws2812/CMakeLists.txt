idf_component_register(
        SRCS driver.c
        INCLUDE_DIRS include
        REQUIRES driver dy_error dy_gfx dy_display led_strip
)
