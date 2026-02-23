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

var intersection_scene = preload("res://scenes/Intersection.tscn")
var edge_scene = preload("res://scenes/Edge.tscn")

# ★ もっと外側に広げたい場合は、この数値を 52.0 や 55.0 に増やしてみてください
var hex_radius_math = 54.0 

var is_my_turn = false

func _ready():
	GameManager.dice_rolled.connect(_on_dice_rolled)
	roll_btn.pressed.connect(_on_roll_pressed)
	
	# ★追加1: ターン終了ボタンを押したら、C++に「ターン終わるよ」と伝える
	turn_end_btn.pressed.connect(func(): GameManager.request_end_turn())
	
	# ★追加2: C++から「ターンが変わったよ」という通知を受け取る
	GameManager.turn_changed.connect(_on_turn_changed)

	# ★追加3: 最初は全員、ボタンを押せなくして暗い色にしておく
	roll_btn.disabled = true
	turn_end_btn.disabled = true
	roll_btn.modulate = Color.DIM_GRAY
	turn_end_btn.modulate = Color.DIM_GRAY
	
	if not multiplayer.is_server():
		roll_btn.disabled = true
		
	if multiplayer.is_server():
		await get_tree().create_timer(1.0).timeout
		GameManager.start_turn_system()

	# 角(交差点)と辺の両方を自動配置する
	GameManager.settlement_built.connect(_on_settlement_built)
	GameManager.road_built.connect(_on_road_built)
	GameManager.resources_updated.connect(_on_resources_updated)

	call_deferred("_generate_intersections")
	call_deferred("_generate_edges")
	
	

func _on_roll_pressed():
	GameManager.rpc_id(1, "request_roll_dice")
	roll_btn.disabled = true

func _on_dice_rolled(roll_value: int):
	dice_label.text = "出目: " + str(roll_value)
	
	if multiplayer.is_server():
		_distribute_resources(roll_value)
		
	# ★ 修正：インデント（字下げ）を左に戻して、サーバー以外もここを実行するようにする！
	await get_tree().create_timer(1.0).timeout
	
	# もし自分のターンの最中なら、サイコロボタンを復活させる
	if is_my_turn:
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
		
		# ★ 追加：C++のサーバーに「この名前の交差点は、この座標だよ」と教える
		GameManager.register_vertex(inst.name, pos)
		
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
		GameManager.register_edge(inst.name, edge_data["midpoint"])
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
		vertex_node.get_node("Sprite2D").scale = Vector2(0.25, 0.25)
		
func _on_road_built(edge_name: String, player_id: int):
	var container = $Edges
	var edge_node = container.get_node_or_null(edge_name)
	
	if edge_node != null and edge_node.has_node("ColorRect"):
		# プレイヤーIDで色を分ける
		if player_id == 1:
			edge_node.get_node("ColorRect").color = Color.RED
		else:
			edge_node.get_node("ColorRect").color = Color.BLUE
			
		# 建築済みの道は、分かりやすく少し太くする（お好みで）
		var rect = edge_node.get_node("ColorRect")
		rect.size.y = 8
		rect.position.y = -4
		
func _on_resources_updated(player_id: int, wood: int, brick: int, sheep: int, wheat: int, ore: int):
	# 今回はテストとして、ログに表示させるだけにします
	# （後でここに、画面右下のカードUIの数字を書き換える処理を入れます！）
	print("--- プレイヤー ", player_id, " の資源 ---")
	print(" 木:", wood, " 鉄:", ore, " 羊:", sheep, " 麦:", wheat, " 土:", brick)
	
	# 自分が対象プレイヤーだったら、自分のUIを更新する（予定地）
	if player_id == multiplayer.get_unique_id():
		wood_label.text = str(wood)
		brick_label.text = str(brick)
		sheep_label.text = str(sheep)
		wheat_label.text = str(wheat)
		ore_label.text = str(ore)

func _distribute_resources(roll: int):
	# 盗賊(7)の処理はとりあえず後回し
	if roll == 7:
		print("盗賊が出た！(処理は未実装)")
		return

	# Boardの子ノード（HexTile）をすべて確認する
	for tile in board.get_children():
		
		# ★修正1：C++側のプロパティ名「number」と「tile_type」に合わせる！
		var tile_num = tile.get("number") 
		var res_type_int = tile.get("tile_type")    
		
		# タイルの数字が、サイコロの出目と同じなら発動！
		if tile_num != null and tile_num == roll:
			
			# ★修正2：C++のenum(数字)を、サーバーが読める文字列に変換する！
			var type_str = ""
			match res_type_int:
				0: type_str = "wood"    # FOREST
				1: type_str = "brick"   # HILL
				2: type_str = "sheep"   # PASTURE
				3: type_str = "wheat"   # FIELD
				4: type_str = "ore"     # MOUNTAIN
				_: continue             # DESERT(5) や SEA(6) の場合はここで処理をスキップ
			
			print("発動: 数字 ", roll, " の ", type_str, " タイル")
			
			# C++の魔法の関数を呼ぶ！
			GameManager.distribute_resources_for_hex(tile.position, hex_radius_math, type_str)

func _on_turn_changed(active_player_id: int):
	# 今のターンが「自分」かどうか？
	is_my_turn = (active_player_id == multiplayer.get_unique_id())
	
	if is_my_turn:
		print("★ 自分のターンが来ました！")
		roll_btn.disabled = false      # 押せるようにする
		turn_end_btn.disabled = false
		
		# ★ 自分のターンは「水色」にする！
		turn_end_btn.modulate = Color.LIGHT_SKY_BLUE
		roll_btn.modulate = Color.LIGHT_SKY_BLUE
		
	else:
		print("☆ プレイヤー ", active_player_id, " のターンです。待機中...")
		roll_btn.disabled = true       # 押せなくする
		turn_end_btn.disabled = true
		
		# ★ 相手のターン（またはボタンを押した後）は「薄暗いグレー」に沈ませる
		turn_end_btn.modulate = Color.DIM_GRAY
		roll_btn.modulate = Color.DIM_GRAY
