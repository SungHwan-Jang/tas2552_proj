//
// Created by hwan on 2022-01-28.
//

#include "rtos_comm_objects.h"


typedef struct rtos_comm_obj {
    TaskHandle_t* taskHandle[TASK_END];
    QueueHandle_t queueHandle[TASK_END];
} s_rtos_comm_obj_t;

static s_rtos_comm_obj_t context;

void init_comm_obj(e_rtos_task_info_t info, TaskHandle_t* handle) {
    /* TODO: add clear handle and init again process */
    context.taskHandle[info] = handle;
    context.queueHandle[info] = xQueueCreate(5, sizeof(comm_obj_t));
}

void ui_queue_send_event(e_rtos_task_info_t taskInfo, comm_obj_t *commObj, uint8_t fromISR) {

    if (fromISR == pdFALSE) {
        xQueueSendToBack(context.queueHandle[taskInfo], commObj, 0);
        portYIELD();
    }
    else {
        xQueueSendToFrontFromISR(context.queueHandle[taskInfo], commObj, NULL);
    }
}

void ui_queue_wait_event(e_rtos_task_info_t taskInfo, comm_obj_t *commObj, uint8_t fromISR) {

    TaskHandle_t handle = xTaskGetCurrentTaskHandle();
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (fromISR == pdFALSE) {
        for (int i = 0; i < TASK_END; i++) {
            if (handle == *(context.taskHandle[i])) {
                xQueueReceive(context.queueHandle[i], commObj, portMAX_DELAY);
                break;
            }
        }
    }
    else {
        for (int i = 0; i < TASK_END; i++) {
            if (handle == *(context.taskHandle[i])) {
                xQueueReceiveFromISR(context.queueHandle[i], commObj, &xHigherPriorityTaskWoken);
                portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
                break;
            }
        }
    }
}

void ui_notify_send_event(e_rtos_task_info_t taskInfo, uint8_t fromISR) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (fromISR == pdTRUE) {
        vTaskNotifyGiveFromISR(*(context.taskHandle[taskInfo]), &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    else {
        xTaskNotifyGive(*(context.taskHandle[taskInfo]));
        portYIELD();
    }
}

uint32_t ui_notify_wait_event(TickType_t xMaxBlockTime){
    uint32_t ulEventsToProcess;
    ulEventsToProcess = ulTaskNotifyTake(pdTRUE, xMaxBlockTime); // can not delay process...

    return ulEventsToProcess;
}
