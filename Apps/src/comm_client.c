//
// Created by hwan on 2022-01-24.
//

#include "comm_client.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "usart.h"
#include "rtos_comm_objects.h"

#include "comm_server.h"
#include "string.h"


extern TaskHandle_t rs232ClientHandle;
static client_context_t context;

void rs232ClientTask(void *argument);

static void client_process(comm_obj_t *commObj);

static void server_receiver(comm_obj_t *commObj);

static void gate_receiver(comm_obj_t *commObj);

void init_comm_client_app(void) {
    /* definition and creation of rs232Server */

    xTaskCreate(rs232ClientTask,
                "rs232Server",
                128,
                NULL,
                0,
                &rs232ClientHandle);

    init_comm_obj(TASK_CLIENT, &rs232ClientHandle);
}

void rs232ClientTask(void *argument) {
    /* USER CODE BEGIN rs232ServerTask */
    comm_obj_t *commObj = (comm_obj_t *) pvPortMalloc(sizeof(comm_obj_t));
    /* Infinite loop */
    for ever {
        ui_queue_wait_event(TASK_CLIENT, commObj, 0);
        client_process(commObj);
    }
    /* USER CODE END rs232ServerTask */
}

static void client_process(comm_obj_t *commObj) {
    switch (commObj->type) {
        case DELIVERY_FROM_HOST: {
            // no exec
            break;
        }
        case DELIVERY_FROM_SERVER: {
            server_receiver(commObj);
            break;
        }
        case DELIVERY_FROM_CLIENT: {
            break;
        }
        case DELIVERY_FROM_GATE: {
            gate_receiver(commObj);
            break;
        }
        default:
            break;
    }
}

static void server_receiver(comm_obj_t *commObj) {
    static comm_obj_t senderObj;
    server_context_t* serverCtx = (server_context_t*)commObj->data;

    memcpy(&context.packet, &serverCtx->packet, sizeof (port_data_t));
    senderObj.data = &context;
    senderObj.type = DELIVERY_FROM_CLIENT;

    switch (serverCtx->packet.cmd) {
        case PORT_CMD_SYSTEM_ON:{
            ui_queue_send_event(TASK_GATE_KEEPER, &senderObj, 0);
            break;
        }
        case PORT_CMD_SYSTEM_OFF:{
            ui_queue_send_event(TASK_GATE_KEEPER, &senderObj, 0);
            break;
        }
        case PORT_CMD_GAIN_SETTING:{
            ui_queue_send_event(TASK_GATE_KEEPER, &senderObj, 0);
            break;
        }
        default:
            break;
    }
}

static void gate_receiver(comm_obj_t *commObj){
    static comm_obj_t senderObj;
    app_service_context_t * appServiceCtx = (app_service_context_t*)commObj->data;
    context.packet.data = appServiceCtx->cmdList;

    switch (appServiceCtx->state) {
        case APP_DONE:{
            context.packet.cmd = PORT_CMD_ACK;
            break;
        }
        case APP_ERR:{
            // TODO: add err exception handler ?
            context.packet.cmd = PORT_CMD_NACK;
            break;
        }
    }

    senderObj.type = DELIVERY_FROM_CLIENT;
    senderObj.data = &context;

    ui_queue_send_event(TASK_SERVER, &senderObj, 0);
}

