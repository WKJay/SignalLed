#include "signal_led.h"
#include "stdlib.h"
#include "math.h"
#include "drv_gpio.h"

static led *first_handle = NULL;
static void led_get_blinkArr(led *handle);

//初始化信号灯对象
//handle:信号灯句柄
//pin_index:信号灯引脚号
//switch_on():开灯函数
//switch_off():关灯函数
void led_init(led *handle, rt_base_t pin_index, void (*switch_on)(void), void (*switch_off)(void))
{
    rt_pin_mode(pin_index, PIN_MODE_OUTPUT);
    memset(handle, 0, sizeof(led));

    handle->switch_on = switch_on;
    handle->switch_off = switch_off;
}

//设置信号灯的工作模式
//loop:         循环次数 (0xff 永久循环)
//blinkMode:    闪烁方式
void led_set_mode(led *handle, uint8_t loop, char *blinkMode)
{
    handle->loop = loop;
    handle->blinkMode = blinkMode;
    led_get_blinkArr(handle);
}

//获得信号灯一个周期内的闪烁时间数组
static void led_get_blinkArr(led *handle)
{
    char *blinkModeTemp = NULL;
    uint8_t blinkCnt = 0;
    uint8_t blinkCntNum = 0;

    //获取数组长度
    for (blinkModeTemp = handle->blinkMode; *blinkModeTemp != '\0'; blinkModeTemp++)
    {
        if (*blinkModeTemp == ',')
        {
            if (*(blinkModeTemp + 1) != '\0')
                handle->modeCnt++;
        }
    }
    if (handle->modeCnt)
        handle->modeCnt++;

    //创建数组
    handle->blinkArr = malloc(sizeof(uint16_t) * handle->modeCnt);

    //存储数据
    for (blinkModeTemp = handle->blinkMode; *blinkModeTemp != '\0'; blinkModeTemp++)
    {
        if (*blinkModeTemp == ',')
        {
            for (; blinkCnt > 1; blinkCnt--)
            {
                handle->blinkArr[blinkCntNum] +=
                    (*(blinkModeTemp - blinkCnt) - '0') * pow(10, blinkCnt - 1);
            }

            //计算出计数变量的值（根据信号灯定时器定时时间）
            handle->blinkArr[blinkCntNum] /= LED_TICK_TIME;

            blinkCntNum++;
        }
        else
        {
            blinkCnt++;
        }
    }
}

//信号灯状态翻转
void led_switch(led *led_handle)
{
    if (led_handle->state)
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

//信号灯工作函数
void led_handle(led *crt_handle)
{
    if (crt_handle->loopTemp)
    {
        crt_handle->handleCnt++;
        if ((crt_handle->handleCnt - 1) == crt_handle->blinkPoint)
        {
            if (crt_handle->modePointer < crt_handle->modeCnt)
            {
                led_switch(crt_handle);
                crt_handle->blinkPoint += crt_handle->blinkArr[crt_handle->modePointer];
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
}

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

//信号灯定时器，LED_TICK_TIME ms 调用一次
void led_ticks(void)
{
    led *current_handle;

    for (current_handle = first_handle; current_handle != NULL; current_handle = current_handle->next)
    {
        led_handle(current_handle);
    }
}
