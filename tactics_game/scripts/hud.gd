class_name HUD
extends CanvasLayer

signal skill_selected(skill_index: int, skill: SkillData)

@onready var move_gauge: ProgressBar = $Control/VBoxContainer/MoveGauge
@onready var skill_container: HBoxContainer = $Control/VBoxContainer/SkillContainer

var current_skills: Array[SkillData] = []
var selected_skill_index: int = -1

func setup_unit_ui(unit_view: UnitView) -> void:
	# ノードの準備（@onready）が完了するのを待機
	if not is_node_ready():
		await ready

	for child in skill_container.get_children():
		child.queue_free()
	current_skills.clear()
	selected_skill_index = -1

	# 移動ゲージ監視
	if not unit_view.move_distance_updated.is_connected(_on_move_distance_updated):
		unit_view.move_distance_updated.connect(_on_move_distance_updated)
	_on_move_distance_updated(unit_view.core.get_remaining_move_distance(), 10.0)

	# スキルボタン生成
	var skills = unit_view.core.get_skills()
	for i in range(skills.size()):
		var skill: SkillData = skills[i]
		current_skills.append(skill)
		
		var btn = Button.new()
		btn.custom_minimum_size = Vector2(140, 50)
		_update_button_appearance(btn, skill, i + 1)
		
		var idx = i
		btn.pressed.connect(func(): select_skill(idx))
		skill_container.add_child(btn)

func _update_button_appearance(btn: Button, skill: SkillData, slot_num: int) -> void:
	var prefix = "[Q] " if skill.get_action_type() == SkillData.ACTION_QUICK else ""
	var cd = skill.get_current_cooldown()
	
	if cd > 0:
		btn.text = "%d: %s (CD: %d)" % [slot_num, skill.get_skill_name(), cd]
		btn.disabled = true
	else:
		btn.text = "%d: %s%s" % [slot_num, prefix, skill.get_skill_name()]
		btn.disabled = false

func _unhandled_input(event: InputEvent) -> void:
	# 1〜4 キーで即座にスキル選択
	if event is InputEventKey and event.pressed and not event.echo:
		if event.keycode >= KEY_1 and event.keycode <= KEY_4:
			select_skill(event.keycode - KEY_1)

func select_skill(index: int) -> void:
	if index < 0 or index >= current_skills.size():
		return
	var skill = current_skills[index]
	if not skill.is_available():
		print("[%s] は使用不可/クールダウン中です" % skill.get_skill_name())
		return

	selected_skill_index = index
	print(">> スキル選択: [%s]" % skill.get_skill_name())
	skill_selected.emit(index, skill)

func _on_move_distance_updated(remaining: float, max_dist: float) -> void:
	move_gauge.max_value = max_dist
	move_gauge.value = remaining
