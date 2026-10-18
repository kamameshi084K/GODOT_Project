extends SceneTree

var net: BlokusNetwork
var role := "client"
var required_players := 2
var target_revision := 4
var port := 24568
var started := false
var submitted_revision := -1
var last_revision := -1

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
	net.problem.connect(func(message): fail(message))
	net.aborted.connect(func(message):
		if last_revision < target_revision:
			fail(message)
	)
	call_deferred("start_network")
	create_timer(45.0).timeout.connect(func():
		if last_revision < target_revision:
			fail("タイムアウト revision=%d" % last_revision)
	)

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
	if state.revision >= target_revision:
		print("%s OK" % role)
		quit(0)
		return
	if state.finished or state.current_seat() != net.local_seat() or submitted_revision == state.revision:
		return
	var move := first_move(state)
	if move.is_empty():
		fail("手番なのに合法手がない")
		return
	submitted_revision = state.revision
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
