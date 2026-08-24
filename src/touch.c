#include "touch.h"
#include "driver/i2c.h"
#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <string.h>

// Name mangling fix for C++
#ifdef __cplusplus
extern "C" {
#endif

// Semaphore to signal the main loop that a touch happened
static SemaphoreHandle_t touch_sem = NULL;

// ISR Handler: Keep it minimal!
static void IRAM_ATTR touch_isr_handler(void* arg) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(touch_sem, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
}

esp_err_t touch_init2(void) {
    // 1. Create Semaphore for synchronization
    touch_sem = xSemaphoreCreateBinary();

    // 2. Hardware Reset (Crucial for LilyGo to wake the chip)
    gpio_set_direction(TOUCH_RST_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(TOUCH_RST_PIN, 0); 
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(TOUCH_RST_PIN, 1); 
    vTaskDelay(pdMS_TO_TICKS(100));

    //i2c initialization is now handled in main.cpp to ensure it's set up before the touch controller is initialized.
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = 23,
        .scl_io_num = 32,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };
    i2c_param_config(I2C_NUM_1, &conf);
    i2c_driver_install(I2C_NUM_1, conf.mode, 0, 0, 0);

    // 3. Configure Interrupt Pin
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_LOW_LEVEL,
        .pin_bit_mask = (1ULL << TOUCH_INT_PIN),
        .pull_up_en = 1
    };
    gpio_config(&io_conf);

    // 4. Install ISR
    gpio_install_isr_service(0);
    gpio_isr_handler_add(TOUCH_INT_PIN, touch_isr_handler, NULL);

    return ESP_OK;
}

bool touch_has_event(void) {
    // Non-blocking check if the interrupt gave the semaphore
    return xSemaphoreTake(touch_sem, 0) == pdTRUE;
}

esp_err_t touch_read_coords(touch_data_t *data) {
    uint8_t reg = 0x02; // Start at TD_STATUS
    uint8_t buf[5];     // Status, X_high, X_low, Y_high, Y_low

    // We read 5 bytes. 
    // The previous error -1 was likely because the device wasn't powered 
    // or SDA/SCL pins weren't initialized in main.cpp
    esp_err_t ret = i2c_master_write_read_device(I2C_NUM_1, FT6236_ADDR, &reg, 1, buf, 5, pdMS_TO_TICKS(1000));
    
    //if (ret != ESP_OK) return ret;
    if (ret != ESP_OK) {
        printf("I2C Error: %s\n", esp_err_to_name(ret));
    }

    data->points = buf[0] & 0x0F; // Masking to get number of touch points

    if (data->points > 0) {
        // Parse X: Top 4 bits from buf[1], all 8 bits from buf[2]
        // We mask buf[1] with 0x0F to ignore the 'Event Flag' bits
        data->x = ((buf[1] & 0x0F) << 8) | buf[2];
        
        // Parse Y: Top 4 bits from buf[3], all 8 bits from buf[4]
        data->y = ((buf[3] & 0x0F) << 8) | buf[4];
    } else {
        data->x = 0;
        data->y = 0;
    }

    return ESP_OK;
}
