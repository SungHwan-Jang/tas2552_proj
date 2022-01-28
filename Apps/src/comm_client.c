//
// Created by hwan on 2022-01-24.
//

#include "comm_client.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "usart.h"

extern TaskHandle_t rs232ClientHandle;

void rs232ClientTask(void *argument);

void init_comm_client_app(void){
    /* definition and creation of rs232Server */
    xTaskCreate(rs232ClientTask,
                "rs232Server",
                128,
                NULL,
                0,
                rs232ClientHandle);
}

void rs232ClientTask(void *argument) {
    /* USER CODE BEGIN rs232ServerTask */
    /* Infinite loop */
    for ever {
//        uint8_t gateKeeperStr[] = "<cd>";
//        HAL_UART_Transmit(&huart1, gateKeeperStr, sizeof(gateKeeperStr), 1);
        osDelay(100);
    }
    /* USER CODE END rs232ServerTask */
}

