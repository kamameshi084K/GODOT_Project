extends Node2D

@onready var board = $Board
@onready var roll_btn = $GameUI/RollButton
@onready var dice_label = $GameUI/DiceLabel

@onready var wood_label = $GameUI/ResourceCards/WoodCard/CountLabel
@onready var brick_label = $GameUI/ResourceCards/BrickCard/CountLabel
@onready var sheep_label = $GameUI/ResourceCards/SheepCard/CountLabel
@onready var wheat_label = $GameUI/ResourceCards/WheatCard/CountLabel
@onready var ore_label = $GameUI/ResourceCards/OreCard/CountLabel
@onready var turn_end_btn = $GameUI/TurnEndButton
@onready var open_trade_btn = $GameUI/OpenTradeButton
@onready var trade_ui = $GameUI/TradeUI
@onready var open_dev_btn = $GameUI/OpenDevCardButton
@onready var dev_ui = $GameUI/DevCardUI
@onready var discard_ui = $GameUI/DiscardUI
@onready var resource_select_ui = $GameUI/ResourceSelectUI
@onready var message_label = $GameUI/MessageWindow/MessageLabel
@onready var trade_list_ui = $GameUI/TradeListUI
@onready var open_player_trade_btn = $GameUI/OpenPlayerTradeButton
@onready var player_trade_ui = $GameUI/PlayerTradeUI
@onready var trade_accept_ui = $GameUI/TradeAcceptUI
@onready var dice_viewport = $GameUI/DiceViewport

# チュートリアル用：MessageLabel の子に置いた NextButton
@onready var tutorial_next_btn = get_node_or_null("GameUI/MessageWindow/MessageLabel/NextButton")

# 右側のプレイヤーリスト用
@onready var player_list = $GameUI/PlayerList
var player_ui_scene = preload("res://scenes/Player_ui.tscn")
var player_uis = {}

var intersection_scene = preload("res://scenes/Intersection.tscn")
var edge_scene = preload("res://scenes/Edge.tscn")

var hex_radius_math = 54.0
var is_my_turn = false
var is_moving_robber = false
var robber_scene = preload("res://scenes/Robber.tscn")
var robber_icon: Node2D
var free_roads_left = 0

var current_largest_army_player = 0
var current_longest_road_player = 0

var player_colors = {}
var base_colors = [Color.RED, Color.BLUE, Color.GREEN, Color.PURPLE]
var current_phase = 0

var tutorial_mode := false
var tutorial_controller_script = preload("res://script/tutorial_controller.gd")
var tutorial_controller
var tutorial_marker: Node = null


func _ready():
	tutorial_mode = GameManager.has_meta("tutorial_mode") and GameManager.get_meta("tutorial_mode") == true
	
	GameManager.dice_rolled.connect(_on_dice_rolled)
	roll_btn.pressed.connect(_on_roll_pressed)
	turn_end_btn.pressed.connect(_on_turn_end_pressed)
	GameManager.turn_changed.connect(_on_turn_changed)
	
	GameManager.player_list_updated.connect(_on_player_list_updated)

	roll_btn.disabled = true
	turn_end_btn.disabled = true
	roll_btn.modulate = Color.DIM_GRAY
	turn_end_btn.modulate = Color.DIM_GRAY
	roll_btn.hide()
	dice_viewport.hide()
	
	if tutorial_next_btn != null:
		tutorial_next_btn.hide()
	
	if not multiplayer.is_server():
		roll_btn.disabled = true
		
	if multiplayer.is_server():
		await get_tree().create_timer(1.0).timeout
		GameManager.start_turn_system()

	GameManager.settlement_built.connect(_on_settlement_built)
	GameManager.road_built.connect(_on_road_built)
	GameManager.resources_updated.connect(_on_resources_updated)
	GameManager.robber_moved.connect(_on_robber_moved)
	GameManager.city_built.connect(_on_city_built)
	GameManager.prompt_knight_robber.connect(_on_prompt_knight_robber)
	GameManager.prompt_discard.connect(_on_prompt_discard)
	GameManager.notify_robber_phase.connect(_on_notify_robber_phase)
	GameManager.prompt_road_building.connect(_on_prompt_road_building)
	GameManager.game_won.connect(_on_game_won)
	GameManager.trade_proposed.connect(_on_trade_proposed)
	GameManager.trade_accepted_by_someone.connect(_on_trade_accepted_by_someone)
	GameManager.trade_completed.connect(_on_trade_completed)
	GameManager.largest_army_changed.connect(_on_largest_army_changed)
	GameManager.longest_road_changed.connect(_on_longest_road_changed)
	GameManager.player_disconnected.connect(_on_player_disconnected)
	GameManager.player_reconnected.connect(_on_player_reconnected)
	GameManager.full_state_received.connect(_on_full_state_received)
	
	open_player_trade_btn.pressed.connect(func(): player_trade_ui.show())
	open_player_trade_btn.disabled = true
	player_trade_ui.hide()
	trade_accept_ui.hide()
	trade_list_ui.hide()
	discard_ui.hide()

	call_deferred("_generate_intersections")
	call_deferred("_generate_edges")
	call_deferred("_setup_robber")
	call_deferred("_setup_ports")

	if tutorial_mode:
		await get_tree().process_frame
		await get_tree().process_frame
		debug_print_board_ids()
	
	open_trade_btn.pressed.connect(_on_open_trade_pressed)
	open_trade_btn.disabled = true
	trade_ui.hide()
	
	open_dev_btn.pressed.connect(func(): dev_ui.show())
	open_dev_btn.disabled = true
	dev_ui.hide()
	
	resource_select_ui.hide()
	
	dev_ui.play_mono_pressed.connect(func():
		resource_select_ui.setup("monopoly")
		dev_ui.hide()
	)
	dev_ui.play_plenty_pressed.connect(func():
		resource_select_ui.setup("plenty")
		dev_ui.hide()
	)
	
	if tutorial_mode:
		tutorial_controller = tutorial_controller_script.new()
		add_child(tutorial_controller)
		tutorial_controller.setup(self)
	else:
		show_info("カタンへようこそ！あなたのターンを待っています...")
	
	if GameManager.has_meta("reconnect_name"):
		var r_name = GameManager.get_meta("reconnect_name")
		GameManager.remove_meta("reconnect_name")
		_request_reconnect_delayed(r_name)


func _on_roll_pressed():
	if tutorial_mode and tutorial_controller != null and tutorial_controller.has_method("request_tutorial_roll"):
		tutorial_controller.request_tutorial_roll()
		return
	
	roll_btn.disabled = true
	GameManager.rpc_id(1, "request_roll_dice")

func _on_turn_end_pressed():
	if tutorial_mode and tutorial_controller != null:
		if tutorial_controller.has_method("notify_turn_end_pressed"):
			tutorial_controller.notify_turn_end_pressed()
			return
	
	GameManager.request_end_turn()

func _on_dice_rolled(d1: int, d2: int):
	var roll_value = d1 + d2
	
	if dice_viewport:
		dice_viewport.show()
		var die1 = dice_viewport.get_node("SubViewport/Die1")
		var die2 = dice_viewport.get_node("SubViewport/Die2")
		die1.start_roll()
		die2.start_roll()
		dice_label.text = "🎲 振っています..."
		
	await get_tree().create_timer(1.0).timeout
	
	if dice_viewport:
		var die1 = dice_viewport.get_node("SubViewport/Die1")
		var die2 = dice_viewport.get_node("SubViewport/Die2")
		
		die1.stop_roll_to_value(d1, 0.5)
		die2.stop_roll_to_value(d2, 0.5)
		
		await get_tree().create_timer(2.0).timeout
		dice_viewport.hide()
	
	dice_label.text = "出目: " + str(roll_value)
	
	if multiplayer.is_server():
		_distribute_resources(roll_value)
	
	if is_my_turn:
		roll_btn.hide()
		open_trade_btn.disabled = false
		open_dev_btn.disabled = false
		open_player_trade_btn.disabled = false
		
		if roll_value == 7:
			show_info("★7が出ました！全員のバースト処理を待っています...")
			turn_end_btn.disabled = true
			turn_end_btn.modulate = Color.DIM_GRAY
		else:
			turn_end_btn.disabled = false
			turn_end_btn.modulate = Color.LIGHT_SKY_BLUE


func _generate_intersections():
	var vertices = board.get_unique_vertices(hex_radius_math)
	var container = Node2D.new()
	container.name = "Intersections"
	add_child(container)

	for i in range(vertices.size()):
		var pos = vertices[i]
		var inst = intersection_scene.instantiate()
		inst.position = pos
		inst.name = "Vertex_" + str(i)
		GameManager.register_vertex(inst.name, pos)
		container.add_child(inst)


func _generate_edges():
	var edges = board.get_unique_edges(hex_radius_math)
	var container = Node2D.new()
	container.name = "Edges"
	add_child(container)

	for i in range(edges.size()):
		var edge_data = edges[i]
		var inst = edge_scene.instantiate()
		inst.position = edge_data["midpoint"]
		var vec = edge_data["end"] - edge_data["start"]
		inst.rotation = vec.angle()
		inst.name = "Edge_" + str(i)
		GameManager.register_edge(inst.name, edge_data["midpoint"])
		container.add_child(inst)


func _on_settlement_built(vertex_name: String, player_id: int):
	var container = $Intersections
	var vertex_node = container.get_node_or_null(vertex_name)
	
	if vertex_node != null:
		var c = player_colors.get(player_id, Color.WHITE)
		vertex_node.update_building(player_id, 1, c)
		
		if multiplayer.is_server() and current_phase == 1:
			for tile in board.get_children():
				if tile.position.distance_to(vertex_node.position) < (hex_radius_math + 5.0):
					var res_type_int = tile.get("tile_type")
					var type_str = ""
					
					match res_type_int:
						0:
							type_str = "wood"
						1:
							type_str = "brick"
						2:
							type_str = "sheep"
						3:
							type_str = "wheat"
						4:
							type_str = "ore"
					
					if type_str != "":
						GameManager.add_resource(player_id, type_str, 1)
		
	if player_id != multiplayer.get_unique_id() and player_uis.has(player_id):
		player_uis[player_id].add_vp(1)
		

func tutorial_give_initial_resources_from_vertex(vertex_name: String, player_id: int):
	if not has_node("Intersections"):
		return
	
	var vertex_node = $Intersections.get_node_or_null(vertex_name)
	
	if vertex_node == null:
		print("Tutorial initial resource vertex not found: ", vertex_name)
		return
	
	for tile in board.get_children():
		if tile.position.distance_to(vertex_node.position) < (hex_radius_math + 5.0):
			var res_type_int = tile.get("tile_type")
			var type_str = ""
			
			match res_type_int:
				0:
					type_str = "wood"
				1:
					type_str = "brick"
				2:
					type_str = "sheep"
				3:
					type_str = "wheat"
				4:
					type_str = "ore"
				_:
					type_str = ""
			
			if type_str != "":
				GameManager.add_resource(player_id, type_str, 1)
				print("Tutorial initial resource: ", type_str, " to player ", player_id)


func _on_road_built(edge_name: String, player_id: int):
	var container = $Edges
	var edge_node = container.get_node_or_null(edge_name)
	
	if edge_node != null:
		var c = player_colors.get(player_id, Color.WHITE)
		edge_node.build_road(player_id, c)
		
	if player_id == multiplayer.get_unique_id() and free_roads_left > 0:
		free_roads_left -= 1
		
		if free_roads_left > 0:
			show_info("★ 街道建設：無料で引ける道は残り【1本】です！")
		else:
			show_info("★ 街道建設：無料分の道をすべて引き終わりました！")

func _on_prompt_road_building():
	free_roads_left = 2
	show_info("★ 街道建設：無料で道を【2本】引けます！好きな場所をクリックしてください。")
	dev_ui.hide()

func _on_resources_updated(player_id: int, wood: int, brick: int, sheep: int, wheat: int, ore: int):
	if player_id == multiplayer.get_unique_id():
		wood_label.text = str(wood)
		brick_label.text = str(brick)
		sheep_label.text = str(sheep)
		wheat_label.text = str(wheat)
		ore_label.text = str(ore)
	else:
		if player_uis.has(player_id):
			var total_hand = wood + brick + sheep + wheat + ore
			player_uis[player_id].update_hand(total_hand)


func _distribute_resources(roll: int):
	if roll == 7:
		return
	
	for tile in board.get_children():
		var tile_num = tile.get("number")
		var res_type_int = tile.get("tile_type")
		
		if tile_num != null and tile_num == roll:
			var type_str = ""
			
			match res_type_int:
				0:
					type_str = "wood"
				1:
					type_str = "brick"
				2:
					type_str = "sheep"
				3:
					type_str = "wheat"
				4:
					type_str = "ore"
				_:
					continue
			
			GameManager.distribute_resources_for_hex(tile.position, hex_radius_math, type_str)


func _on_turn_changed(active_player_id: int, phase: int = 2):
	current_phase = phase
	is_my_turn = (active_player_id == multiplayer.get_unique_id())
	
	var active_name = "Player " + str(active_player_id)
	
	if is_my_turn:
		active_name = "あなた"
	elif player_uis.has(active_player_id):
		active_name = player_uis[active_player_id].name_label.text
	
	if not tutorial_mode:
		if phase == 0 or phase == 1:
			show_info("【初期配置】 " + active_name + " のターンです！")
		else:
			show_info("🎲 " + active_name + " のターンです！")
	
	if is_my_turn:
		if phase == 0 or phase == 1:
			open_trade_btn.disabled = true
			open_dev_btn.disabled = true
			open_player_trade_btn.disabled = true
		else:
			roll_btn.show()
			roll_btn.disabled = false
			roll_btn.modulate = Color.LIGHT_SKY_BLUE
			open_trade_btn.disabled = true
			open_dev_btn.disabled = true
			open_player_trade_btn.disabled = true
			turn_end_btn.disabled = true
			turn_end_btn.modulate = Color.DIM_GRAY
	else:
		open_trade_btn.disabled = true
		open_dev_btn.disabled = true
		open_player_trade_btn.disabled = true
		trade_ui.hide()
		dev_ui.hide()
		player_trade_ui.hide()


func _input(event):
	if event is InputEventMouseButton and event.pressed and event.button_index == MOUSE_BUTTON_LEFT:
		if is_my_turn and is_moving_robber:
			var mouse_pos = board.get_local_mouse_position()
			var closest_tile = null
			var min_dist = 999.0
			
			for tile in board.get_children():
				var dist = tile.position.distance_to(mouse_pos)
				
				if dist < min_dist:
					min_dist = dist
					closest_tile = tile
			
			if min_dist < hex_radius_math and closest_tile != null:
				GameManager.request_move_robber(closest_tile.position)
				is_moving_robber = false


func _on_robber_moved(pos: Vector2, victims: Array):
	for tile in board.get_children():
		if tile.position.distance_to(pos) < 1.0:
			if robber_icon.get_parent() != null:
				robber_icon.get_parent().remove_child(robber_icon)
			
			tile.add_child(robber_icon)
			robber_icon.position = Vector2(-15, -15)
			
			if is_my_turn:
				if victims.size() > 0:
					show_info("★奪える相手のUIが光ります！クリックして奪ってください。")
					
					for vid in victims:
						if player_uis.has(vid):
							player_uis[vid].set_stealable(true)
				else:
					turn_end_btn.disabled = false
					turn_end_btn.modulate = Color.LIGHT_SKY_BLUE
			
			break


func _setup_robber():
	robber_icon = robber_scene.instantiate()
	robber_icon.name = "Robber"
	
	for tile in board.get_children():
		var res_type_int = tile.get("tile_type")
		
		if res_type_int == 5:
			tile.add_child(robber_icon)
			robber_icon.position = Vector2(-15, -15)
			
			if multiplayer.is_server():
				GameManager.set_initial_robber_pos(tile.position)
			
			break


func _on_player_list_updated(player_info_list: Array):
	for child in player_list.get_children():
		child.queue_free()
	
	player_uis.clear()
	player_colors.clear()
	
	var my_id = multiplayer.get_unique_id()
	
	for info in player_info_list:
		var pid = info["id"]
		var t_idx = info["turn_index"]
		
		player_colors[pid] = base_colors[t_idx % 4]
		
		if pid != my_id:
			var ui = player_ui_scene.instantiate()
			player_list.add_child(ui)
			ui.setup(pid, info["turn_index"], info["name"], info["vp"], info["hand_count"], info["dev_cards"])
			ui.steal_requested.connect(_on_steal_requested)
			player_uis[pid] = ui

	if has_node("Intersections"):
		for v_node in $Intersections.get_children():
			if v_node.owner_id != 0:
				var c = player_colors.get(v_node.owner_id, Color.WHITE)
				v_node.update_building(v_node.owner_id, v_node.building_level, c)
	
	if has_node("Edges"):
		for e_node in $Edges.get_children():
			if e_node.owner_id != 0:
				var c = player_colors.get(e_node.owner_id, Color.WHITE)
				e_node.build_road(e_node.owner_id, c)


func _on_steal_requested(target_id: int):
	GameManager.request_steal(target_id)
	
	for ui in player_uis.values():
		ui.set_stealable(false)
		
	turn_end_btn.disabled = false
	turn_end_btn.modulate = Color.LIGHT_SKY_BLUE


func _on_city_built(vertex_name: String, player_id: int):
	var container = $Intersections
	var vertex_node = container.get_node_or_null(vertex_name)
	
	if vertex_node != null:
		var c = player_colors.get(player_id, Color.WHITE)
		vertex_node.update_building(player_id, 2, c)
		
	if player_id != multiplayer.get_unique_id() and player_uis.has(player_id):
		player_uis[player_id].add_vp(1)


func _on_prompt_knight_robber():
	show_info("★ 騎士カードを使用しました！盗賊を移動させてください。")
	dev_ui.hide()
	is_moving_robber = true
	turn_end_btn.disabled = true
	turn_end_btn.modulate = Color.DIM_GRAY


func _on_prompt_discard(amount: int, w: int = 0, b: int = 0, s: int = 0, wh: int = 0, o: int = 0):
	show_info("★ バーストしました！ " + str(amount) + " 枚捨ててください。")
	discard_ui.setup(amount, w, b, s, wh, o)


func _on_notify_robber_phase():
	show_info("★ 全員のバースト処理が完了しました！盗賊を移動させてください。")
	is_moving_robber = true
	turn_end_btn.disabled = true
	turn_end_btn.modulate = Color.DIM_GRAY


func show_info(msg: String):
	if message_label != null:
		message_label.text = msg
	
	print(msg)


func show_tutorial(msg: String, can_next: bool = false):
	show_info(msg)
	
	if tutorial_next_btn != null:
		tutorial_next_btn.visible = can_next
		
func _on_open_trade_pressed():
	trade_ui.show()
	
	if tutorial_mode and tutorial_controller != null:
		if tutorial_controller.has_method("notify_trade_opened"):
			tutorial_controller.notify_trade_opened()


func tutorial_enable_bank_trade():
	open_trade_btn.disabled = false
	open_trade_btn.modulate = Color.LIGHT_SKY_BLUE


func tutorial_disable_bank_trade():
	open_trade_btn.disabled = true
	open_trade_btn.modulate = Color.DIM_GRAY
	trade_ui.hide()



func tutorial_distribute_resources_for_roll(roll: int, player_id: int) -> Dictionary:
	var gained := {
		"wood": 0,
		"brick": 0,
		"sheep": 0,
		"wheat": 0,
		"ore": 0
	}
	
	if not has_node("Intersections"):
		return gained
	
	for vertex_node in $Intersections.get_children():
		if not ("owner_id" in vertex_node):
			continue
		
		if vertex_node.owner_id != player_id:
			continue
		
		var building_level = 1
		
		if "building_level" in vertex_node:
			building_level = vertex_node.building_level
		
		for tile in board.get_children():
			var tile_num = tile.get("number")
			
			if tile_num == null:
				continue
			
			if tile_num != roll:
				continue
			
			if tile.position.distance_to(vertex_node.position) < (hex_radius_math + 5.0):
				var res_type_int = tile.get("tile_type")
				var resource_name = tutorial_tile_type_to_resource(res_type_int)
				
				if resource_name == "":
					continue
				
				var amount = 1
				
				if building_level >= 2:
					amount = 2
				
				GameManager.add_resource(player_id, resource_name, amount)
				gained[resource_name] += amount
				
				print("Tutorial resource: ", resource_name, " +", amount, " from ", vertex_node.name)
	
	return gained


func tutorial_tile_type_to_resource(tile_type: int) -> String:
	match tile_type:
		0:
			return "wood"
		1:
			return "brick"
		2:
			return "sheep"
		3:
			return "wheat"
		4:
			return "ore"
		_:
			return ""

func setup_tutorial_enemies():
	tutorial_add_enemy_player(1001, "敵A", 1, Color.BLUE)
	tutorial_add_enemy_player(1002, "敵B", 2, Color.GREEN)
	tutorial_add_enemy_player(1003, "敵C", 3, Color.PURPLE)


func tutorial_add_enemy_player(player_id: int, player_name: String, turn_index: int, color: Color):
	player_colors[player_id] = color
	
	if player_uis.has(player_id):
		return
	
	var ui = player_ui_scene.instantiate()
	player_list.add_child(ui)
	ui.setup(player_id, turn_index, player_name, 0, 0, 0)
	
	if ui.has_signal("steal_requested"):
		ui.steal_requested.connect(_on_steal_requested)
	
	player_uis[player_id] = ui

func debug_print_board_ids():
	print("========== DEBUG BOARD IDS ==========")

	if has_node("Intersections"):
		print("--- Intersections / 家を置く場所 ---")
		for v in $Intersections.get_children():
			print(v.name, " position=", v.position, " global=", v.global_position)
	else:
		print("Intersections が見つかりません")

	if has_node("Edges"):
		print("--- Edges / 道を置く場所 ---")
		for e in $Edges.get_children():
			print(e.name, " position=", e.position, " global=", e.global_position)
	else:
		print("Edges が見つかりません")

	print("========== END DEBUG BOARD IDS ==========")


func debug_show_board_ids_on_map():
	if has_node("Intersections"):
		for v in $Intersections.get_children():
			var label = Label.new()
			label.text = v.name
			label.position = Vector2(-24, -34)
			label.scale = Vector2(0.45, 0.45)
			label.z_index = 300
			label.modulate = Color.YELLOW
			v.add_child(label)

	if has_node("Edges"):
		for e in $Edges.get_children():
			var label = Label.new()
			label.text = e.name
			label.position = Vector2(-28, -20)
			label.scale = Vector2(0.38, 0.38)
			label.z_index = 300
			label.modulate = Color.CYAN
			e.add_child(label)


func _on_game_won(winner_id: int):
	if winner_id == multiplayer.get_unique_id():
		show_info("おめでとうございます！あなたが10点に到達して勝利しました！ 🎉")
	else:
		var winner_name = "Player " + str(winner_id)
		
		if player_uis.has(winner_id):
			winner_name = player_uis[winner_id].name_label.text
		
		show_info("残念！ " + winner_name + " が10点に到達し、ゲームに勝利しました！")
		
	turn_end_btn.disabled = true
	roll_btn.disabled = true
	open_trade_btn.disabled = true
	open_dev_btn.disabled = true
	turn_end_btn.modulate = Color.DIM_GRAY
	roll_btn.modulate = Color.DIM_GRAY


func _setup_ports():
	var ports_container = get_node_or_null("Ports")
	
	if ports_container == null:
		return
	
	var intersections = $Intersections.get_children()
	var all_descendants = ports_container.find_children("*", "", true, false)
	
	for port in all_descendants:
		if "port_type" in port:
			var distances = []
			
			for vertex in intersections:
				var dist = port.global_position.distance_to(vertex.global_position)
				distances.append({"node": vertex, "dist": dist})
			
			distances.sort_custom(func(a, b): return a["dist"] < b["dist"])
			
			var v1 = distances[0]["node"]
			GameManager.register_port(v1.name, port.port_type)


func _on_trade_proposed(proposer_id: int, gw: int, gb: int, gs: int, gwh: int, go: int, ww: int, wb: int, ws: int, wwh: int, wo: int):
	if proposer_id == multiplayer.get_unique_id():
		trade_list_ui.setup()
		show_info("★ トレードを提案しました。他のプレイヤーの返答を待っています...")
		return
		
	var proposer_name = "Player " + str(proposer_id)
	
	if player_uis.has(proposer_id):
		proposer_name = player_uis[proposer_id].name_label.text
		
	trade_accept_ui.setup(proposer_name, gw, gb, gs, gwh, go, ww, wb, ws, wwh, wo)


func _on_trade_accepted_by_someone(accepter_id: int):
	var accepter_name = "Player " + str(accepter_id)
	
	if player_uis.has(accepter_id):
		accepter_name = player_uis[accepter_id].name_label.text
	
	trade_list_ui.add_accepter(accepter_id, accepter_name)


func _on_trade_completed():
	trade_accept_ui.hide()
	player_trade_ui.hide()
	trade_list_ui.hide()
	show_info("★ プレイヤートレードが成立（またはキャンセル）しました！")


func _on_largest_army_changed(player_id: int):
	if current_largest_army_player != 0 and current_largest_army_player != player_id:
		if player_uis.has(current_largest_army_player):
			player_uis[current_largest_army_player].add_vp(-2)

	if player_id != 0:
		if player_uis.has(player_id):
			player_uis[player_id].add_vp(2)
			
		var p_name = "Player " + str(player_id)
		
		if player_uis.has(player_id):
			p_name = player_uis[player_id].name_label.text
		elif player_id == multiplayer.get_unique_id():
			p_name = "あなた"
			
		show_info("⚔️ " + p_name + " が【最大騎士力】(2VP) を獲得しました！")
	
	current_largest_army_player = player_id


func _on_longest_road_changed(player_id: int):
	if current_longest_road_player != 0 and current_longest_road_player != player_id:
		if player_uis.has(current_longest_road_player):
			player_uis[current_longest_road_player].add_vp(-2)

	if player_id != 0:
		if player_uis.has(player_id):
			player_uis[player_id].add_vp(2)
			
		var p_name = "Player " + str(player_id)
		
		if player_uis.has(player_id):
			p_name = player_uis[player_id].name_label.text
		elif player_id == multiplayer.get_unique_id():
			p_name = "あなた"
			
		show_info("🛣️ " + p_name + " が【最長交易路】(2VP) を獲得しました！")
	else:
		show_info("🛣️ 【最長交易路】が分断され、該当者なしになりました！")
		
	current_longest_road_player = player_id


func _on_player_disconnected(player_name: String):
	show_info("⚠️ 通信エラー: " + player_name + " が切断されました。復帰するまでターンをスキップします。")


func _on_player_reconnected(old_id: int, new_id: int, p_name: String):
	show_info("✅ " + p_name + " が再接続しました！ (ID: " + str(old_id) + " -> " + str(new_id) + ")")


func _on_full_state_received(state: Dictionary):
	show_info("🔄 最新の盤面データを復元しています...")
	
	var vertices = state["vertices"]
	
	for v in vertices:
		var v_node = $Intersections.get_node_or_null(v["name"])
		
		if v_node:
			var c = player_colors.get(v["owner"], Color.WHITE)
			v_node.update_building(v["owner"], v["type"], c)
			
	var edges = state["edges"]
	
	for e in edges:
		var e_node = $Edges.get_node_or_null(e["name"])
		
		if e_node:
			var c = player_colors.get(e["owner"], Color.WHITE)
			e_node.build_road(e["owner"], c)
			
	var r_pos = state["robber_pos"]
	
	if r_pos != Vector2(-9999, -9999):
		if robber_icon.get_parent() != null:
			robber_icon.get_parent().remove_child(robber_icon)
		
		for tile in board.get_children():
			if tile.position.distance_to(r_pos) < 1.0:
				tile.add_child(robber_icon)
				robber_icon.position = Vector2(-15, -15)
				break


func _request_reconnect_delayed(r_name: String):
	await get_tree().create_timer(0.5).timeout
	GameManager.request_reconnect(r_name)

func clear_tutorial_marker():
	if tutorial_marker != null and is_instance_valid(tutorial_marker):
		tutorial_marker.queue_free()
	
	tutorial_marker = null


func show_tutorial_target_vertex(vertex_name: String):
	clear_tutorial_marker()
	
	if not has_node("Intersections"):
		print("Intersections がまだありません")
		return
	
	var vertex_node = $Intersections.get_node_or_null(vertex_name)
	
	if vertex_node == null:
		print("Tutorial target vertex not found: ", vertex_name)
		return
	
	print("Tutorial target vertex: ", vertex_name, " position=", vertex_node.position, " global=", vertex_node.global_position)
	
	var marker = Line2D.new()
	marker.name = "TutorialTargetMarker"
	marker.width = 6.0
	marker.default_color = Color.RED
	marker.z_index = 1000
	
	var radius = 30.0
	var segments = 48
	
	for i in range(segments + 1):
		var angle = TAU * float(i) / float(segments)
		var point = Vector2(cos(angle), sin(angle)) * radius
		marker.add_point(point)
	
	vertex_node.add_child(marker)
	tutorial_marker = marker


func show_tutorial_target_edge(edge_name: String):
	clear_tutorial_marker()
	
	if not has_node("Edges"):
		print("Edges がまだありません")
		return
	
	var edge_node = $Edges.get_node_or_null(edge_name)
	
	if edge_node == null:
		print("Tutorial target edge not found: ", edge_name)
		return
	
	print("Tutorial target edge: ", edge_name, " position=", edge_node.position, " global=", edge_node.global_position)
	
	var marker = Line2D.new()
	marker.name = "TutorialTargetMarker"
	marker.width = 6.0
	marker.default_color = Color.RED
	marker.z_index = 1000
	
	var radius = 26.0
	var segments = 48
	
	for i in range(segments + 1):
		var angle = TAU * float(i) / float(segments)
		var point = Vector2(cos(angle), sin(angle)) * radius
		marker.add_point(point)
	
	edge_node.add_child(marker)
	tutorial_marker = marker


func tutorial_can_build_settlement(vertex_name: String) -> bool:
	if not tutorial_mode:
		return true
	
	if tutorial_controller == null:
		return true
	
	if tutorial_controller.has_method("can_build_settlement"):
		return tutorial_controller.can_build_settlement(vertex_name)
	
	return true

func tutorial_force_build_settlement(vertex_name: String, player_id: int):
	_on_settlement_built(vertex_name, player_id)
	
	if tutorial_controller != null and tutorial_controller.has_method("notify_settlement_built"):
		tutorial_controller.notify_settlement_built(vertex_name, player_id)

func tutorial_can_build_city(vertex_name: String) -> bool:
	if not tutorial_mode:
		return true
	
	if tutorial_controller == null:
		return true
	
	if tutorial_controller.has_method("can_build_city"):
		return tutorial_controller.can_build_city(vertex_name)
	
	return true


func tutorial_force_build_city(vertex_name: String, player_id: int):
	_on_city_built(vertex_name, player_id)
	
	if tutorial_controller != null and tutorial_controller.has_method("notify_city_built"):
		tutorial_controller.notify_city_built(vertex_name, player_id)

func tutorial_force_build_road(edge_name: String, player_id: int):
	_on_road_built(edge_name, player_id)
	
	if tutorial_controller != null and tutorial_controller.has_method("notify_road_built"):
		tutorial_controller.notify_road_built(edge_name, player_id)

func tutorial_can_build_road(edge_name: String) -> bool:
	if not tutorial_mode:
		return true
	
	if tutorial_controller == null:
		return true
	
	if tutorial_controller.has_method("can_build_road"):
		return tutorial_controller.can_build_road(edge_name)
	
	return true
