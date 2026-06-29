extends Node

var main
var step_index := 0
var is_tutorial_rolling := false

const ENEMY_A_ID := 1001
const ENEMY_B_ID := 1002
const ENEMY_C_ID := 1003

var steps := [
	{
		"id": "intro",
		"message": "正順で配置します。",
		"wait": "next"
	},
	{
		"id": "player_first_settlement",
		"message": "赤丸に拠点。",
		"wait": "settlement_built",
		"target": "Vertex_2"
	},
	{
		"id": "player_first_road",
		"message": "赤丸に道。",
		"wait": "road_built",
		"target": "Edge_16"
	},
	{
		"id": "enemy_a_first",
		"message": "敵Aの番です。",
		"wait": "auto",
		"player_id": ENEMY_A_ID,
		"settlement": "Vertex_25",
		"road": "Edge_38"
	},
	{
		"id": "enemy_b_first",
		"message": "敵Bの番です。",
		"wait": "auto",
		"player_id": ENEMY_B_ID,
		"settlement": "Vertex_35",
		"road": "Edge_48"
	},
	{
		"id": "enemy_c_first",
		"message": "敵Cの番です。",
		"wait": "auto",
		"player_id": ENEMY_C_ID,
		"settlement": "Vertex_47",
		"road": "Edge_62"
	},
	{
		"id": "reverse_intro",
		"message": "次は逆順です。",
		"wait": "next"
	},
	{
		"id": "enemy_c_second",
		"message": "敵Cの2回目。",
		"wait": "auto",
		"player_id": ENEMY_C_ID,
		"settlement": "Vertex_52",
		"road": "Edge_69"
	},
	{
		"id": "enemy_b_second",
		"message": "敵Bの2回目。",
		"wait": "auto",
		"player_id": ENEMY_B_ID,
		"settlement": "Vertex_9",
		"road": "Edge_10"
	},
	{
		"id": "enemy_a_second",
		"message": "敵Aの2回目。",
		"wait": "auto",
		"player_id": ENEMY_A_ID,
		"settlement": "Vertex_43",
		"road": "Edge_56"
	},
	{
		"id": "player_second_settlement",
		"message": "あなたの2回目。赤丸に拠点。",
		"wait": "settlement_built",
		"target": "Vertex_31"
	},
	{
		"id": "player_second_road",
		"message": "赤丸に道。",
		"wait": "road_built",
		"target": "Edge_39"
	},
	{
		"id": "setup_complete",
		"message": "初期配置完了。",
		"wait": "next"
	},
	{
		"id": "roll_dice",
		"message": "サイコロを振ります。",
		"wait": "dice_rolled"
	},
	{
		"id": "resource_check",
		"message": "資源が増えました。",
		"wait": "next"
	},
	{
		"id": "road_cost",
		"message": "道は木材1・レンガ1。",
		"wait": "next"
	},
	{
		"id": "give_road_resources",
		"message": "練習用に資源を追加。",
		"wait": "auto",
		"action": "give_road_resources"
	},
	{
		"id": "build_extra_road",
		"message": "赤丸に道を建てます。",
		"wait": "road_built",
		"target": "Edge_23"
	},
	{
		"id": "settlement_cost",
		"message": "拠点は木・レンガ・羊・小麦。",
		"wait": "next"
	},
	{
		"id": "give_settlement_resources",
		"message": "練習用に資源を追加。",
		"wait": "auto",
		"action": "give_settlement_resources"
	},
	{
		"id": "build_extra_settlement",
		"message": "赤丸に拠点を建てます。",
		"wait": "settlement_built",
		"target": "Vertex_19"
	},
	{
		"id": "finish",
		"message": "拠点建築までOKです。",
		"wait": "finish"
	}
]


func setup(main_node):
	main = main_node
	
	if main.has_method("setup_tutorial_enemies"):
		main.setup_tutorial_enemies()
	
	GameManager.settlement_built.connect(_on_settlement_built)
	GameManager.road_built.connect(_on_road_built)
	GameManager.dice_rolled.connect(_on_dice_rolled)
	
	if main.tutorial_next_btn != null:
		main.tutorial_next_btn.pressed.connect(advance)
	
	_show_current_step()


func _show_current_step():
	if step_index >= steps.size():
		main.clear_tutorial_marker()
		main.show_tutorial("【チュートリアル】終了。", false)
		return
	
	var step = steps[step_index]
	var wait_type = step["wait"]
	
	main.show_tutorial("【チュートリアル】" + step["message"], wait_type == "next" or wait_type == "finish")
	
	if wait_type == "settlement_built" and step.has("target"):
		main.show_tutorial_target_vertex(step["target"])
	elif wait_type == "road_built" and step.has("target"):
		main.show_tutorial_target_edge(step["target"])
	elif wait_type == "auto":
		main.clear_tutorial_marker()
		await main.get_tree().create_timer(0.7).timeout
		await _run_auto_step(step)
		_next_step()
	elif wait_type == "dice_rolled":
		main.clear_tutorial_marker()
		_enable_roll_button()
	else:
		main.clear_tutorial_marker()


func advance():
	if _is_waiting("next"):
		_next_step()
		return
	
	if _is_waiting("finish"):
		GameManager.remove_meta("tutorial_mode")
		main.clear_tutorial_marker()
		main.get_tree().change_scene_to_file("res://scenes/title.tscn")
		return


func _next_step():
	step_index += 1
	_show_current_step()


func _on_settlement_built(vertex_name: String, player_id: int):
	if not _is_waiting("settlement_built"):
		return
	
	if player_id != main.multiplayer.get_unique_id():
		return
	
	var step = steps[step_index]
	
	if step.has("target") and vertex_name != step["target"]:
		main.show_tutorial("【チュートリアル】赤丸に置いてください。", false)
		return
	
	main.clear_tutorial_marker()
	
	if step["id"] == "player_second_settlement":
		if main.has_method("tutorial_give_initial_resources_from_vertex"):
			main.tutorial_give_initial_resources_from_vertex(vertex_name, player_id)
		
		main.show_tutorial("【チュートリアル】初期資源を獲得。", false)
		await main.get_tree().create_timer(1.0).timeout
	
	_next_step()


func _on_road_built(edge_name: String, player_id: int):
	if not _is_waiting("road_built"):
		return
	
	if player_id != main.multiplayer.get_unique_id():
		return
	
	var step = steps[step_index]
	
	if step.has("target") and edge_name != step["target"]:
		main.show_tutorial("【チュートリアル】赤丸に置いてください。", false)
		return
	
	main.clear_tutorial_marker()
	_next_step()


func notify_settlement_built(vertex_name: String, player_id: int):
	_on_settlement_built(vertex_name, player_id)


func notify_road_built(edge_name: String, player_id: int):
	_on_road_built(edge_name, player_id)


func request_tutorial_roll():
	if is_tutorial_rolling:
		return
	
	if not _is_waiting("dice_rolled"):
		return
	
	is_tutorial_rolling = true
	
	main.roll_btn.disabled = true
	main.roll_btn.modulate = Color.DIM_GRAY
	
	var d1 = 3
	var d2 = 3
	
	await main._on_dice_rolled(d1, d2)
	
	_on_dice_rolled(d1, d2)
	is_tutorial_rolling = false


func _on_dice_rolled(d1: int, d2: int):
	if not _is_waiting("dice_rolled"):
		return
	
	var total = d1 + d2
	var my_id = main.multiplayer.get_unique_id()
	
	if main.has_method("tutorial_distribute_resources_for_roll"):
		var gained = main.tutorial_distribute_resources_for_roll(total, my_id)
		print("Tutorial gained resources: ", gained)
	
	main.show_tutorial("【チュートリアル】出目は " + str(total) + "。資源獲得。", false)
	
	await main.get_tree().create_timer(1.5).timeout
	_next_step()


func _run_auto_step(step: Dictionary):
	if step.has("action"):
		match step["action"]:
			"give_road_resources":
				await _give_road_resources()
			"give_settlement_resources":
				await _give_settlement_resources()
		return
	
	var enemy_id = step["player_id"]
	var settlement_name = step["settlement"]
	var road_name = step["road"]
	
	await _enemy_build_settlement(enemy_id, settlement_name)
	await _enemy_build_road(enemy_id, road_name)


func _give_road_resources():
	var my_id = main.multiplayer.get_unique_id()
	
	GameManager.add_resource(my_id, "wood", 1)
	GameManager.add_resource(my_id, "brick", 1)
	
	main.show_tutorial("【チュートリアル】木材とレンガを追加。", false)
	await main.get_tree().create_timer(1.0).timeout


func _give_settlement_resources():
	var my_id = main.multiplayer.get_unique_id()
	
	GameManager.add_resource(my_id, "wood", 1)
	GameManager.add_resource(my_id, "brick", 1)
	GameManager.add_resource(my_id, "sheep", 1)
	GameManager.add_resource(my_id, "wheat", 1)
	
	main.show_tutorial("【チュートリアル】拠点用の資源を追加。", false)
	await main.get_tree().create_timer(1.0).timeout


func _enemy_build_settlement(enemy_id: int, vertex_name: String):
	main.show_tutorial("【チュートリアル】敵が拠点を置きます。", false)
	main.show_tutorial_target_vertex(vertex_name)
	
	await main.get_tree().create_timer(0.9).timeout
	
	main._on_settlement_built(vertex_name, enemy_id)
	main.clear_tutorial_marker()
	
	await main.get_tree().create_timer(0.5).timeout


func _enemy_build_road(enemy_id: int, edge_name: String):
	main.show_tutorial("【チュートリアル】敵が道を置きます。", false)
	main.show_tutorial_target_edge(edge_name)
	
	await main.get_tree().create_timer(0.9).timeout
	
	main._on_road_built(edge_name, enemy_id)
	main.clear_tutorial_marker()
	
	await main.get_tree().create_timer(0.5).timeout


func _enable_roll_button():
	main.roll_btn.show()
	main.roll_btn.disabled = false
	main.roll_btn.modulate = Color.LIGHT_SKY_BLUE
	
	main.turn_end_btn.disabled = true
	main.open_trade_btn.disabled = true
	main.open_dev_btn.disabled = true
	main.open_player_trade_btn.disabled = true


func _is_waiting(wait_name: String) -> bool:
	if step_index >= steps.size():
		return false
	
	return steps[step_index]["wait"] == wait_name


func can_build_settlement(vertex_name: String) -> bool:
	if not _is_waiting("settlement_built"):
		main.show_tutorial("【チュートリアル】今は拠点を置けません。", false)
		return false
	
	var step = steps[step_index]
	
	if step.has("target") and vertex_name != step["target"]:
		main.show_tutorial("【チュートリアル】赤丸に置いてください。", false)
		return false
	
	return true


func can_build_road(edge_name: String) -> bool:
	if not _is_waiting("road_built"):
		main.show_tutorial("【チュートリアル】今は道を置けません。", false)
		return false
	
	var step = steps[step_index]
	
	if step.has("target") and edge_name != step["target"]:
		main.show_tutorial("【チュートリアル】赤丸に置いてください。", false)
		return false
	
	return true
