#include <driver/i2c.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include "sdkconfig.h"

#define SDA_PIN 18
#define SCL_PIN 19

static char tag[] = "i2cscanner";

typedef void (*i2c_scan_callback_t)(uint8_t address, bool success);

void i2c_scanner_task(void *parameters) {
    ESP_LOGD(tag, ">> i2cScanner");

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SDA_PIN,
        .scl_io_num = SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master = {
            .clk_speed = 100000,
        },
    };
    i2c_param_config(I2C_NUM_0, &conf);
    i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);

    printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
    printf("00:         ");

    for (int i = 3; i < 0x78; i++) {
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        if (!cmd) {
            ESP_LOGE(tag, "Failed to create I2C command");
            continue;
        }
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (i << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);

        esp_err_t espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
        bool success = (espRc == ESP_OK);
        if (success) {
            printf(" %.2x", i);
        } else {
            printf(" --");
        }
        i2c_cmd_link_delete(cmd);
    }
    printf("\n");

    vTaskDelete(NULL);
}

void i2c_scanner_init(i2c_scan_callback_t callback) {
    xTaskCreate(i2c_scanner_task, "i2c_scanner_task", configMINIMAL_STACK_SIZE * 3, NULL, 5, NULL);
}

void i2c_scan_callback(uint8_t address, bool success) {
    if (success) {
        ESP_LOGI(tag, "Device found at address 0x%x", address);
    } else {
        ESP_LOGI(tag, "No device found at address 0x%x", address);
    }
}


