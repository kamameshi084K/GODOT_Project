class_name PoisonPuddle
extends Area3D

@export var poison_stacks_per_turn: int = 2
@export var duration_turns: int = 3

var current_turns_left: int

func _ready() -> void:
	current_turns_left = duration_turns

# ターン交代時に呼び出され、持続ターンを減らす
func on_turn_tick() -> void:
	# 範囲内のユニットに毒を付与
	for body in get_overlapping_bodies():
		if body is UnitView and body.core:
			body.core.add_poison(poison_stacks_per_turn)
			print("[%s] が毒沼を踏んで毒 +%d！" % [body.unit_name, poison_stacks_per_turn])

	current_turns_left -= 1
	if current_turns_left <= 0:
		queue_free()
