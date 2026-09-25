# Gemini-S1 / openvela 适配状态

最后核验：2026-09-25（Asia/Shanghai）

## 平台定位

Gemini-S1（Allwinner R528S3）是 Living Canvas 的正式目标主控，目标软件栈为 openvela、LVGL 与 ai_agent。仓库中的应用入口、manifest 映射、Agent 安全桥接、自定义 Skill 和目标构建均围绕这条路线维护。

本页把“源码与构建已经完成”和“实体板端运行已经验证”分开记录。构建成功不等同于已经刷入设备；辅助交互原型也不等同于 openvela 板端运行证据。

## 已验证事项

| 层级 | 结果 | 可复核证据 |
| --- | --- | --- |
| 应用逻辑 | 12 个严格 C 主机测试通过 | `app/hello_app/tests/host/` |
| openvela 映射 | 应用映射到 `packages/demos/contest2026_482_hello_app/` | `contest2026_482_xingguangyinli.xml`、`app/hello_app/` |
| 目标构建 | Gemini-S1 产品配置构建退出状态为 0，`nuttx.elf` 含 `living_canvas_main` | `tests/evidence/build/gemini-s1-product-20260918.txt` |
| 镜像封装 | 目标应用分区和 128 MB NAND LiveSuit 整包生成并校验 | `tests/evidence/build/gemini-s1-product-20260918.txt` |
| 设备基线 | USB/ADB 身份和板载麦克风非内容信号已验证 | `tests/evidence/device/` |
| 恢复通道 | FEL 身份、R528/T113 芯片 ID、Winbond 256 MiB SPI NAND 已识别 | `tests/evidence/device/gemini-s1-fel-spinand-20260920.txt` |
| 写入前保护 | 完整 SPI NAND 只读备份已生成并校验 | `tests/evidence/device/gemini-s1-fel-spinand-20260920.txt` |

## 当前适配边界

首次分区写入流程在 FES DRAM 初始化阶段超时，流程在进入存储、MBR 和分区写入阶段之前自动停止。停止后 FEL 芯片身份仍可读取，未发生持久化 NAND 写入。

因此，当前可以准确陈述：

- Living Canvas 的 Gemini-S1/openvela 目标源码、主机逻辑、目标构建和镜像封装已经完成并留下证据；
- Gemini-S1 的 USB/ADB、麦克风、FEL 和 SPI NAND 基线已经实板验证；
- Gemini-S1 的首次持久化写入、启动以及 LVGL、音频、网络、ai_agent 的板端端到端链路仍在适配验证中；
- 当前不把尚未完成的板端运行描述为已完成，也不把其他主控的演示结果替代为 openvela 运行结果。

这类首次刷写问题位于板级恢复、DRAM 初始化和存储写入链路，处理方式是保留原始存储备份、限制首次写入范围、核对硬件版本和恢复路径，并在每个阶段生成可回溯证据。

## 代码完成度与运行门禁

应用侧已经具备：

- 存在事件触发的主动问候和冷却控制；
- 晚餐约束收集、最多三个候选和信息不足时追问；
- 模型输出到本地动作之间的白名单与二次校验；
- 用户明确确认后的手机交接、灯光意图和可删除偏好记忆；
- 网络、超时、取消、迟到回复和时钟未同步的确定性回退；
- LVGL 状态、角色反馈、选择与二维码界面；
- 按 ai_agent 官方 Markdown 格式编写、目标部署到
  `/data/agent/skills/dinner-assistant.md` 的 Dinner Assistant Skill。

板端验收只有在以下门禁全部通过后才会标记完成：

1. 核对板卡版本、FES DRAM 参数、恢复工具和可回滚镜像；
2. 在可恢复条件下完成受控写入并校验分区；
3. 通过串口或等价通道确认 openvela 启动；
4. 分别验证 LVGL 显示、触摸、音频、网络和 ai_agent；
5. 完成从存在事件到确认、执行和手机交接的实体板端演示；
6. 将脱敏结果补入 `tests/evidence/`。

## 辅助交互原型

仓库保留一个 ESP32-S3 辅助交互原型，用来复核竖屏布局、触摸选择和产品流程。它有独立的源码与自动化测试，但不属于 Gemini-S1/openvela 运行证据，不改变正式目标平台和上述验收门禁。
