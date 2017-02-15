#ifndef ARDUINOPC_DEFS_H
#define ARDUINOPC_DEFS_H

/*
 * Un/comment the below line to enable and disable
 * the CAN bus, using the SEEEDStudio CAN Shield
 * https://github.com/Seeed-Studio/CAN_BUS_Shield
 */
#define __HAVE_CAN_BUS__

/*
 * Un/comment the below line to enable and disable
 * the LIN bus, using the MCP2004A LIN transceiver,
 * using a fork of the LIN master implementation originally
 * written by gandrewstone, located on his Github at
 * https://github.com/gandrewstone/LIN
 */
//#define __HAVE_LIN_BUS__

#endif //ARDUINOPC_DEFS_H