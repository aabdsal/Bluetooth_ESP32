idf_component_register(
    SRCS 
        "main.c"
        "src/mando.c"
        "src/ble_client.c"
    INCLUDE_DIRS 
        "include"
    REQUIRES bt nvs_flash driver)