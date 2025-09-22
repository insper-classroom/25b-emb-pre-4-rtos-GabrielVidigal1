#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

const int BTN_PIN_R = 28;
const int BTN_PIN_Y = 21;

const int LED_PIN_R = 5;
const int LED_PIN_Y = 10;

QueueHandle_t xQueueCommands;

SemaphoreHandle_t xSemaphore_r;
SemaphoreHandle_t xSemaphore_y;

typedef struct {
    int led_pin;
    bool status;
} led_command_t;

void btn_callback(uint gpio, uint32_t events) {
    if (events == GPIO_IRQ_EDGE_FALL) {
        if (gpio == BTN_PIN_R) {
            xSemaphoreGiveFromISR(xSemaphore_r, 0);
        } else if (gpio == BTN_PIN_Y) {
            xSemaphoreGiveFromISR(xSemaphore_y, 0);
        }
    }
}

void btn_r_task(void *p) {
    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);
    gpio_set_irq_enabled_with_callback(BTN_PIN_R, GPIO_IRQ_EDGE_FALL, true, &btn_callback);

    bool is_blinking = false;
    while (true) {
        if (xSemaphoreTake(xSemaphore_r, portMAX_DELAY)) {
            is_blinking = !is_blinking;
            
            led_command_t cmd = { .led_pin = LED_PIN_R, .status = is_blinking };
            xQueueSend(xQueueCommands, &cmd, 0);
        }
    }
}

void btn_y_task(void *p) {
    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y, GPIO_IN);
    gpio_pull_up(BTN_PIN_Y);
    gpio_set_irq_enabled_with_callback(BTN_PIN_Y, GPIO_IRQ_EDGE_FALL, true, &btn_callback);

    bool is_blinking = false;
    while (true) {
        if (xSemaphoreTake(xSemaphore_y, portMAX_DELAY)) {
            is_blinking = !is_blinking;
            
            led_command_t cmd = { .led_pin = LED_PIN_Y, .status = is_blinking };
            xQueueSend(xQueueCommands, &cmd, 0);
        }
    }
}

void led_handler_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);
    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);

    bool r_is_blinking = false;
    bool y_is_blinking = false;
    led_command_t received_cmd;

    while (true) {
        if (xQueueReceive(xQueueCommands, &received_cmd, 0)) {
            if (received_cmd.led_pin == LED_PIN_R) {
                r_is_blinking = received_cmd.status;
                if (!r_is_blinking) {
                    gpio_put(LED_PIN_R, 0);
                }
            } else if (received_cmd.led_pin == LED_PIN_Y) {
                y_is_blinking = received_cmd.status;
                if (!y_is_blinking) {
                    gpio_put(LED_PIN_Y, 0);
                }
            }
        }

       
        if (r_is_blinking) {
            gpio_put(LED_PIN_R, 1);
        }
        if (y_is_blinking) {
            gpio_put(LED_PIN_Y, 1);
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));

        
        if (r_is_blinking) {
            gpio_put(LED_PIN_R, 0);
        }
        if (y_is_blinking) {
            gpio_put(LED_PIN_Y, 0);
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

int main() {
    stdio_init_all();
    
    xSemaphore_r = xSemaphoreCreateBinary();
    xSemaphore_y = xSemaphoreCreateBinary();

    xQueueCommands = xQueueCreate(10, sizeof(led_command_t));

    xTaskCreate(led_handler_task, "LED_Handler_Task", 256, NULL, 1, NULL);
    xTaskCreate(btn_r_task, "BTN_Task_R", 256, NULL, 1, NULL);
    xTaskCreate(btn_y_task, "BTN_Task_Y", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while (1) {}
    return 0;
}