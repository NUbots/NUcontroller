/*
 *  opencr_op3
 *
 *
 *  Created on: 2016. 10. 21.
 *      Author: Baram
 *
 *  Edited on: 2022. 02. 16.
 *      Author: Dexter Konijn
 */
#include "./src/protocol/dxl_node_op3.h"


extern void dxl_hw_tx_enable(void);

void setup() {
    dxl_node_op3_init();
}

void loop() {
    dxl_node_op3_loop();
}
