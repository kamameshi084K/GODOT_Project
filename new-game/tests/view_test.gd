extends SceneTree

var board_view: BlokusBoardView

func _initialize() -> void:
	board_view = BlokusBoardView.new()
	root.add_child(board_view)
	call_deferred("run_checks")

func run_checks() -> void:
	for spin in 4:
		for cell in [Vector2i(0, 0), Vector2i(10, 10), Vector2i(19, 19)]:
			var screen := board_view.camera.unproject_position(board_view.cell_position(cell, 0))
			if board_view.screen_cell(screen) != cell:
				push_error("回転後のマス位置が一致しません: %s -> %s" % [cell, board_view.screen_cell(screen)])
				quit(1)
				return
		board_view.rotate_camera(Vector2(200, 0))
	print("view_test: カメラ4方向でマス位置一致")
	quit(0)
