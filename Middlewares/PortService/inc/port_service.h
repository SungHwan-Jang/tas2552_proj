//
// Created by hwan on 2022-01-21.
//

#ifndef TAS2552_PROJ_PORT_SERVICE_H
#define TAS2552_PROJ_PORT_SERVICE_H

#include "stdint.h"

#define PORT_SERVICE_DEBUGx
#define MAX_RINGBUFFER_SIZE             64

typedef uint8_t port_service_state_t;

typedef enum {
    PORT_CMD_INIT = 0x00,
    PORT_CMD_SYSTEM_OFF = 0xFF,
    PORT_CMD_SYSTEM_ON = 0xFE,
    PORT_CMD_GAIN_SETTING = 0x12,
    PORT_CMD_ACK = 0xFD,
} e_comm_cmd_list_t;

typedef enum {
    PACKET_OK = 0x00,
    PACKET_LENGTH_ERROR = 0x01,
    PACKET_EMPTY = 0x02,
    PACKET_FORMAT_ERROR = 0x03,
    PACKET_INIT = 0xFF,
} e_packet_err_state_t;

typedef struct portData {
    e_comm_cmd_list_t cmd;
    uint8_t data;
} s_port_data_t;

typedef struct port_ring_buffer {
    uint8_t buffer[MAX_RINGBUFFER_SIZE];
    volatile uint32_t head;
    volatile uint32_t tail;
    const uint32_t maxSize;
    volatile uint8_t full;
    e_packet_err_state_t state;

    void (*init)(struct port_ring_buffer *this);

    void (*insert_item)(struct port_ring_buffer *this, uint8_t data);

    s_port_data_t (*parsing_buffer)(struct port_ring_buffer *this);

} s_port_ring_buffer_t;

typedef struct server_context {
    s_port_ring_buffer_t *commBuffer;
    uint8_t* (*make_packet) (s_port_data_t portData);

    // add state, callback, make packet
} s_port_api_t;

void initialize_port_service(void);

s_port_api_t *get_port_service_api(void);

#endif //TAS2552_PROJ_PORT_SERVICE_H
