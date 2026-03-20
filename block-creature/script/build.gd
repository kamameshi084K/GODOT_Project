extends Node3D

@onready var camera_pivot = $CameraPivot
@onready var camera = $CameraPivot/Camera3D
@onready var cursor = $CursorMesh
@onready var creature = $BlockCreatureMesh

var camera_sensitivity: float = 0.005 

# ★追加：マウス入力の処理関数
func _input(event: InputEvent) -> void:
	# 中クリック（マウスホイール押し込み）を押しながらドラッグした時
	if event is InputEventMouseMotion and Input.is_mouse_button_pressed(MOUSE_BUTTON_MIDDLE):
		
		# 左右のドラッグでピボットをY軸回転（横ぐるぐる）
		camera_pivot.rotation.y -= event.relative.x * camera_sensitivity
		
		# 上下のドラッグでピボットをX軸回転（縦ぐるぐる）
		camera_pivot.rotation.x -= event.relative.y * camera_sensitivity
		
		# 縦回転がひっくり返らないように、-90度〜90度（真上〜真下）で制限する
		camera_pivot.rotation.x = clamp(camera_pivot.rotation.x, -PI/2.0, PI/2.0)

func _process(delta: float) -> void:
	var mouse_pos = get_viewport().get_mouse_position()
	var ray_origin = camera.project_ray_origin(mouse_pos)
	var ray_dir = camera.project_ray_normal(mouse_pos)

	# 1. 物理エンジン空間（SpaceState）を取得し、カメラから奥へ1000mの光線を作る
	var space_state = get_world_3d().direct_space_state
	var ray_query = PhysicsRayQueryParameters3D.create(ray_origin, ray_origin + ray_dir * 1000.0)
	
	# 2. 光線を飛ばして、何かにぶつかったか結果を取得！
	var result = space_state.intersect_ray(ray_query)

	# 何か（床や青いコアパーツ）にぶつかっていたら
	if result:
		var hit_pos = result.position
		var hit_normal = result.normal

		# ★ マイクラ等のプロゲーで使われる「究極のボクセル配置計算」
		
		# 1. ワールド座標を、機体(BlockCreatureMesh)のローカル座標に変換する
		var local_pos = creature.to_local(hit_pos)
		
		# 法線（向いている方向）もローカル向きに変換
		var local_normal = (creature.to_local(hit_pos + hit_normal) - local_pos).normalized()

		# 2. ぶつかった表面から、ブロックの「内側」へ少し(0.1)だけ入り込む！
		# （これにより、端っこをクリックしても絶対にズレない）
		var inside_pos = local_pos - local_normal * 0.1
		
		# 3. それを四捨五入して、「今クリックした対象ブロックの座標」を確定させる
		var target_block = Vector3(
			round(inside_pos.x),
			round(inside_pos.y),
			round(inside_pos.z)
		)

		# 4. 新しく置く座標は、対象ブロックから法線方向（外側）に1マス進んだ場所
		var grid_pos = target_block + local_normal.round()

		cursor.global_position = grid_pos
		cursor.visible = true 

		# 左クリックで装甲（ID=0）を置く
		if Input.is_action_just_pressed("click"):
			creature.add_block(grid_pos, 0)
			
		# 右クリックでスラスター（ID=1）を置く
		elif Input.is_action_just_pressed("right_click"):
			creature.add_block(grid_pos, 1)
			
	else:
		# 空を向いている時はカーソルを隠す
		cursor.visible = false
