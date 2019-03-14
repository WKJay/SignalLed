# 信号灯LED

# 1、介绍

在我们的日常开发中，led虽小，但却起着不可忽视的作用。我们总是用小灯的闪烁来表示一个事件的发生或停止。单个小灯的闪烁直接调用设置高低电平的函数会非常方便，然而，当你有多个led闪烁事件的时候，一个个的循环，一个个的delay可能就让你非常烦躁了。<br>
这个时候，快来试试这款简单的小工具吧，抛开所有的循环和延时，解放你的双手，只需几个设置，便可轻松实现各种不同的信号闪烁！
# 2、获取方式

使用SignalLed软件包需要在RT-Thread的包管理中选中它，具体路径如下：

```
RT-Thread online packages
    peripheral libraries and drivers  --->
        [*] SignalLed:a signal led package for rt-thread
```

# 3、使用方式

1. 首先定义信号灯引脚

```C
#define LED0_PIN GET_PIN(E, 7)
```

2. 声明一个信号灯对象

```C
led led_0;
```

3. 定义信号灯的开关函数

```C
//定义开灯函数
void led0_switch_on(void)
{
    rt_pin_write(LED0_PIN, PIN_LOW);
}
//定义关灯函数
void led0_switch_off(void)
{
    rt_pin_write(LED0_PIN, PIN_HIGH);
}
```

4. 设置闪烁模式（字符串形式，以亮、灭、亮、灭……的顺序，依次写入持续时间，单位：ms）

```C
char *led0BlinkMode = "200,200,200,200,200,1000,";
```

注意：时间参数必须以英文逗号间隔开，并且整个字符串必须以英文逗号结尾！
 
5. 初始化信号灯并绑定信号灯开关函数

```C
led_init(&led0, LED0_PIN, led0_switch_on, led0_switch_off);
```

6. 设置信号灯工作模式（循环10次，闪烁方式为 `led0BlinkMode` 中的设定）

```C
led_set_mode(&led0, 10, led0BlinkMode);
```

7. 开启信号灯

```C
led_start(&led0);
```

8. 创建一个信号灯线程循环调用信号灯心跳函数

```C
//每隔 LED_TICK_TIME（50） 毫秒循环调用心跳函数
while (1)
{
    led_ticks();
    rt_thread_mdelay(LED_TICK_TIME);
}
```

注意：LED_TICK_TIME 为 signal_led.h 中的宏，默认值为50，单位为ms。信号灯内核依赖该宏，不可删除，该宏作为信号灯闪烁计时的时基，因此信号灯处于亮或灭状态的时间一定大于该宏所定义的时间，即若该宏定义为50，则信号灯最快50毫秒变化一次状态，用户可根据自己工程的需求改变该值得大小。
# 4、使用案例

```C
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "drv_gpio.h"
#include "signal_led.h"

//定义信号灯引脚
#define LED0_PIN GET_PIN(E, 7)

//定义信号灯对象
led led0;

/*  设置信号灯一个周期内的闪烁模式
 *  格式为 “亮、灭、亮、灭、亮、灭 …………” 长度不限
 *  注意：  该配置单位为毫秒，且必须大于 “LED_TICK_TIME” 宏，且为整数倍（不为整数倍则向下取整处理）
 *          必须以英文逗号为间隔，且以英文逗号结尾，字符串内只允许有数字及逗号，不得有其他字符出现
 */
char *led0BlinkMode = "200,200,200,200,200,1000,";

//定义开灯函数
void led0_switch_on(void)
{
    rt_pin_write(LED0_PIN, PIN_LOW);
}
//定义关灯函数
void led0_switch_off(void)
{
    rt_pin_write(LED0_PIN, PIN_HIGH);
}

int main(void)
{   
    //初始化信号灯对象
    led_init(&led0, LED0_PIN, led0_switch_on, led0_switch_off);
    //设置信号灯工作模式，循环十次
    led_set_mode(&led0, 10, led0BlinkMode);
    //开启信号灯
    led_start(&led0);
    
    //每隔 LED_TICK_TIME（50） 毫秒循环调用心跳函数
    while (1)
    {
        led_ticks();
        rt_thread_mdelay(LED_TICK_TIME);
    }

    return RT_EOK;
}


```

# 5、联系方式&感谢


- 作者：WKJay
- 主页：https://github.com/WKJay/SignalLed
- email: 1931048074@qq.com
- 如有意见或者建议请与作者取得联系，该说明文档会不定期更新。
