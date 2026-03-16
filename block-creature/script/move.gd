extends CharacterBody3D


const SPEED = 5.0
const JUMP_VELOCITY = 4.5


func _physics_process(delta: float) -> void:
	print("処理中: ", velocity.y) # これを追記
	
	# Add the gravity.
	if not is_on_floor():
		velocity += get_gravity() * delta

	# Handle jump.
	if Input.is_action_just_pressed("ui_accept") and is_on_floor():
		velocity.y = JUMP_VELOCITY

	# Get the input direction and handle the movement/deceleration.
	# As good practice, you should replace UI actions with custom gameplay actions.
	var input_dir := Input.get_vector("move_left", "move_right", "move_forward", "move_backward")
	var direction := (transform.basis * Vector3(input_dir.x, 0, input_dir.y)).normalized()
	if direction:
		velocity.x = direction.x * SPEED
		velocity.z = direction.z * SPEED
	else:
		velocity.x = move_toward(velocity.x, 0, SPEED)
		velocity.z = move_toward(velocity.z, 0, SPEED)

	move_and_slide()
	
# エサ側から呼ばれる吸収用の関数
func absorb_block(global_food_pos: Vector3):
	var mesh_node = $BlockCreatureMesh # 子ノードのメッシュを取得
	
	# 課題解決1：エサのワールド座標を、プレイヤー自身から見たローカル座標に変換する
	var local_pos = mesh_node.to_local(global_food_pos)
	
	# 課題解決2：小数の座標を四捨五入して、1x1x1の「綺麗なグリッド（マス目）」に吸着させる
	var grid_pos = Vector3(
		snapped(local_pos.x, 1.0), 
		snapped(local_pos.y, 1.0), 
		snapped(local_pos.z, 1.0)
	)
	
	# メッシュにブロック追加を指示
	mesh_node.add_block(grid_pos)
