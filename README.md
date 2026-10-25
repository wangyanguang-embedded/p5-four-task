# P5_Four_task — STM32F103 + FreeRTOS 四任务数据采集系统

> 用 FreeRTOS 把「采集 → 滤波 → 通信 → 显示」拆成四个独立任务，
> 任务之间只通过队列传递数据，任何一环变慢都不会阻塞其他任务。


## 1. 项目简介

这个项目的重点不是"点亮外设"，而是验证一套**任务解耦**的嵌入式软件结构：

- 采集任务只负责按周期读传感器，不关心数据怎么处理
- 滤波任务只做数据处理，不直接碰硬件
- 通信任务只负责对外输出
- 显示任务只刷新 OLED，优先级最低，可以被抢占

四个任务通过 3 个队列连接，采样周期和刷新周期相互独立。

| 项 | 值 |
|---|---|
| MCU | **STM32F103C8T6**（Cortex-M3，72 MHz） |
| RTOS | FreeRTOS（CMSIS-RTOS v2 封装） |
| HAL | STM32Cube FW_F1 V1.8.7 |
| 构建 | CMake + Ninja + arm-none-eabi-gcc |
| 配置 | STM32CubeMX（`P5_Four_task.ioc`） |

## 2. 硬件与外设

| 外设 | 用途 | 接口 | 引脚 |
|---|---|---|---|
| OLED | 显示实时数据 | I2C1 | SCL=PB6，SDA=PB7 |
| MPU6050 | 三轴加速度 | I2C1 | SCL=PB6，SDA=PB7（地址 0x68） |
| INA219 | 电流/电压检测 | I2C1 | SCL=PB6，SDA=PB7（地址 0x40） |
| DS18B20 | 温度 | 1-Wire | PB0 |
| USART1 | 调试/上报 | UART | TX=PA9，RX=PA10 |
| ADC1 | 模拟量采集 | ADC | PA0（通道 0） |
| SWD | 下载调试 | SWD | PA13=SWDIO，PA14=SWCLK |

> 三个 I2C 设备共用一条总线（PB6/PB7），通过从机地址区分。

## 3. 任务设计

| 任务 | 优先级 | 栈大小 | 职责 | 数据来源 | 输出去向 |
|---|---|---|---|---|---|
| `sensortask` | High | 1024 B | 周期采集 MPU6050 / INA219 / DS18B20 | 硬件 | `sensorqueue` |
| `filtertask` | Normal | 1024 B | 滤波、单位换算、组包 | `sensorqueue` | `displayqueue`、`commuqueue` |
| `commutask` | BelowNormal | 1024 B | 串口上报 | `commuqueue` | USART1 |
| `displaytask` | Low | 2048 B | 刷新 OLED | `displayqueue` | OLED |

**数据流：**

```
传感器 ──► sensortask ──sensorqueue──► filtertask ──┬─displayqueue─► displaytask ──► OLED
                                                   └─commuqueue───► commutask ───► USART1
```

**设计要点：**

1. **采集优先级最高**：保证采样周期稳定，不被刷屏打断。
2. **显示优先级最低**：OLED 刷新慢，允许被抢占。
3. **显示栈最大（2 KB）**：OLED 刷新函数调用层级较深，栈需求更高。
4. **队列解耦**：某一环阻塞时数据在队列里排队，不至于丢帧或死锁。

## 4. 目录结构

```
P5_Four_task/
├── Core/
│   ├── app/
│   │   ├── drivers/        # 业务驱动：oled / ds18b20 / mpu6050 / ina219 / font
│   │   ├── tasks/          # 四个任务实现
│   │   └── types/          # 公共数据类型
│   ├── Inc/                # CubeMX 生成的配置头文件
│   └── Src/                # 初始化代码 + freertos.c
├── Drivers/                # HAL + CMSIS（由 CubeMX 生成）
├── Middlewares/            # FreeRTOS 源码
├── cmake/                  # 交叉编译工具链配置
├── CMakeLists.txt
├── CMakePresets.json
├── P5_Four_task.ioc        # CubeMX 工程
├── STM32F103XX_FLASH.ld
└── startup_stm32f103xb.s
```

## 5. 编译与烧录

### CLion（推荐）

1. 用 CLion 打开本目录，等待 CMake 自动配置
2. 选择 `Debug` 预设
3. 配置 OpenOCD / ST-Link 后直接下载

### 命令行

```bash
cmake --preset Debug
cmake --build build/Debug
```

产物：`build/Debug/P5_Four_task.elf`



## 6. 遇到的问题与解决

### 1. DS18B20 时序不稳：DWT 微秒延时 + 临界区保护

**现象**：单总线读数偶发失败，或读到的值明显异常。

**原因**：DS18B20 对时序要求极严，微秒级的电平变化必须连续完成。
一开始用 `osDelay` / `HAL_Delay` 做延时，但精度只有 1 ms，
根本满足不了 1-Wire 的微秒级要求；同时 FreeRTOS 的调度和中断
随时可能抢占 CPU，把时序打断。

**尝试一：只用临界区**

用 `taskENTER_CRITICAL()` 把读取过程包起来，时序确实稳了，
但临界区关中断时间过长，影响其他任务的实时响应，不是好方案。

**尝试二：只用 DWT 延时**

改用 DWT 周期计数器做微秒级延时，精度到 CPU 周期：

```c
static void delay_us(uint32_t us)
{
    uint32_t start  = DWT->CYCCNT;
    uint32_t cycles = us * (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - start) < cycles);
}
```

但延时准了不代表不被打断，中断仍会插在电平翻转中间。

**最终方案：DWT + 临界区结合**

用 DWT 保证每个延时精确，只在**最短的位操作区间**加临界区，
既保证时序连续，又不会长时间关中断。

初始化时使能计数器：

```c
DWT->CYCCNT = 0;
DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
```

### 2. OLED 刚上电显示正常，随后花屏

**现象**：开机瞬间显示正常，运行很短时间后出现乱码、花屏。

**原因**：排查后定位到 `displaytask` 的栈空间不足。
OLED 刷新函数调用层级较深，加上局部缓冲区占用较大，
栈溢出后破坏了相邻内存和函数返回地址，表现为显示异常。

**解决**：把显示任务栈从 1 KB 提升到 2 KB（`stack_size = 512 * 4`），
问题消失。栈不是越大越好，但调用链深的任务要留足余量。


## 7. 后续计划

- [ ] 增加参数掉电保存（Flash 模拟 EEPROM）
- [ ] 用 DMA + 空闲中断重写串口接收
- [ ] 加一个 Modbus RTU 从机，支持上位机读取
- [ ] 补 PC 端单元测试（滤波算法）

## 8. 作者

王艳广 · 嵌入式方向 · 2027 届
GitHub：[@wangyanguang-embedded](https://github.com/wangyanguang-embedded)
