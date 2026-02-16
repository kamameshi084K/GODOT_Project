extends Town


# Called when the node enters the scene tree for the first time.
func _ready():
	# 準備フェーズ（町）に入った瞬間にマウスカーソルを表示して操作可能にする
	Input.set_mouse_mode(Input.MOUSE_MODE_VISIBLE)
	$Player.set_operable(false)


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	pass
