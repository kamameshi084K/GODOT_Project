extends SceneTree

func _initialize() -> void:
	call_deferred("capture")

func capture() -> void:
	var game: Variant = load("res://scenes/main.tscn").instantiate()
	root.add_child(game)
	game.count_selector.select(0)
	game.start_local_match()
	for color in 4:
		var origin: Vector2i = BlokusRules.STARTS[color]
		var result: String = game.match_state.play(game.match_state.current_seat(), 0, 0, false, origin, game.match_state.match_id, game.match_state.revision)
		if result != "":
			push_error(result)
			quit(1)
			return
	game.board_view.show_board(game.match_state.board)
	game.refresh_ui()
	game.selected_piece = 1
	game.board_view.show_preview(1, 0, false, Vector2i(1, 1), 0, true)
	await create_timer(1.5).timeout
	var path := ProjectSettings.globalize_path("res://.test-home/visual.png")
	var error := root.get_texture().get_image().save_png(path)
	if error != OK:
		push_error("画像保存失敗: %s" % error_string(error))
		quit(1)
		return
	print("画面保存: " + path)
	quit(0)
