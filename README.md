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

# 4、API简介
本软件包给用户提供的API接口有：

###	初始化信号灯对象
```C
void led_init(led *handle, rt_base_t pin_index, void (*switch_on)(void), void (*switch_off)(void));
```

|参数|描述|
|----|----|
|handle|信号灯句柄|
|pin_index|信号灯引脚号|
|switch_on|开灯函数（用户自定义）|
|switch_off|关灯函数（用户自定义）|

该函数用于初始化一个信号灯对象，其中的引脚号为RT-Thread 提供的引脚编号。有2种方式可以获取引脚编号：使用宏定义或者查看PIN 驱动文件。具体获取方法请参照[RT-Thread文档中心->设备和驱动->PIN设备->访问PIN设备->获取引脚编号](https://www.rt-thread.org/document/site/programming-manual/device/pin/pin/)

开关灯函数由用户自定义，格式参照如下：

```C
void switch_on (void) //待传入的函数
{
    rt_pin_write(LED0_PIN, PIN_LOW);//实现具体操作的函数
}
```

### 设置信号灯工作模式

```C
void led_set_mode(led* handle,uint8_t loop,char* blinkMode);
```

|参数|描述|
|----|----|
|handle|信号灯句柄|
|loop|循环的次数|
|blinkMode|一个周期内的闪烁方式|

该函数定义了信号灯的闪烁模式以及按照固定的模式循环多少次，若要设置为永久循环则loop的参数为 `LOOP_PERMANENT` 。该宏在`signal_led.h`中定义，其数值为oxff。

###### loop 接受的参数
|loop值|描述|
|----|----|
|0-255|循环0-225次|
|LOOP_PERMANENT|永久循环|

###### blinkMode

"blinkMode" 参数为用户设置的信号灯闪烁方式，以字符串类型进行设置，用户仅需输入信号灯一个循环周期内对应的亮灭时间数即可，单位为毫秒。
例如：我想设置一个亮300ms，灭200ms,亮300ms,灭200ms,亮300ms,灭1000ms，并以这个方式循环工作下去的信号灯，那我的blinkMode设置如下：

```C
char *binkMode = "300,200,300,200,300,1000,"
```

注意：时间参数必须以英文逗号间隔开，并且整个字符串必须以英文逗号结尾！

###   !部署信号灯主心跳

本信号灯软件的工作机制完全基于一个核心处理函数，即信号灯心跳函数：

```C
void led_ticks (void);
```

使用该软件包定义的所有信号灯对象都由上述心跳函数统一管理，无需用户关注。用户需要将该函数部署在一个按固定时基触发的定时器内，在 `signal_led.h` 文件中定义有心跳时基宏 `LED_TICK_TIME`，单位为毫秒，用户可以直接使用该宏配置定时器的时基，也可自行写入定时器时基并修改该宏。但需要注意的时，必须保持该宏和调用心跳函数的时基保持相等，否则信号灯将无法正常工作！

同时，心跳函数的时基决定了信号灯的最快闪烁频率，若时基为50ms，则信号灯闪烁频率最高为 1000ms/50ms = 20Hz，且信号灯在一个状态保持的时间也必须为时基的整数倍，时基为50ms，则可以设置的保持时间为 50ms、100ms、150ms…………不是整数倍的内核自动向下取整。

最后需注意，信号灯主心跳函数必须部署，否则信号灯将无法工作。


### 开启信号灯

当对信号灯的设置都完成之后，我们就可以通过下面的函数开启信号灯：

```C
uint8_t led_start(led* led_handle);
```

|参数|描述|
|----|----|
|handle|信号灯句柄|

开启信号灯后信号灯将按照用户的配置循环工作，直至循环次数用完。当循环完成后或用户手动关闭后，用户仍可以通过该函数重启信号灯。

### 关闭信号灯

信号灯开启后，用户可根据自己的需求关闭信号灯：

```C
void led_stop(struct led *led_handle);
```

|参数|描述|
|----|----|
|handle|信号灯句柄|

关闭信号灯后信号灯配置不会消失，用户仍可调用开启函数重启信号灯。


# 5、使用案例

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

# 6、联系方式&感谢


- 作者：WKJay
- 主页：https://github.com/WKJay/SignalLed
- email: 1931048074@qq.com
- 如有意见或者建议请与作者取得联系，该说明文档会不定期更新。
