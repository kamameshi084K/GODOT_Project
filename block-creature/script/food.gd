extends Area3D

# エディタの右側（インスペクター）から種類を変えられるようにする
@export var block_type: String = "normal" 

func _ready() -> void:
	# シグナルをコードから接続する（エディタの「ノード」タブから繋いでもOKです）
	body_entered.connect(_on_body_entered)

# 何か（物理ボディ）がこのエリアに入ってきた時に呼ばれる関数
func _on_body_entered(body: Node3D) -> void:
	# ぶつかった相手が「absorb_block」関数を持っているか？（＝プレイヤーかどうか）を確認
	if body.has_method("absorb_block"):
		
		# プレイヤーの吸収関数を呼び出し、自分の「ワールド座標」と「種類」を渡す
		body.absorb_block(global_position, block_type)
		
		# 吸収されたので、自分自身をシーンから削除（消滅）する
		queue_free()
