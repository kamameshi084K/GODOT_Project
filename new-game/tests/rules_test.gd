extends SceneTree

var failures: Array[String] = []

func _initialize() -> void:
	test_shapes()
	test_placement()
	test_turns_and_scores()
	if failures.is_empty():
		print("rules_test: OK")
		quit(0)
	else:
		for failure in failures:
			push_error(failure)
		quit(1)

func expect(condition: bool, message: String) -> void:
	if not condition:
		failures.append(message)

func test_shapes() -> void:
	expect(BlokusRules.PIECES.size() == 21, "21種類")
	var counts := [0, 0, 0, 0, 0]
	var squares := 0
	var canonical: Dictionary = {}
	for id in BlokusRules.PIECES.size():
		var count: int = BlokusRules.PIECES[id].size()
		counts[count - 1] += 1
		squares += count
		var variants: Array[String] = []
		for flipped in [false, true]:
			for rotation in 4:
				var shape := BlokusRules.cells(id, rotation, flipped)
				shape.sort()
				var signature := str(shape)
				if signature not in variants:
					variants.append(signature)
		variants.sort()
		expect(not canonical.has(variants[0]), "重複形状: %d" % id)
		canonical[variants[0]] = true
	expect(counts == [1, 1, 2, 5, 12], "サイズ別個数")
	expect(squares == 89, "合計89マス")
	expect(BlokusRules.cells(9, 0, false) == BlokusRules.cells(9, 4, false), "4回転で元に戻る")
	expect(BlokusRules.cells(9, 0, false) != BlokusRules.cells(9, 0, true), "非対称駒の反転")

func test_placement() -> void:
	var state := BlokusMatch.new()
	state.start(4)
	expect(BlokusRules.check(state.board, state.used, 0, 0, 0, false, Vector2i(0, 0)) == "", "青の初手")
	expect(BlokusRules.check(state.board, state.used, 1, 0, 0, false, Vector2i(0, 0)) != "", "黄の開始角")
	expect(BlokusRules.check(state.board, state.used, 0, 0, 0, false, Vector2i(-1, 0)) != "", "盤外")
	expect(state.play(0, 0, 0, false, Vector2i(0, 0), state.match_id, state.revision) == "", "青の配置")
	expect(state.play(1, 0, 0, false, Vector2i(19, 0), state.match_id, state.revision) == "", "黄の配置")
	expect(state.play(2, 0, 0, false, Vector2i(19, 19), state.match_id, state.revision) == "", "赤の配置")
	expect(state.play(3, 0, 0, false, Vector2i(0, 19), state.match_id, state.revision) == "", "緑の配置")
	expect(BlokusRules.check(state.board, state.used, 0, 1, 0, false, Vector2i(1, 1)) == "", "同色角接触")
	expect(BlokusRules.check(state.board, state.used, 0, 1, 0, false, Vector2i(1, 0)) != "", "同色辺接触")
	expect(BlokusRules.check(state.board, state.used, 0, 0, 0, false, Vector2i(1, 1)) != "", "使用済み")
	state.board[BlokusRules.index(Vector2i(1, 0))] = 1
	expect(BlokusRules.check(state.board, state.used, 0, 1, 0, false, Vector2i(1, 1)) == "", "異色との辺接触")
	expect(BlokusRules.check(state.board, state.used, 0, 1, 0, false, Vector2i(0, 0)) != "", "重なり")
	var old_revision := state.revision - 1
	expect(state.play(0, 1, 0, false, Vector2i(1, 1), state.match_id, old_revision) != "", "古いrevision")
	expect(state.play(1, 1, 0, false, Vector2i(1, 1), state.match_id, state.revision) != "", "別席")

func test_turns_and_scores() -> void:
	var state := BlokusMatch.new()
	state.start(2)
	expect(state.seat_for_color(0) == 0 and state.seat_for_color(2) == 0, "2人の席1")
	expect(state.seat_for_color(1) == 1 and state.seat_for_color(3) == 1, "2人の席2")
	state.start(3)
	state.color_turn = 3
	expect(state.current_seat() == 0, "共有色の席1")
	state.advance_color()
	state.color_turn = 3
	expect(state.current_seat() == 1, "共有色の席2")
	state.advance_color()
	state.color_turn = 3
	expect(state.current_seat() == 2, "共有色の席3")
	expect(state.player_scores().size() == 3, "3人の得点")
	for id in BlokusRules.PIECES.size():
		state.used[0][id] = true
	expect(BlokusRules.score(state.used, 0, 1).bonus == 15, "全配置ボーナス")
	expect(BlokusRules.score(state.used, 0, 0).bonus == 20, "1マス最後のボーナス")
