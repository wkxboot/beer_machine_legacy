#ifndef  __LED_H__
#define  __LED_H__

#include "stdint.h"


#ifdef  __cplusplus
#define LED_BEGIN  extern "C" {
#define LED_END    }
#else
#define LED_BEGIN  
#define LED_END   
#endif


LED_BEGIN


#define  LED_DISPLAY_ON              1
#define  LED_DISPLAY_OFF             0

#define  LED_BRIGHTNESS_DEFAULT      7
#define  LED_NULL_VALUE              0xFF


/*显示初始化*/
void led_display_init();
/*显示刷新到芯片*/
void led_display_refresh();
/*显示灰度*/
void led_display_brightness(uint8_t brightness);
/*温度单位*/
void led_display_temperature_unit(uint8_t on_off);
/*温度图标*/
void led_display_temperature_icon(uint8_t on_off);
/*温度数值*/
void led_display_temperature(int16_t t);
/*压力单位*/
void led_display_pressure_unit(uint8_t on_off);
/*压力图标*/
void led_display_pressure_icon(uint8_t on_off);
/*压力小数点*/
void led_display_pressure_point(uint8_t on_off);
/*压力数值*/
void led_display_pressure(uint8_t p);
/*容积图标框架*/
void led_display_capacity_icon_frame(uint8_t on_off);
/*容积图标1-5*/
void led_display_capacity_icon_level(uint8_t level);
/*容积单位*/
void led_display_capacity_unit(uint8_t on_off);
/*容积数值*/
void led_display_capacity(uint8_t c);
/*WIFI图标*/
void led_display_wifi_icon(uint8_t on_off);
/*循环图标*/
void led_display_circle_icon(uint8_t on_off_up,uint8_t on_off_dwn);
/*商标图标*/
void led_display_brand_icon(uint8_t on_off);




LED_END


#endif
