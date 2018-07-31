#include "spi.h"
#include "main.h"
#include "beer_machine.h"

void bsp_board_init(void)
{
  
 
}

/*压缩机控制*/
void bsp_compressor_ctrl_on(void)
{
  HAL_GPIO_WritePin(COMPRESSOR_CTRL_GPIO_Port, COMPRESSOR_CTRL_Pin, BSP_CTRL_COMPRESSOR_ON); 
}
void bsp_compressor_ctrl_off(void)
{
  HAL_GPIO_WritePin(COMPRESSOR_CTRL_GPIO_Port, COMPRESSOR_CTRL_Pin, BSP_CTRL_COMPRESSOR_OFF); 
}

/*蜂鸣器控制*/
void bsp_buzzer_ctrl_on(void)
{
  HAL_GPIO_WritePin(BUZZER_CTRL_GPIO_Port, BUZZER_CTRL_Pin, BSP_CTRL_BUZZER_ON); 
}
void bsp_buzzer_ctrl_off(void)
{
  HAL_GPIO_WritePin(BUZZER_CTRL_GPIO_Port, BUZZER_CTRL_Pin, BSP_CTRL_BUZZER_OFF); 
}

/*eeprom wp控制*/
void bsp_eeprom_wp_ctrl_enable(void)
{
  HAL_GPIO_WritePin(EEPROM_WP_CTRL_GPIO_Port, EEPROM_WP_CTRL_Pin, BSP_EEPROM_WP_ENABLE); 
}
void bsp_eeprom_wp_ctrl_disable(void)
{
  HAL_GPIO_WritePin(EEPROM_WP_CTRL_GPIO_Port, EEPROM_WP_CTRL_Pin, BSP_EEPROM_WP_DISABLE); 
}

/*tm1629a cs控制*/
void bsp_tm1629a_cs_ctrl_set(void)
{
  HAL_GPIO_WritePin(TM1629A_CS_GPIO_Port, TM1629A_CS_Pin, BSP_TM1629A_CS_SET); 
}
void bsp_tm1629a_cs_ctrl_clr(void)
{
  HAL_GPIO_WritePin(TM1629A_CS_GPIO_Port, TM1629A_CS_Pin, BSP_TM1629A_CS_CLR); 
}

/*tm1629a interface*/
void bsp_tm1629a_write_byte(uint8_t byte)
{
 HAL_SPI_Transmit(&hspi2,&byte,1,0xff); 
}
uint8_t bsp_tm1629a_read_byte(void)
{
return 0;
}





