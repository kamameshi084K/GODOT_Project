class_name TurnManager
extends Node

@export var unit_nodes: Array[UnitView] = []
@export var poison_puddle_scene: PackedScene = preload("res://scenes/poison_puddle.tscn")
@onready var arena_grid: ArenaGrid = get_tree().get_first_node_in_group("arena_grid")

var core: TurnManagerCore
var hud: HUD
var active_unit_view: UnitView = null
var current_selected_skill: SkillData = null

func _ready() -> void:
	hud = get_tree().get_first_node_in_group("hud")
	if hud:
		hud.skill_selected.connect(_on_skill_selected)

	core = TurnManagerCore.new()
	core.turn_changed.connect(_on_turn_changed)
	core.match_ended.connect(_on_match_ended)

	for unit_view in unit_nodes:
		core.register_unit(unit_view.core)
		unit_view.core.died.connect(func(): _on_unit_died(unit_view))

	core.start_match()

func _on_turn_changed(active_core: UnitCore, turn_num: int) -> void:
	print("\n=== ターン %d: 手番開始 ===" % turn_num)
	_clear_skill_selection()

	if arena_grid:
		arena_grid.on_turn_tick_hazards()

	for unit_view in unit_nodes:
		var is_turn = (unit_view.core == active_core)
		unit_view.set_active_turn(is_turn)
		if is_turn:
			active_unit_view = unit_view
			if hud:
				hud.setup_unit_ui(unit_view)

func _input(event: InputEvent) -> void:
	# Spaceキーで手番終了
	if event.is_action_pressed("ui_accept"):
		_clear_skill_selection()
		end_current_turn()
		return

	# 右クリックでスキル選択キャンセル
	if event is InputEventMouseButton and event.pressed and event.button_index == MOUSE_BUTTON_RIGHT:
		print(">> 右クリック: スキル選択解除")
		_clear_skill_selection()
		return

	# 左クリックでスキル発動
	if event is InputEventMouseButton and event.pressed and event.button_index == MOUSE_BUTTON_LEFT:
		if current_selected_skill:
			print(">> 左クリック検知！ スキル発動処理に入ります")
			_cast_center_raycast_skill()

func _on_skill_selected(_idx: int, skill: SkillData) -> void:
	current_selected_skill = skill
	print(">> スキル選択完了: [%s] (TargetType: %d)" % [skill.get_skill_name(), skill.get_target_type()])

	if skill.get_target_type() == SkillData.TARGET_SELF:
		_execute_skill(null, Vector3.ZERO)
		return

	if active_unit_view and active_unit_view.has_method("show_range"):
		active_unit_view.show_range(skill.get_range())

func _cast_center_raycast_skill() -> void:
	if not active_unit_view or not current_selected_skill:
		return

	var cam = active_unit_view.camera
	if not cam:
		print(">> エラー: active_unit_view に camera がありません")
		return

	var viewport_size = get_viewport().get_visible_rect().size
	var center_screen = viewport_size * 0.5
	var ray_origin = cam.project_ray_origin(center_screen)
	var ray_end = ray_origin + cam.project_ray_normal(center_screen) * 100.0

	var space_state = active_unit_view.get_world_3d().direct_space_state
	var query = PhysicsRayQueryParameters3D.create(ray_origin, ray_end)
	query.exclude = [active_unit_view.get_rid()]

	var result = space_state.intersect_ray(query)
	if not result:
		print(">> レイキャスト不発: レティクルの先に床や敵がありません（虚空を狙っています）")
		return

	var hit_collider = result.collider
	var hit_pos: Vector3 = result.position
	var dist = active_unit_view.global_position.distance_to(hit_pos)
	print(">> レイキャスト命中: 対象=%s, 距離=%.1f" % [hit_collider.name, dist])

	if dist > current_selected_skill.get_range():
		print(">> 射程外です（最大射程: %.1f / 対象距離: %.1f）" % [current_selected_skill.get_range(), dist])
		return

	# 単体敵対象
	if current_selected_skill.get_target_type() == SkillData.TARGET_SINGLE_ENEMY:
		if hit_collider is UnitView and hit_collider != active_unit_view:
			_execute_skill(hit_collider, hit_pos)
		else:
			print(">> 敵ユニットに照準を合わせてください")

	# 地面・範囲指定対象
	elif current_selected_skill.get_target_type() == SkillData.TARGET_GROUND_CELL or current_selected_skill.get_target_type() == SkillData.TARGET_CIRCLE_AREA:
		_execute_skill(null, hit_pos)

func _execute_skill(target: UnitView = null, target_pos: Vector3 = Vector3.ZERO) -> void:
	if current_selected_skill.get_action_type() == SkillData.ACTION_MAIN:
		if not core.can_perform_main_action():
			print(">> 通常アクションは既に使用済みです")
			return
		core.consume_main_action()

	# 1. 自身対象
	if current_selected_skill.get_target_type() == SkillData.TARGET_SELF:
		if current_selected_skill.get_self_damage() > 0:
			active_unit_view.core.apply_damage(current_selected_skill.get_self_damage())
		if current_selected_skill.get_attack_buff_multiplier() > 1.0:
			active_unit_view.core.set_attack_multiplier(current_selected_skill.get_attack_buff_multiplier())
		if current_selected_skill.get_shield_amount() > 0:
			active_unit_view.core.add_shield(current_selected_skill.get_shield_amount())

	# 2. 単体敵対象
	elif target != null:
		var dmg = active_unit_view.core.calculate_damage_output(current_selected_skill.get_base_damage())
		target.core.apply_damage(dmg)

		if current_selected_skill.get_poison_stacks() > 0:
			target.core.add_poison(current_selected_skill.get_poison_stacks())

		var push_dir = (target.global_position - active_unit_view.global_position).normalized()
		push_dir.y = 0
		target.apply_knockback(push_dir, current_selected_skill.get_knockback_force())

		if current_selected_skill.get_breaks_floor() and arena_grid:
			arena_grid.damage_tile_at_position(target.global_position)

	# 3. 地面/範囲指定（C++ Core 幾何計算経由）
	elif target_pos != Vector3.ZERO:
		if current_selected_skill.get_breaks_floor() and arena_grid:
			arena_grid.damage_tile_at_position(target_pos)

		if current_selected_skill.get_poison_stacks() > 0 and poison_puddle_scene and arena_grid:
			var radius = current_selected_skill.get_area_radius()
			if radius <= 0.0:
				radius = 5.0

			var target_tiles = arena_grid.get_tiles_in_range(target_pos, radius)
			for tile_pos in target_tiles:
				arena_grid.apply_poison_puddle(
					tile_pos,
					poison_puddle_scene,
					current_selected_skill.get_poison_stacks(),
					3
				)
			print(">> C++経由で %d マスに毒床を展開しました" % target_tiles.size())

	current_selected_skill.trigger_use()
	_clear_skill_selection()

	if hud and active_unit_view:
		hud.setup_unit_ui(active_unit_view)

func _clear_skill_selection() -> void:
	current_selected_skill = null
	if active_unit_view and active_unit_view.has_method("hide_range"):
		active_unit_view.hide_range()

func end_current_turn() -> void:
	core.next_turn()

func _on_unit_died(unit_view: UnitView) -> void:
	core.remove_dead_unit(unit_view.core)
	unit_nodes.erase(unit_view)

func _on_match_ended(_winner_core: UnitCore) -> void:
	print("試合終了！勝者が決定しました")
