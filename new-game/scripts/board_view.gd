class_name BlokusBoardView
extends Node3D

const COLORS := [
	Color(0.10, 0.37, 0.85),
	Color(0.97, 0.76, 0.08),
	Color(0.86, 0.17, 0.16),
	Color(0.10, 0.64, 0.28),
]

var camera: Camera3D
var placed_root: Node3D
var preview_root: Node3D
var yaw := 0.65
var pitch := 0.95
var distance := 30.0
var dragging_camera := false

func _ready() -> void:
	camera = Camera3D.new()
	add_child(camera)
	camera.current = true
	var light := DirectionalLight3D.new()
	light.rotation_degrees = Vector3(-55, -35, 0)
	light.light_energy = 1.1
	add_child(light)
	var ambient := WorldEnvironment.new()
	var environment := Environment.new()
	environment.background_mode = Environment.BG_COLOR
	environment.background_color = Color(0.12, 0.16, 0.19)
	environment.ambient_light_source = Environment.AMBIENT_SOURCE_COLOR
	environment.ambient_light_color = Color(0.8, 0.8, 0.8)
	environment.ambient_light_energy = 0.6
	ambient.environment = environment
	add_child(ambient)
	var table := MeshInstance3D.new()
	var table_mesh := BoxMesh.new()
	table_mesh.size = Vector3(22, 0.35, 22)
	table.mesh = table_mesh
	table.position.y = -0.24
	table.material_override = material(Color(0.30, 0.21, 0.13))
	add_child(table)
	for y in BlokusRules.SIZE:
		for x in BlokusRules.SIZE:
			var tile := MeshInstance3D.new()
			var mesh := BoxMesh.new()
			mesh.size = Vector3(0.97, 0.08, 0.97)
			tile.mesh = mesh
			tile.position = cell_position(Vector2i(x, y), 0)
			var color := Color(0.78, 0.73, 0.62) if (x + y) % 2 == 0 else Color(0.69, 0.64, 0.54)
			for start_color in 4:
				if Vector2i(x, y) == BlokusRules.STARTS[start_color]:
					color = COLORS[start_color].lightened(0.45)
			tile.material_override = material(color)
			add_child(tile)
	placed_root = Node3D.new()
	add_child(placed_root)
	preview_root = Node3D.new()
	add_child(preview_root)
	reset_camera()

func material(color: Color, transparent: bool = false) -> StandardMaterial3D:
	var result := StandardMaterial3D.new()
	result.albedo_color = color
	result.metallic = 0.1
	result.roughness = 0.72
	if transparent:
		result.transparency = BaseMaterial3D.TRANSPARENCY_ALPHA
	return result

func cell_position(cell: Vector2i, height: float) -> Vector3:
	return Vector3(cell.x - 9.5, height, cell.y - 9.5)

func make_piece_cell(cell: Vector2i, color: Color, height: float, translucent: bool = false) -> MeshInstance3D:
	var cube := MeshInstance3D.new()
	var mesh := BoxMesh.new()
	mesh.size = Vector3(0.88, 0.24, 0.88)
	cube.mesh = mesh
	cube.position = cell_position(cell, height)
	cube.material_override = material(color, translucent)
	return cube

func clear_root(root: Node3D) -> void:
	for child in root.get_children():
		child.queue_free()

func show_board(board: Array) -> void:
	clear_root(placed_root)
	for y in BlokusRules.SIZE:
		for x in BlokusRules.SIZE:
			var color: int = board[y * BlokusRules.SIZE + x]
			if color >= 0:
				placed_root.add_child(make_piece_cell(Vector2i(x, y), COLORS[color], 0.20))

func show_preview(piece_id: int, rotation: int, flipped: bool, origin: Vector2i, color: int, legal: bool) -> void:
	clear_root(preview_root)
	if piece_id < 0:
		return
	var tint: Color = COLORS[color] if legal else Color(0.95, 0.13, 0.12)
	tint.a = 0.55
	for delta in BlokusRules.cells(piece_id, rotation, flipped):
		preview_root.add_child(make_piece_cell(origin + delta, tint, 0.38, true))

func screen_cell(screen_position: Vector2) -> Vector2i:
	var ray_origin := camera.project_ray_origin(screen_position)
	var direction := camera.project_ray_normal(screen_position)
	if absf(direction.y) < 0.0001:
		return Vector2i(-100, -100)
	var step := -ray_origin.y / direction.y
	if step < 0:
		return Vector2i(-100, -100)
	var hit := ray_origin + direction * step
	return Vector2i(floori(hit.x + 10.0), floori(hit.z + 10.0))

func reset_camera() -> void:
	yaw = 0.65
	pitch = 0.95
	distance = 30.0
	update_camera()

func rotate_camera(delta: Vector2) -> void:
	yaw -= delta.x * 0.007
	pitch = clampf(pitch + delta.y * 0.007, 0.3, 1.45)
	update_camera()

func zoom(amount: float) -> void:
	distance = clampf(distance + amount, 18.0, 55.0)
	update_camera()

func update_camera() -> void:
	camera.position = Vector3(sin(yaw) * cos(pitch), sin(pitch), cos(yaw) * cos(pitch)) * distance
	camera.look_at(Vector3.ZERO, Vector3.UP)
