extends Node3D

@onready var anim_player = $AnimationPlayer

# 各面が上(Y+)を向くための回転角(Euler度数)
const FACE_ROTATIONS = {
	1: Vector3(-90, 0, 0),    # 1(手前) を 上に向ける
	2: Vector3(0, 0, 90),     # 2(右) を 上に向ける
	3: Vector3(0, 0, 0),      # 3(上) は 最初から上なのでそのまま
	4: Vector3(-180, 0, 0),    # 4(下) を 上に向ける
	5: Vector3(0, -90, -90),     # 5(奥) を 上に向ける
	6: Vector3(90, 0, 0)     # 6(左) を 上に向ける
}

func _ready():
	anim_player.stop()

# サイコロを振り始める演出（ぐるぐる回す）
func start_roll():
	anim_player.play("roll")

# サーバーから出目が決まった時、最終姿勢へ遷移させる
# value: 決定した出目(1〜6)
# duration: アニメーション時間(秒)
func stop_roll_to_value(value: int, duration: float = 0.5):
	if not anim_player.is_playing(): return
	if not FACE_ROTATIONS.has(value): return
	
	var target_rotation = FACE_ROTATIONS[value]
	
	# AnimationPlayer を停止。現在の回転角が保持される。
	anim_player.stop()
	
	# 現在の回転
	var current_rot = rotation_degrees
	# 目標回転
	var target_rot = target_rotation
	
	# Euler Tweenの問題（逆回転、ジンバルロック）を避けるために、
	# 目標角度を、現在の角度に最も近い同等角度にする関数 `wrap_rotation` を作る。
	target_rot.x = wrap_rotation(current_rot.x, target_rot.x)
	target_rot.y = wrap_rotation(current_rot.y, target_rot.y)
	target_rot.z = wrap_rotation(current_rot.z, target_rot.z)
	
	# Tweenを作成。 set_parallel(true) は不要。回転角全体を Tweenする。
	var tween = get_tree().create_tween().set_trans(Tween.TRANS_QUART).set_ease(Tween.EASE_OUT)
	tween.tween_property(self, "rotation_degrees", target_rot, duration)
	
	# Tween終了時に、回転角を FACE_ROTATIONS の値にリセットする（0〜360度の範囲に戻す）
	tween.chain().tween_callback(func(): rotation_degrees = target_rotation)

# `wrap_rotation` 関数：目標角度を現在の角度に最も近い同等角度にする
func wrap_rotation(current: float, target: float) -> float:
	var diff = target - current
	# diff を -180 〜 180 の範囲にする
	diff = wrapf(diff, -180, 180)
	return current + diff
