/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>


#include <stdio.h>
#include <string.h> 
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

const int BTN_PIN_R = 28;
const int BTN_PIN_Y = 21;

const int LED_PIN_R = 5;
const int LED_PIN_Y = 10;

QueueHandle_t xQueueButId;
QueueHandle_t xQueueButId2;


void btn_1_task(void *p) {
    gpio_init(BTN_PIN_R);
    gpio_set_dir(BTN_PIN_R, GPIO_IN);
    gpio_pull_up(BTN_PIN_R);

    int ligado_desligado = 0;
    while (true) {
        if (!gpio_get(BTN_PIN_R)) {

            while (!gpio_get(BTN_PIN_R)) {
                vTaskDelay(pdMS_TO_TICKS(1));
            }

            //printf("delay btn %d \n");
            ligado_desligado= !ligado_desligado;
            xQueueSend(xQueueButId, &(ligado_desligado), 0);
        }
    }
}

void led_1_task(void *p) {
    gpio_init(LED_PIN_R);
    gpio_set_dir(LED_PIN_R, GPIO_OUT);

    int ligado_desligado = 0;
    while (true) {
        if (xQueueReceive(xQueueButId, &ligado_desligado, 0)) {
            if(!ligado_desligado){
                gpio_put(LED_PIN_R, 0);
            }
            //printf("%d\n", delay);
        }

        if (ligado_desligado== 1) {
            gpio_put(LED_PIN_R, 1);
            vTaskDelay(pdMS_TO_TICKS(250));
            gpio_put(LED_PIN_R, 0);
            vTaskDelay(pdMS_TO_TICKS(250));
        } else{
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}






void btn_2_task(void *p) {
    gpio_init(BTN_PIN_Y);
    gpio_set_dir(BTN_PIN_Y, GPIO_IN);
    gpio_pull_up(BTN_PIN_Y);

    int ligado_desligado = 0;
    while (true) {
        if (!gpio_get(BTN_PIN_Y)) {

            while (!gpio_get(BTN_PIN_Y)) {
                vTaskDelay(pdMS_TO_TICKS(1));
            }

            //printf("delay btn %d \n");
            ligado_desligado= !ligado_desligado;
            xQueueSend(xQueueButId2, &(ligado_desligado), 0);
        }
    }
}

void led_2_task(void *p) {
    gpio_init(LED_PIN_Y);
    gpio_set_dir(LED_PIN_Y, GPIO_OUT);

    int ligado_desligado = 0;
    while (true) {
        if (xQueueReceive(xQueueButId2, &ligado_desligado, 0)) {
            if(!ligado_desligado){
                gpio_put(LED_PIN_Y, 0);
            }
            //printf("%d\n", delay);
        }

        if (ligado_desligado== 1) {
            gpio_put(LED_PIN_Y, 1);
            vTaskDelay(pdMS_TO_TICKS(250));
            gpio_put(LED_PIN_Y, 0);
            vTaskDelay(pdMS_TO_TICKS(250));
        } else{
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}



int main() {
    stdio_init_all();
    xQueueButId = xQueueCreate(32, sizeof(int));
    xQueueButId2 = xQueueCreate(32, sizeof(int));


    xTaskCreate(led_1_task, "LED_Task 1", 256, NULL, 1, NULL);
    xTaskCreate(btn_1_task, "BTN_Task 1", 256, NULL, 1, NULL);

    xTaskCreate(led_2_task, "LED_Task 2", 256, NULL, 1, NULL);
    xTaskCreate(btn_2_task, "BTN_Task 2", 256, NULL, 1, NULL);

    vTaskStartScheduler();

    while(1){}

    return 0;
}