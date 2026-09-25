# Contest AI Coding Log Collection

Last verified: 2026-09-25 (Asia/Shanghai)

## Current status

The official contest collector is installed and its verification script passes
all 10 checks for team `contest2026_482_xingguangyinli` and GitHub login
`abbyxu123`.

At the latest check, the collector staging directory contained no sessions.
The repository therefore keeps only `logs/README.md`; it does not create a
substitute or hand-written conversation record.

## Submission rule

Only sessions produced by the official collector from a supported AI tool in
the openvela workspace are eligible. Before a session is committed:

1. list the collector staging sessions;
2. preview the selected session;
3. review it for project relevance and sensitive information;
4. export it without editing the generated conversation content;
5. validate the JSONL with the official validator;
6. commit only the reviewed log and its generated manifest entry.

An unwanted session is removed as a whole from the submission set rather than
rewritten. If no eligible session exists, no AI Coding log is claimed.

## Privacy controls

- Do not place passwords, API keys, private keys, personal files, private paths,
  payment information, or unrelated material in a submitted log.
- The collector does not push to GitHub; repository review and commit remain a
  participant action.
- Direct API transcripts and manually constructed JSONL are not treated as
  official collector output.

The authoritative procedure is the
[AI Coding 日志归集与提交手册](https://github.com/open-vela/docs/blob/dev-ai-contest-2026/zh-cn/contest_2026/ai_coding_log_guide.md).
