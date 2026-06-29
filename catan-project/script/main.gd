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
@onready var tutorial_guide = get_node_or_null("GameUI/TutorialGuide")
@onready var tutorial_guide_label = get_node_or_null("GameUI/TutorialGuide/GuideLabel")
@onready var tutorial_next_btn = get_node_or_null("GameUI/TutorialGuide/NextButton")

# ★追加：右側のプレイヤーリスト用
@onready var player_list = $GameUI/PlayerList
var player_ui_scene = preload("res://scenes/Player_ui.tscn")
var player_uis = {}

var intersection_scene = preload("res://scenes/Intersection.tscn")
var edge_scene = preload("res://scenes/Edge.tscn")

var hex_radius_math = 54.0 
var is_my_turn = false
var is_moving_robber = false
var robber_scene = preload("res://scenes/Robber.tscn")
var robber_icon: Node2D # Sprite2DからNode2Dに変更（シーンを実体化するため）
var free_roads_left = 0

var current_largest_army_player = 0
var current_longest_road_player = 0

var player_colors = {}
var base_colors = [Color.RED, Color.BLUE, Color.GREEN, Color.PURPLE]
var current_phase = 0

var tutorial_mode := false
var tutorial_message_index := 0

var tutorial_controller_script = preload("res://script/tutorial_controller.gd")
var tutorial_controller

func _ready():
	tutorial_mode = GameManager.has_meta("tutorial_mode") and GameManager.get_meta("tutorial_mode") == true
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
	dice_viewport.hide()
	
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
	
	# トレード画面を開くボタンの設定
	open_trade_btn.pressed.connect(func(): trade_ui.show())
	open_trade_btn.disabled = true
	trade_ui.hide() # ゲーム開始時はトレード画面を隠しておく
	
	open_dev_btn.pressed.connect(func(): dev_ui.show())
	open_dev_btn.disabled = true
	dev_ui.hide()
	
	resource_select_ui.hide()
	
	# DevUIからシグナルが来たら、リソース選択画面を開く
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
		GameManager.remove_meta("reconnect_name") # 読み取ったらメモを消す
		_request_reconnect_delayed(r_name)
		
	if tutorial_guide != null:
		tutorial_guide.hide()

func _on_roll_pressed():
	roll_btn.disabled = true
	# アニメーションの処理は消して、サーバーにお願いするだけ！
	GameManager.rpc_id(1, "request_roll_dice")

func _on_dice_rolled(d1: int, d2: int):
	# 合計値はここで計算する
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
		
		# ▼▼▼ 修正：ランダム分割処理を消して、サーバーから来た数字をそのまま使う！ ▼▼▼
		die1.stop_roll_to_value(d1, 0.5)
		die2.stop_roll_to_value(d2, 0.5)
		
		await get_tree().create_timer(2.0).timeout
		dice_viewport.hide()
	
	# ▼ これ以降は既存の処理（資源を配るなど） ▼
	dice_label.text = "出目: " + str(roll_value)
	if multiplayer.is_server():
		_distribute_resources(roll_value)
	
	if is_my_turn:
		roll_btn.hide() # ここでボタンを完全に隠す
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
			
	if tutorial_mode and is_my_turn:
		if roll_value == 7:
			show_info("【チュートリアル】7が出ました。盗賊を移動させる必要があります。")
		else:
			show_info("【チュートリアル】資源を確認して、道・家・都市・交換・発展カードのどれをするか考えましょう。")

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
		var c = player_colors.get(player_id, Color.WHITE) # ★追加
		vertex_node.update_building(player_id, 1, c) # ★変更
		if multiplayer.is_server() and current_phase == 1:
			for tile in board.get_children():
				# 家の座標とタイルの中心座標の距離を測り、くっついているタイルを探す
				if tile.position.distance_to(vertex_node.position) < (hex_radius_math + 5.0):
					var res_type_int = tile.get("tile_type")
					var type_str = ""
					match res_type_int:
						0: type_str = "wood"
						1: type_str = "brick"
						2: type_str = "sheep"
						3: type_str = "wheat"
						4: type_str = "ore"
					
					# 砂漠以外なら、サーバーに「この資源を1つ追加して！」とお願いする
					if type_str != "":
						GameManager.add_resource(player_id, type_str, 1)
		
	if player_id != multiplayer.get_unique_id() and player_uis.has(player_id):
		player_uis[player_id].add_vp(1)
		
	if tutorial_mode and player_id == multiplayer.get_unique_id():
		show_info("【チュートリアル】家を建てました！家は1点です。隣の土地から資源を得られるようになります。")
		
func _on_road_built(edge_name: String, player_id: int):
	var container = $Edges
	var edge_node = container.get_node_or_null(edge_name)
	
	if edge_node != null:
		var c = player_colors.get(player_id, Color.WHITE) # ★追加
		edge_node.build_road(player_id, c) # ★変更
		
	if player_id == multiplayer.get_unique_id() and free_roads_left > 0:
		free_roads_left -= 1
		if free_roads_left > 0:
			show_info("★ 街道建設：無料で引ける道は残り【1本】です！")
		else:
			show_info("★ 街道建設：無料分の道をすべて引き終わりました！")
			
	if tutorial_mode and player_id == multiplayer.get_unique_id():
		show_info("【チュートリアル】道を建てました！道を伸ばすと、新しい場所に家を建てられます。")

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

func _on_turn_changed(active_player_id: int, phase: int = 2):
	current_phase = phase
	is_my_turn = (active_player_id == multiplayer.get_unique_id())
	var active_name = "Player " + str(active_player_id)
	if is_my_turn:
		active_name = "あなた"
	elif player_uis.has(active_player_id):
		active_name = player_uis[active_player_id].name_label.text
		
	if phase == 0 or phase == 1:
		show_info("【初期配置】 " + active_name + " のターンです！")
	else:
		show_info("🎲 " + active_name + " のターンです！")
	if is_my_turn:
		if phase == 0 or phase == 1:
			open_trade_btn.disabled = true 
			open_dev_btn.disabled = true
			open_player_trade_btn.disabled = true # ★追加
		else:
			roll_btn.show()
			roll_btn.disabled = false
			open_trade_btn.disabled = true 
			open_dev_btn.disabled = true 
			open_player_trade_btn.disabled = true # ★追加（サイコロ振るまでは押せない）
			turn_end_btn.disabled = true
	else:
		open_trade_btn.disabled = true 
		open_dev_btn.disabled = true 
		open_player_trade_btn.disabled = true # ★追加
		trade_ui.hide() 
		dev_ui.hide() 
		player_trade_ui.hide() # ★追加
	if tutorial_mode and is_my_turn:
		if phase == 0 or phase == 1:
			show_info("【チュートリアル】初期配置です。まず家を置き、そのあと道を置きましょう。")
		else:
			show_info("【チュートリアル】あなたの通常ターンです。まずサイコロを振りましょう。")

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
					show_info("★奪える相手のUIが光ります！クリックして奪ってください。")
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
		if res_type_int == 5: # 5が砂漠タイル
			tile.add_child(robber_icon)
			
			# ▼ 見た目の位置を綺麗に調整する
			robber_icon.position = Vector2(-15, -15) 
			
			# ▼ サーバー(C++)に砂漠の座標を教えて、同じ場所に置けないようにする！
			if multiplayer.is_server():
				GameManager.set_initial_robber_pos(tile.position)
				
			break

# ★追加：右側のリストを作成する処理
func _on_player_list_updated(player_info_list: Array):
	for child in player_list.get_children():
		child.queue_free()
	player_uis.clear()
	player_colors.clear() # リセット
	
	var my_id = multiplayer.get_unique_id()
	for info in player_info_list:
		var pid = info["id"]
		var t_idx = info["turn_index"]
		
		# ★ ターン順（0〜3）に応じて色を割り当てて記憶する！
		player_colors[pid] = base_colors[t_idx % 4]
		
		if pid != my_id:
			var ui = player_ui_scene.instantiate()
			player_list.add_child(ui)
			ui.setup(pid, info["turn_index"], info["name"], info["vp"], info["hand_count"], info["dev_cards"])
			ui.steal_requested.connect(_on_steal_requested)
			player_uis[pid] = ui

	# ★ すでに盤面にある家や道の色を、決定した色で塗り直す！
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

# ★追加：光ったUIをクリックした時の処理
func _on_steal_requested(target_id: int):
	GameManager.request_steal(target_id)
	# 奪い終わったので全プレイヤーの光を消す
	for ui in player_uis.values():
		ui.set_stealable(false)
		
	turn_end_btn.disabled = false
	turn_end_btn.modulate = Color.LIGHT_SKY_BLUE

func _on_city_built(vertex_name: String, player_id: int):
	var container = $Intersections
	var vertex_node = container.get_node_or_null(vertex_name)
	
	if vertex_node != null:
		var c = player_colors.get(player_id, Color.WHITE) # ★追加
		vertex_node.update_building(player_id, 2, c) # ★変更
		
	if player_id != multiplayer.get_unique_id() and player_uis.has(player_id):
		player_uis[player_id].add_vp(1)
		
	if tutorial_mode and player_id == multiplayer.get_unique_id():
		show_info("【チュートリアル】都市にアップグレードしました！都市は2点で、資源も多くもらえます。")

func _on_prompt_knight_robber():
	show_info("★ 騎士カードを使用しました！盗賊を移動させてください。")
	dev_ui.hide()
	is_moving_robber = true
	turn_end_btn.disabled = true
	turn_end_btn.modulate = Color.DIM_GRAY

func _on_prompt_discard(amount: int, w: int=0, b: int=0, s: int=0, wh: int=0, o: int=0):
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
	print(msg) # コンソールにも履歴として残しておく

func _on_game_won(winner_id: int):
	# メッセージウィンドウに結果を表示
	if winner_id == multiplayer.get_unique_id():
		show_info("おめでとうございます！あなたが10点に到達して勝利しました！ 🎉")
	else:
		var winner_name = "Player " + str(winner_id)
		if player_uis.has(winner_id):
			winner_name = player_uis[winner_id].name_label.text
		show_info("残念！ " + winner_name + " が10点に到達し、ゲームに勝利しました！")
		
	# ゲーム終了なので、すべての操作ボタンを押せなくする
	turn_end_btn.disabled = true
	roll_btn.disabled = true
	open_trade_btn.disabled = true
	open_dev_btn.disabled = true
	turn_end_btn.modulate = Color.DIM_GRAY
	roll_btn.modulate = Color.DIM_GRAY

func _setup_ports():
	var ports_container = get_node_or_null("Ports")
	if ports_container == null: return
	
	var intersections = $Intersections.get_children()
	
	var all_descendants = ports_container.find_children("*", "", true, false)
	
	for port in all_descendants:
		if "port_type" in port:
			var distances = []
			for vertex in intersections:
				var dist = port.global_position.distance_to(vertex.global_position)
				distances.append({"node": vertex, "dist": dist})
			
			# 距離が短い（近い）順に並び替え
			distances.sort_custom(func(a, b): return a["dist"] < b["dist"])
			
			# ★変更: 画像の配置に合わせて、一番近い【1つ】の頂点だけを登録する！
			var v1 = distances[0]["node"]
			
			# C++(サーバー)に登録！
			GameManager.register_port(v1.name, port.port_type)

func _on_trade_proposed(proposer_id: int, gw: int, gb: int, gs: int, gwh: int, go: int, ww: int, wb: int, ws: int, wwh: int, wo: int):
	# 自分が提案した本人の場合はリストUIを表示して待つ
	if proposer_id == multiplayer.get_unique_id():
		trade_list_ui.setup()
		show_info("★ トレードを提案しました。他のプレイヤーの返答を待っています...")
		return
		
	# 他のプレイヤーには承諾UI（ポップアップ）を出す
	var proposer_name = "Player " + str(proposer_id)
	if player_uis.has(proposer_id):
		proposer_name = player_uis[proposer_id].name_label.text
		
	trade_accept_ui.setup(proposer_name, gw, gb, gs, gwh, go, ww, wb, ws, wwh, wo)

func _on_trade_accepted_by_someone(accepter_id: int):
	# 誰かが「承諾」を押したら、提案者のリストにボタンを追加する
	var accepter_name = "Player " + str(accepter_id)
	if player_uis.has(accepter_id):
		accepter_name = player_uis[accepter_id].name_label.text
	
	trade_list_ui.add_accepter(accepter_id, accepter_name)

func _on_trade_completed():
	# トレードが成立（またはキャンセル）したら全UIを閉じる
	trade_accept_ui.hide()
	player_trade_ui.hide()
	trade_list_ui.hide()
	show_info("★ プレイヤートレードが成立（またはキャンセル）しました！")

func _on_largest_army_changed(player_id: int):
	# 1. もし前の保持者がいたら、その人から2点引く
	if current_largest_army_player != 0 and current_largest_army_player != player_id:
		if player_uis.has(current_largest_army_player):
			player_uis[current_largest_army_player].add_vp(-2)

	# 2. 新しい保持者に2点足してメッセージを出す
	if player_id != 0:
		if player_uis.has(player_id):
			player_uis[player_id].add_vp(2)
			
		var p_name = "Player " + str(player_id)
		if player_uis.has(player_id):
			p_name = player_uis[player_id].name_label.text
		elif player_id == multiplayer.get_unique_id():
			p_name = "あなた"
			
		show_info("⚔️ " + p_name + " が【最大騎士力】(2VP) を獲得しました！")
	
	# 保持者の記録を更新
	current_largest_army_player = player_id

func _on_longest_road_changed(player_id: int):
	# 1. もし前の保持者がいたら、その人から2点引く
	if current_longest_road_player != 0 and current_longest_road_player != player_id:
		if player_uis.has(current_longest_road_player):
			player_uis[current_longest_road_player].add_vp(-2)

	# 2. 新しい保持者に2点足してメッセージを出す
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
		
	# 保持者の記録を更新
	current_longest_road_player = player_id

func _on_player_disconnected(player_name: String):
	# 画面を止めず、メッセージだけで「スキップしたこと」を伝える
	show_info("⚠️ 通信エラー: " + player_name + " が切断されました。復帰するまでターンをスキップします。")

func _on_player_reconnected(old_id: int, new_id: int, p_name: String):
	show_info("✅ " + p_name + " が再接続しました！ (ID: " + str(old_id) + " -> " + str(new_id) + ")")

# サーバーから送られてきた「過去の盤面」を一瞬で復元する処理
func _on_full_state_received(state: Dictionary):
	show_info("🔄 最新の盤面データを復元しています...")
	
	var vertices = state["vertices"]
	for v in vertices:
		var v_node = $Intersections.get_node_or_null(v["name"])
		if v_node:
			var c = player_colors.get(v["owner"], Color.WHITE) # ★追加
			v_node.update_building(v["owner"], v["type"], c) # ★変更
			
	var edges = state["edges"]
	for e in edges:
		var e_node = $Edges.get_node_or_null(e["name"])
		if e_node:
			var c = player_colors.get(e["owner"], Color.WHITE) # ★追加
			e_node.build_road(e["owner"], c) # ★変更
			
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
	# 盤面（家や道）の受け入れ準備が完全に終わるまで0.5秒だけ待つ
	await get_tree().create_timer(0.5).timeout
	
	# ▼▼▼ 修正：rpc_idを消して、直接関数を呼ぶ！ ▼▼▼
	GameManager.request_reconnect(r_name)

func show_tutorial(msg: String, can_next: bool = false):
	if tutorial_guide != null:
		tutorial_guide.show()
	
	if tutorial_guide_label != null:
		tutorial_guide_label.text = msg
	
	if tutorial_next_btn != null:
		tutorial_next_btn.visible = can_next
	
	show_info(msg)
