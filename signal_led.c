/*
 * @File:   signal_led.c
 * @Author: WKJay
 * @Data:   2019/03/14 20:54
 * 
 * @e-mail: 1931048074@qq.com
 * 
 */

#include "signal_led.h"


static led *first_handle = NULL;
static void led_get_blinkArr(led *handle);


/*@brief   初始化信号灯对象
 *@param   handle:     信号灯句柄
 *@param   pin_index:  信号灯引脚号
 *@param   switch_on:  开灯函数（用户自定义）
 *@param   switch_off: 关灯函数（用户自定义）
 */
void led_init(led *handle, rt_base_t pin_index, void (*switch_on)(void), void (*switch_off)(void))
{
    switch_off();
    rt_pin_mode(pin_index, PIN_MODE_OUTPUT);
    memset(handle, 0, sizeof(led));

    handle->switch_on = switch_on;
    handle->switch_off = switch_off;
}

/*@brief    设置信号灯的工作模式
 *@param    loop:         循环次数 (0xff 永久循环)
 *@param    blinkMode:    一个周期的闪烁方式(字符串形式,如："200,200,200,2000,"表示亮200ms,灭200ms,亮200ms,灭2000ms)
 *@note     blinkMode字符串必须以英文逗号分隔，且最后以英文逗号结尾
 */
void led_set_mode(led *handle, uint8_t loop, char *blinkMode)
{
    handle->loop = loop;
    handle->blinkMode = blinkMode;
    handle->handleCnt = 0;
    handle->blinkPoint = 0;
    handle->modePointer = 0;
    led_get_blinkArr(handle);
}


/*@brief    获得信号灯一个周期内闪烁时间的数组（仅内部调用，用户无需关注）
 *@param    handle:    信号灯句柄
 * */
static void led_get_blinkArr(led *handle)
{
    char *blinkModeTemp = NULL;
    uint8_t blinkCnt = 0;
    uint8_t blinkCntNum = 0;

    if (handle->blinkArr)
    {
        free(handle->blinkArr);
    }
    
    //获取数组长度
    for (blinkModeTemp = handle->blinkMode; *blinkModeTemp != '\0'; blinkModeTemp++)
    {
        if (*blinkModeTemp == ',')
        {
            handle->modeCnt++;
        }
    }
    if (*(blinkModeTemp-1) != ',')
        handle->modeCnt++;

    //创建数组
    handle->blinkArr = malloc(sizeof(uint16_t) * handle->modeCnt);

    //存储数据
    for (blinkModeTemp = handle->blinkMode; *blinkModeTemp != '\0'; blinkModeTemp++)
    {
        handle->blinkArr[blinkCntNum] = atol(blinkModeTemp);
        //计算出计数变量的值（根据信号灯定时器定时时间）
        handle->blinkArr[blinkCntNum] /= LED_TICK_TIME;
        blinkCntNum++;
        while(*blinkModeTemp != ',')
        {
            if(*blinkModeTemp == '\0')
            {
                blinkModeTemp--;
                break;
            }
            blinkModeTemp++;
        }
    }
}

/*@brief    信号灯状态翻转
 *@param    handle:    信号灯句柄
 * */
void led_switch(led *led_handle)
{
    if (led_handle->modePointer%2)
    {
        led_handle->switch_off();
        led_handle->state = LED_OFF;
    }
    else
    {
        led_handle->switch_on();
        led_handle->state = LED_ON;
    }
}

/*@brief    信号灯运作函数
 *@param    crt_handle:    当前信号灯句柄
 * */
void led_handle(led *crt_handle)
{
    if (crt_handle->loopTemp)
    {
        crt_handle->handleCnt++;
__repeat:
        if ((crt_handle->handleCnt - 1) == crt_handle->blinkPoint)
        {
            if (crt_handle->modePointer < crt_handle->modeCnt)
            {
                crt_handle->blinkPoint += crt_handle->blinkArr[crt_handle->modePointer];
                if(crt_handle->blinkPoint == 0)	//时间为0的直接跳过
                {
                    crt_handle->modePointer++;
                    goto __repeat;
                }
                else led_switch(crt_handle);
                crt_handle->modePointer++;
            }
            else
            {
                crt_handle->modePointer = 0;
                crt_handle->blinkPoint = 0;
                crt_handle->handleCnt = 0;
                if (crt_handle->loopTemp != LOOP_PERMANENT)
                    crt_handle->loopTemp--;
            }
        }
    }
    else
        led_stop(crt_handle);
}

/*@brief    信号灯开启（若没有调用次函数开启信号灯则信号灯不会工作）
 *@param    led_handle:     要开启的信号灯句柄
 *@return   0：             正常
 *          HANDLE_EXIST：  句柄冲突
 * */
uint8_t led_start(led *led_handle)
{
    led_handle->loopTemp = led_handle->loop; //启动时将其重置

    led *temp_handle = first_handle;
    while (temp_handle)
    {
        if (temp_handle == led_handle)
            return HANDLE_EXIST;         //已存在，则退出
        temp_handle = temp_handle->next; //设置链表当前指针
    }

    led_handle->next = first_handle;
    first_handle = led_handle;

    return 0;
}

/*@brief    信号灯关闭（关闭信号灯后仍可调用开启函数开启）
 *@param    led_handle:    要关闭的信号灯句柄
 * */
void led_stop(struct led *led_handle)
{
    struct led **handle_pointer;

    led_handle->switch_off(); //关闭灯

    for (handle_pointer = &first_handle; *handle_pointer;)
    {
        led *handle_destory = *handle_pointer;

        if (handle_destory == led_handle)
        {
            *handle_pointer = handle_destory->next;
        }
        else
        {
            handle_pointer = &handle_destory->next;
        }
    }
}

/*@brief    信号灯心跳函数
 *@note     必须循环调用该函数，否则信号灯将不会工作。
 *          可以将其放入线程或定时器中，保证每隔LED_TICK_TIME毫秒调用即可
 *          LED_TICK_TIME是在"signal_led.h"中定义的宏，信号灯的工作基于该宏
 *          保证该宏所定义的时间为信号灯心跳函数调用的周期！
 * */
void led_ticks(void)
{
    led *current_handle;

    for (current_handle = first_handle; current_handle != NULL; current_handle = current_handle->next)
    {
        led_handle(current_handle);
    }
}
