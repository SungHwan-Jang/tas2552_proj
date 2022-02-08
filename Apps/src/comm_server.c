//
// Created by hwan on 2022-01-24.
//

#include "comm_server.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "usart.h"
#include "stdio.h"
#include "string.h"

#include "rtos_comm_objects.h"
#include "comm_client.h"

extern TaskHandle_t rs232ServerHandle;
static server_context_t context;

void rs232ServerTask(void *argument);

static void server_process(comm_obj_t *commObj);

static void host_receiver(void);

static void client_receiver(comm_obj_t *clientContext);

void init_comm_server_app(void) {
    /* definition and creation of rs232Server */

    xTaskCreate(rs232ServerTask,
                "rs232Server",
                128,
                NULL,
                0,
                &rs232ServerHandle);

    init_comm_obj(TASK_SERVER, &rs232ServerHandle);
}

void rs232ServerTask(void *argument) {
    /* USER CODE BEGIN rs232ServerTask */
    comm_obj_t *commObj = (comm_obj_t *) pvPortMalloc(sizeof(comm_obj_t));
    /* Infinite loop */
    for ever {
        ui_queue_wait_event(TASK_SERVER, commObj, pdFALSE);
        server_process(commObj);
    }
    /* USER CODE END rs232ServerTask */
}

static void server_process(comm_obj_t *commObj) {
    switch (commObj->type) {
        case DELIVERY_FROM_HOST: {
            host_receiver();
            break;
        }
        case DELIVERY_FROM_SERVER: {

            break;
        }
        case DELIVERY_FROM_CLIENT: {
            client_receiver(commObj);
            break;
        }
        case DELIVERY_FROM_GATE: {

            break;
        }
        default:
            break;
    }
}

static void host_receiver(void) {
    static comm_obj_t commObj;
    port_api_t *portApi = get_port_service_api();
    port_data_t data = portApi->receive_msg(portApi);

    switch (portApi->state) {
        case PACKET_EMPTY: {
            portYIELD();
            break;
        }
        case PACKET_FORMAT_ERROR: {
            portYIELD();
            break;
        }
        case PACKET_LENGTH_ERROR: {
            portYIELD();
            break;
        }
        case PACKET_OK: {
            memcpy(&context.packet, &data, sizeof(port_data_t));
            commObj.type = DELIVERY_FROM_SERVER;
            commObj.data = &context;
            ui_queue_send_event(TASK_CLIENT, &commObj, 0);
            break;
        }
        default:
            break;
    }
}

static void client_receiver(comm_obj_t *commObj) {
    port_api_t *portApi = get_port_service_api();
    client_context_t* clientCtx = (client_context_t*)commObj->data;
    port_data_t data;

    memcpy(&data, &clientCtx->packet, sizeof (port_data_t));

//    data.cmd = PORT_CMD_ACK;
//    data.data = clientCtx->data;

    portApi->post_msg(data);
}
