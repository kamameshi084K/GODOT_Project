extends Node3D

const PIECE_NAMES := ["1", "2", "3直", "3角", "4直", "4T", "4四角", "4L", "4Z", "5F", "5I", "5L", "5P", "5N", "5T", "5U", "5V", "5W", "5X", "5Y", "5Z"]

var match_state := BlokusMatch.new()
var network: BlokusNetwork
var board_view: BlokusBoardView
var selected_piece := -1
var piece_rotation := 0
var flipped := false
var preview_origin := Vector2i(-100, -100)
var reason_label: Label
var status_label: Label
var result_label: Label
var pieces_grid: GridContainer
var start_panel: PanelContainer
var game_panel: Control
var count_selector: OptionButton
var name_field: LineEdit
var address_field: LineEdit
var port_field: LineEdit
var lobby_panel: PanelContainer
var lobby_label: Label
var ready_button: Button
var launch_button: Button
var return_button: Button
var waiting := false

func _ready() -> void:
	board_view = BlokusBoardView.new()
	add_child(board_view)
	network = BlokusNetwork.new()
	network.name = "Network"
	add_child(network)
	network.lobby_changed.connect(update_lobby)
	network.prepare_match.connect(prepare_online_match)
	network.state_changed.connect(update_online_state)
	network.problem.connect(show_problem)
	network.aborted.connect(show_abort)
	build_ui()
	start_panel.visible = true
	lobby_panel.visible = false
	game_panel.visible = false

func build_ui() -> void:
	var ui := Control.new()
	ui.set_anchors_preset(Control.PRESET_FULL_RECT)
	ui.mouse_filter = Control.MOUSE_FILTER_IGNORE
	add_child(ui)
	start_panel = PanelContainer.new()
	start_panel.position = Vector2(390, 150)
	start_panel.custom_minimum_size = Vector2(500, 470)
	ui.add_child(start_panel)
	var start_content := VBoxContainer.new()
	start_panel.add_child(start_content)
	var title := Label.new()
	title.text = "ブロックス"
	title.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	title.add_theme_font_size_override("font_size", 32)
	start_content.add_child(title)
	var subtitle := Label.new()
	subtitle.text = "Phase 1：このPCで順番に操作する対戦"
	subtitle.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	start_content.add_child(subtitle)
	name_field = LineEdit.new()
	name_field.placeholder_text = "ニックネーム"
	name_field.text = "プレイヤー"
	start_content.add_child(name_field)
	address_field = LineEdit.new()
	address_field.placeholder_text = "ホストのIPアドレスまたはホスト名"
	address_field.text = "127.0.0.1"
	start_content.add_child(address_field)
	port_field = LineEdit.new()
	port_field.placeholder_text = "UDPポート"
	port_field.text = str(BlokusNetwork.DEFAULT_PORT)
	start_content.add_child(port_field)
	var online_buttons := HBoxContainer.new()
	start_content.add_child(online_buttons)
	var host_button := Button.new()
	host_button.text = "部屋を作る"
	host_button.pressed.connect(create_room)
	online_buttons.add_child(host_button)
	var join_button := Button.new()
	join_button.text = "部屋に参加する"
	join_button.pressed.connect(join_room)
	online_buttons.add_child(join_button)
	count_selector = OptionButton.new()
	for count in [2, 3, 4]:
		count_selector.add_item("%d人" % count, count)
	start_content.add_child(count_selector)
	var start_button := Button.new()
	start_button.text = "1台で交代して遊ぶ"
	start_button.pressed.connect(start_local_match)
	start_content.add_child(start_button)
	var instructions := Label.new()
	instructions.text = "青:左上  黄:右上  赤:右下  緑:左下\n駒を選び、盤面を左クリックして配置\nQ/E:回転  F:反転  右クリック/Esc:選択解除\n中ボタンのドラッグ:視点回転  ホイール:ズーム"
	start_content.add_child(instructions)
	lobby_panel = PanelContainer.new()
	lobby_panel.position = Vector2(390, 150)
	lobby_panel.custom_minimum_size = Vector2(500, 420)
	ui.add_child(lobby_panel)
	var lobby_content := VBoxContainer.new()
	lobby_panel.add_child(lobby_content)
	var lobby_title := Label.new()
	lobby_title.text = "プライベートロビー"
	lobby_title.add_theme_font_size_override("font_size", 24)
	lobby_content.add_child(lobby_title)
	lobby_label = Label.new()
	lobby_label.custom_minimum_size = Vector2(470, 240)
	lobby_content.add_child(lobby_label)
	ready_button = Button.new()
	ready_button.text = "準備完了にする"
	ready_button.pressed.connect(toggle_ready)
	lobby_content.add_child(ready_button)
	launch_button = Button.new()
	launch_button.text = "対戦開始"
	launch_button.pressed.connect(func(): network.start_match())
	lobby_content.add_child(launch_button)
	var leave_button := Button.new()
	leave_button.text = "退出"
	leave_button.pressed.connect(func():
		network.disconnect_session()
		start_panel.visible = true
		lobby_panel.visible = false
	)
	lobby_content.add_child(leave_button)
	game_panel = Control.new()
	game_panel.set_anchors_preset(Control.PRESET_FULL_RECT)
	game_panel.mouse_filter = Control.MOUSE_FILTER_IGNORE
	ui.add_child(game_panel)
	var left := PanelContainer.new()
	left.position = Vector2(8, 8)
	left.custom_minimum_size = Vector2(240, 680)
	game_panel.add_child(left)
	var left_content := VBoxContainer.new()
	left.add_child(left_content)
	status_label = Label.new()
	status_label.custom_minimum_size = Vector2(230, 110)
	left_content.add_child(status_label)
	var scroll := ScrollContainer.new()
	scroll.custom_minimum_size = Vector2(230, 455)
	left_content.add_child(scroll)
	pieces_grid = GridContainer.new()
	pieces_grid.columns = 2
	scroll.add_child(pieces_grid)
	var controls := HBoxContainer.new()
	left_content.add_child(controls)
	for spec in [["左回転 Q", -1], ["右回転 E", 1]]:
		var button := Button.new()
		button.text = spec[0]
		button.pressed.connect(rotate_piece.bind(spec[1]))
		controls.add_child(button)
	var flip_button := Button.new()
	flip_button.text = "反転 F"
	flip_button.pressed.connect(flip_piece)
	left_content.add_child(flip_button)
	var camera_button := Button.new()
	camera_button.text = "視点を戻す"
	camera_button.pressed.connect(func(): board_view.reset_camera())
	left_content.add_child(camera_button)
	var bottom := PanelContainer.new()
	bottom.set_anchors_preset(Control.PRESET_BOTTOM_WIDE)
	bottom.offset_top = -90
	game_panel.add_child(bottom)
	var bottom_content := VBoxContainer.new()
	bottom.add_child(bottom_content)
	reason_label = Label.new()
	reason_label.text = "駒を選んでください"
	bottom_content.add_child(reason_label)
	result_label = Label.new()
	bottom_content.add_child(result_label)
	return_button = Button.new()
	return_button.text = "人数選択へ戻る"
	return_button.pressed.connect(return_from_game)
	bottom_content.add_child(return_button)

func create_room() -> void:
	var message := network.host(name_field.text, port_field.text.to_int())
	if message != "":
		show_problem(message)
		return
	start_panel.visible = false
	lobby_panel.visible = true
	update_lobby()

func join_room() -> void:
	var message := network.join(name_field.text, address_field.text, port_field.text.to_int())
	if message != "":
		show_problem(message)
		return
	start_panel.visible = false
	lobby_panel.visible = true
	update_lobby()

func toggle_ready() -> void:
	var own: Dictionary = network.players.get(network.local_peer_id(), {})
	network.set_ready(not own.get("ready", false))

func update_lobby() -> void:
	if lobby_label == null:
		return
	if network.phase == "lobby" and game_panel.visible and match_state.finished:
		game_panel.visible = false
		lobby_panel.visible = true
	var roster := ""
	if network.phase == "connecting":
		roster = "接続待ち…"
	else:
		for seat in network.seat_order.size():
			var id := network.seat_order[seat]
			var info: Dictionary = network.players.get(id, {})
			var color_description := ""
			if network.seat_order.size() == 2:
				color_description = ["青・赤", "黄・緑"][seat]
			elif network.seat_order.size() == 3:
				color_description = ["青", "黄", "赤"][seat] + "（緑は共有）"
			elif network.seat_order.size() == 4:
				color_description = BlokusRules.COLORS[seat]
			roster += "席%d  %s  %s  %s\n" % [seat + 1, info.get("name", ""), color_description, "準備完了" if info.get("ready", false) else "未準備"]
	lobby_label.text = roster
	var own: Dictionary = network.players.get(network.local_peer_id(), {})
	ready_button.text = "準備を取り消す" if own.get("ready", false) else "準備完了にする"
	ready_button.disabled = network.phase != "lobby" or own.is_empty()
	launch_button.visible = network.phase == "lobby" and multiplayer.is_server()
	launch_button.disabled = not network.can_start()

func prepare_online_match() -> void:
	match_state = network.match_state
	waiting = true
	selected_piece = -1
	start_panel.visible = false
	lobby_panel.visible = false
	game_panel.visible = true
	board_view.show_preview(-1, 0, false, Vector2i.ZERO, 0, false)
	reason_label.text = "全員の画面を読み込み中…"
	status_label.text = "対戦準備中"
	result_label.text = ""
	for child in pieces_grid.get_children():
		child.queue_free()
	network.notify_loaded()

func update_online_state() -> void:
	if network.phase != "playing":
		return
	waiting = false
	selected_piece = -1
	board_view.show_board(match_state.board)
	board_view.show_preview(-1, 0, false, Vector2i.ZERO, 0, false)
	refresh_ui()

func show_problem(message: String) -> void:
	waiting = false
	if game_panel != null and game_panel.visible:
		reason_label.text = message
	elif lobby_panel != null and lobby_panel.visible:
		lobby_label.text += "\n" + message
	else:
		push_warning(message)

func show_abort(message: String) -> void:
	waiting = false
	selected_piece = -1
	game_panel.visible = false
	lobby_panel.visible = network.phase == "lobby"
	start_panel.visible = network.phase != "lobby"
	update_lobby()
	if lobby_panel.visible:
		lobby_label.text += "\n" + message

func return_from_game() -> void:
	selected_piece = -1
	board_view.show_preview(-1, 0, false, Vector2i.ZERO, 0, false)
	if network.phase == "offline":
		game_panel.visible = false
		start_panel.visible = true
	elif network.match_state.finished and multiplayer.is_server():
		network.back_to_lobby()
		game_panel.visible = false
		lobby_panel.visible = true
	else:
		network.disconnect_session()
		game_panel.visible = false
		start_panel.visible = true

func start_local_match() -> void:
	match_state = BlokusMatch.new()
	match_state.start(count_selector.get_selected_id())
	selected_piece = -1
	piece_rotation = 0
	flipped = false
	board_view.show_board(match_state.board)
	board_view.show_preview(-1, 0, false, Vector2i.ZERO, 0, false)
	start_panel.visible = false
	game_panel.visible = true
	refresh_ui()

func refresh_ui() -> void:
	for child in pieces_grid.get_children():
		child.queue_free()
	var color := match_state.color_turn
	var seat := match_state.current_seat()
	var remaining := BlokusRules.remaining(match_state.used, color)
	var shared := "（共有色）" if match_state.player_count == 3 and color == 3 else ""
	var person := "席%d" % (seat + 1)
	if network.phase == "playing" and seat < network.seat_order.size():
		person = network.players.get(network.seat_order[seat], {}).get("name", person)
	status_label.text = "%sの手番 / %s%s\n残り %d個・%dマス\n青:左上 黄:右上\n赤:右下 緑:左下" % [person, BlokusRules.COLORS[color], shared, remaining.pieces, remaining.squares]
	if match_state.finished:
		status_label.text = "対戦終了"
		var results := ""
		for entry in match_state.player_scores():
			results += "%d位 席%d: %d点  " % [entry.rank, entry.seat + 1, entry.total]
			for component in entry.components:
				var detail: Dictionary = component.score
				results += "%s 残%d ボーナス%d  " % [BlokusRules.COLORS[component.color], detail.remaining, detail.bonus]
			results += "\n"
		result_label.text = results
		return_button.text = "ロビーへ戻る" if network.phase == "playing" and multiplayer.is_server() else "ホストがロビーへ戻すのを待つ" if network.phase == "playing" else "人数選択へ戻る"
		return_button.disabled = network.phase == "playing" and not multiplayer.is_server()
		return
	return_button.text = "対戦から退出"
	return_button.disabled = false
	result_label.text = match_state.pass_log.back() if not match_state.pass_log.is_empty() else ""
	for piece_id in BlokusRules.PIECES.size():
		var button := Button.new()
		button.custom_minimum_size = Vector2(105, 70)
		button.text = "%s (%d)\n%s" % [PIECE_NAMES[piece_id], BlokusRules.PIECES[piece_id].size(), piece_icon(piece_id)]
		button.disabled = match_state.used[color][piece_id]
		button.pressed.connect(select_piece.bind(piece_id))
		pieces_grid.add_child(button)
	reason_label.text = "駒を選んでください"
	if network.phase == "playing" and seat != network.local_seat():
		reason_label.text = "相手の手番を待っています"

func piece_icon(piece_id: int) -> String:
	var shape := BlokusRules.cells(piece_id, 0, false)
	var width := 0
	var height := 0
	for p in shape:
		width = maxi(width, p.x + 1)
		height = maxi(height, p.y + 1)
	var lines: Array[String] = []
	for y in height:
		var line := ""
		for x in width:
			line += "■" if Vector2i(x, y) in shape else " "
		lines.append(line)
	return " ".join(lines)

func select_piece(piece_id: int) -> void:
	if network.phase == "playing" and (match_state.current_seat() != network.local_seat() or waiting):
		return
	selected_piece = piece_id
	piece_rotation = 0
	flipped = false
	update_preview()

func rotate_piece(amount: int) -> void:
	if selected_piece < 0:
		return
	piece_rotation = posmod(piece_rotation + amount, 4)
	update_preview()

func flip_piece() -> void:
	if selected_piece < 0:
		return
	flipped = not flipped
	update_preview()

func update_preview() -> void:
	if selected_piece < 0 or not game_panel.visible or match_state.finished:
		board_view.show_preview(-1, 0, false, Vector2i.ZERO, 0, false)
		return
	if network.phase == "playing" and (waiting or match_state.current_seat() != network.local_seat()):
		board_view.show_preview(-1, 0, false, Vector2i.ZERO, 0, false)
		return
	preview_origin = board_view.screen_cell(get_viewport().get_mouse_position())
	var reason := BlokusRules.check(match_state.board, match_state.used, match_state.color_turn, selected_piece, piece_rotation, flipped, preview_origin)
	reason_label.text = "配置できます" if reason == "" else reason
	board_view.show_preview(selected_piece, piece_rotation, flipped, preview_origin, match_state.color_turn, reason == "")

func _unhandled_input(event: InputEvent) -> void:
	if not game_panel.visible:
		return
	if event is InputEventKey and event.pressed and not event.echo:
		match event.keycode:
			KEY_Q:
				rotate_piece(-1)
			KEY_E:
				rotate_piece(1)
			KEY_F:
				flip_piece()
			KEY_ESCAPE:
				selected_piece = -1
				update_preview()
	if event is InputEventMouseMotion:
		if board_view.dragging_camera:
			board_view.rotate_camera(event.relative)
		else:
			update_preview()
	if event is InputEventMouseButton:
		if event.button_index == MOUSE_BUTTON_MIDDLE:
			board_view.dragging_camera = event.pressed
		elif event.pressed and event.button_index == MOUSE_BUTTON_WHEEL_UP:
			board_view.zoom(-2)
		elif event.pressed and event.button_index == MOUSE_BUTTON_WHEEL_DOWN:
			board_view.zoom(2)
		elif event.pressed and event.button_index == MOUSE_BUTTON_RIGHT:
			selected_piece = -1
			update_preview()
		elif event.pressed and event.button_index == MOUSE_BUTTON_LEFT and selected_piece >= 0 and not match_state.finished:
			var origin := board_view.screen_cell(event.position)
			if network.phase == "playing":
				if waiting or match_state.current_seat() != network.local_seat():
					return
				waiting = true
				reason_label.text = "サーバーの確定待ち…"
				network.submit(selected_piece, piece_rotation, flipped, origin, match_state.match_id, match_state.revision)
				return
			var result := match_state.play(match_state.current_seat(), selected_piece, piece_rotation, flipped, origin, match_state.match_id, match_state.revision)
			if result == "":
				selected_piece = -1
				board_view.show_board(match_state.board)
				board_view.show_preview(-1, 0, false, Vector2i.ZERO, 0, false)
				refresh_ui()
			else:
				reason_label.text = result
