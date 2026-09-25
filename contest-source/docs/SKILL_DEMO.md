# Dinner Assistant Skill 部署与演示

最后核验：2026-09-25（Asia/Shanghai）

## 目标

`app/hello_app/skills/dinner-assistant.md` 使用 ai_agent 官方 Skill
Markdown 格式。Gemini-S1 上的运行路径为：

```text
/data/agent/skills/dinner-assistant.md
```

源码格式和安全边界已由主机测试检查；实际设备部署与使用演示将在
Gemini-S1 的 openvela、网络和 ai_agent 运行门禁通过后执行。当前文档不把
未完成的板端 Skill 加载描述为已经完成。

## 部署

在 Gemini-S1 已启动 openvela、ADB 可用且 ai_agent 已配置后，从仓库根目录
执行：

```bash
adb push app/hello_app/skills/dinner-assistant.md \
  /data/agent/skills/dinner-assistant.md
```

重新启动 ai_agent，使其扫描 `/data/agent/skills/*.md` 并把 Skill 摘要加入
Agent 上下文。部署前不把模型 Token 或网络凭据写进 Skill 文件。

## 演示脚本

在设备的 openvela shell 中输入：

```text
ask 帮我决定今晚吃什么，预算50元，不吃辣，不想做饭
```

演示需要依次证明：

1. Agent 能匹配 Dinner Assistant Skill；
2. 返回不超过三个候选，且不会编造未提供的实时价格、库存或配送时间；
3. 回复要求用户明确确认，不直接下单或付款；
4. 应用再次校验预算、忌口和临时偏好；
5. 用户确认后才进入手机交接；支付始终留在手机端；
6. 用户未选择“记住”时，临时的“不吃辣”不会写入长期偏好。

## 主机检查

```bash
bash app/hello_app/tests/host/test_skill_metadata.sh
```

该检查验证官方格式的标题、触发条件、使用步骤、示例和目标部署路径。它
证明仓库内 Skill 可部署，不替代 Gemini-S1 实体板上的加载与演示证据。
