extends CharacterBody3D

# 定数(const)ではなく、変動する変数に変更
var current_speed: float = 5.0
var base_jump_velocity: float = 4.5
var current_mass: float = 1.0

# 毎回 $BlockCreatureMesh を探さなくていいように変数にキャッシュ
@onready var mesh_node = $BlockCreatureMesh

func _ready() -> void:
	# ゲーム開始時にC++側から初期ステータスをもらう
	if mesh_node and mesh_node.has_method("get_speed"):
		current_speed = mesh_node.get_speed()
		current_mass = mesh_node.get_mass()

func _physics_process(delta: float) -> void:
	# 重力処理
	if not is_on_floor():
		velocity += get_gravity() * delta

	# ジャンプ処理（重いほどジャンプ力が下がるペナルティの例）
	var jump_power = max(2.0, base_jump_velocity - (current_mass * 0.1))
	if Input.is_action_just_pressed("ui_accept") and is_on_floor():
		velocity.y = jump_power

	# 移動入力の取得（キーボードもコントローラーもここで処理される）
	var input_dir := Input.get_vector("move_left", "move_right", "move_forward", "move_backward")
	var direction := (transform.basis * Vector3(input_dir.x, 0, input_dir.y)).normalized()
	
	if direction:
		# C++で計算した current_speed を使用
		velocity.x = direction.x * current_speed
		velocity.z = direction.z * current_speed
	else:
		velocity.x = move_toward(velocity.x, 0, current_speed)
		velocity.z = move_toward(velocity.z, 0, current_speed)

	move_and_slide()
	
# エサ側から呼ばれる吸収用の関数
func absorb_block(global_food_pos: Vector3, block_type: String = "normal") -> void:
	var local_pos = mesh_node.to_local(global_food_pos)
	
	var grid_pos = Vector3(
		snapped(local_pos.x, 1.0), 
		snapped(local_pos.y, 1.0), 
		snapped(local_pos.z, 1.0)
	)
	
	# 1. C++側のメッシュにブロック追加を指示
	mesh_node.add_block(grid_pos, block_type)
	
	# 2. C++から最新のステータスを取得して更新！
	current_speed = mesh_node.get_speed()
	current_mass = mesh_node.get_mass()
	
	# 3. 追加されたブロック用の「当たり判定」を生成する
	add_collision_box(grid_pos)


# 新しくくっついたブロック用の当たり判定（CollisionShape3D）を作る関数
func add_collision_box(pos: Vector3) -> void:
	var shape_node = CollisionShape3D.new()
	var box_shape = BoxShape3D.new()
	
	# C++で作っているメッシュと同じ 1x1x1 のサイズにする
	box_shape.size = Vector3(1, 1, 1) 
	
	shape_node.shape = box_shape
	shape_node.position = pos
	
	# 自分（CharacterBody3D）の子ノードとして追加することで、全体の当たり判定が拡張される
	add_child(shape_node)
