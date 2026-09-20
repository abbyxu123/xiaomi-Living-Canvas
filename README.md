# 画间 Living Canvas 代码备份

备份时间：2026-09-21（Asia/Shanghai）

## Gemini-S1 / openvela 赛事工程

- 本地工程：`local-setup/contest2026_482_xingguangyinli`
- 分支：`codex/gemini-s1-choice-ui`
- 提交：`17c68b44f365afdef6626f24d410bdf724616ebc`
- 当前提交源码快照：`contest-source/`

源码快照取自当前提交，并移除了本机路径和设备序列号。工作区中未跟踪的主机测试可执行文件属于构建产物，未纳入备份。

## ESP32-S3 演示工程

- 本地工程：`local-setup/living_canvas_esp32s3_preview`
- 分支：`codex/esp32s3-preview`
- 当前提交：`ea94156b153de93e83439ec7fa480205841a3212`
- 当前工作树源码快照：`esp32-working-tree/`

`esp32-working-tree/` 包含提交后的语音适配在研改动，包括：

- `src/living_canvas_voice.cpp`
- `src/living_canvas_voice.h`
- `tests/test_voice.cpp`
- 与语音交互相关的入口、状态机、板级引脚及测试更新

该快照保留了当前可读源码，但没有把未提交改动写入 ESP32 原工程的 Git 历史。

## 排除内容

以下内容不属于项目源码，未上传到个人仓库：

- 编译目录、主机测试二进制和 Python 缓存
- 原始媒体素材及生成的 RGB565 大文件
- 本机工具链、虚拟环境、安装包和 Ubuntu 镜像
- 固件镜像、设备 NAND 备份及其他恢复包
- macOS `.DS_Store` 和临时下载目录
- 含旧本机路径或设备标识的 Git 历史包

## 恢复方法

直接查看或恢复当前源码时，使用 `contest-source/` 与 `esp32-working-tree/`。ESP32 的语音适配在研改动已包含在工作树快照中。
