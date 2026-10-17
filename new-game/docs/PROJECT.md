# プロジェクト状況

## 確定事項

- Godotの新しいゲーム開発用。ゲーム内容は **ユーザーからの説明待ち**。
- フォルダ：`C:\Users\ka07z\Documents\GitHub\GODOT_Project\new-game`
- 親のGit管理を継続する。今回の範囲は開発の土台づくりだけ。
- 作成物はAGENTS.md、3つのCodex TOML、3つの運用文書の計7ファイル。
- メイン：gpt-6-sol / medium。調査godot_scout・レビューgodot_reviewer：gpt-6-luna / medium。サブ最大2体、全体最大3体。編集はメインのみ。

## 未決事項

ゲーム内容、2D/3D、言語、ゲームルール、描画方式、Godotの採用バージョン、対象プラットフォーム、最初の実装範囲、ゲームの完了条件。推測で決めない。

## 環境・設定の確認（2026-10-17、日本時間）

- 初回チャットの作業ディレクトリとGitルート：`C:\Users\ka07z\Documents\GitHub\GODOT_Project`。
- ドライブ直下から作業ディレクトリまでのAGENTS.md、およびユーザー全体のAGENTS.md / AGENTS.override.mdは見つからなかった。親階層の `.codex/config.toml` も見つからなかった。
- ユーザー設定 `C:\Users\ka07z\.codex\config.toml` は既定gpt-6.1-sol / medium、Windows設定と親リポジトリの信頼登録あり。既存の権限・承認・管理設定を継承し、ここでは上書きしない。
- Codex CLI：`0.162.0-alpha.17.2`。実行ファイル：`C:\Users\ka07z\AppData\Local\OpenAI\Codex\bin\2e5e00daee91c61d\codex.exe`。CLI呼び出し時にホームディレクトリを見つけられない旨の警告あり。
- ローカル `models_cache.json` にgpt-6-sol、gpt-6-lunaとそれぞれのmedium対応を確認。これは実際の推論成功や現在のアカウント利用可否を保証しない。
- Godot：PATH（godot / godot4）、実行中プロセス、Downloads・Desktop直下と代表的なGodot配置先を簡易確認したが、実行ファイルを特定できず。バージョン未確認。インストール・更新はしていない。
- `.gitmodules` には既存の `pikumin/godot-cpp`（https://github.com/godotengine/godot-cpp）のみ。新ゲームには導入しない。既存の追跡済み変更・未追跡ファイルがあるため、それらはそのまま保持する。

## 設定書式と適用状況

2026-10-17に以下の公式文書で、configの指定キー、`.codex/agents/*.toml` の自動検出、独立定義のname・description・developer_instructions・model・model_reasoning_effortを確認した。

- [サブエージェント](https://learn.chatgpt.com/docs/agent-configuration/subagents)
- [設定リファレンス](https://learn.chatgpt.com/docs/config-file/config-reference)

上限キーの2はメインを除く同時サブ数。再帰起動禁止と役割制限は指示に明記し、未確認の権限キーは追加しない。別メイン定義や独自の並列実行基盤は作成しない。

公式書式を採用した。インストール版での読み込み確認の結果は `WORK_LOG.md` に記録する。TOMLの構文・静的整合性と実際の読み込み・推論は別物。この親フォルダの初回チャットに子設定が適用されたとは扱わない。

## 現在の段階と次の作業

開発の土台を作成。`project.godot`・コード・シーン・素材は未作成。初回の完了条件にゲーム起動は含めない。

次は `WORKFLOW.md` の手順でnew-gameをCodexの別プロジェクトとして登録し、新しいチャットで設定適用・実効モデル・カスタム役割を確認する。その後ユーザーからゲーム内容を受け取り、仕様・最初の実装範囲・完了条件を整理する。Godotの実行ファイルが分かり次第、パスと実際のバージョンを記録する。
