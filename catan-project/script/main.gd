extends Node2D

@onready var board = $Board
@onready var roll_btn = $GameUI/RollButton
@onready var dice_label = $GameUI/DiceLabel

var intersection_scene = preload("res://scenes/Intersection.tscn")
var edge_scene = preload("res://scenes/Edge.tscn")

# ★ もっと外側に広げたい場合は、この数値を 52.0 や 55.0 に増やしてみてください
var hex_radius_math = 54.0 

func _ready():
	GameManager.dice_rolled.connect(_on_dice_rolled)
	roll_btn.pressed.connect(_on_roll_pressed)
	
	if not multiplayer.is_server():
		roll_btn.disabled = true

	# 角(交差点)と辺の両方を自動配置する
	GameManager.settlement_built.connect(_on_settlement_built)

	call_deferred("_generate_intersections")
	call_deferred("_generate_edges")

func _on_roll_pressed():
	GameManager.request_roll_dice()
	roll_btn.disabled = true

func _on_dice_rolled(roll_value: int):
	dice_label.text = "出目: " + str(roll_value)
	if board.has_method("process_dice_roll"):
		board.process_dice_roll(roll_value)
	
	if multiplayer.is_server():
		await get_tree().create_timer(1.0).timeout
		roll_btn.disabled = false

# 消えてしまっていた「角」を配置する関数
func _generate_intersections():
	var vertices = board.get_unique_vertices(hex_radius_math)
	print("計算された交差点の数: ", vertices.size())

	var container = Node2D.new()
	container.name = "Intersections"
	add_child(container)

	for i in range(vertices.size()):
		var pos = vertices[i]
		var inst = intersection_scene.instantiate()
		inst.position = pos
		inst.name = "Vertex_" + str(i)
		container.add_child(inst)

# 新しく追加した「辺」を配置する関数
func _generate_edges():
	var edges = board.get_unique_edges(hex_radius_math)
	print("計算された辺の数: ", edges.size())

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
		container.add_child(inst)

func _on_settlement_built(vertex_name: String, player_id: int):
	# Intersections フォルダの中から、該当する名前の交差点ノードを探す
	var container = $Intersections
	var vertex_node = container.get_node_or_null(vertex_name)
	
	if vertex_node != null and vertex_node.has_node("Sprite2D"):
		# プレイヤーIDによって色を変える（1=ホスト(赤), それ以外=クライアント(青)）
		if player_id == 1:
			vertex_node.get_node("Sprite2D").modulate = Color.RED
		else:
			vertex_node.get_node("Sprite2D").modulate = Color.BLUE
			
		# 家らしく見せるために少し大きくする
		vertex_node.get_node("Sprite2D").scale = Vector2(2.0, 2.0)
