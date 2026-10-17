class_name UnitView
extends CharacterBody3D

signal move_distance_updated(remaining: float, max_dist: float)

enum CameraViewMode { FIRST_PERSON, THIRD_PERSON }

@export var unit_name: String = "Fighter"
@export var move_speed: float = 5.0
@export var mouse_sensitivity: float = 0.003
@export var is_my_turn: bool = false
@export var skills: Array[SkillData] = []

var core: UnitCore
var knockback_velocity: Vector3 = Vector3.ZERO
var view_mode: CameraViewMode = CameraViewMode.FIRST_PERSON


@onready var head: Node3D = $Head
@onready var spring_arm: SpringArm3D = $Head/SpringArm3D
@onready var camera: Camera3D = $Head/SpringArm3D/Camera3D
@onready var range_indicator: MeshInstance3D = $RangeIndicator
@onready var visual_mesh: MeshInstance3D = $MeshInstance3D
@onready var overhead_ui: UnitOverheadUI = $UnitOverheadUI


func _ready() -> void:
	add_to_group("units")
	core = UnitCore.new()
	core.hp_changed.connect(_on_hp_changed)
	core.shield_changed.connect(_on_shield_changed)
	core.died.connect(_on_died)

	for skill in skills:
		if skill:
			core.add_skill(skill)

	_update_camera_mode()
	
	if overhead_ui:
		overhead_ui.update_hp(core.get_current_hp(), 100)
		overhead_ui.update_shield(core.get_shield())

func set_active_turn(active: bool) -> void:
	is_my_turn = active
	camera.current = active
	_update_camera_mode() # 手番切り替え時に表示状態を更新
	if active:
		Input.mouse_mode = Input.MOUSE_MODE_CAPTURED
	else:
		hide_range()

func _input(event: InputEvent) -> void:
	# 画面をクリックした時にマウスを固定
	if event is InputEventMouseButton and event.pressed:
		Input.mouse_mode = Input.MOUSE_MODE_CAPTURED

	# Tab または Escape でマウス解放
	if event is InputEventKey and event.pressed and (event.keycode == KEY_ESCAPE or event.keycode == KEY_TAB):
		if Input.mouse_mode == Input.MOUSE_MODE_CAPTURED:
			Input.mouse_mode = Input.MOUSE_MODE_VISIBLE
		else:
			Input.mouse_mode = Input.MOUSE_MODE_CAPTURED

	if not is_my_turn:
		return

	# マウス視点回転
	if event is InputEventMouseMotion and Input.mouse_mode == Input.MOUSE_MODE_CAPTURED:
		rotate_y(-event.relative.x * mouse_sensitivity)
		head.rotate_x(-event.relative.y * mouse_sensitivity)
		head.rotation.x = clamp(head.rotation.x, deg_to_rad(-70.0), deg_to_rad(70.0))

	# F5キーで FPS / TPS 切り替え
	if event is InputEventKey and event.pressed and event.keycode == KEY_F5:
		if view_mode == CameraViewMode.THIRD_PERSON:
			view_mode = CameraViewMode.FIRST_PERSON
		else:
			view_mode = CameraViewMode.THIRD_PERSON
		_update_camera_mode()

func _update_camera_mode() -> void:
	# 手番でない敵ユニットは常に身体を表示
	if not is_my_turn:
		visual_mesh.visible = true
		return

	# 手番ユニットのみ FPS視点で自分の体を消す
	if view_mode == CameraViewMode.FIRST_PERSON:
		spring_arm.spring_length = 0.0
		visual_mesh.visible = false
	else:
		spring_arm.spring_length = 3.5
		visual_mesh.visible = true

func _physics_process(delta: float) -> void:
	if not is_on_floor():
		velocity.y -= ProjectSettings.get_setting("physics/3d/default_gravity") * delta
	else:
		velocity.y = 0.0

	# ノックバック物理
	if knockback_velocity.length() > 0.1:
		velocity.x = knockback_velocity.x
		velocity.z = knockback_velocity.z
		knockback_velocity = knockback_velocity.move_toward(Vector3.ZERO, 20.0 * delta)
		move_and_slide()
		return

	if not is_my_turn or core.get_is_rooted():
		velocity.x = 0
		velocity.z = 0
		move_and_slide()
		return

	# カメラの向きを基準にした移動ベクトル計算
	var input_dir = Input.get_vector("ui_left", "ui_right", "ui_up", "ui_down")
	var direction = (transform.basis * Vector3(input_dir.x, 0, input_dir.y)).normalized()

	if direction != Vector3.ZERO:
		var move_step = move_speed * delta
		if core.consume_move_distance(move_step):
			velocity.x = direction.x * move_speed
			velocity.z = direction.z * move_speed
			move_distance_updated.emit(core.get_remaining_move_distance(), 10.0)
		else:
			velocity.x = 0
			velocity.z = 0
	else:
		velocity.x = 0
		velocity.z = 0

	move_and_slide()

func show_range(range_radius: float) -> void:
	if range_indicator:
		range_indicator.scale = Vector3(range_radius, 1.0, range_radius)
		range_indicator.visible = true

func hide_range() -> void:
	if range_indicator:
		range_indicator.visible = false

func apply_knockback(dir: Vector3, force: float) -> void:
	knockback_velocity = dir.normalized() * force

func die_by_fall() -> void:
	core.apply_damage(9999)

func _on_hp_changed(current: int, max_hp: int) -> void:
	if overhead_ui:
		overhead_ui.update_hp(current, max_hp)

func _on_shield_changed(shield: int) -> void:
	if overhead_ui:
		overhead_ui.update_shield(shield)

func _on_died() -> void:
	print("[%s] 退場" % unit_name)
	queue_free()
