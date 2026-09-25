# Repository and Submission Map

Last verified: 2026-09-25 (Asia/Shanghai)

## Repository roles

| Role | Repository | Submission use |
| --- | --- | --- |
| Official team repository | `https://github.com/open-vela/contest2026_482_xingguangyinli` | 正式参赛仓；合入目标为 `dev-ai-contest-2026` |
| Personal fork | `https://github.com/abbyxu123/contest2026_482_xingguangyinli` | 开发分支和 Pull Request 来源，不替代正式参赛仓 |
| Personal project backup | `https://github.com/abbyxu123/xiaomi-Living-Canvas` | 项目过程备份，不作为组委会验收地址 |
| Official contest docs | `https://github.com/open-vela/docs` | 赛道、提交和 AI Coding 日志规则来源 |

现有提交使用 Pull Request：

```text
abbyxu123/contest2026_482_xingguangyinli:codex/gemini-s1-choice-ui
  -> open-vela/contest2026_482_xingguangyinli:dev-ai-contest-2026
```

PR 地址：`https://github.com/open-vela/contest2026_482_xingguangyinli/pull/1`

## Manifest mapping

团队 manifest 只把本队目录映射到 openvela 构建树：

| Team source path | Build-tree destination |
| --- | --- |
| `app/hello_app` | `packages/demos/contest2026_482_hello_app` |
| `quickapp/hello_quickapp` | `packages/apps/contest2026_482_hello_quickapp` |
| `board/contest_board` | `vendor/openvela/boards/contest2026_482_board` |

产品源码应在本队仓库维护，并通过团队 manifest 进入 openvela 工作区；不会把本队实现直接复制到共享上游仓库。

## 提交流程

1. 在个人 fork 的开发分支完成代码、测试和脱敏证据。
2. 通过 PR #1 请求合入正式参赛仓。
3. PR 检查和人工复核通过后，合入官方 `dev-ai-contest-2026`。
4. 以官方仓库目标分支上的提交作为最终参赛状态。
5. 合入后在个人项目备份仓创建新的备份节点；历史备份不覆盖。

官方仓库是最终交付位置；个人 fork 只是 PR 工作副本；个人项目仓用于备份。三者用途不同。
