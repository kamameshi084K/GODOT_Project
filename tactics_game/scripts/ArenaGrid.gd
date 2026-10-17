class_name ArenaGrid
extends Node3D

signal tile_destroyed(cell_pos: Vector3i, world_pos: Vector3)

const TILE_NORMAL: int = 0
const TILE_CRACKED: int = 1

@onready var grid_map: GridMap = $GridMap
@onready var death_zone: Area3D = $DeathZone

var core: ArenaGridCore
var puddle_instances: Dictionary = {} # cell_pos -> PoisonPuddle

func _ready() -> void:
	core = ArenaGridCore.new()
	death_zone.body_entered.connect(_on_death_zone_body_entered)
	_initialize_field()

func _initialize_field() -> void:
	for cell in grid_map.get_used_cells():
		core.init_tile(cell, 2)
	print("初期配置タイル数 (Core初期化): ", grid_map.get_used_cells().size())

func damage_tile_at_position(world_pos: Vector3) -> void:
	var local_pos = grid_map.to_local(world_pos)
	var cell_pos: Vector3i = grid_map.local_to_map(local_pos)

	# C++ Coreで耐久度を減少計算
	var hp = core.damage_tile(cell_pos)
	if hp < 0:
		return

	var tile_center = get_tile_center_world(world_pos)

	if hp == 1:
		grid_map.set_cell_item(cell_pos, TILE_CRACKED)
	elif hp <= 0:
		grid_map.set_cell_item(cell_pos, GridMap.INVALID_CELL_ITEM)
		_remove_puddle_at(cell_pos)
		tile_destroyed.emit(cell_pos, tile_center)

# C++ Coreを使った高速な範囲内セル抽出
func get_tiles_in_range(center_world_pos: Vector3, radius: float) -> Array[Vector3]:
	var all_positions: PackedVector3Array = []
	for cell in grid_map.get_used_cells():
		all_positions.append(grid_map.to_global(grid_map.map_to_local(cell)))

	return Array(core.get_cells_in_radius(center_world_pos, radius, all_positions))

func apply_poison_puddle(world_pos: Vector3, puddle_scene: PackedScene, stacks: int, duration: int) -> void:
	if puddle_scene == null:
		print(">> エラー: puddle_scene が null です！TurnManager のインスペクターを確認してください")
		return

	var local_pos = grid_map.to_local(world_pos)
	var cell_pos: Vector3i = grid_map.local_to_map(local_pos)

	if grid_map.get_cell_item(cell_pos) == GridMap.INVALID_CELL_ITEM:
		print(">> タイルが存在しないセルです: ", cell_pos)
		return

	# C++ Coreにハザードを記録
	core.add_hazard(cell_pos, stacks, duration)

	_remove_puddle_at(cell_pos)

	var puddle = puddle_scene.instantiate()
	get_tree().current_scene.add_child(puddle)
	var center = grid_map.to_global(grid_map.map_to_local(cell_pos))
	# 床に埋まらないように少し浮かせる (Y + 0.05)
	puddle.global_position = Vector3(center.x, center.y + 0.05, center.z)
	puddle_instances[cell_pos] = puddle
	print(">> 毒床生成成功: セル ", cell_pos, " 座標: ", puddle.global_position)

func on_turn_tick_hazards() -> void:
	# C++ Coreでターン減少計算し、消滅したセルを受け取る
	var expired_cells: Array = core.tick_hazards()
	for cell in expired_cells:
		_remove_puddle_at(cell)

func _remove_puddle_at(cell_pos: Vector3i) -> void:
	if puddle_instances.has(cell_pos):
		var p = puddle_instances[cell_pos]
		if is_instance_valid(p):
			p.queue_free()
		puddle_instances.erase(cell_pos)

func get_tile_center_world(world_pos: Vector3) -> Vector3:
	var local_pos = grid_map.to_local(world_pos)
	var cell_pos: Vector3i = grid_map.local_to_map(local_pos)
	return grid_map.to_global(grid_map.map_to_local(cell_pos))

func _on_death_zone_body_entered(body: Node3D) -> void:
	if body.has_method("die_by_fall"):
		body.die_by_fall()
	else:
		body.queue_free()
