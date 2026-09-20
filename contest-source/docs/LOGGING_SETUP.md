# Contest AI Coding Log Safety Plan

Last reviewed: 2026-09-14 (Asia/Shanghai)

## Official behavior

The official contest collector activates only below an openvela workspace containing `.repo/`. Its installer creates global user files below `~/.claude/` and a command below `~/.local/bin/`. When a supported AI session ends inside the workspace, the collector can write conversation records into the team repository's `logs/` directory.

Officially documented record fields include conversation text, model thinking, tool names, tool inputs and outputs, model name, token counts, and sequence numbers. The collector does not push; the participant remains responsible for reviewing and committing logs.

## Current decision

`NOT_INSTALLED`

The global collector was deliberately not installed during unattended setup because:

- the installer changes global user configuration;
- this setup conversation includes security-sensitive screenshots and should not be exported blindly;
- GitHub identity and final log ownership must be confirmed by the participant;
- every exported JSONL must be reviewed before commit.

The complete `.repo/` workspace now exists. That resolves the workspace prerequisite but does not authorize a global hook installation or exporting this setup conversation.

Status: `BLOCKED_FOR_USER_REVIEW`

## Safe installation sequence

1. Use the completed isolated openvela workspace and inspect the exact collector scripts from the pinned checkout.
2. Review all writes to `~/.claude/`, `~/.local/bin/`, shell startup files, and the team repository.
3. Confirm team ID `contest2026_482_xingguangyinli` and the participant's exact GitHub login.
4. Run the official installer from the pinned workspace.
5. Run the official verification script and preserve its non-sensitive result.
6. Start a new, project-only AI session from inside the `.repo/` workspace.
7. Before any Git commit, render or preview the generated log and remove the session from the submission set if it contains passwords, API keys, private paths, personal files, or unrelated conversations.

## Credential rules

- Never paste Ubuntu, Wi-Fi, GitHub, or device passwords into an AI session.
- Never place LLM, search, or other API keys in source, docs, shell history, evidence, or AI logs.
- Configure device secrets only through the board's confirmed interactive interface while the user is present.
- Do not submit this current setup session automatically.
- Do not run `git add logs/` until the specific files have been manually reviewed.

## Verification checklist

- Collector source revision is recorded.
- Installation diff is understood.
- `TEAM_ID` and `GITHUB_LOGIN` are correct.
- Workspace gate activates only below the intended `.repo/` root.
- A harmless test session produces a JSONL in the expected team repository path.
- Rendered test log contains no secret or unrelated content.
- No automatic push or remote mutation occurs.
