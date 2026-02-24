extends TextureRect

signal steal_requested(target_id)
var player_id: int = 0
var vp: int = 0
var hand_count: int = 0
var dev_count: int = 0

@onready var name_label = $NameLabel
@onready var vp_label = $VPLabel
@onready var hand_label = $HandLabel
@onready var dev_label = $DevLabel      # ★追加: 発展カード
@onready var glow_effect = $GlowEffect  # ★追加: 光るエフェクト
@onready var click_overlay = $ClickOverlay # ★追加: 透明なクリック判定

@export var bg_textures: Array[Texture2D] = []

# 引数に p_dev（発展カード枚数）を追加！
func setup(pid: int, turn_index: int, p_name: String, p_vp: int, p_hand: int, p_dev: int):
	player_id = pid
	vp = p_vp
	hand_count = p_hand
	dev_count = p_dev
	name_label.text = p_name
	
	if turn_index < bg_textures.size():
		texture = bg_textures[turn_index]
		
	update_display()
	
	# 最初は光らせず、クリックもできない状態にしておく
	set_stealable(false)
	
	# 透明なボタンが押されたらシグナルを送る
	click_overlay.pressed.connect(func(): steal_requested.emit(player_id))

func update_hand(new_count: int):
	hand_count = new_count
	update_display()

func update_dev_cards(new_count: int):
	dev_count = new_count
	update_display()

func add_vp(amount: int):
	vp += amount
	update_display()

func update_display():
	vp_label.text = str(vp)
	hand_label.text = str(hand_count)
	dev_label.text = str(dev_count)

# ★変更：ボタンを表示するのではなく、光らせてクリックを解禁する関数
func set_stealable(can_steal: bool):
	glow_effect.visible = can_steal
	click_overlay.disabled = not can_steal
