extends Node

var main
var step_index := 0

var steps := [
	{
		"id": "intro",
		"message": "チュートリアル開始です。まずは盤面を見てみましょう。",
		"wait": "next"
	},
	{
		"id": "build_first_settlement",
		"message": "最初の家を建てます。数字が出やすい土地の近くを選びましょう。",
		"wait": "settlement_built"
	},
	{
		"id": "build_first_road",
		"message": "次に道を建てます。家から伸びる場所を選びましょう。",
		"wait": "road_built"
	},
	{
		"id": "enemy_first_move",
		"message": "敵が初期配置をします。敵の動きも見てみましょう。",
		"wait": "auto"
	},
	{
		"id": "roll_dice",
		"message": "通常ターンです。まずサイコロを振りましょう。",
		"wait": "dice_rolled"
	}
]

func setup(main_node):
	main = main_node
	
	GameManager.settlement_built.connect(_on_settlement_built)
	GameManager.road_built.connect(_on_road_built)
	GameManager.dice_rolled.connect(_on_dice_rolled)
	
	if main.tutorial_next_btn != null:
		main.tutorial_next_btn.pressed.connect(advance)
	
	_show_current_step()

func _show_current_step():
	var step = steps[step_index]
	var wait_type = step["wait"]
	
	main.show_tutorial("【チュートリアル】" + step["message"], wait_type == "next")
	
	if wait_type == "auto":
		await main.get_tree().create_timer(1.0).timeout
		await _run_auto_step(step)
		_next_step()

func advance():
	if not _is_waiting("next"):
		return
	
	_next_step()

func _next_step():
	step_index += 1
	
	if step_index >= steps.size():
		main.show_tutorial("【チュートリアル】ここまでです。", false)
		return
	
	_show_current_step()

func _on_settlement_built(vertex_name: String, player_id: int):
	if not _is_waiting("settlement_built"):
		return
	
	if player_id != multiplayer.get_unique_id():
		return
	
	_next_step()

func _on_road_built(edge_name: String, player_id: int):
	if not _is_waiting("road_built"):
		return
	
	if player_id != multiplayer.get_unique_id():
		return
	
	_next_step()

func _on_dice_rolled(d1: int, d2: int):
	if not _is_waiting("dice_rolled"):
		return
	
	_next_step()

func _is_waiting(wait_name: String) -> bool:
	if step_index >= steps.size():
		return false
	
	return steps[step_index]["wait"] == wait_name

func _run_auto_step(step: Dictionary):
	if step["id"] == "enemy_first_move":
		main.show_tutorial("【敵】敵は資源が取りやすい場所に家を置こうとしています。", false)
		await main.get_tree().create_timer(1.5).timeout
