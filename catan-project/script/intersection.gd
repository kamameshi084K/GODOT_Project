extends Area2D

func _input_event(_viewport, event, _shape_idx):
	if event is InputEventMouseButton and event.pressed and event.button_index == MOUSE_BUTTON_LEFT:
		# ローカルでの色変えはやめて、GameManager(C++)にリクエストを送る！
		GameManager.request_build_settlement(name)
