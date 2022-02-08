//
// Created by hwan on 2022-01-27.
//

#include "tas2552.h"
#include "i2c.h"
#include "string.h"

#define TAS2552_I2C_ADDRESS                                 0x80

static const struct tas2552_reg_def tas2552_default_reg = {
        {TAS2552_CFG_1,           0x22},
        {TAS2552_CFG_3,           0x80},
        {TAS2552_DOUT,            0x00},
        {TAS2552_OUTPUT_DATA,     0xc0},
        {TAS2552_PDM_CFG,         0x01},
        {TAS2552_PGA_GAIN,        0x00},
        {TAS2552_BOOST_APT_CTRL,  0x0f},
        {TAS2552_RESERVED_0D,     0xbe},
        {TAS2552_LIMIT_RATE_HYS,  0x08},
        {TAS2552_CFG_2,           0xef},
        {TAS2552_SER_CTRL_1,      0x00},
        {TAS2552_SER_CTRL_2,      0x00},
        {TAS2552_PLL_CTRL_1,      0x10},
        {TAS2552_PLL_CTRL_2,      0x00},
        {TAS2552_PLL_CTRL_3,      0x00},
        {TAS2552_BTIP,            0x8f},
        {TAS2552_BTS_CTRL,        0x80},
        {TAS2552_LIMIT_RELEASE,   0x04},
        {TAS2552_LIMIT_INT_COUNT, 0x00},
        {TAS2552_EDGE_RATE_CTRL,  0x40},
        {TAS2552_VBAT_DATA,       0x00},
};

// initialize I2S function map.
static const s_tas2552_reg_t tas2552_i2s_init_seq[] = {
        {TAS2552_CFG_1,          0x12},                 //w 80 01 12
        {TAS2552_PLL_CTRL_1,     0x20},                 //w 80 08 20
        {TAS2552_CFG_2,          0xea},                 //w 80 02 EA
        {TAS2552_CFG_3,          0x5d},                 //w 80 03 5D
        {TAS2552_DOUT,           0x00},                 //w 80 04 00
        {TAS2552_SER_CTRL_1,     0x00},                 //w 80 05 00
        {TAS2552_SER_CTRL_2,     0x00},                 //w 80 06 00
        {TAS2552_OUTPUT_DATA,    0xc8},                 //w 80 07 C8
        {TAS2552_PLL_CTRL_2,     0x00},                 //w 80 09 00
        {TAS2552_PLL_CTRL_3,     0x00},                 //w 80 0A 00
        {TAS2552_PGA_GAIN,       0x16},                 //w 80 12 16
        {TAS2552_BOOST_APT_CTRL, 0x0f},                 //w 80 14 0F
        {TAS2552_CFG_1,          0x10}                  //w 80 01 10
};

// initialize analog function map.
static const s_tas2552_reg_t tas2552_analog_init_seq[] = {
        {TAS2552_CFG_1,          0x12},                 //w 80 01 12
        {TAS2552_PLL_CTRL_1,     0x20},                 //w 80 08 20
        {TAS2552_CFG_2,          0xea},                 //w 80 02 EA
        {TAS2552_CFG_3,          0xdd},                 //w 80 03 5D
        {TAS2552_DOUT,           0x00},                 //w 80 04 00
        {TAS2552_SER_CTRL_1,     0x00},                 //w 80 05 00
        {TAS2552_SER_CTRL_2,     0x00},                 //w 80 06 00
        {TAS2552_OUTPUT_DATA,    0xc8},                 //w 80 07 C8
        {TAS2552_PLL_CTRL_2,     0x00},                 //w 80 09 00
        {TAS2552_PLL_CTRL_3,     0x00},                 //w 80 0A 00
        {TAS2552_PGA_GAIN,       0x1b},                 //w 80 12 16
        {TAS2552_BOOST_APT_CTRL, 0x0f},                 //w 80 14 0F
        {TAS2552_CFG_1,          0x10}                  //w 80 01 10
};

static tas2552_reg_map_t tas2552_reg;
static const uint8_t prepare_receive_cmd[] = {0xFF, 0x00};
static uint8_t tas2552_reg_info[20] = {0x00,};

static void convert_reg_map(uint8_t *receivedData);

void init_tas2552_device(void) {
    // TODO: initialize
    memcpy(&tas2552_reg, &tas2552_default_reg, sizeof(tas2552_default_reg));
}

tas2552_reg_map_t get_tas2552_default_reg_map(void) {
    return tas2552_default_reg;
}

const s_tas2552_reg_t *get_tas2552_i2s_mode_seq(void) {
    return tas2552_i2s_init_seq;
}

const s_tas2552_reg_t *get_tas2552_analog_mode_seq(void) {
    return tas2552_analog_init_seq;
}

e_tas2552_state tas2552_post_data(s_tas2552_reg_t regMap) {
    // TODO: tas2552 register update
    e_tas2552_state state = TAS2552_FAIL;
    uint8_t msg[] = {regMap.reg_addr, regMap.reg_data};

    state = (e_tas2552_state) HAL_I2C_Master_Transmit(&hi2c1,
                                                      (uint16_t) TAS2552_I2C_ADDRESS,
                                                      (uint8_t *) msg,
                                                      sizeof(msg),
                                                      10);

//    while (HAL_I2C_GetError(&hi2c1) != HAL_I2C_STATE_READY);

    return state;
}

tas2552_reg_map_t tas2552_get_all_reg_info(void) {
    // TODO: tas2552 get register info
    if (HAL_I2C_Master_Transmit(&hi2c1,
                                (uint16_t) TAS2552_I2C_ADDRESS,
                                (uint8_t *) prepare_receive_cmd,
                                sizeof(prepare_receive_cmd),
                                1) != HAL_OK) {
        Error_Handler();
    }

    if (HAL_I2C_Master_Receive(&hi2c1,
                               (uint16_t) TAS2552_I2C_ADDRESS,
                               tas2552_reg_info,
                               sizeof(tas2552_reg_info),
                               10) !=
        HAL_OK) {
        Error_Handler();
    }

    convert_reg_map(tas2552_reg_info);

    return tas2552_reg;
}

static void convert_reg_map(uint8_t *receivedData) {
    tas2552_reg.tas2552_cfg_1.reg_data = receivedData[TAS2552_CFG_1];
    tas2552_reg.tas2552_cfg_2.reg_data = receivedData[TAS2552_CFG_2];
    tas2552_reg.tas2552_cfg_3.reg_data = receivedData[TAS2552_CFG_3];
    tas2552_reg.tas2552_dout.reg_data = receivedData[TAS2552_DOUT];
    tas2552_reg.tas2552_ser_ctrl_1.reg_data = receivedData[TAS2552_SER_CTRL_1];
    tas2552_reg.tas2552_ser_ctrl_2.reg_data = receivedData[TAS2552_SER_CTRL_2];
    tas2552_reg.tas2552_output_data.reg_data = receivedData[TAS2552_OUTPUT_DATA];
    tas2552_reg.tas2552_pll_ctrl_1.reg_data = receivedData[TAS2552_PLL_CTRL_1];
    tas2552_reg.tas2552_pll_ctrl_2.reg_data = receivedData[TAS2552_PLL_CTRL_2];
    tas2552_reg.tas2552_pll_ctrl_3.reg_data = receivedData[TAS2552_PLL_CTRL_3];
    tas2552_reg.tas2552_btip.reg_data = receivedData[TAS2552_BTIP];
    tas2552_reg.tas2552_bts_ctrl.reg_data = receivedData[TAS2552_BTS_CTRL];
    tas2552_reg.tas2552_reserved_0d.reg_data = receivedData[TAS2552_RESERVED_0D];
    tas2552_reg.tas2552_limit_rate_hys.reg_data = receivedData[TAS2552_LIMIT_RATE_HYS];
    tas2552_reg.tas2552_limit_release.reg_data = receivedData[TAS2552_LIMIT_RELEASE];
    tas2552_reg.tas2552_limit_int_count.reg_data = receivedData[TAS2552_LIMIT_INT_COUNT];
    tas2552_reg.tas2552_pdm_cfg.reg_data = receivedData[TAS2552_PDM_CFG];
    tas2552_reg.tas2552_pga_gain.reg_data = receivedData[TAS2552_PGA_GAIN];
    tas2552_reg.tas2552_edge_rate_ctrl.reg_data = receivedData[TAS2552_EDGE_RATE_CTRL];
    tas2552_reg.tas2552_boost_apt_ctrl.reg_data = receivedData[TAS2552_BOOST_APT_CTRL];
    tas2552_reg.tas2552_vbat_data.reg_data = receivedData[TAS2552_VBAT_DATA];
}

