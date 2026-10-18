class_name BlokusRules
extends RefCounted

const SIZE := 20
const COLORS := ["青", "黄", "赤", "緑"]
const STARTS := [Vector2i(0, 0), Vector2i(19, 0), Vector2i(19, 19), Vector2i(0, 19)]
const PIECES := [
	[[0, 0]],
	[[0, 0], [1, 0]],
	[[0, 0], [1, 0], [2, 0]],
	[[0, 0], [1, 0], [0, 1]],
	[[0, 0], [1, 0], [2, 0], [3, 0]],
	[[0, 0], [1, 0], [2, 0], [1, 1]],
	[[0, 0], [1, 0], [0, 1], [1, 1]],
	[[0, 0], [0, 1], [0, 2], [1, 2]],
	[[0, 0], [1, 0], [1, 1], [2, 1]],
	[[1, 0], [2, 0], [0, 1], [1, 1], [1, 2]], # F
	[[0, 0], [0, 1], [0, 2], [0, 3], [0, 4]], # I
	[[0, 0], [0, 1], [0, 2], [0, 3], [1, 3]], # L
	[[0, 0], [1, 0], [0, 1], [1, 1], [0, 2]], # P
	[[0, 0], [0, 1], [1, 1], [1, 2], [1, 3]], # N
	[[0, 0], [1, 0], [2, 0], [1, 1], [1, 2]], # T
	[[0, 0], [2, 0], [0, 1], [1, 1], [2, 1]], # U
	[[0, 0], [0, 1], [0, 2], [1, 2], [2, 2]], # V
	[[0, 0], [0, 1], [1, 1], [1, 2], [2, 2]], # W
	[[1, 0], [0, 1], [1, 1], [2, 1], [1, 2]], # X
	[[0, 0], [0, 1], [0, 2], [0, 3], [1, 1]], # Y
	[[0, 0], [1, 0], [1, 1], [1, 2], [2, 2]], # Z
]
const ORTHOGONAL := [Vector2i.LEFT, Vector2i.RIGHT, Vector2i.UP, Vector2i.DOWN]
const DIAGONAL := [Vector2i(-1, -1), Vector2i(1, -1), Vector2i(-1, 1), Vector2i(1, 1)]

static func cells(piece_id: int, rotation: int, flipped: bool) -> Array[Vector2i]:
	var result: Array[Vector2i] = []
	if piece_id < 0 or piece_id >= PIECES.size():
		return result
	var min_x := 100
	var min_y := 100
	for pair in PIECES[piece_id]:
		var p := Vector2i(pair[0], pair[1])
		if flipped:
			p.x = -p.x
		for step in range(posmod(rotation, 4)):
			p = Vector2i(-p.y, p.x)
		result.append(p)
		min_x = mini(min_x, p.x)
		min_y = mini(min_y, p.y)
	for i in result.size():
		result[i] -= Vector2i(min_x, min_y)
	return result

static func inside(p: Vector2i) -> bool:
	return p.x >= 0 and p.y >= 0 and p.x < SIZE and p.y < SIZE

static func index(p: Vector2i) -> int:
	return p.y * SIZE + p.x

static func check(board: Array, used: Array, color: int, piece_id: int, rotation: int, flipped: bool, origin: Vector2i) -> String:
	if color < 0 or color >= 4 or piece_id < 0 or piece_id >= PIECES.size():
		return "駒が無効です"
	if used[color][piece_id]:
		return "使用済みの駒です"
	var shape := cells(piece_id, rotation, flipped)
	var owns_piece := false
	for value in board:
		if value == color:
			owns_piece = true
			break
	var corner_touch := false
	for delta in shape:
		var p := origin + delta
		if not inside(p):
			return "盤外には置けません"
		if board[index(p)] != -1:
			return "ほかの駒と重なります"
		for offset in ORTHOGONAL:
			var adjacent: Vector2i = p + offset
			if inside(adjacent) and board[index(adjacent)] == color:
				return "同じ色の辺には接触できません"
		if owns_piece:
			for offset in DIAGONAL:
				var adjacent: Vector2i = p + offset
				if inside(adjacent) and board[index(adjacent)] == color:
					corner_touch = true
		elif p == STARTS[color]:
			corner_touch = true
	if not corner_touch:
		if owns_piece:
			return "同じ色の角に接触させてください"
		return "開始角を覆ってください"
	return ""

static func has_move(board: Array, used: Array, color: int) -> bool:
	var candidates: Dictionary = {}
	var owns_piece := false
	for y in SIZE:
		for x in SIZE:
			if board[y * SIZE + x] != color:
				continue
			owns_piece = true
			for offset in DIAGONAL:
				var p: Vector2i = Vector2i(x, y) + offset
				if inside(p) and board[index(p)] == -1:
					candidates[p] = true
	if not owns_piece:
		candidates[STARTS[color]] = true
	for piece_id in PIECES.size():
		if used[color][piece_id]:
			continue
		var seen: Dictionary = {}
		for flipped in [false, true]:
			for rotation in 4:
				var shape := cells(piece_id, rotation, flipped)
				var signature := str(shape)
				if seen.has(signature):
					continue
				seen[signature] = true
				for target in candidates:
					for delta in shape:
						if check(board, used, color, piece_id, rotation, flipped, target - delta) == "":
							return true
	return false

static func remaining(used: Array, color: int) -> Dictionary:
	var count := 0
	var squares := 0
	for piece_id in PIECES.size():
		if not used[color][piece_id]:
			count += 1
			squares += PIECES[piece_id].size()
	return {"pieces": count, "squares": squares}

static func score(used: Array, color: int, last_piece: int) -> Dictionary:
	var left := remaining(used, color)
	var bonus := 0
	if left.pieces == 0:
		bonus = 15
		if last_piece == 0:
			bonus += 5
	return {"remaining": left.squares, "bonus": bonus, "total": -left.squares + bonus}
