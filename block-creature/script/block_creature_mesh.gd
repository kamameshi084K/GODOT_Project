extends BlockCreatureMesh

func _ready():
	# C++側の初期化関数を呼ぶ（1ブロックだけになり、メッシュが作られる）
	setup()

	# 全体の色設定
	var material = StandardMaterial3D.new()
	material.albedo_color = Color(0.2, 0.8, 0.4) 
	self.material_override = material

# ※今後は add_block(Vector3(1, 0, 0), "red") などと呼ぶだけで、
# 裏でC++がメッシュ更新とステータス計算をすべて一瞬で終わらせてくれます。
