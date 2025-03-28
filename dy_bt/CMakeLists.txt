if(CONFIG_BT_ENABLED)
idf_component_register(
        SRCS bt.c
        INCLUDE_DIRS include
        REQUIRES bt dy_error
)
endif ()