#ifndef __LED_H_
#define __LED_H_

#include "stdint.h"
#include "string.h"
#include "rtthread.h"

#define LED_TICK_TIME 50        //led_handle调用的时间间隔（单位:ms）
#define LOOP_PERMANENT 0XFF

#define HANDLE_EXIST 1

#define LED_OFF     0
#define LED_ON      1


typedef struct led {
    uint16_t    tickCnt;
    uint16_t    blinkPoint;     //闪烁节点
    uint8_t     activeState;    //信号灯亮时的引脚状态 (PIN_HIGH/PIN_LOW)
    uint8_t     state;          //信号灯当前的状态
    uint8_t     modePointer;    //用于指明当前参照闪烁数组中的第几个成员
    uint8_t     modeCnt;        //一个周期内的闪烁次数
    uint8_t     handleCnt;      //handle函数计数
    uint8_t     loop;           //周期
    uint8_t     loopTemp;       //存储周期的临时变量，当重启led时会被重置
    char*       blinkMode;      //亮、灭、亮、灭、亮、灭………………（注意时间之间以“,”隔开，最后必须以“,”结尾）
    uint16_t*   blinkArr;
    void        (*switch_on)(void);
    void        (*switch_off)(void);
    struct led* next;
}led;

void led_init(led *handle, rt_base_t pin_index, void (*switch_on)(void), void (*switch_off)(void));
void led_set_mode(led* handle,uint8_t loop,char* blinkMode);
void led_ticks (void);
uint8_t led_start(led* led_handle);
void led_stop(struct led *led_handle);
#endif
