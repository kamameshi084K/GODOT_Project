extends SceneTree

var net: BlokusNetwork
var role := "client"
var required_players := 2
var target_revision := 4
var port := 24568
var started := false
var submitted_revision := -1
var last_revision := -1
var completed := false
var stale_sent := false
var turn_violation_sent := false

func _initialize() -> void:
	for arg in OS.get_cmdline_user_args():
		var bits: PackedStringArray = arg.split("=")
		if bits.size() != 2:
			continue
		match bits[0]:
			"role": role = bits[1]
			"players": required_players = bits[1].to_int()
			"moves": target_revision = bits[1].to_int()
			"port": port = bits[1].to_int()
	net = BlokusNetwork.new()
	net.name = "Network"
	root.add_child(net)
	net.lobby_changed.connect(on_lobby)
	net.prepare_match.connect(func(): net.notify_loaded())
	net.state_changed.connect(on_state)
	net.problem.connect(on_problem)
	net.aborted.connect(func(message):
		if not completed:
			fail(message)
	)
	call_deferred("start_network")
	create_timer(45.0).timeout.connect(func():
		if not completed:
			fail("タイムアウト revision=%d" % last_revision)
	)

func on_problem(message: String) -> void:
	if message in ["現在の担当者ではありません", "古い操作です"]:
		print("%s REJECT %s" % [role, message])
	else:
		fail(message)

func start_network() -> void:
	var error := ""
	if role == "host":
		error = net.host("ホスト", port)
	else:
		error = net.join(role, "127.0.0.1", port)
	if error != "":
		fail(error)

func on_lobby() -> void:
	if net.phase != "lobby":
		return
	if role == "host":
		if not net.players[1]["ready"]:
			net.set_ready(true)
		if not started and net.seat_order.size() == required_players and net.can_start():
			started = true
			net.start_match()
	else:
		var own: Dictionary = net.players.get(net.local_peer_id(), {})
		if not own.is_empty() and not own.get("ready", false):
			net.set_ready(true)

func on_state() -> void:
	if net.phase != "playing":
		return
	var state := net.match_state
	if state.revision < last_revision:
		fail("revisionが巻き戻った")
		return
	last_revision = state.revision
	print("%s STATE %d %s" % [role, state.revision, str(state.board.hash())])
	if state.finished or state.revision >= target_revision:
		completed = true
		print("%s OK scores=%s" % [role, str(state.player_scores())])
		create_timer(0.5).timeout.connect(func(): quit(0))
		return
	if role != "host" and not turn_violation_sent and state.revision == 0:
		turn_violation_sent = true
		net.request_place.rpc_id(1, 0, 0, false, Vector2i(0, 0), state.match_id, state.revision)
	if state.finished or state.current_seat() != net.local_seat() or submitted_revision == state.revision:
		return
	var move := first_move(state)
	if move.is_empty():
		fail("手番なのに合法手がない")
		return
	submitted_revision = state.revision
	if role != "host" and not stale_sent and state.revision >= 1:
		stale_sent = true
		net.request_place.rpc_id(1, move.id, move.rotation, move.flipped, move.origin, state.match_id, state.revision - 1)
	if role == "host" and state.revision == 0:
		create_timer(0.3).timeout.connect(func():
			net.submit(move.id, move.rotation, move.flipped, move.origin, state.match_id, state.revision)
		)
	else:
		net.submit(move.id, move.rotation, move.flipped, move.origin, state.match_id, state.revision)

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

func fail(message: String) -> void:
	push_error("%s: %s" % [role, message])
	quit(1)
