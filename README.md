# 主动式三步安全联锁控制台

这是一个基于 Arduino Uno R3 的主动交互项目：用户触摸启动后，先按提示推动摇杆并按下摇杆按键，再将装置倾斜到指定状态。每一步正确后自动进入下一步并播放短提示音；全部步骤正确后，8x8 点阵显示对勾并播放最终成功音；方向错误、倾斜错误或超时，点阵显示叉号并播放错误音。LCD1602 用于显示当前步骤和倒计时。

LCD 文案全部使用英文 ASCII。标准 LCD1602A 字符 ROM 通常不支持直接显示中文，使用中文字符串可能出现乱码。

本版本不使用舵机、继电器、风扇、RTC、红外遥控、步进电机、PCF8591、激光模块或外部 Arduino 库。

## 器件

- Arduino Uno R3
- LCD1602A（并口版本）
- 一块带 MAX7219 类串行驱动的 8x8 LED Matrix F3
- TTP223B 电容触摸模块
- 一个摇杆模块
- SW-520D 倾斜传感器模块
- 蜂鸣器模块（S/IN、VCC、GND 三针）
- 面包板和跳线
- 可选：旋转电位器模块，用于调节 LCD 对比度

## 接线

| 模块 | 模块引脚 | Uno 引脚/电源 |
|---|---|---|
| LCD1602 | RS | D2 |
| LCD1602 | E | D3 |
| LCD1602 | D4 | D4 |
| LCD1602 | D5 | D5 |
| LCD1602 | D6 | D6 |
| LCD1602 | D7 | D7 |
| LCD1602 | VSS | GND |
| LCD1602 | VDD | 5V |
| LCD1602 | RW | GND（固定写入模式） |
| LCD1602 | VO | 电位器中间端；电位器两端接 5V/GND |
| LCD1602 | A/LED+ | 5V（若背光板已有电阻） |
| LCD1602 | K/LED- | GND |
| TTP223B | OUT | D8 |
| TTP223B | VCC/GND | 5V/GND |
| 摇杆 | SW | D9 |
| 摇杆 | VRx | A0 |
| 摇杆 | VCC/GND | 5V/GND |
| 摇杆 | VRy | 暂不接 |
| SW-520D | S/DO | D10 |
| SW-520D | VCC/GND | 5V/GND |
| 蜂鸣器模块 | S/IN | A1 |
| 蜂鸣器模块 | VCC/GND | 5V/GND |
| 点阵 F3 | DIN | D11 |
| 点阵 F3 | CS | D12 |
| 点阵 F3 | CLK | D13 |
| 点阵 F3 | VCC/GND | 5V/GND |

点阵左侧的 DOUT 不接。它只有在级联第二块点阵时才使用。点阵右侧的 DIN、CLK、CS 是本项目需要接的输入端。

## 面包板布线

1. Uno 5V 接面包板正电源轨，Uno GND 接负电源轨。
2. 所有模块的 VCC 接正电源轨，GND 接负电源轨。
3. LCD1602 的 RS/E/D4-D7 按表格接到 D2-D7。
4. 点阵接 D11-D13。建议点阵亮度先保持代码中的中等值，避免 USB 供电不足。
5. LCD 无字或全黑时，旋转电位器调节 VO 对比度。
6. 蜂鸣器模块的 `S`、`SIG` 或 `IN` 接 A1；`+`/`VCC` 接 5V；`-`/`GND` 接 GND。

本项目不接电机和继电器，因此 Uno USB 供电通常可以用于测试。如果点阵出现闪烁或 Uno 重启，改用稳定的 5V 外部电源给面包板供电，并把外部电源 GND 与 Uno GND 相连。

## 操作流程

1. 上电后 LCD 显示 `TOUCH TO START`，点阵熄灭。
2. 触摸 TTP223B。
3. 按提示把摇杆推向左侧，再按下摇杆顶部按键。正确后播放短提示音并自动进入下一步。
4. 按提示倾斜装置，保持倾斜约 0.35 秒。正确后点阵显示对勾并播放最终成功音。
5. 任一步骤方向错误、倾斜错误或超时，点阵立即显示叉号并播放错误音，不会进入下一步。
6. 再次触摸 TTP223B 返回待机。

如果 SW-520D 的高低电平与代码相反，把草图中的 `TILT_ACTIVE_LEVEL` 从 `LOW` 改为 `HIGH`。

如果点阵图案方向反了，不需要改线，只需在草图中的 `CHECK_ICON` 和 `CROSS_ICON` 调整行顺序或位顺序。

## 打开与上传

用 Arduino IDE 打开当前文件夹中的 `111.ino`，开发板选择 `Arduino Uno`，选择对应串口后上传。代码只使用 Arduino 自带的 `LiquidCrystal` 库和内置 `shiftOut()`、`tone()`，不需要安装 `LedControl` 等第三方库。

## 文件

- `111.ino`：Arduino Uno 程序
- `docs/接线图-清晰版.svg`：端口映射式清晰接线图（推荐查看）
- `docs/接线图.svg`：早期线路版接线图
- `diagnostics/lcd1602_test/`：LCD1602 独立排障程序
- `tests/test_controller_model.py`：状态机行为测试
- `src/controller_model.py`：与 Arduino 状态机对应的可测试模型
