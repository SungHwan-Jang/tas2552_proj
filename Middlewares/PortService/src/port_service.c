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
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

#define newRingBuffer(init_callback, insert_callback, find_callback) \
                        {{0,}, 0, 0, MAX_RINGBUFFER_SIZE, pdFAIL, (e_packet_err_state_t)PACKET_INIT, (init_callback), (insert_callback), (find_callback)}

#define newContext(buf) {buf}

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

static s_port_ring_buffer_t portRingBuffer;
static s_port_api_t ctx;
static uint8_t rxBuf;

/* USER CODE END Variables */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void init_buffer(struct port_ring_buffer *this);

void insert_buffer_item(struct port_ring_buffer *this, uint8_t data);

s_port_data_t parse_buffer(struct port_ring_buffer *this);

uint8_t *make_packet(s_port_data_t portData);

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

/* USER CODE END FunctionPrototypes */

void initialize_port_service(void) {
    s_port_ring_buffer_t initBuffer = (s_port_ring_buffer_t) newRingBuffer(init_buffer, insert_buffer_item,
                                                                           parse_buffer);
    memcpy(&portRingBuffer, &initBuffer, sizeof(s_port_ring_buffer_t));

    s_port_api_t initCtx = newContext(&portRingBuffer);
    memcpy(&ctx, &initCtx, sizeof(s_port_api_t));

    HAL_UART_Receive_IT(&huart1, &rxBuf, 1);
}

s_port_api_t *get_port_service_api(void) {
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

s_port_data_t parse_buffer(struct port_ring_buffer *this) {
    s_port_data_t pData = {PORT_CMD_INIT, 0x00};
    int32_t stxIndex = -1;
    int32_t etxIndex = -1;
    uint32_t i = 0;
    uint32_t packetCnt = 0;

    if (get_buf_data_size(this) == 0) {
        this->state = PACKET_EMPTY;
        return pData;
    }

    for (i = this->head; i < (get_buf_data_size(this) + this->head); i++) {
        if (this->buffer[i % this->maxSize] == '<') {
            stxIndex = i % this->maxSize;
            break;
        }
    }

    for (i = stxIndex; i < (get_buf_data_size(this) + this->head); i++) {
        if (this->buffer[i % this->maxSize] == '>') {
            etxIndex = i % this->maxSize;
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
            this->head = (etxIndex + 1) % this->maxSize;
            this->full = pdFALSE;
            this->state = PACKET_LENGTH_ERROR;
        }
        else {
            pData.cmd = (e_comm_cmd_list_t) this->buffer[stxIndex + 1];
            pData.data = this->buffer[etxIndex - 1];

            this->head = (etxIndex + 1) % this->maxSize;
            this->full = pdFALSE;
            this->state = PACKET_OK;
        }
    }
    else {
        this->state = PACKET_FORMAT_ERROR;
        this->init(this);
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
    if(huart->Instance == USART1){
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_8);
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {

    if (huart->Instance == USART1) {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_9);
        portRingBuffer.insert_item(&portRingBuffer, rxBuf);
        HAL_UART_Receive_IT(huart, &rxBuf, 1);
    }
}
/* USER CODE END Application */

