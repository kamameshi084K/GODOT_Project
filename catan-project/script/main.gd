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

# ★追加：右側のプレイヤーリスト用
@onready var player_list = $GameUI/PlayerList
var player_ui_scene = preload("res://scenes/player_ui.tscn")
var player_uis = {}

var intersection_scene = preload("res://scenes/Intersection.tscn")
var edge_scene = preload("res://scenes/Edge.tscn")

var hex_radius_math = 54.0 
var is_my_turn = false
var is_moving_robber = false
var robber_scene = preload("res://scenes/Robber.tscn")
var robber_icon: Node2D # Sprite2DからNode2Dに変更（シーンを実体化するため）

func _ready():
	GameManager.dice_rolled.connect(_on_dice_rolled)
	roll_btn.pressed.connect(_on_roll_pressed)
	turn_end_btn.pressed.connect(func(): GameManager.request_end_turn())
	GameManager.turn_changed.connect(_on_turn_changed)
	
	# ★追加：リスト更新シグナルを受信
	GameManager.player_list_updated.connect(_on_player_list_updated)

	roll_btn.disabled = true
	turn_end_btn.disabled = true
	roll_btn.modulate = Color.DIM_GRAY
	turn_end_btn.modulate = Color.DIM_GRAY
	roll_btn.hide()
	
	if not multiplayer.is_server():
		roll_btn.disabled = true
		
	if multiplayer.is_server():
		await get_tree().create_timer(1.0).timeout
		GameManager.start_turn_system()

	GameManager.settlement_built.connect(_on_settlement_built)
	GameManager.road_built.connect(_on_road_built)
	GameManager.resources_updated.connect(_on_resources_updated)
	GameManager.robber_moved.connect(_on_robber_moved)

	call_deferred("_generate_intersections")
	call_deferred("_generate_edges")
	call_deferred("_setup_robber")

func _on_roll_pressed():
	GameManager.rpc_id(1, "request_roll_dice")
	roll_btn.disabled = true
	roll_btn.hide()

func _on_dice_rolled(roll_value: int):
	dice_label.text = "出目: " + str(roll_value)
	if multiplayer.is_server():
		_distribute_resources(roll_value)
	await get_tree().create_timer(1.0).timeout
	
	if is_my_turn:
		if roll_value == 7:
			print("★盗賊を移動させてください！")
			is_moving_robber = true
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
	
	if vertex_node != null and vertex_node.has_node("Sprite2D"):
		if player_id == 1:
			vertex_node.get_node("Sprite2D").modulate = Color.RED
		else:
			vertex_node.get_node("Sprite2D").modulate = Color.BLUE
		vertex_node.get_node("Sprite2D").scale = Vector2(0.25, 0.25)
		
	# ★追加：家が建ったら右側のUIのVPを増やす！
	if player_id != multiplayer.get_unique_id() and player_uis.has(player_id):
		player_uis[player_id].add_vp(1)
		
func _on_road_built(edge_name: String, player_id: int):
	var container = $Edges
	var edge_node = container.get_node_or_null(edge_name)
	if edge_node != null and edge_node.has_node("ColorRect"):
		if player_id == 1:
			edge_node.get_node("ColorRect").color = Color.RED
		else:
			edge_node.get_node("ColorRect").color = Color.BLUE
		var rect = edge_node.get_node("ColorRect")
		rect.size.y = 8
		rect.position.y = -4
		
func _on_resources_updated(player_id: int, wood: int, brick: int, sheep: int, wheat: int, ore: int):
	if player_id == multiplayer.get_unique_id():
		wood_label.text = str(wood)
		brick_label.text = str(brick)
		sheep_label.text = str(sheep)
		wheat_label.text = str(wheat)
		ore_label.text = str(ore)
	else:
		# ★追加：相手なら右側のUIの手札枚数を更新する！
		if player_uis.has(player_id):
			var total_hand = wood + brick + sheep + wheat + ore
			player_uis[player_id].update_hand(total_hand)

func _distribute_resources(roll: int):
	if roll == 7: return
	for tile in board.get_children():
		var tile_num = tile.get("number") 
		var res_type_int = tile.get("tile_type")    
		if tile_num != null and tile_num == roll:
			var type_str = ""
			match res_type_int:
				0: type_str = "wood"
				1: type_str = "brick"
				2: type_str = "sheep"
				3: type_str = "wheat"
				4: type_str = "ore"
				_: continue
			GameManager.distribute_resources_for_hex(tile.position, hex_radius_math, type_str)

func _on_turn_changed(active_player_id: int):
	is_my_turn = (active_player_id == multiplayer.get_unique_id())
	if is_my_turn:
		roll_btn.show()
		roll_btn.disabled = false
		roll_btn.modulate = Color.LIGHT_SKY_BLUE
		turn_end_btn.disabled = true
		turn_end_btn.modulate = Color.DIM_GRAY
	else:
		roll_btn.hide()
		roll_btn.disabled = true
		turn_end_btn.disabled = true
		turn_end_btn.modulate = Color.DIM_GRAY
		roll_btn.modulate = Color.DIM_GRAY

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

# ★変更：引数に victims を追加し、奪う処理を実装
func _on_robber_moved(pos: Vector2, victims: Array):
	for tile in board.get_children():
		if tile.position.distance_to(pos) < 1.0:
			if robber_icon.get_parent() != null:
				robber_icon.get_parent().remove_child(robber_icon)
			tile.add_child(robber_icon)
			robber_icon.position = Vector2(-15, -15)
			
			if is_my_turn:
				if victims.size() > 0:
					print("★奪える相手のUIが光ります！クリックして奪ってください。")
					for vid in victims:
						if player_uis.has(vid):
							player_uis[vid].set_stealable(true) # UIを光らせてクリック解禁！
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
			break

# ★追加：右側のリストを作成する処理
func _on_player_list_updated(player_info_list: Array):
	for child in player_list.get_children():
		child.queue_free()
	player_uis.clear()
	
	var my_id = multiplayer.get_unique_id()
	for info in player_info_list:
		var pid = info["id"]
		# 自分以外だったらUIを作る
		if pid != my_id:
			var ui = player_ui_scene.instantiate()
			player_list.add_child(ui)
			# 発展カード枚数も含めてセットアップ
			ui.setup(pid, info["turn_index"], info["name"], info["vp"], info["hand_count"], info["dev_cards"])
			ui.steal_requested.connect(_on_steal_requested)
			player_uis[pid] = ui

# ★追加：光ったUIをクリックした時の処理
func _on_steal_requested(target_id: int):
	GameManager.request_steal(target_id)
	# 奪い終わったので全プレイヤーの光を消す
	for ui in player_uis.values():
		ui.set_stealable(false)
		
	turn_end_btn.disabled = false
	turn_end_btn.modulate = Color.LIGHT_SKY_BLUE
