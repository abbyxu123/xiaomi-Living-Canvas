# Living Canvas ESP32-S3 Preview

这是小米 openvela 大赛 Living Canvas 的独立 ESP32-S3 竖屏保底版。它不替代、也不修改 Gemini-S1/openvela 工程。

- 目标板：Waveshare ESP32-S3-Touch-AMOLED-1.8 V2
- 屏幕：CO5300，368×448 原生竖屏
- 触摸：CST820（CST816 兼容协议）
- Flash / PSRAM：16 MB / 8 MB OPI
- 模式：离线触摸演示；主页循环播放由 `circle.mp4` 转换的帧序列
- 菜单：正确的 `sitting.png` 底图，左侧外卖、中间盲盒、右侧在家做饭
- 操作：单击选中并高亮，再点一次确认；左上角可返回
- 演示：外卖需求与多 Agent 分析、四格盲盒、番茄鸡蛋菜谱，结果页自动回到待机

所有媒体源文件保持原样，板端资源生成到被 Git 忽略的 `data/`。后续 UI/视频替换优先修改 `tools/build_assets.sh` 中的素材源；状态机和热区保持独立，不影响 Gemini-S1/openvela 版本。
