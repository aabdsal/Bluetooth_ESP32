idf_component_register(
    SRC_DIRS "." "src"
    PRIV_INCLUDE_DIRS "include"
    PRIV_REQUIRES spi_flash driver log bt nvs_flash
)
