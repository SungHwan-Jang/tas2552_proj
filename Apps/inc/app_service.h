//
// Created by hwan on 2022-02-08.
//

#ifndef TAS2552_PROJ_APP_SERVICE_H
#define TAS2552_PROJ_APP_SERVICE_H

#include "port_service.h"

// inner state
typedef enum{
    APP_ERR,
    APP_DONE,
}e_app_service_state_t;

typedef struct service_context{
    e_comm_cmd_list_t cmdList;
    e_app_service_state_t state;
}app_service_context_t;

void init_service_app(void);

#endif //TAS2552_PROJ_APP_SERVICE_H
