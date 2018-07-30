#include "beer_machine.h"
#include "tm1629a.h"
#include "led.h"
#include "log.h"
#define  LOG_MODULE_LEVEL    LOG_LEVEL_DEBUG
#define  LOG_MODULE_NAME     "[led]"


tm1629a_hal_driver_t hal_driver={
.write_byte =bsp_tm1629a_write_byte,
.read_byte = bsp_tm1629a_read_byte,
.stb_set = bsp_tm1629a_cs_ctrl_set,
.stb_clr = bsp_tm1629a_cs_ctrl_clr
};

#define  LED_ABCDEFG_BITS          (0xfe)


#define  LED_P_HI_POS              15
#define  LED_P_LO_POS              14
#define  LED_T_HI_POS              13
#define  LED_T_LO_POS              12
#define  LED_C_HI_POS              11
#define  LED_C_LO_POS              10


#define  LED_P_DP_POS              15
#define  LED_P_DP_BITS             (1<<0)

#define  LED_P_UNIT_L_POS          14
#define  LED_P_UNIT_L_BITS         (1<<0)

#define  LED_P_UNIT_R_POS          13
#define  LED_P_UNIT_R_BITS         (1<<0)

#define  LED_T_UNIT_POS            12
#define  LED_T_UNIT_BITS           (1<<0)



#define  LED_P_ICON_POS             9
#define  LED_P_ICON_BITS           (1<<7)|(1<<6)|(1<<5)|(1<<4)          

#define  LED_T_ICON_POS             9
#define  LED_T_ICON_BITS           (1<<3)|(1<<2)|(1<<1)|(1<<0)    

#define  LED_RESERVED_POS           8
#define  LED_RESERVED_BITS          (1<<7)

#define  LED_WIFI_ICON_POS          8
#define  LED_WIFI_ICON_BITS         (1<<6)

#define  LED_CIRCLE_ICON_POS        8
#define  LED_CIRCLE_ICON_BITS       (1<<5)|(1<<4)


#define  LED_BRAND_ICON_POS         8
#define  LED_BRAND_ICON_BITS        (1<<3)|(1<<2)|(1<<1)

#define  LED_C_UNIT_POS             7
#define  LED_C_UNIT_BITS            (1<<7)|(1<<6)

#define  LED_C_ICON_LEVEL_3_5_POS   7

#define  LED_C_ICON_LEVEL_5_BITS    (1<<5)|(1<<4)
#define  LED_C_ICON_LEVEL_4_BITS    (1<<3)|(1<<2)
#define  LED_C_ICON_LEVEL_3_BITS    (1<<1)|(1<<0)


#define  LED_C_ICON_LEVEL_1_2_POS   6

#define  LED_C_ICON_LEVEL_2_BITS    (1<<7)|(1<<6)
#define  LED_C_ICON_LEVEL_1_BITS    (1<<5)|(1<<4)


#define  LED_C_ICON_FRAME_1_POS      6
#define  LED_C_ICON_FRAME_1_BITS    (1<<3)|(1<<2)|(1<<1)|(1<<0)

#define  LED_C_ICON_FRAME_2_POS      5
#define  LED_C_ICON_FRAME_2_BITS    (0xff)




#define  TEMPERATURE_DATA_POS       1
#define  TEMPERATURE_DATA_CNT       2


static uint8_t const hex_code[]=
{
/*定制的共阳数码管在特定主控板上的编码*/
/*0    1     2   3    4    5    6   7 */ 
0xfc,0x60,0xda,0xf2,0x66,0xb6,0xbe,0xe0,
/*8    9     A   b    C    d    E   F */
0xfe,0xf6,0xee,0x3e,0x9c,0x7a,0x9e,0x8e
};

static uint8_t const negative_code=0x02;
static uint8_t const point_code=0x01;

#define  LED_DISPLAY_ON              1
#define  LED_DISPLAY_OFF             0

#define  LED_BRIGHTNESS_DEFAULT      7


/*显示初始化*/
void led_display_init()
{
 uint8_t dis;
 int result;
 result =tm1629a_register_hal_driver(&hal_driver);
 log_assert(result == 0);
 result =tm1629a_init();
 log_assert(result == 0);
 tm1629a_buffer_clean();
 
 led_display_brightness(LED_BRIGHTNESS_DEFAULT);
 
 led_display_temperature_unit(LED_DISPLAY_ON);
 led_display_pressure_unit(LED_DISPLAY_ON);
 led_display_capacity_unit(LED_DISPLAY_ON);
 
 led_display_temperature_icon(LED_DISPLAY_ON);
 led_display_pressure_icon(LED_DISPLAY_ON);
 led_display_capacity_icon(LED_DISPLAY_ON);
 
 led_display_pressure_point(LED_DISPLAY_ON);
 led_display_capacity_icon_frame(LED_DISPLAY_ON);
 /*依次显示 8-7....0.*/
 for(dis=8;dis > 0;dis--){
 led_display_temperature(dis);
 led_display_pressure(dis);
 led_display_capacity(dis);
 }
 
 log_debug("led display init done.\r\n");
 
}
/*显示刷新到芯片*/
void led_display_refresh()
{
 tm1629a_display_refresh(); 
}
/*显示灰度*/
void led_display_brightness(uint8_t brightness)
{
 tm1629a_brightness(brightness); 
}
/*温度单位*/
void led_display_temperature_unit(uint8_t on_off)
{
  if(on_off > 0 ){
  tm1629a_buffer_update(LED_T_UNIT_POS,LED_T_UNIT_BITS,LED_T_UNIT_BITS); 
 }else{
  tm1629a_buffer_update(LED_T_UNIT_POS,0,LED_T_UNIT_BITS); 
 }
}

/*温度图标*/
void led_display_temperature_icon(uint8_t on_off)
{
  if(on_off > 0 ){
  tm1629a_buffer_update(LED_T_ICON_POS,LED_T_ICON_BITS,LED_T_ICON_BITS); 
 }else{
  tm1629a_buffer_update(LED_T_ICON_POS,0,LED_T_ICON_BITS); 
 }
}

/*温度数值*/
void led_display_temperature(int8_t t)
{
uint8_t dis[2];
if(t < 0){
 dis[0]=negative_code;
 t*=-1;
 dis[1]=hex_code[t];
 
}else{
dis[0]=hex_code[t/10];
dis[1]=hex_code[t%10];
}

tm1629a_buffer_update(LED_T_HI_POS,dis[0],LED_ABCDEFG_BITS);
tm1629a_buffer_update(LED_T_LO_POS,dis[1],LED_ABCDEFG_BITS);
}

/*压力单位*/
void led_display_pressure_unit(uint8_t on_off)
{
  if(on_off > 0 ){
  tm1629a_buffer_update(LED_P_UNIT_L_POS,LED_P_UNIT_L_BITS,LED_P_UNIT_L_BITS); 
  tm1629a_buffer_update(LED_P_UNIT_R_POS,LED_P_UNIT_R_BITS,LED_P_UNIT_R_BITS); 
 }else{
  tm1629a_buffer_update(LED_P_UNIT_L_POS,0,LED_P_UNIT_L_BITS); 
  tm1629a_buffer_update(LED_P_UNIT_R_POS,0,LED_P_UNIT_R_BITS); 
 }
}
/*压力图标*/
void led_display_pressure_icon(uint8_t on_off)
{
  if(on_off > 0 ){
  tm1629a_buffer_update(LED_P_ICON_POS,LED_P_ICON_BITS,LED_P_ICON_BITS); 
 }else{
  tm1629a_buffer_update(LED_P_ICON_POS,0,LED_P_ICON_BITS); 
 }
}

/*压力小数点*/
void led_display_pressure_point(uint8_t on_off)
{
 if(on_off > 0 ){
  tm1629a_buffer_update(LED_P_DP_POS,point_code,LED_P_DP_BITS); 
 }else{
  tm1629a_buffer_update(LED_P_DP_POS,0,LED_P_DP_BITS); 
 }
}


/*压力数值*/
void led_display_pressure(uint8_t p)
{
uint8_t dis[2]; 

dis[0]=hex_code[p/10];
dis[1]=hex_code[p%10];
tm1629a_buffer_update(LED_P_HI_POS,dis[0],LED_ABCDEFG_BITS);
tm1629a_buffer_update(LED_P_LO_POS,dis[1],LED_ABCDEFG_BITS);

}

/*容积图标框架*/
void led_display_capacity_icon_frame(uint8_t on_off)
{
 if(on_off > 0 ){
  tm1629a_buffer_update(LED_P_DP_POS,point_code,LED_P_DP_BITS); 
 }else{
  tm1629a_buffer_update(LED_P_DP_POS,0,LED_P_DP_BITS); 
 }  
  
}
/*容积图标1-5*/
void led_display_capacity_icon(uint8_t on_off)
{
 uint8_t bits12,bits35;

 switch(on_off)
 {
 case 0:
   bits12=0;
   bits35=0;
   break;
 case 1:
   bits12=LED_C_ICON_LEVEL_1_BITS;
   bits35=0;
   break;
 case 2:
   bits12=LED_C_ICON_LEVEL_2_BITS|LED_C_ICON_LEVEL_1_BITS;
   bits35=0;
   break;
 case 3:
   bits12=LED_C_ICON_LEVEL_2_BITS|LED_C_ICON_LEVEL_1_BITS;
   bits35=LED_C_ICON_LEVEL_3_BITS;
 case 4:
   bits12=LED_C_ICON_LEVEL_2_BITS|LED_C_ICON_LEVEL_1_BITS;
   bits35=LED_C_ICON_LEVEL_4_BITS|LED_C_ICON_LEVEL_3_BITS;
   break;
 case 5:
   bits12=LED_C_ICON_LEVEL_3_BITS;
   bits35=LED_C_ICON_LEVEL_5_BITS|LED_C_ICON_LEVEL_4_BITS|LED_C_ICON_LEVEL_3_BITS;;
 default:
   break;
 }
  tm1629a_buffer_update(LED_C_ICON_LEVEL_1_2_POS,bits12,LED_C_ICON_LEVEL_1_BITS|LED_C_ICON_LEVEL_2_BITS); 
  tm1629a_buffer_update(LED_C_ICON_LEVEL_3_5_POS,bits35,LED_C_ICON_LEVEL_3_BITS|LED_C_ICON_LEVEL_4_BITS|LED_C_ICON_LEVEL_5_BITS); 
}
/*容积单位*/
void led_display_capacity_unit(uint8_t on_off)
{
 if(on_off > 0 ){
  tm1629a_buffer_update(LED_C_ICON_FRAME_1_POS,LED_C_ICON_FRAME_1_BITS,LED_C_ICON_FRAME_1_BITS); 
  tm1629a_buffer_update(LED_C_ICON_FRAME_2_POS,LED_C_ICON_FRAME_2_BITS,LED_C_ICON_FRAME_2_BITS); 
 }else{
  tm1629a_buffer_update(LED_C_ICON_FRAME_1_POS,0,LED_C_ICON_FRAME_1_BITS); 
  tm1629a_buffer_update(LED_C_ICON_FRAME_2_POS,0,LED_C_ICON_FRAME_2_BITS); 
 }
}

/*容积数值*/
void led_display_capacity(uint8_t c)
{
uint8_t dis[2]; 

dis[0]=hex_code[c/10];
dis[1]=hex_code[c%10];
tm1629a_buffer_update(LED_C_HI_POS,dis[0],LED_ABCDEFG_BITS);
tm1629a_buffer_update(LED_C_LO_POS,dis[1],LED_ABCDEFG_BITS);
}





