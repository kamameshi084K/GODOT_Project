extends SceneTree

func _initialize() -> void:
	for count in [2, 3, 4]:
		var state := BlokusMatch.new()
		state.start(count)
		var moves := 0
		while not state.finished and moves < 84:
			var move := first_move(state)
			if move.is_empty():
				push_error("合法手探索と手番が不一致")
				quit(1)
				return
			var result := state.play(state.current_seat(), move.id, move.rotation, move.flipped, move.origin, state.match_id, state.revision)
			if result != "":
				push_error("合法手が拒否された: " + result)
				quit(1)
				return
			moves += 1
		if not state.finished:
			push_error("%d人対戦が終了しない" % count)
			quit(1)
			return
		print("%d人: %d手、終了、revision=%d" % [count, moves, state.revision])
	print("full_match_test: OK")
	quit(0)

func first_move(state: BlokusMatch) -> Dictionary:
	var color := state.color_turn
	var targets: Dictionary = {}
	var owns_piece := false
	for y in 20:
		for x in 20:
			if state.board[y * 20 + x] != color:
				continue
			owns_piece = true
			for offset in BlokusRules.DIAGONAL:
				var p: Vector2i = Vector2i(x, y) + offset
				if BlokusRules.inside(p) and state.board[BlokusRules.index(p)] == -1:
					targets[p] = true
	if not owns_piece:
		targets[BlokusRules.STARTS[color]] = true
	for id in BlokusRules.PIECES.size():
		if state.used[color][id]:
			continue
		for flipped in [false, true]:
			for rotation in 4:
				for target in targets:
					for delta in BlokusRules.cells(id, rotation, flipped):
						var origin: Vector2i = target - delta
						if BlokusRules.check(state.board, state.used, color, id, rotation, flipped, origin) == "":
							return {"id": id, "rotation": rotation, "flipped": flipped, "origin": origin}
	return {}
