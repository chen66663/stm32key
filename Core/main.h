#ifndef MAIN_H
#define MAIN_H

#include "cmsis_os.h"

extern osMessageQId g_cdMsgQueue;
extern osMessageQId g_oledMsgQueue;
extern osMessageQId g_powerMsgQueue;
extern osMailQId g_cdOledMailQueue;

#endif /* MAIN_H */
