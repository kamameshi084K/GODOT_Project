class_name BlokusMatch
extends RefCounted

var board: Array = []
var used: Array = []
var last_piece := [-1, -1, -1, -1]
var player_count := 2
var color_turn := 0
var green_turn_count := 0
var revision := 0
var match_id := 0
var finished := false
var pass_log: Array[String] = []

func start(count: int) -> void:
	assert(count >= 2 and count <= 4)
	player_count = count
	board.clear()
	board.resize(BlokusRules.SIZE * BlokusRules.SIZE)
	board.fill(-1)
	used.clear()
	for color in 4:
		var row: Array = []
		row.resize(BlokusRules.PIECES.size())
		row.fill(false)
		used.append(row)
	last_piece = [-1, -1, -1, -1]
	color_turn = 0
	green_turn_count = 0
	revision = 0
	var old_match_id := match_id
	match_id = randi()
	while match_id == old_match_id:
		match_id = randi()
	finished = false
	pass_log.clear()
	advance_unplayable()

func seat_for_color(color: int) -> int:
	if player_count == 2:
		return color % 2
	if player_count == 3 and color == 3:
		return green_turn_count % 3
	return color

func current_seat() -> int:
	return seat_for_color(color_turn)

func play(seat: int, piece_id: int, rotation: int, flipped: bool, origin: Vector2i, request_match_id: int, request_revision: int) -> String:
	if finished:
		return "対戦は終了しています"
	if request_match_id != match_id or request_revision != revision:
		return "古い操作です"
	if seat != current_seat():
		return "現在の担当者ではありません"
	var reason := BlokusRules.check(board, used, color_turn, piece_id, rotation, flipped, origin)
	if reason != "":
		return reason
	for delta in BlokusRules.cells(piece_id, rotation, flipped):
		board[BlokusRules.index(origin + delta)] = color_turn
	used[color_turn][piece_id] = true
	last_piece[color_turn] = piece_id
	revision += 1
	advance_color()
	advance_unplayable()
	return ""

func advance_color() -> void:
	if player_count == 3 and color_turn == 3:
		green_turn_count += 1
	color_turn = (color_turn + 1) % 4

func advance_unplayable() -> void:
	var skipped := 0
	while skipped < 4 and not BlokusRules.has_move(board, used, color_turn):
		pass_log.append("%sは置ける駒がないため自動パス" % BlokusRules.COLORS[color_turn])
		revision += 1
		skipped += 1
		advance_color()
	if skipped == 4:
		finished = true

func player_scores() -> Array[Dictionary]:
	var scores: Array[Dictionary] = []
	for seat in player_count:
		var total := 0
		var components: Array = []
		for color in 4:
			if player_count == 3 and color == 3:
				continue
			if seat_for_color(color) == seat:
				var result := BlokusRules.score(used, color, last_piece[color])
				total += result.total
				components.append({"color": color, "score": result})
		scores.append({"seat": seat, "total": total, "components": components})
	for entry in scores:
		var higher := 0
		for other in scores:
			if other.total > entry.total:
				higher += 1
		entry["rank"] = higher + 1
	return scores

func snapshot() -> Dictionary:
	return {
		"board": board.duplicate(),
		"used": used.duplicate(true),
		"last_piece": last_piece.duplicate(),
		"player_count": player_count,
		"color_turn": color_turn,
		"green_turn_count": green_turn_count,
		"revision": revision,
		"match_id": match_id,
		"finished": finished,
		"pass_log": Array(pass_log),
	}

func load_snapshot(data: Dictionary) -> void:
	board = data["board"].duplicate()
	used = data["used"].duplicate(true)
	last_piece = data["last_piece"].duplicate()
	player_count = data["player_count"]
	color_turn = data["color_turn"]
	green_turn_count = data["green_turn_count"]
	revision = data["revision"]
	match_id = data["match_id"]
	finished = data["finished"]
	pass_log.clear()
	for entry in data["pass_log"]:
		pass_log.append(entry)
