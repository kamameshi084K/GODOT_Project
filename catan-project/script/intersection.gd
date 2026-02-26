extends Area2D

var owner_id: int = 0
var building_level: int = 0 # 0: 空き地, 1: 家, 2: 都市
var my_color: Color = Color.WHITE

# ★追加：マウスが乗っているかどうかのフラグ
var is_hovered: bool = false 

func _ready():
	# ★追加：マウスが乗った時と離れた時の処理を紐付け
	mouse_entered.connect(_on_mouse_entered)
	mouse_exited.connect(_on_mouse_exited)

# ★追加：マウスが乗った時の処理
func _on_mouse_entered():
	is_hovered = true
	queue_redraw() # 画面を更新して _draw をもう一度呼ばせる

# ★追加：マウスが離れた時の処理
func _on_mouse_exited():
	is_hovered = false
	queue_redraw()

func _input_event(_viewport, event, _shape_idx):
	if event is InputEventMouseButton and event.pressed and event.button_index == MOUSE_BUTTON_LEFT:
		if owner_id == multiplayer.get_unique_id() and building_level == 1:
			GameManager.request_build_city(name)
		else:
			GameManager.request_build_settlement(name)

func _draw():
	# 通常時の描画
	if building_level == 0:
		draw_circle(Vector2.ZERO, 8.0, Color(1.0, 1.0, 1.0, 0.7)) 
	elif building_level == 1:
		# 黒いフチ取り（draw_arc）を削除し、丸だけを描く
		draw_circle(Vector2.ZERO, 12.0, my_color)
	elif building_level == 2:
		# 都市も同様にフチ取りを削除
		draw_circle(Vector2.ZERO, 20.0, my_color)

	# マウスが乗っている時だけ、黄色く光らせる（一番上に描画）
	if is_hovered:
		# 半径15の半透明の黄色い丸を描画してハイライト
		draw_circle(Vector2.ZERO, 15.0, Color(1.0, 1.0, 0.0, 0.7))

func update_building(p_id: int, level: int):
	owner_id = p_id
	building_level = level
	
	if p_id == 1:
		my_color = Color.RED
	else:
		my_color = Color.BLUE
		
	queue_redraw()
