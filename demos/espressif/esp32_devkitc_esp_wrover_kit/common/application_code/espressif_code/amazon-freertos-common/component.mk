AMAZON_FREERTOS_LIB_DIR := ../../../../../../../lib
AMAZON_FREERTOS_DEMOS_DIR := ../../../../../../../demos

COMPONENT_SRCDIRS := $(AMAZON_FREERTOS_LIB_DIR)/mqtt \
        $(AMAZON_FREERTOS_LIB_DIR)/defender \
        $(AMAZON_FREERTOS_LIB_DIR)/shadow \
        $(AMAZON_FREERTOS_LIB_DIR)/greengrass \
        $(AMAZON_FREERTOS_LIB_DIR)/utils \
        $(AMAZON_FREERTOS_LIB_DIR)/utils/taskpool \
        $(AMAZON_FREERTOS_LIB_DIR)/common \
        $(AMAZON_FREERTOS_LIB_DIR)/utils/platform \
        $(AMAZON_FREERTOS_LIB_DIR)/bufferpool \
        $(AMAZON_FREERTOS_LIB_DIR)/tls \
        $(AMAZON_FREERTOS_LIB_DIR)/ota \
        $(AMAZON_FREERTOS_LIB_DIR)/crypto \
        $(AMAZON_FREERTOS_LIB_DIR)/wifi/portable/espressif/esp32_devkitc_esp_wrover_kit \
        $(AMAZON_FREERTOS_LIB_DIR)/ota/portable/espressif/esp32_devkitc_esp_wrover_kit \
        $(AMAZON_FREERTOS_LIB_DIR)/secure_sockets \
        $(AMAZON_FREERTOS_LIB_DIR)/secure_sockets/portable/freertos_plus_tcp \
        $(AMAZON_FREERTOS_LIB_DIR)/pkcs11/portable/espressif/esp32_devkitc_esp_wrover_kit \
        $(AMAZON_FREERTOS_LIB_DIR)/pkcs11/mbedtls \
        $(AMAZON_FREERTOS_LIB_DIR)/FreeRTOS-Plus-TCP/source \
        $(AMAZON_FREERTOS_LIB_DIR)/FreeRTOS-Plus-TCP/source/portable/BufferManagement \
        $(AMAZON_FREERTOS_LIB_DIR)/FreeRTOS-Plus-TCP/source/portable/NetworkInterface/esp32 \
        $(AMAZON_FREERTOS_LIB_DIR)/bluetooth_low_energy \
        $(AMAZON_FREERTOS_LIB_DIR)/bluetooth_low_energy/portable/espressif \
        $(AMAZON_FREERTOS_LIB_DIR)/bluetooth_low_energy/services/device_information \
        $(AMAZON_FREERTOS_LIB_DIR)/bluetooth_low_energy/services/mqtt_ble \
        $(AMAZON_FREERTOS_LIB_DIR)/bluetooth_low_energy/services/wifi_provisioning \
        $(AMAZON_FREERTOS_LIB_DIR)/metrics \
        $(AMAZON_FREERTOS_LIB_DIR)/serializer/json \
        $(AMAZON_FREERTOS_LIB_DIR)/serializer/cbor \
        $(AMAZON_FREERTOS_LIB_DIR)/provisioning/ \
        $(AMAZON_FREERTOS_LIB_DIR)/third_party/jsmn \
        $(AMAZON_FREERTOS_LIB_DIR)/third_party/tinycbor

COMPONENT_ADD_INCLUDEDIRS := $(AMAZON_FREERTOS_LIB_DIR)/FreeRTOS-Plus-TCP/include \
                             $(AMAZON_FREERTOS_LIB_DIR)/FreeRTOS-Plus-TCP/source/portable/Compiler/GCC \
                             $(AMAZON_FREERTOS_LIB_DIR)/include/bluetooth_low_energy \
                             $(AMAZON_FREERTOS_LIB_DIR)/bluetooth_low_energy/portable/espressif \
                             $(AMAZON_FREERTOS_LIB_DIR)/third_party/jsmn \
                             $(AMAZON_FREERTOS_LIB_DIR)/third_party/tinycbor

COMPONENT_OBJEXCLUDE := $(AMAZON_FREERTOS_LIB_DIR)/FreeRTOS-Plus-TCP/source/portable/BufferManagement/BufferAllocation_1.o

ifndef AMAZON_FREERTOS_ENABLE_UNIT_TEST
COMPONENT_SRCDIRS += ../.. \
    $(AMAZON_FREERTOS_DEMOS_DIR)/common/demo_runner \
    $(AMAZON_FREERTOS_DEMOS_DIR)/common/shadow \
    $(AMAZON_FREERTOS_DEMOS_DIR)/common/defender \
    $(AMAZON_FREERTOS_DEMOS_DIR)/common/mqtt \
    $(AMAZON_FREERTOS_DEMOS_DIR)/common/mqtt_v4 \
    $(AMAZON_FREERTOS_DEMOS_DIR)/common/mqtt_v4/network/bluetooth \
    $(AMAZON_FREERTOS_DEMOS_DIR)/common/greengrass_connectivity \
    $(AMAZON_FREERTOS_DEMOS_DIR)/common/tcp \
    $(AMAZON_FREERTOS_DEMOS_DIR)/common/ota \
    $(AMAZON_FREERTOS_DEMOS_DIR)/common/ble \
    $(AMAZON_FREERTOS_DEMOS_DIR)/common/network_manager \
    $(AMAZON_FREERTOS_DEMOS_DIR)/common/wifi_provisioning

COMPONENT_ADD_INCLUDEDIRS += $(AMAZON_FREERTOS_DEMOS_DIR)/common/include
endif

COMPONENT_PRIV_INCLUDEDIRS := $(AMAZON_FREERTOS_LIB_DIR)/third_party/pkcs11 $(AMAZON_FREERTOS_LIB_DIR)/ota/portable/espressif/esp32_devkitc_esp_wrover_kit

CFLAGS += -DIOT_CONFIG_FILE="\"iot_demo_config.h\""

lib/greengrass/aws_greengrass_discovery.o: CFLAGS+=-Wno-format
lib/common/aws_logging_task_dynamic_buffers.o: CFLAGS+=-Wno-format -Wno-uninitialized
demos/common/tcp/aws_tcp_echo_client_single_task.o: CFLAGS+=-Wno-format
