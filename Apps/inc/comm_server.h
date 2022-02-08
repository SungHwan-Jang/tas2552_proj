//
// Created by hwan on 2022-01-24.
//

#ifndef TAS2552_PROJ_COMM_SERVER_H
#define TAS2552_PROJ_COMM_SERVER_H

#include "port_service.h"

typedef struct server_context{
    port_data_t packet;
}server_context_t;

void init_comm_server_app(void);

#endif //TAS2552_PROJ_COMM_SERVER_C_H
