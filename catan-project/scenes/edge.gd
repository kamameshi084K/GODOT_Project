extends Area2D

func _input_event(_viewport, event, _shape_idx):
	if event is InputEventMouseButton and event.pressed and event.button_index == MOUSE_BUTTON_LEFT:
		print(name + " がクリックされました！")
		
		# テスト用：クリックされたら色を青くする（ColorRectがある場合）
		if has_node("ColorRect"):
			$ColorRect.color = Color.BLUE
