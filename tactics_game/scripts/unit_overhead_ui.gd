class_name UnitOverheadUI
extends Sprite3D

@onready var sub_viewport: SubViewport = $SubViewport
@onready var container: VBoxContainer = $SubViewport/VBoxContainer
@onready var hp_bar: ProgressBar = $SubViewport/VBoxContainer/HPBar
@onready var shield_bar: ProgressBar = $SubViewport/VBoxContainer/ShieldBar
@onready var status_label: Label = $SubViewport/VBoxContainer/StatusLabel

func _ready() -> void:
	texture = sub_viewport.get_texture()
	# 中身の最小サイズに合わせて Viewport を自動リサイズ
	_adjust_viewport_size()

func _adjust_viewport_size() -> void:
	await get_tree().process_frame
	var min_size = container.get_combined_minimum_size()
	sub_viewport.size = Vector2i(int(min_size.x), int(min_size.y))

func update_hp(current: int, max_hp: int) -> void:
	hp_bar.max_value = max_hp
	hp_bar.value = current

func update_shield(shield: int) -> void:
	shield_bar.max_value = 50
	shield_bar.value = shield
	shield_bar.visible = (shield > 0)
	_adjust_viewport_size()

func update_status(text: String) -> void:
	status_label.text = text
	_adjust_viewport_size()
