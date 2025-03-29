idf_component_register(
        SRCS pixel.c buffer.c geometry.c text.c animation.c
        INCLUDE_DIRS include
        REQUIRES dy_error
)
