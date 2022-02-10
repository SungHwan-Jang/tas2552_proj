//
// Created by hwan on 2022-02-08.
//

#include "app_service.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "string.h"

#include "tas2552.h"
#include "rtos_comm_objects.h"
#include "comm_client.h"

extern TaskHandle_t gateKeeperHandle;
static app_service_context_t context;

void serviceTask(void *argument);

static void app_service_process(comm_obj_t *commObj);

void init_service_app(void) {
    xTaskCreate(serviceTask,
                "serviceTask",
                128,
                NULL,
                3,
                &gateKeeperHandle);

    init_comm_obj(TASK_GATE_KEEPER, &gateKeeperHandle);
}

void serviceTask(void *argument) {
    init_tas2552_device();
    comm_obj_t *commObj = (comm_obj_t *) pvPortMalloc(sizeof(comm_obj_t));
    /* Infinite loop */
    for ever {
        ui_queue_wait_event(TASK_GATE_KEEPER, commObj, pdFALSE);
        app_service_process(commObj);

    }
    /* USER CODE END gateKeeperTask */
}

static void app_service_process(comm_obj_t *commObj) {
    static comm_obj_t senderObj;
    client_context_t *clientCtx = (client_context_t *) commObj->data;
    e_tas2552_state state;

    if (commObj->type == DELIVERY_FROM_CLIENT) {
        context.cmdList = clientCtx->packet.cmd;
        context.state = APP_DONE;

        switch (clientCtx->packet.cmd) {
            case PORT_CMD_SYSTEM_ON: {
                const tas2552_reg_t *seq = get_tas2552_i2s_mode_seq();
                for (int i = 0; i < INIT_SETTING_SEQ_NUM; i++) {
                    state = tas2552_post_data(seq[i]);

                    if (state != TAS2552_OK)
                        context.state = APP_ERR;
                }
                break;
            }
            case PORT_CMD_SYSTEM_OFF: {
                const tas2552_reg_t *seq = get_tas2552_analog_mode_seq();
                for (int i = 0; i < INIT_SETTING_SEQ_NUM; i++) {
                    state = tas2552_post_data(seq[i]);

                    if (state != TAS2552_OK)
                        context.state = APP_ERR;
                }
                break;
            }
            case PORT_CMD_GAIN_SETTING: {
                uint8_t gain = clientCtx->packet.data;
                tas2552_reg_t gainCtrRegInfo = {TAS2552_PGA_GAIN, gain};
                state = tas2552_post_data(gainCtrRegInfo);

                if (state != TAS2552_OK)
                    context.state = APP_ERR;
                break;
            }
#if 0
            case PORT_CMD_GAIN_INFO:{
                tas2552_reg_map_t regInfo = tas2552_get_all_reg_info();
                uint8_t gainLevel = regInfo.tas2552_pga_gain.reg_data;
                context.data = gainLevel;
                break;
            }
#endif
            case PORT_CMD_FIND_SYS:{
                break;
            }
            default:
                context.cmdList = PORT_CMD_ERR;
                break;
        }

        senderObj.type = DELIVERY_FROM_GATE;
        senderObj.data = &context;

        ui_queue_send_event(TASK_CLIENT, &senderObj, 0);
    }
    else {

    }
}
