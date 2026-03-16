extends BlockCreatureMesh

# ブロックの座標リストをクラス変数として保持する
var current_blocks: Array = []

func _ready():
	# 初期の体
	current_blocks = [
		Vector3(0, 0, 0),
		Vector3(1, 0, 0),
		Vector3(-1, 0, 0),
		Vector3(0, 1, 0)
	]
	generate_mesh(current_blocks)

	var material = StandardMaterial3D.new()
	material.albedo_color = Color(0.2, 0.8, 0.4) 
	self.material_override = material

# ★新機能：外部から呼ばれてブロックを追加し、メッシュを再生成する
func add_block(local_pos: Vector3):
	# すでに同じ場所にブロックがなければ追加する（重複防止）
	if not current_blocks.has(local_pos):
		current_blocks.append(local_pos)
		generate_mesh(current_blocks) # C++の処理を再度呼んで一瞬で合体！
