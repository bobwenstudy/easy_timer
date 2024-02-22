# 简介

在嵌入式开发中，经常有计时以及时间比较的需要，如时间是否到达或者时间间隔等，本项目主要解决各种场景下的时间计算。

本项目地址：[bobwenstudy/easy_timer (github.com)](https://github.com/bobwenstudy/easy_timer)

嵌入式环境的时间是回环的，也就是说以32bit为单位，计时器单位为1us的话，就是每经过0x100000000(us)=4,294,967.296(ms)=4,294.967296(s)≈1.193(h)，时间就会回环一次，如下图所示。

![image-20240222114308729](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240222114308729.png)

这样就会有个问题，在边界场景进行时间运算时，会有可能出现计算异常。

在对timer进行计算时，通常需要用到如下几个操作函数past，sub和add函数，一般的实现如下所示。

```c
/**
 * @brief  Check two absolute times past: time1<time2.
 * @param[in]  time1: Absolute time expressed in internal time units.
 * @param[in]  time2: Absolute time expressed in internal time units.
 * @return resulting 1 means past(time1<time2).
 */
int timer_past(uint32_t time1, uint32_t time2)
{
	return time1 < time2;
}
/**
 * @brief  Returns the difference between two absolute times: time1-time2.
 * @param[in]  time1: Absolute time expressed in internal time units.
 * @param[in]  time2: Absolute time expressed in internal time units.
 * @return resulting signed relative time expressed in internal time units.
 */
int32_t timer_sub(uint32_t time1, uint32_t time2)
{
	return time1 - time2;
}

/**
 * @brief This function returns the sum of an absolute time and a signed relative time.
 * @param[in]  time1: Absolute time expressed in internal time units.
 * @param[in]  ticks: Signed relative time expressed in internal time units.
 * @return 32bit resulting absolute time expressed in internal time units.
 */
uint32_t timer_add(uint32_t time1, int32_t ticks)
{
	return time1 + ticks;
}
```

下面分别以如下2种场景进行分析。

## 0xFFFFFFFF的时间回环问题

如下图所示，有`A(0xFFFFFFF0)`，`B(0x10)`，`C(0x20)`三个时间点，从图片上，可以直观的知道（假定时间点间隔不超过总时间一半），时间先后关系是，A最早，B次之，C最晚。

C和B的时间差是0x10，B和A的时间差是0x20。

![image-20240222114610498](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240222114610498.png)



带入上面的函数计算，会发现涉及到`A`的计算，`timer_past`结果都是错的。不过`timer_add`和`timer_sub`的计算是对的，这是因为最大值是`0xFFFFFFFF`，溢出的部分，自动做了处理。

```c
void test_work(void)
{
    SUITE_START("test_work");

    uint32_t A = 0xFFFFFFF0;
    uint32_t B = 0x10;
    uint32_t C = 0x20;

    int res;
    int32_t diff;
    uint32_t tmp;

    // timer past test
    res = timer_past(A, B); // ERROR, Get res=0, Expect res=1;
    ASSERT(res == 1);
    res = timer_past(B, C); // SUCCESS, Get res=1, Expect res=1;
    ASSERT(res == 1);

    // timer add test
    tmp = timer_add(A, 0x20); // SUCCESS, Get tmp=0x10, Expect tmp=0x10;
    ASSERT(tmp == 0x10);
    tmp = timer_add(B, 0x10); // SUCCESS, Get tmp=0x20, Expect tmp=0x20;
    ASSERT(tmp == 0x20);
    tmp = timer_add(C, -0x10); // SUCCESS, Get tmp=0x10, Expect tmp=0x10;
    ASSERT(tmp == 0x10);

    // timer sub test
    diff = timer_sub(B, A); // SUCCESS, Get diff=0x20, Expect res=0x20;
    ASSERT(diff == 0x20);
    diff = timer_sub(C, B); // SUCCESS, Get diff=0x10, Expect res=0x10;
    ASSERT(diff == 0x10);

    SUITE_END();
}
```



## 非0xFFFFFFFF的时间回环问题

在某些场景，计数值并不能以0xFFFFFFFF为最大值（如蓝牙时钟以28bit为周期等），这时候时间计算问题会更复杂。

依然是上面的例子，最大值为0x00FFFFFF。如下图所示，有`A(0x00FFFFF0)`，`B(0x10)`，`C(0x20)`三个时间点，从图片上，可以直观的知道（假定时间点间隔不超过总时间一半），时间先后关系是，A最早，B次之，C最晚。

C和B的时间差是0x10，B和A的时间差是0x20。

![image-20240222143537713](https://markdown-1306347444.cos.ap-shanghai.myqcloud.com/img/image-20240222143537713.png)

带入上面的函数计算，会发现涉及到`A`的计算，`timer_past`、`timer_add`和`timer_sub`的计算都是错的，这是因为没人帮忙做溢出处理了。

```c
void test_work_insuff(void)
{
    SUITE_START("test_work_insuff");

    uint32_t A = 0x00FFFFF0;
    uint32_t B = 0x10;
    uint32_t C = 0x20;

    int res;
    int32_t diff;
    uint32_t tmp;

    // timer past test
    res = timer_past(A, B); // ERROR, Get res=0, Expect res=1;
    ASSERT(res == 1);
    res = timer_past(B, C); // SUCCESS, Get res=1, Expect res=1;
    ASSERT(res == 1);

    // timer add test
    tmp = timer_add(A, 0x20); // ERROR, Get tmp=0x01000010, Expect tmp=0x10;
    ASSERT(tmp == 0x10);
    tmp = timer_add(B, 0x10); // SUCCESS, Get tmp=0x20, Expect tmp=0x20;
    ASSERT(tmp == 0x20);
    tmp = timer_add(C, -0x10); // SUCCESS, Get tmp=0x10, Expect tmp=0x10;
    ASSERT(tmp == 0x10);

    // timer sub test
    diff = timer_sub(B, A); // ERROR, Get diff=0xFF000020, Expect res=0x20;
    ASSERT(diff == 0x20);
    diff = timer_sub(C, B); // SUCCESS, Get diff=0x10, Expect res=0x10;
    ASSERT(diff == 0x10);

    SUITE_END();
}

```



## 总结

综上，可以看出，涉及时间回环，不能单靠CPU自身处理，还是需要根据具体情况进行不同的处理。




# 代码结构

代码结构如下所示：

- **etimer.h**：EasyTimer管理API，都是inline实现，可以根据需要转成c实现。
- **etimer16.h**：EasyTimer管理16bit处理API，都是inline实现，可以根据需要转成c实现。
- **main.c**：测试例程。
- **build.mk**和**Makefile**：Makefile编译环境。
- **README.md**：说明文档

```shell
easy_timer
 ├── etimer.h
 ├── etimer16.h
 ├── build.mk
 ├── main.c
 ├── Makefile
 └── README.md
```





# 使用说明

具体如何使用直接看例程就行，非常简单，看函数名和变量名即可。

## 0xFFFFFFFF的时间回环问题处理

使用提供etimer接口操作即可。

```c
void test_work_etimer(void)
{
    SUITE_START("test_work_etimer");

    uint32_t A = 0xFFFFFFF0;
    uint32_t B = 0x10;
    uint32_t C = 0x20;

    int res;
    int32_t diff;
    uint32_t tmp;

    // timer past test
    res = etimer_past(A, B); // SUCCESS, Get res=1, Expect res=1;
    ASSERT(res == 1);
    res = etimer_past(B, C); // SUCCESS, Get res=1, Expect res=1;
    ASSERT(res == 1);

    // timer add test
    tmp = etimer_add(A, 0x20); // SUCCESS, Get tmp=0x10, Expect tmp=0x10;
    ASSERT(tmp == 0x10);
    tmp = etimer_add(B, 0x10); // SUCCESS, Get tmp=0x20, Expect tmp=0x20;
    ASSERT(tmp == 0x20);
    tmp = etimer_add(C, -0x10); // SUCCESS, Get tmp=0x10, Expect tmp=0x10;
    ASSERT(tmp == 0x10);

    // timer sub test
    diff = etimer_sub(B, A); // SUCCESS, Get diff=0x20, Expect res=0x20;
    ASSERT(diff == 0x20);
    diff = etimer_sub(C, B); // SUCCESS, Get diff=0x10, Expect res=0x10;
    ASSERT(diff == 0x10);

    SUITE_END();
}
```



## 非0xFFFFFFFF的时间回环问题处理

使用提供etimer的raw接口，并配置好max_value和overflow值即可。

注意，overflow值一般取max_value的一半。

```c
void test_work_etimer_insuff(void)
{
    SUITE_START("test_work_etimer_insuff");

    uint32_t A = 0x00FFFFF0;
    uint32_t B = 0x10;
    uint32_t C = 0x20;

    uint32_t max_value = 0x00FFFFFF;
    uint32_t overflow = max_value / 2;

    int res;
    int32_t diff;
    uint32_t tmp;

    // timer past test
    res = etimer_past_raw(A, B, overflow); // SUCCESS, Get res=1, Expect res=1;
    ASSERT(res == 1);
    res = etimer_past_raw(B, C, overflow); // SUCCESS, Get res=1, Expect res=1;
    ASSERT(res == 1);

    // timer add test
    tmp = etimer_add_raw(A, 0x20, max_value); // SUCCESS, Get tmp=0x10, Expect tmp=0x10;
    ASSERT(tmp == 0x10);
    tmp = etimer_add_raw(B, 0x10, max_value); // SUCCESS, Get tmp=0x20, Expect tmp=0x20;
    ASSERT(tmp == 0x20);
    tmp = etimer_add_raw(C, -0x10, max_value); // SUCCESS, Get tmp=0x10, Expect tmp=0x10;
    ASSERT(tmp == 0x10);

    // timer sub test
    diff = etimer_sub_raw(B, A, overflow, max_value); // SUCCESS, Get diff=0x20, Expect res=0x20;
    ASSERT(diff == 0x20);
    diff = etimer_sub_raw(C, B, overflow, max_value); // SUCCESS, Get diff=0x10, Expect res=0x10;
    ASSERT(diff == 0x10);

    SUITE_END();
}
```



## API说明

主要有以下API。可以看到每个api都提供了一个带`_raw`的接口，用于处理非`0xFFFFFFFF`的场景。

```c
static inline int etimer_past_raw(uint32_t time1, uint32_t time2, uint32_t overflow);
static inline int etimer_past(uint32_t time1, uint32_t time2);
static inline uint32_t etimer_add_raw(uint32_t time1, int32_t ticks, uint32_t max_value);
static inline uint32_t etimer_add(uint32_t time1, int32_t ticks);
static inline int32_t etimer_sub_raw(uint32_t time1, uint32_t time2, uint32_t overflow,
                                     uint32_t max_value);
static inline int32_t etimer_sub(uint32_t time1, uint32_t time2);
```



## API说明16bit

部分场景下，嵌入式只需要16bit的计数器，这时用raw来运算有点浪费性能，所以提供了16bit的操作API，详见`etimer16.h`。

带`_raw`的接口，用于处理非`0xFFFF`的场景。

```c
static inline int etimer16_past_raw(uint16_t time1, uint16_t time2, uint16_t overflow);
static inline int etimer16_past(uint16_t time1, uint16_t time2);
static inline uint16_t etimer16_add_raw(uint16_t time1, int16_t ticks, uint16_t max_value);
static inline uint16_t etimer16_add(uint16_t time1, int16_t ticks);
static inline int16_t etimer16_sub_raw(uint16_t time1, uint16_t time2, uint16_t overflow,
                                       uint16_t max_value);
static inline int16_t etimer16_sub(uint16_t time1, uint16_t time2);
```







# 测试说明

## 环境搭建

目前测试暂时只支持Windows编译，最终生成exe，可以直接在PC上跑。

目前需要安装如下环境：
- GCC环境，笔者用的msys64+mingw，用于编译生成exe，参考这个文章安装即可。[Win7下msys64安装mingw工具链 - Milton - 博客园 (cnblogs.com)](https://www.cnblogs.com/milton/p/11808091.html)。


## 编译说明

本项目都是由makefile组织编译的，编译整个项目只需要执行`make all`即可。


也就是可以通过如下指令来编译工程：

```shell
make all
```

而后运行执行`make run`即可运行例程，例程中实现了上述文档说明的问题和API的基本测试。

```shell
PS D:\workspace\github\easy_timer> make run
Building   : "output/main.exe"
Start Build Image.
objcopy -v -O binary output/main.exe output/main.bin
copy from `output/main.exe' [pei-i386] to `output/main.bin' [binary]
objdump --source --all-headers --demangle --line-numbers --wide output/main.exe > output/main.lst
Print Size
   text    data     bss     dec     hex filename
  57888    2888    2644   63420    f7bc output/main.exe
./output/main.exe
failed assert [main.c:810] res == 1
Testing test_work .......................................................... fail
failed assert [main.c:845] res == 1
failed assert [main.c:851] tmp == 0x10
failed assert [main.c:859] diff == 0x20
Testing test_work_insuff ................................................... fail
Testing test_work_etimer ................................................... pass
Testing test_work_etimer_insuff ............................................ pass
Testing test_etimer_past ................................................... pass
Testing test_etimer_sub .................................................... pass
Testing test_etimer_add .................................................... pass
Testing test_etimer_raw_past ............................................... pass
Testing test_etimer_raw_sub ................................................ pass
Testing test_etimer_raw_add ................................................ pass
Testing test_etimer16_past ................................................. pass
Testing test_etimer16_sub .................................................. pass
Testing test_etimer16_add .................................................. pass
Testing test_etimer16_raw_past ............................................. pass
Testing test_etimer16_raw_sub .............................................. pass
Testing test_etimer16_raw_add .............................................. pass
Executing 'run: all' complete!
```

可以看到，所有涉及到etimer的测试都通过。



