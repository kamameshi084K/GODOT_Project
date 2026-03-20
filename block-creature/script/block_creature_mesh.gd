extends BlockCreatureMesh

var block_list: Array[Dictionary] = []
var static_body: StaticBody3D

func _ready():
	var material = StandardMaterial3D.new()
	material.vertex_color_use_as_albedo = true 
	self.material_override = material

	# ★ 物理エンジン用の親玉ノードを一つ作っておく
	static_body = StaticBody3D.new()
	add_child(static_body)

	# 最初にコアパーツ（青: ID=2）を原点に置く
	add_block(Vector3(0, 0, 0), 2)

func add_block(pos: Vector3, type: int):
	# 重複チェック
	for b in block_list:
		if b["pos"] == pos:
			return 
			
	block_list.append({"pos": pos, "type": type})
	_update_mesh()

	# ★ 超重要：置いた場所に「1x1x1の当たり判定の箱」を作る！
	var col_shape = CollisionShape3D.new()
	var box = BoxShape3D.new()
	box.size = Vector3(1, 1, 1) # ブロックとピッタリ同じサイズ
	col_shape.shape = box
	col_shape.position = pos # ブロックと同じ座標に配置
	
	static_body.add_child(col_shape) # 親玉に追加

func _update_mesh():
	var pos_array = PackedVector3Array()
	var type_array = PackedInt32Array()

	for b in block_list:
		pos_array.append(b["pos"])
		type_array.append(b["type"])

	generate_mesh(pos_array, type_array)
