//
// Created by hwan on 2022-01-28.
//

#ifndef TAS2552_PROJ_RTOS_COMM_OBJECTS_H
#define TAS2552_PROJ_RTOS_COMM_OBJECTS_H

#include "stdint.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* how to use */
/*
 * init_comm_obj(TASK_SERVER, task_handle);
 * init_comm_obj(TASK_CLIENT, task_handle); .....
 *
 *
 *
 * */

#define PORT_NOTIFY_CALL                0
#define CLIENT_NOTIFY_CALL              1

typedef enum{
    TASK_SERVER,
    TASK_CLIENT,
    TASK_GATE_KEEPER,
    TASK_END
}e_rtos_task_info_t;

typedef enum{
    DELIVERY_FROM_HOST,
    DELIVERY_FROM_CLIENT,
    DELIVERY_FROM_SERVER,
    DELIVERY_FROM_GATE,
    DELIVERY_END,
}e_delivery_cmd_type_t;

typedef struct comm_obj{
    e_delivery_cmd_type_t type;
    void* data;
}comm_obj_t;

void init_comm_obj(e_rtos_task_info_t info, TaskHandle_t* handle);

void ui_queue_send_event(e_rtos_task_info_t taskInfo, comm_obj_t* commObj, uint8_t fromISR);

void ui_queue_wait_event(e_rtos_task_info_t taskInfo, comm_obj_t* commObj, uint8_t fromISR);

void ui_notify_send_event(e_rtos_task_info_t taskInfo, uint8_t fromISR);

uint32_t ui_notify_wait_event(TickType_t xMaxBlockTime);

#endif //TAS2552_PROJ_RTOS_COMM_OBJECTS_H
