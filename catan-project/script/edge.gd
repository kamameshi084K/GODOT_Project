extends Area2D

@onready var color_rect = $ColorRect

var is_hovered: bool = false
var owner_id: int = 0
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
		GameManager.request_build_road(name)

# ▼ 変更：引数に color を追加し、そのまま使う！
func build_road(player_id: int, color: Color):
	owner_id = player_id 
	base_color = color 
	
	color_rect.size.y = 8
	color_rect.position.y = -4
	
	_update_visuals()

func _update_visuals():
	if is_hovered and owner_id == 0:
		color_rect.color = Color(1.0, 1.0, 0.0, 1)
	else:
		color_rect.color = base_color
