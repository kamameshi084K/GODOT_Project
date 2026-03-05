extends Control

@onready var info_label = $InfoLabel
@onready var selected_label = $SelectedLabel
@onready var confirm_btn = $ConfirmButton

var mode = ""
var max_select = 1
var selected_resources = []

func _ready():
	confirm_btn.pressed.connect(_on_confirm)
	for res in ["wood", "brick", "sheep", "wheat", "ore"]:
		var btn = get_node(res.capitalize() + "Button")
		btn.pressed.connect(_on_res_btn_pressed.bind(res))

func setup(new_mode: String):
	mode = new_mode
	selected_resources.clear()
	if mode == "monopoly":
		max_select = 1
		info_label.text = "独占する資源を1つ選んでください"
	elif mode == "plenty":
		max_select = 2
		info_label.text = "欲しい資源を2つ選んでください"
	_update_ui()
	show()

func _on_res_btn_pressed(res: String):
	# 上限に達していたら一番古い選択を消す（選び直しができるように）
	if selected_resources.size() >= max_select:
		selected_resources.pop_front()
	selected_resources.append(res)
	_update_ui()

func _update_ui():
	var text_map = {"wood":"木", "brick":"土", "sheep":"羊", "wheat":"麦", "ore":"鉄"}
	
	if selected_resources.size() == 0:
		selected_label.text = "資源を選択してください..."
	elif mode == "monopoly":
		# ★独占モード（1つ選択された時）
		var res_name = text_map[selected_resources[0]]
		selected_label.text = res_name + "資源を全員から奪う！"
	elif mode == "plenty":
		# ★収穫モード（1〜2つ選択された時）
		if selected_resources.size() == 1:
			var res_name1 = text_map[selected_resources[0]]
			selected_label.text = res_name1 + " と ？？ を収穫"
		elif selected_resources.size() == 2:
			var res_name1 = text_map[selected_resources[0]]
			var res_name2 = text_map[selected_resources[1]]
			selected_label.text = res_name1 + " と " + res_name2 + " を収穫！"
			
	# 指定した数ピッタリ選ばないと確定できない
	confirm_btn.disabled = (selected_resources.size() != max_select)

func _on_confirm():
	if mode == "monopoly":
		GameManager.request_play_monopoly(selected_resources[0])
	elif mode == "plenty":
		GameManager.request_play_plenty(selected_resources[0], selected_resources[1])
	hide()
