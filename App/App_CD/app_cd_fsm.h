#ifndef APP_CD_FSM_H
#define APP_CD_FSM_H

#include "app_types.h"
#include "com_err.h"
#include "com_type.h"

typedef void (*CdStateAction)(void);

err_t app_cd_fsm_init(void);
void app_cd_fsm_dispatch(MatrixEvent event);
void app_cd_fsm_set_repeat_event(MatrixEvent event);
MatrixEvent app_cd_fsm_get_repeat_event(void);
MatrixEvent app_cd_fsm_get_disc_done_event(void);
SysState app_cd_fsm_get_state(void);
uint16_t app_cd_fsm_get_music(void);

#endif /* APP_CD_FSM_H */
