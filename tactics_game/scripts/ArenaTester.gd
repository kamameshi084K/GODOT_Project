# ArenaTester.gd (Arenaノードに追加でアタッチまたはArenaGridに統合)
extends ArenaGrid

@onready var camera: Camera3D = $Camera3D

func _ready() -> void:
	var unit = UnitCore.new()
	print("ユニットHP: ", unit.get_current_hp()) # 100
	unit.add_shield(20)
	unit.apply_damage(30)
	print("被弾後HP: ", unit.get_current_hp()) # 90

func _unhandled_input(event: InputEvent) -> void:
	if event is InputEventMouseButton and event.pressed and event.button_index == MOUSE_BUTTON_LEFT:
		var mouse_pos = event.position
		var ray_origin = camera.project_ray_origin(mouse_pos)
		var ray_end = ray_origin + camera.project_ray_normal(mouse_pos) * 100.0

		var space_state = get_world_3d().direct_space_state
		var query = PhysicsRayQueryParameters3D.create(ray_origin, ray_end)
		var result = space_state.intersect_ray(query)

		if result:
			damage_tile_at_position(result.position)
