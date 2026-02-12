extends Control

@onready var party_list = $HBoxContainer/VBoxContainer/PartyList
@onready var standby_list = $HBoxContainer/VBoxContainer2/StandbyList
@onready var party_info_label = $HBoxContainer/VBoxContainer/PartyInfoLabel

func _ready():
	refresh_ui()

func refresh_ui():
	# ★修正点: GameManager ではなく GlobalGameManager を使う
	if not GlobalGameManager:
		return

	party_list.clear()
	standby_list.clear()
	
	# ★修正点
	var party = GlobalGameManager.get_party()
	var current_rank_sum = 0
	
	for i in range(party.size()):
		var monster = party[i]
		if monster:
			var text = "%s (Rank:%d HP:%d)" % [monster.get_monster_name(), monster.get_rank(), monster.get_current_hp()]
			party_list.add_item(text)
			current_rank_sum += monster.get_rank()
	
	# ★修正点
	var standby = GlobalGameManager.get_standby()
	for i in range(standby.size()):
		var monster = standby[i]
		if monster:
			var text = "%s (Rank:%d)" % [monster.get_monster_name(), monster.get_rank()]
			standby_list.add_item(text)
			
	party_info_label.text = "現在のパーティ (Rank: %d / 7)" % current_rank_sum

func _on_to_party_button_pressed():
	var selected_indices = standby_list.get_selected_items()
	if selected_indices.size() == 0:
		return
	
	var index = selected_indices[0]
	# ★修正点
	var standby = GlobalGameManager.get_standby()
	var monster = standby[index]
	
	# ★修正点
	if GlobalGameManager.move_standby_to_party(monster):
		print("パーティに入れました")
		refresh_ui()
	else:
		print("コストオーバーか満員です")

func _on_to_standby_button_pressed():
	var selected_indices = party_list.get_selected_items()
	if selected_indices.size() == 0:
		return
		
	var index = selected_indices[0]
	# ★修正点
	var party = GlobalGameManager.get_party()
	var monster = party[index]
	
	# ★修正点
	GlobalGameManager.move_party_to_standby(monster)
	refresh_ui()

func _on_close_button_pressed():
	queue_free()
