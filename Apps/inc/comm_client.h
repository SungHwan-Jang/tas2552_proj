//
// Created by hwan on 2022-01-24.
//

#ifndef TAS2552_PROJ_COMM_CLIENT_H
#define TAS2552_PROJ_COMM_CLIENT_H

#include "stdint.h"
#include "stdio.h"
#include "port_service.h"
#include "app_service.h"

typedef struct client_context {
    port_data_t packet;
} client_context_t;

void init_comm_client_app(void);

#endif //TAS2552_PROJ_COMM_CLIENT_H
