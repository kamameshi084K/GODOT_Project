class_name BlokusNetwork
extends Node

signal lobby_changed
signal prepare_match
signal state_changed
signal problem(message: String)
signal aborted(message: String)

const DEFAULT_PORT := 24567

var match_state := BlokusMatch.new()
var players: Dictionary = {}
var seat_order: Array[int] = []
var loaded: Dictionary = {}
var phase := "offline"
var local_name := ""
var pending_match_id := 0

func _ready() -> void:
	multiplayer.peer_connected.connect(_peer_connected)
	multiplayer.peer_disconnected.connect(_peer_disconnected)
	multiplayer.connected_to_server.connect(_connected)
	multiplayer.connection_failed.connect(_connection_failed)
	multiplayer.server_disconnected.connect(_server_disconnected)

func host(name: String, port: int) -> String:
	if port < 1 or port > 65535:
		return "ポートは1〜65535を指定してください"
	disconnect_session()
	var peer := ENetMultiplayerPeer.new()
	var error := peer.create_server(port, 3)
	if error != OK:
		return "部屋を作れません（ポート使用中など）: %s" % error_string(error)
	multiplayer.multiplayer_peer = peer
	phase = "lobby"
	local_name = clean_name(name)
	players[1] = {"name": local_name, "ready": false}
	seat_order = [1]
	lobby_changed.emit()
	return ""

func join(name: String, address: String, port: int) -> String:
	if address.strip_edges().is_empty():
		return "ホストのIPアドレスかホスト名を入力してください"
	if port < 1 or port > 65535:
		return "ポートは1〜65535を指定してください"
	disconnect_session()
	var peer := ENetMultiplayerPeer.new()
	var error := peer.create_client(address.strip_edges(), port)
	if error != OK:
		return "接続を開始できません: %s" % error_string(error)
	multiplayer.multiplayer_peer = peer
	local_name = clean_name(name)
	phase = "connecting"
	lobby_changed.emit()
	return ""

func clean_name(value: String) -> String:
	var cleaned := value.strip_edges().substr(0, 16)
	return cleaned if not cleaned.is_empty() else "プレイヤー"

func disconnect_session() -> void:
	if phase == "offline":
		return
	multiplayer.multiplayer_peer = OfflineMultiplayerPeer.new()
	phase = "offline"
	players.clear()
	seat_order.clear()
	loaded.clear()
	pending_match_id = 0
	lobby_changed.emit()

func local_peer_id() -> int:
	return multiplayer.get_unique_id()

func local_seat() -> int:
	return seat_order.find(local_peer_id())

func set_ready(value: bool) -> void:
	if phase != "lobby":
		return
	if multiplayer.is_server():
		apply_ready(1, value)
	else:
		rpc_set_ready.rpc_id(1, value)

func apply_ready(id: int, value: bool) -> void:
	if phase != "lobby" or not players.has(id):
		return
	players[id]["ready"] = value
	broadcast_lobby()

func can_start() -> bool:
	if not multiplayer.is_server() or phase != "lobby" or seat_order.size() < 2 or seat_order.size() > 4:
		return false
	for id in seat_order:
		if not players[id]["ready"]:
			return false
	return true

func start_match() -> void:
	if not can_start():
		problem.emit("2〜4人全員が準備完了すると開始できます")
		return
	match_state.start(seat_order.size())
	pending_match_id = match_state.match_id
	loaded.clear()
	phase = "preparing"
	var order: Array = seat_order.duplicate()
	receive_prepare.rpc(pending_match_id, order)
	prepare_match.emit()

func notify_loaded() -> void:
	if phase != "preparing":
		return
	if multiplayer.is_server():
		mark_loaded(1, pending_match_id)
	else:
		request_loaded.rpc_id(1, pending_match_id)

func mark_loaded(id: int, id_match: int) -> void:
	if phase != "preparing" or id_match != pending_match_id or id not in seat_order:
		return
	loaded[id] = true
	if loaded.size() == seat_order.size():
		phase = "playing"
		broadcast_state()

func submit(piece_id: int, rotation: int, flipped: bool, origin: Vector2i, id_match: int, revision: int) -> void:
	if phase != "playing":
		problem.emit("対戦の開始を待っています")
		return
	if multiplayer.is_server():
		apply_request(1, piece_id, rotation, flipped, origin, id_match, revision)
	else:
		request_place.rpc_id(1, piece_id, rotation, flipped, origin, id_match, revision)

func apply_request(sender: int, piece_id: int, rotation: int, flipped: bool, origin: Vector2i, id_match: int, revision: int) -> void:
	if phase != "playing" or sender not in seat_order:
		reject(sender, "対戦に参加していません")
		return
	var seat := seat_order.find(sender)
	var result := match_state.play(seat, piece_id, rotation, flipped, origin, id_match, revision)
	if result != "":
		reject(sender, result)
		return
	broadcast_state()

func reject(sender: int, message: String) -> void:
	if sender == 1:
		problem.emit(message)
		state_changed.emit()
	else:
		receive_rejection.rpc_id(sender, message)
		receive_state.rpc_id(sender, match_state.snapshot())

func broadcast_state() -> void:
	receive_state.rpc(match_state.snapshot())
	state_changed.emit()

func broadcast_lobby() -> void:
	if not multiplayer.is_server():
		return
	receive_lobby.rpc(players, seat_order)
	lobby_changed.emit()

func back_to_lobby() -> void:
	if not multiplayer.is_server() or not match_state.finished:
		return
	for id in players:
		players[id]["ready"] = false
	phase = "lobby"
	receive_back_to_lobby.rpc()
	broadcast_lobby()

func _peer_connected(id: int) -> void:
	if multiplayer.is_server() and phase != "lobby":
		multiplayer.multiplayer_peer.disconnect_peer(id)

func _peer_disconnected(id: int) -> void:
	if not multiplayer.is_server():
		return
	if phase == "playing" or phase == "preparing":
		phase = "lobby"
		for peer_id in players:
			players[peer_id]["ready"] = false
		receive_abort.rpc("参加者が切断したため対戦を中止しました")
		aborted.emit("参加者が切断したため対戦を中止しました")
	players.erase(id)
	seat_order.erase(id)
	loaded.erase(id)
	broadcast_lobby()

func _connected() -> void:
	register_player.rpc_id(1, local_name)

func _connection_failed() -> void:
	disconnect_session()
	problem.emit("接続に失敗しました。IP・ポート・UDP通信を確認してください")

func _server_disconnected() -> void:
	var was_connecting := phase == "connecting"
	disconnect_session()
	aborted.emit("接続が拒否されました（満員または対戦中の可能性があります）" if was_connecting else "ホストとの接続が切れました")

@rpc("any_peer", "call_remote", "reliable")
func register_player(name: String) -> void:
	if not multiplayer.is_server():
		return
	var id := multiplayer.get_remote_sender_id()
	if phase != "lobby" or seat_order.size() >= 4 or players.has(id):
		multiplayer.multiplayer_peer.disconnect_peer(id)
		return
	players[id] = {"name": clean_name(name), "ready": false}
	seat_order.append(id)
	broadcast_lobby()

@rpc("any_peer", "call_remote", "reliable")
func rpc_set_ready(value: bool) -> void:
	if multiplayer.is_server():
		apply_ready(multiplayer.get_remote_sender_id(), value)

@rpc("any_peer", "call_remote", "reliable")
func request_loaded(id_match: int) -> void:
	if multiplayer.is_server():
		mark_loaded(multiplayer.get_remote_sender_id(), id_match)

@rpc("any_peer", "call_remote", "reliable")
func request_place(piece_id: int, rotation: int, flipped: bool, origin: Vector2i, id_match: int, revision: int) -> void:
	if multiplayer.is_server():
		apply_request(multiplayer.get_remote_sender_id(), piece_id, rotation, flipped, origin, id_match, revision)

@rpc("authority", "call_remote", "reliable")
func receive_lobby(roster: Dictionary, order: Array) -> void:
	if multiplayer.get_remote_sender_id() != 1:
		return
	players = roster.duplicate(true)
	seat_order.clear()
	for id in order:
		seat_order.append(id)
	phase = "lobby"
	lobby_changed.emit()

@rpc("authority", "call_remote", "reliable")
func receive_prepare(id_match: int, order: Array) -> void:
	if multiplayer.get_remote_sender_id() != 1:
		return
	pending_match_id = id_match
	seat_order.clear()
	for id in order:
		seat_order.append(id)
	phase = "preparing"
	prepare_match.emit()

@rpc("authority", "call_remote", "reliable")
func receive_state(data: Dictionary) -> void:
	if multiplayer.get_remote_sender_id() != 1:
		return
	if data["match_id"] != pending_match_id or phase not in ["preparing", "playing"]:
		return
	if phase == "playing" and data["revision"] < match_state.revision:
		return
	match_state.load_snapshot(data)
	phase = "playing"
	state_changed.emit()

@rpc("authority", "call_remote", "reliable")
func receive_rejection(message: String) -> void:
	if multiplayer.get_remote_sender_id() == 1:
		problem.emit(message)

@rpc("authority", "call_remote", "reliable")
func receive_abort(message: String) -> void:
	if multiplayer.get_remote_sender_id() != 1:
		return
	phase = "lobby"
	aborted.emit(message)

@rpc("authority", "call_remote", "reliable")
func receive_back_to_lobby() -> void:
	if multiplayer.get_remote_sender_id() != 1:
		return
	phase = "lobby"
	lobby_changed.emit()
