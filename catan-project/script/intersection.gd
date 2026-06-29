extends Area2D

var owner_id: int = 0
var building_level: int = 0
var my_color: Color = Color.WHITE

var is_hovered: bool = false 

func _ready():
	mouse_entered.connect(_on_mouse_entered)
	mouse_exited.connect(_on_mouse_exited)

func _on_mouse_entered():
	is_hovered = true
	queue_redraw()

func _on_mouse_exited():
	is_hovered = false
	queue_redraw()

func _input_event(_viewport, event, _shape_idx):
	if event is InputEventMouseButton and event.pressed and event.button_index == MOUSE_BUTTON_LEFT:
		var main = get_tree().current_scene
		var my_id = multiplayer.get_unique_id()
		var is_my_settlement = owner_id == my_id and building_level == 1
		
		if main != null and is_my_settlement and main.has_method("tutorial_can_build_city"):
			if not main.tutorial_can_build_city(name):
				return
			
			if main.has_method("tutorial_force_build_city"):
				main.tutorial_force_build_city(name, my_id)
				return
		
		if main != null and main.has_method("tutorial_can_build_settlement"):
			if not main.tutorial_can_build_settlement(name):
				return
			
			if main.has_method("tutorial_force_build_settlement"):
				main.tutorial_force_build_settlement(name, my_id)
				return
		
		if owner_id == multiplayer.get_unique_id() and building_level == 1:
			GameManager.request_build_city(name)
		else:
			GameManager.request_build_settlement(name)

func _draw():
	if building_level == 0:
		draw_circle(Vector2.ZERO, 8.0, Color(1.0, 1.0, 1.0, 0.7)) 
	elif building_level == 1:
		draw_circle(Vector2.ZERO, 12.0, my_color)
	elif building_level == 2:
		draw_circle(Vector2.ZERO, 20.0, my_color)

	if is_hovered:
		draw_circle(Vector2.ZERO, 15.0, Color(1.0, 1.0, 0.0, 0.7))

# ▼ 変更：引数に color を追加し、そのまま使う！
func update_building(p_id: int, level: int, color: Color):
	owner_id = p_id
	building_level = level
	my_color = color 
	
	queue_redraw()
