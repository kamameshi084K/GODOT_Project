extends Area2D

@onready var color_rect = $ColorRect

var is_hovered: bool = false
var owner_id: int = 0 # ★追加：誰の道か（0なら誰も建てていない空き地）
var base_color: Color = Color(1.0, 1.0, 1.0, 0.6) 

func _ready():
	mouse_entered.connect(_on_mouse_entered)
	mouse_exited.connect(_on_mouse_exited)
	color_rect.color = base_color

func _on_mouse_entered():
	is_hovered = true
	_update_visuals()

func _on_mouse_exited():
	is_hovered = false
	_update_visuals()

func _input_event(_viewport, event, _shape_idx):
	if event is InputEventMouseButton and event.pressed and event.button_index == MOUSE_BUTTON_LEFT:
		# C++(GameManager)に街道の建築をリクエストする
		GameManager.request_build_road(name)

func build_road(player_id: int):
	owner_id = player_id # ★追加：持ち主のIDを記憶する
	
	if player_id == 1:
		base_color = Color.RED
	else:
		base_color = Color.BLUE
		
	# 道を少し太くする
	color_rect.size.y = 8
	color_rect.position.y = -4
	
	_update_visuals()

func _update_visuals():
	# ★変更：空き地（owner_id == 0）で、かつマウスが乗っている時だけ黄色にする
	if is_hovered and owner_id == 0:
		# 拠点と同じ「黄色」を設定（線の細さを考慮して透明度は少し濃いめの0.6にしています）
		color_rect.color = Color(1.0, 1.0, 0.0, 1)
	else:
		# 誰かの道が建っている、またはマウスが離れている時は元の色
		color_rect.color = base_color
