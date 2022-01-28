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
        s_port_data_t data = portApi->receive_msg(portApi);

        switch (portApi->state) {
            case PACKET_EMPTY:{
                portYIELD();
                break;
            }
            case PACKET_FORMAT_ERROR:{
                portYIELD();
                break;
            }
            case PACKET_LENGTH_ERROR:{
                portYIELD();
                break;
            }
            case PACKET_OK:{
                portApi->post_msg(data);
                break;
            }
            default:
                break;
        }
    }
    /* USER CODE END rs232ServerTask */
}
