//
// Created by hwan on 2022-01-21.
//

/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "port_service.h"
#include "usart.h"
#include "string.h"
#include "rtos_comm_objects.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define PACKET_LENGTH                           5

/* USER CODE END PTD */

#define newRingBuffer(init_callback, insert_callback, find_callback) \
                        {{0,}, 0, 0, MAX_RINGBUFFER_SIZE, pdFAIL, (init_callback), (insert_callback)}

#define newContext(buf, post_msg, receive_msg) \
                        {(buf), (e_packet_state_t)PORT_CMD_INIT, (e_noti_mode_t) PORT_MODE_END,(post_msg), (receive_msg)}

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

static s_port_ring_buffer_t portRingBuffer;
static port_api_t ctx;
static uint8_t rxBuf;

/* USER CODE END Variables */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void init_buffer(struct port_ring_buffer *this);

void insert_buffer_item(struct port_ring_buffer *this, uint8_t data);

port_data_t parse_buffer(struct port_api *this);

void post_msg(port_data_t portData);

// =================================================== //

#if 0
static port_service_state_t is_buf_full(struct port_ring_buffer *this) {
    return this->full;
}

static port_service_state_t is_buf_empty(struct port_ring_buffer *this) {
    return (!this->full && (this->head == this->tail));
}

static uint32_t get_buf_max_size(struct port_ring_buffer *this) {
    return this->maxSize;
}
#endif

static uint32_t get_buf_data_size(struct port_ring_buffer *this) {
    uint32_t size = this->maxSize;

    if (!this->full) {
        if (this->head <= this->tail) {
            size = (this->tail - this->head);
        }
        else {
            size = (this->maxSize - this->head + this->tail);
        }
    }
    return size;
}

static void insert_fix_buf_pos(struct port_ring_buffer *this) {

    this->tail = ++(this->tail) % this->maxSize;

    if (this->head == this->tail) {
        this->head = ++(this->head) % this->maxSize;
    }

#ifdef PORT_SERVICE_DEBUG
    printf("useful buf = %d\n", get_buf_data_size(this));
    printf("full state = %d\n", this->full);
    printf("head = %d || tail = %d\n\n", this->head, this->tail);
#endif
}

static uint8_t *make_packet(port_data_t portData) {
    static uint8_t packet[PACKET_LENGTH];

    memset(packet, 0x00, sizeof(packet));

    packet[0] = '<';
    packet[1] = portData.cmd;
    packet[2] = portData.data;
    packet[3] = '>';
    packet[4] = '\0';

    return packet;
}

/* USER CODE END FunctionPrototypes */

void initialize_port_service(e_noti_mode_t mode) {
    s_port_ring_buffer_t initBuffer = (s_port_ring_buffer_t) newRingBuffer(init_buffer, insert_buffer_item,
                                                                           parse_buffer);
    memcpy(&portRingBuffer, &initBuffer, sizeof(s_port_ring_buffer_t));

    port_api_t initCtx = newContext(&portRingBuffer, post_msg, parse_buffer);
    initCtx.mode = mode;

    memcpy(&ctx, &initCtx, sizeof(port_api_t));
    HAL_UART_Receive_IT(&huart1, &rxBuf, 1);
}

port_api_t *get_port_service_api(void) {
    return &ctx;
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

void init_buffer(struct port_ring_buffer *this) {
    this->head = 0;
    this->tail = 0;
    this->full = pdFALSE;
}

void insert_buffer_item(struct port_ring_buffer *this, uint8_t data) {
    this->buffer[this->tail] = data;

    if (get_buf_data_size(this) == this->maxSize - 1) {
        this->full = pdTRUE;
    }

    insert_fix_buf_pos(this);
}

void post_msg(port_data_t portData) {
    uint8_t *packet = make_packet(portData);

    if (portData.cmd != PORT_CMD_INIT) {
        HAL_UART_Transmit_DMA(&huart1, packet, PACKET_LENGTH);
    }
}

port_data_t parse_buffer(struct port_api *this) {
    port_data_t pData = {PORT_CMD_INIT, 0x00};
    int32_t stxIndex = -1;
    int32_t etxIndex = -1;
    uint32_t i = 0;
    uint32_t packetCnt = 0;

    if (get_buf_data_size(this->commBuffer) == 0) {
        this->state = PACKET_EMPTY;
        return pData;
    }

    for (i = this->commBuffer->head; i < (get_buf_data_size(this->commBuffer) + this->commBuffer->head); i++) {
        if (this->commBuffer->buffer[i % this->commBuffer->maxSize] == '<') {
            stxIndex = i % this->commBuffer->maxSize;
            break;
        }
    }

    for (i = stxIndex; i < (get_buf_data_size(this->commBuffer) + this->commBuffer->head); i++) {
        if (this->commBuffer->buffer[i % this->commBuffer->maxSize] == '>') {
            etxIndex = i % this->commBuffer->maxSize;
            break;
        }
    }

#ifdef PORT_SERVICE_DEBUG
    printf("sIndex = %d, eIndex = %d\n", stxIndex, etxIndex);
#endif

    if (stxIndex != -1 && etxIndex != -1 && stxIndex != etxIndex) {

        if (stxIndex < etxIndex) {
            packetCnt = etxIndex - stxIndex - 1;
        }
        else {
            packetCnt = -1 * (etxIndex - stxIndex - 1);
        }

        if (packetCnt != 2) {
            this->commBuffer->head = (etxIndex + 1) % this->commBuffer->maxSize;
            this->commBuffer->full = pdFALSE;
            this->state = PACKET_LENGTH_ERROR;
        }
        else {
            pData.cmd = (e_comm_cmd_list_t) this->commBuffer->buffer[stxIndex + 1];
            pData.data = this->commBuffer->buffer[etxIndex - 1];

            this->commBuffer->head = (etxIndex + 1) % this->commBuffer->maxSize;
            this->commBuffer->full = pdFALSE;
            this->state = PACKET_OK;
        }
    }
    else {
        this->state = PACKET_FORMAT_ERROR;
        this->commBuffer->init(this->commBuffer);
    }

#ifdef PORT_SERVICE_DEBUG
    printf("useful buf = %d\n", get_buf_data_size(this));
    printf("full state = %d\n", this->full);
    printf("head = %d || tail = %d\n", this->head, this->tail);
    printf("packet length = %d\n", packetCnt);
    printf("RESULT CMD = %d, DATA = %d\n\n", pData.cmd, pData.data);
#endif

    return pData;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_8); //blue
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {

    static comm_obj_t commObj;

    commObj.type = DELIVERY_FROM_HOST;

    if (huart->Instance == USART1) {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_9); // green
        portRingBuffer.insert_item(&portRingBuffer, rxBuf);

        switch (ctx.mode) {
            case PORT_NOTIFICATION_MODE: {
                if (get_buf_data_size(&portRingBuffer) >= 3) {
                    ui_notify_send_event(TASK_SERVER, 1);
                }
                break;
            }
            case PORT_QUEUE_SEND_MODE: {
                if (get_buf_data_size(&portRingBuffer) >= 3) {
                    ui_queue_send_event(TASK_SERVER, &commObj, 1);
                }
                break;
            }
            default:
                // TODO: add err state;
                break;
        }

        HAL_UART_Receive_IT(huart, &rxBuf, 1);
    }
}
/* USER CODE END Application */

