extends SceneTree

var net: BlokusNetwork
var role := "client"
var scenario := "rematch"
var port := 24571
var first_id := 0
var marked_finished := false
var complete := false

func _initialize() -> void:
	for arg in OS.get_cmdline_user_args():
		var bits: PackedStringArray = arg.split("=")
		if bits.size() != 2:
			continue
		match bits[0]:
			"role": role = bits[1]
			"scenario": scenario = bits[1]
			"port": port = bits[1].to_int()
	net = BlokusNetwork.new()
	net.name = "Network"
	root.add_child(net)
	net.lobby_changed.connect(on_lobby)
	net.prepare_match.connect(func(): net.notify_loaded())
	net.state_changed.connect(on_state)
	net.problem.connect(func(message): fail(message))
	net.aborted.connect(on_abort)
	call_deferred("start_network")
	create_timer(20.0).timeout.connect(func():
		if not complete:
			fail("ライフサイクル試験のタイムアウト")
	)

func start_network() -> void:
	var error := net.host("ホスト", port) if role == "host" else net.join("クライアント", "127.0.0.1", port)
	if error != "":
		fail(error)

func on_lobby() -> void:
	if net.phase != "lobby":
		return
	if role == "host":
		if not net.players[1]["ready"]:
			net.set_ready(true)
		if net.can_start():
			net.start_match()
	else:
		var own: Dictionary = net.players.get(net.local_peer_id(), {})
		if not own.is_empty() and not own.get("ready", false):
			net.set_ready(true)

func on_state() -> void:
	var state := net.match_state
	if state.match_id == 0:
		return
	if first_id == 0:
		first_id = state.match_id
		if scenario == "client_disconnect" and role != "host":
			net.disconnect_session()
			succeed("CLIENT_LEFT")
		elif scenario == "host_disconnect" and role == "host":
			net.disconnect_session()
			succeed("HOST_LEFT")
		elif scenario == "rematch" and role == "host" and not marked_finished:
			marked_finished = true
			state.finished = true
			net.broadcast_state()
			create_timer(0.3).timeout.connect(func(): net.back_to_lobby())
		return
	if state.match_id != first_id:
		var empty := state.revision == 0 and state.green_turn_count == 0 and state.board.count(-1) == 400
		for row in state.used:
			empty = empty and row.count(false) == 21
		if not empty:
			fail("再戦の状態が初期化されていません")
			return
		succeed("REMATCH_NEW_ID=%d" % state.match_id)

func on_abort(message: String) -> void:
	if scenario == "client_disconnect" and role == "host":
		succeed("HOST_ABORT=%s" % message)
	elif scenario == "host_disconnect" and role != "host":
		succeed("CLIENT_ABORT=%s" % message)

func succeed(message: String) -> void:
	if complete:
		return
	complete = true
	print("%s %s" % [role, message])
	create_timer(0.5).timeout.connect(func(): quit(0))

func fail(message: String) -> void:
	push_error("%s: %s" % [role, message])
	quit(1)
