# Gemini-S1 厂商/赛事技术支持确认清单

更新时间：2026-09-14

用途：在首次刷写 Gemini-S1 前，向润芯微或赛事官方一次性确认恢复与兼容信息。不要在公开 Issue、聊天截图或附件中发送 Wi-Fi 密码、模型 Token、Ubuntu 密码、SSH 私钥、订单地址或其他个人信息。

## 当前实物事实

- 板卡：Gemini-S1 / Allwinner R528S3
- USB：`18d1:4e11`，Manufacturer `NuttX`，Product `Debug Bridge`
- ADB 序列号：`1234`
- 出厂属性：Manufacturer `Xiaomi`，Product `ASX4B`，版本 `1.51.32`，活动 OTA 槽 `A`
- 只读检查确认 NAND 设备节点，`/data` 为 YAFFS，`/etc` 与 `/resource` 为 ROMFS
- 当前出厂系统可启动；尚未执行刷写、擦除、分区、OTA 或恢复模式操作
- 计划使用赛事分支 `dev-ai-contest-2026` 和 `nsh_minidisplay` 配置
- AI Agent + Living Canvas 产品镜像已构建并记录 SHA-256，尚未写入设备

## 请官方逐项确认

1. 这批比赛板的准确硬件版本、板号、NAND 型号和容量是什么？出厂属性 `ASX4B / 1.51.32` 与公开目标 `r528s3-gemini-s1` 的对应关系是什么？从板上丝印或只读命令如何进一步核对？
2. 与该硬件版本完全匹配的出厂恢复镜像从哪里下载？请提供版本、发布日期、文件大小和官方 SHA-256。
3. 官方支持的首次刷写工具是什么？请提供工具名称、准确版本、支持的主机系统、官方下载地址和校验值。
4. 进入 FEL/烧录/恢复模式的准确按键、USB 端口、上电顺序和退出步骤是什么？失败或断电后如何恢复？
5. 是否必须使用串口观察启动与恢复？若需要，请提供 UART 引脚、电平（必须明确是否 3.3 V）、波特率、接线图和推荐 USB-TTL 型号。
6. `vendor/allwinnertech/boards/r528/r528s3-gemini-s1/configs/nsh_minidisplay/` 生成的镜像是否可直接用于本批实物？是否还需要闭源 bootloader、分区表、字体或板卡校准文件？
7. Gemini-S1 的公开 `generate_ota_package.sh` 引用了 `r528s3/evb4_nand`。这是赛事板的预期兼容路径，还是脚本尚未切换到 `gemini-s1_nand`？
8. 对已经带出厂 openvela 的板，官方推荐先走应用部署/ADB、OTA，还是整包刷写？哪种方式保留校准、MAC、序列号和出厂数据？
9. 首次自编译固件前，是否有官方方法只读备份 bootloader、分区表、校准区和用户数据？请给出不会触发擦除的命令与恢复验证方法。
10. 赛事要求的 `gemini-s1_defconfig` 与本批出厂系统的 openvela 版本是否匹配？是否有指定的已验证 commit/manifest 或补丁集合？

## 可接受的确认形式

- 官方文档链接、厂商下载页或由官方人员明确回复；
- 对每个镜像/工具记录版本、来源 URL、文件大小和 SHA-256；
- 若回复只给截图或网盘，仍需确认文件适用的硬件版本与校验值；
- 在信息完整前，维持 `BLOCKED_FOR_FLASH`，先完成主机逻辑、构建和非破坏性设备验证。
