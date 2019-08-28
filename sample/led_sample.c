#include <rtthread.h>
#include <rtdevice.h>
#include "signal_led.h"

/* defined the LED pin */
#define LED0_PIN    GET_PIN(A, 8)


static rt_timer_t led_timer;

//定义信号灯对象
led led0;


/*  设置信号灯一个周期内的闪烁模式
 *  格式为 “亮、灭、亮、灭、亮、灭 …………” 长度不限
 *  注意：  该配置单位为毫秒，且必须大于 “LED_TICK_TIME” 宏，且为整数倍（不为整数倍则向下取整处理）
 *          必须以英文逗号为间隔，且以英文逗号结尾，字符串内只允许有数字及逗号，不得有其他字符出现
 */
char *ledBlinkMode0 = "500,500,"; //1Hz闪烁
char *ledBlinkMode1 = "50,50,";   //10Hz闪烁
char *ledBlinkMode2 = "0,100,";   //常灭
char *ledBlinkMode3 = "100,0,";   //常亮


//定义开灯函数
void led0_switch_on(void)
{
    rt_pin_write(LED0_PIN, PIN_HIGH);
}


//定义关灯函数
void led0_switch_off(void)
{
    rt_pin_write(LED0_PIN, PIN_LOW);
}

//中途切换模式测试
void led_switch (void *param)
{
    while(1)
    {
        rt_thread_mdelay(5000);
        led_set_mode(&led0, LOOP_PERMANENT, ledBlinkMode0);
        rt_thread_mdelay(5000);
        led_set_mode(&led0, LOOP_PERMANENT, ledBlinkMode1);
    }
    
}

static void led_timeout(void *parameter)
{
    led_ticks();
}

int rt_led_timer_init(void)
{
    //初始化信号灯对象
    led_init(&led0, LED0_PIN, led0_switch_on, led0_switch_off);
  
    //设置信号灯工作模式，循环十次
    led_set_mode(&led0, LOOP_PERMANENT, ledBlinkMode0);
    
    //开启信号灯
    led_start(&led0);
    
    /* 创建定时器1  周期定时器 */    
#ifdef RT_USING_TIMER_SOFT
    led_timer = rt_timer_create("led_timer", led_timeout,
                                RT_NULL, LED_TICK_TIME,
                                RT_TIMER_FLAG_PERIODIC|RT_TIMER_FLAG_SOFT_TIMER);
#else
    led_timer = rt_timer_create("led_timer", led_timeout,
                                RT_NULL, LED_TICK_TIME,
                                RT_TIMER_FLAG_PERIODIC);
#endif	
    /* 启动定时器1 */
    if (led_timer != RT_NULL)
        rt_timer_start(led_timer);
    
    /* start software timer thread */    
    rt_thread_t tid = RT_NULL;
    /* 创建线程1 */
    tid = rt_thread_create("led_switch_test",
                            led_switch, 
                            RT_NULL,
                            512,
                            RT_THREAD_PRIORITY_MAX/2,
                            100);
    if (tid != RT_NULL)
        rt_thread_startup(tid);
    return RT_EOK;
}
INIT_APP_EXPORT(rt_led_timer_init);

