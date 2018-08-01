#ifndef  __BEER_MACHINE_H__
#define  __BEER_MACHINE_H__
#include "stdint.h"

#ifdef  __cplusplus
#define BEER_MACHINE_BEGIN  extern "C" {
#define BEER_MACHINE_END    }
#else
#define BEER_MACHINE_BEGIN  
#define BEER_MACHINE_END   
#endif


BEER_MACHINE_BEGIN

#define  BSP_SW_STATUS_PRESS     GPIO_PIN_RESET
#define  BSP_SW_STATUS_RELEASE   GPIO_PIN_SET

#define  BSP_CTRL_COMPRESSOR_ON  GPIO_PIN_SET
#define  BSP_CTRL_COMPRESSOR_OFF GPIO_PIN_RESET

#define  BSP_CTRL_BUZZER_ON      GPIO_PIN_SET
#define  BSP_CTRL_BUZZER_OFF     GPIO_PIN_RESET

#define  BSP_EEPROM_WP_ENABLE    GPIO_PIN_SET
#define  BSP_EEPROM_WP_DISABLE   GPIO_PIN_RESET

#define  BSP_TM1629A_CS_SET      GPIO_PIN_SET
#define  BSP_TM1629A_CS_CLR      GPIO_PIN_RESET

/*外部清除压力按键*/
uint8_t bsp_is_alarm_sw_press();

/*压缩机控制*/
void bsp_compressor_ctrl_on(void);
void bsp_compressor_ctrl_off(void);
/*蜂鸣器控制*/
void bsp_buzzer_ctrl_on(void);
void bsp_buzzer_ctrl_off(void);
/*eeprom wp控制*/
void bsp_eeprom_wp_ctrl_enable(void);
void bsp_eeprom_wp_ctrl_disable(void);
/*tm1629a cs控制*/
void bsp_tm1629a_cs_ctrl_set(void);
void bsp_tm1629a_cs_ctrl_clr(void);
/*tm1629a interface*/
void bsp_tm1629a_write_byte(uint8_t byte);
uint8_t bsp_tm1629a_read_byte(void);



















BEER_MACHINE_END

#endif