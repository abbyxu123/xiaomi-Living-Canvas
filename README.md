# 画间 Living Canvas 代码备份

备份时间：2026-09-25（Asia/Shanghai）

## Gemini-S1 / openvela 赛事工程

- 正式参赛仓：`https://github.com/open-vela/contest2026_482_xingguangyinli`
- 目标分支：`dev-ai-contest-2026`
- PR：`https://github.com/open-vela/contest2026_482_xingguangyinli/pull/1`
- 官方合入提交：`79fa55f1b641e5118514a99781803da458bdd28c`
- Git tree：`90a93b0281ee0afbbcfd6ea8ee9a4f240318a351`
- 合入后源码快照：`contest-source/`

`contest-source/` 与官方合入提交具有相同 Git tree，包含 Living Canvas
openvela 应用、Dinner Assistant Skill、独立决策后端、测试、Gemini-S1
适配说明和脱敏证据。未跟踪的构建产物、固件和设备备份未纳入本分支。

## 辅助交互原型历史快照

- 本地工程：`local-setup/living_canvas_esp32s3_preview`
- 分支：`codex/esp32s3-preview`
- 当前提交：`ea94156b153de93e83439ec7fa480205841a3212`
- 当前工作树源码快照：`esp32-working-tree/`

`esp32-working-tree/` 保留 2026-09-21 的辅助交互原型工作树快照，包括：

- `src/living_canvas_voice.cpp`
- `src/living_canvas_voice.h`
- `tests/test_voice.cpp`
- 与语音交互相关的入口、状态机、板级引脚及测试更新

该目录是历史辅助快照，不是 Gemini-S1/openvela 运行证据，也不替代
`contest-source/` 中的正式参赛源码。

## 排除内容

以下内容不属于项目源码，未上传到个人仓库：

- 编译目录、主机测试二进制和 Python 缓存
- 原始媒体素材及生成的 RGB565 大文件
- 本机工具链、虚拟环境、安装包和 Ubuntu 镜像
- 固件镜像、设备 NAND 备份及其他恢复包
- macOS `.DS_Store` 和临时下载目录
- 含旧本机路径或设备标识的 Git 历史包

## 恢复方法

恢复正式参赛源码时使用 `contest-source/`。需要查阅 2026-09-21 的辅助
交互原型时使用 `esp32-working-tree/`。
