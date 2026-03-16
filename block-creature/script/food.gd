extends Area3D

func _on_body_entered(body: Node3D) -> void:
	# ぶつかってきた相手（body）が「吸収機能」を持っていたら
	if body.has_method("absorb_block"):
		# 自分のワールド座標を渡して食べてもらう
		body.absorb_block(global_position)
		# 食べられたので自分自身を消滅させる
		queue_free()
