extends SceneTree

var game: Node3D

func _initialize() -> void:
	game = load("res://scenes/main.tscn").instantiate()
	root.add_child(game)
	call_deferred("run_checks")

func run_checks() -> void:
	if not game.start_panel.visible or game.game_panel.visible:
		fail("初期画面")
		return
	game.count_selector.select(0)
	game.start_local_match()
	if game.match_state.player_count != 2 or not game.game_panel.visible:
		fail("ローカル対戦開始")
		return
	game.select_piece(0)
	game.rotate_piece(1)
	game.flip_piece()
	if game.selected_piece != 0 or game.piece_rotation != 1 or not game.flipped:
		fail("駒の選択・回転・反転")
		return
	var result: String = game.match_state.play(0, 0, 0, false, Vector2i(0, 0), game.match_state.match_id, game.match_state.revision)
	if result != "":
		fail("UI開始後の配置: " + result)
		return
	game.refresh_ui()
	if game.match_state.color_turn != 1 or not game.status_label.text.contains("黄"):
		fail("次の色の手持ち表示")
		return
	game.count_selector.select(1)
	game.start_local_match()
	game.match_state.finished = true
	game.refresh_ui()
	if not game.result_panel.visible or not game.full_result_label.text.contains("共有色"):
		fail("3人戦の結果表示")
		return
	print("ui_flow_test: 画面遷移・駒操作・結果表示 OK")
	quit(0)

func fail(message: String) -> void:
	push_error(message)
	quit(1)
