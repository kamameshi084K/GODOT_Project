extends Node3D

const PIECE_NAMES := ["1", "2", "3直", "3角", "4直", "4T", "4四角", "4L", "4Z", "5F", "5I", "5L", "5P", "5N", "5T", "5U", "5V", "5W", "5X", "5Y", "5Z"]

var match_state := BlokusMatch.new()
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

func _ready() -> void:
	board_view = BlokusBoardView.new()
	add_child(board_view)
	build_ui()
	start_panel.visible = true
	game_panel.visible = false

func build_ui() -> void:
	var ui := Control.new()
	ui.set_anchors_preset(Control.PRESET_FULL_RECT)
	ui.mouse_filter = Control.MOUSE_FILTER_IGNORE
	add_child(ui)
	start_panel = PanelContainer.new()
	start_panel.position = Vector2(390, 150)
	start_panel.custom_minimum_size = Vector2(500, 360)
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
	count_selector = OptionButton.new()
	for count in [2, 3, 4]:
		count_selector.add_item("%d人" % count, count)
	start_content.add_child(count_selector)
	var start_button := Button.new()
	start_button.text = "対戦を始める"
	start_button.pressed.connect(start_local_match)
	start_content.add_child(start_button)
	var instructions := Label.new()
	instructions.text = "青:左上  黄:右上  赤:右下  緑:左下\n駒を選び、盤面を左クリックして配置\nQ/E:回転  F:反転  右クリック/Esc:選択解除\n中ボタンのドラッグ:視点回転  ホイール:ズーム"
	start_content.add_child(instructions)
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
	var new_match := Button.new()
	new_match.text = "人数選択へ戻る"
	new_match.pressed.connect(func():
		game_panel.visible = false
		start_panel.visible = true
		selected_piece = -1
		board_view.show_preview(-1, 0, false, Vector2i.ZERO, 0, false)
	)
	bottom_content.add_child(new_match)

func start_local_match() -> void:
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
	status_label.text = "席%dの手番 / %s%s\n残り %d個・%dマス\n青:左上 黄:右上\n赤:右下 緑:左下" % [seat + 1, BlokusRules.COLORS[color], shared, remaining.pieces, remaining.squares]
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
		return
	result_label.text = match_state.pass_log.back() if not match_state.pass_log.is_empty() else ""
	for piece_id in BlokusRules.PIECES.size():
		var button := Button.new()
		button.custom_minimum_size = Vector2(105, 70)
		button.text = "%s (%d)\n%s" % [PIECE_NAMES[piece_id], BlokusRules.PIECES[piece_id].size(), piece_icon(piece_id)]
		button.disabled = match_state.used[color][piece_id]
		button.pressed.connect(select_piece.bind(piece_id))
		pieces_grid.add_child(button)
	reason_label.text = "駒を選んでください"

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
			var result := match_state.play(match_state.current_seat(), selected_piece, piece_rotation, flipped, origin, match_state.match_id, match_state.revision)
			if result == "":
				selected_piece = -1
				board_view.show_board(match_state.board)
				board_view.show_preview(-1, 0, false, Vector2i.ZERO, 0, false)
				refresh_ui()
			else:
				reason_label.text = result
