//
// Created by hwan on 2022-01-24.
//

#include "comm_server.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "usart.h"
#include "port_service.h"
#include "stdio.h"

extern TaskHandle_t rs232ServerHandle;

void rs232ServerTask(void *argument);

void init_comm_server_app(void) {
    /* definition and creation of rs232Server */
    xTaskCreate(rs232ServerTask,
                "rs232Server",
                128,
                NULL,
                0,
                rs232ServerHandle);
}

void rs232ServerTask(void *argument) {
    /* USER CODE BEGIN rs232ServerTask */
//    initialize_port_service();
    s_port_api_t *portApi = get_port_service_api();

    /* Infinite loop */
    for ever {

        uint8_t str[4] = {0,};
        s_port_data_t data = portApi->commBuffer->parsing_buffer(portApi->commBuffer);

        str[0] = '<';
        str[1] = data.cmd;
        str[2] = data.data;
        str[3] = '>';

//        sprintf((char*)str, "0x%x\n\r", data.data);
        if (data.cmd != PORT_CMD_INIT) {
            HAL_UART_Transmit_DMA(&huart1, str, sizeof(str));
        }

        osDelay(200);
    }
    /* USER CODE END rs232ServerTask */
}
