extends Area2D

func _input_event(_viewport, event, _shape_idx):
	if event is InputEventMouseButton and event.pressed and event.button_index == MOUSE_BUTTON_LEFT:
		# C++(GameManager)に街道の建築をリクエストする
		GameManager.request_build_road(name)
