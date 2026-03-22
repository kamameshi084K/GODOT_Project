extends BlockCreatureMesh

var block_list: Array[Dictionary] = []
var static_body: StaticBody3D

func _ready():
	var material = StandardMaterial3D.new()
	material.vertex_color_use_as_albedo = true 
	self.material_override = material

	static_body = StaticBody3D.new()
	add_child(static_body)

	# 最初にコアパーツ（青: ID=2）を原点に置く
	add_block(Vector3(0, 0, 0), 2)
	set_physics_process(false)

func add_block(pos: Vector3, type: int):
	for b in block_list:
		if b["pos"] == pos:
			return 
			
	block_list.append({"pos": pos, "type": type})
	_update_mesh()

# ★新規追加：ブロックを削除する関数
func remove_block(pos: Vector3):
	# コアパーツ（原点の青ブロック）は機体の核なので消せないように保護する！
	if pos == Vector3(0, 0, 0):
		return
		
	# リストから指定された座標のブロックを探して削除する
	for i in range(block_list.size()):
		if block_list[i]["pos"] == pos:
			block_list.remove_at(i)
			_update_mesh()
			return

# ★改良：ブロックが減った時のために、当たり判定を毎回作り直す設計に変更
func _update_mesh():
	var pos_array = PackedVector3Array()
	var type_array = PackedInt32Array()

	# 1. 古い当たり判定を一旦すべて破棄する
	for child in static_body.get_children():
		child.queue_free()

	# 2. 最新のリストに合わせて、メッシュ配列の準備と当たり判定の再生成を行う
	for b in block_list:
		pos_array.append(b["pos"])
		type_array.append(b["type"])

		var col_shape = CollisionShape3D.new()
		var box = BoxShape3D.new()
		box.size = Vector3(1, 1, 1)
		col_shape.shape = box
		col_shape.position = b["pos"]
		static_body.add_child(col_shape)

	# 3. 最後にC++で一気にメッシュを描画！
	generate_mesh(pos_array, type_array)

var is_launched: bool = false
var rigid_body: RigidBody3D

# ★出撃！ 組み立てた機体を物理空間（剛体）にコンバートする
func launch_machine():
	if is_launched: return
	is_launched = true

	# 1. 新しい物理の体（RigidBody3D）を生み出す
	rigid_body = RigidBody3D.new()
	
	# 2. 現在の親（BuildScene）にRigidBodyを追加し、同じ位置に置く
	get_parent().add_child(rigid_body)
	rigid_body.global_position = self.global_position

	# 3. 描画用の自分自身（メッシュ）を、RigidBodyの子にお引っ越しさせる
	get_parent().remove_child(self)
	rigid_body.add_child(self)
	self.position = Vector3.ZERO # 親からのズレをリセット

	# 4. 当たり判定の箱たちも、RigidBodyの体にお引っ越しさせる
	for child in static_body.get_children():
		static_body.remove_child(child)
		rigid_body.add_child(child)
	
	# 5. 用済みになった見えない壁（StaticBody）を消去
	static_body.queue_free()

	# 6. スラスターの物理計算を開始！
	set_physics_process(true)

# ★毎フレームの物理計算（スラスターの推力）
func _physics_process(delta: float):
	if not is_launched or rigid_body == null:
		return

	# スラスターを吹かすキー（thrust）が押されている時だけ力を加える
	if Input.is_action_pressed("thrust"):
		for b in block_list:
			if b["type"] == 1: # スラスターパーツ（赤）なら
				# 推力の強さ（必要に応じて調整してください）
				var thrust_power = 15.0 
				
				# 機体のローカルな「前方（-Z方向）」を、現在のワールドの向きに変換して推力ベクトルを作る
				var force_dir = rigid_body.global_basis * Vector3(0, 0, -1) * thrust_power
				
				# 力の加わる位置（重心からのオフセット）を計算
				var offset = rigid_body.global_basis * b["pos"]
				
				# 物理エンジンに「指定されたパーツの位置から、前方向へ力を加えよ」と命令！
				rigid_body.apply_force(force_dir, offset)
