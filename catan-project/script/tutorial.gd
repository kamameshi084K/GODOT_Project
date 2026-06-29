extends Control

@onready var title_label = $Panel/VBoxContainer/TitleLabel
@onready var body_label = $Panel/VBoxContainer/BodyLabel
@onready var progress_label = $Panel/VBoxContainer/ProgressLabel
@onready var prev_btn = $Panel/VBoxContainer/ButtonRow/PrevButton
@onready var next_btn = $Panel/VBoxContainer/ButtonRow/NextButton
@onready var start_btn = $Panel/VBoxContainer/ButtonRow/StartButton
@onready var back_btn = $Panel/VBoxContainer/ButtonRow/BackButton

var current_step := 0

var steps := [
	{
		"title": "チュートリアルへようこそ",
		"body": "カタンは、資源を集めて、道・家・都市を建て、10点を目指すゲームです。"
	},
	{
		"title": "1. 盤面を見る",
		"body": "六角形の土地には資源と数字があります。サイコロの合計と土地の数字が同じなら、隣の家や都市から資源がもらえます。"
	},
	{
		"title": "2. 初期配置",
		"body": "最初に家と道を置きます。よく出る数字の近くに家を置くと、資源を集めやすくなります。"
	},
	{
		"title": "3. サイコロを振る",
		"body": "自分のターンではまずサイコロを振ります。7が出ると盗賊が動きます。"
	},
	{
		"title": "4. 建築する",
		"body": "木材＋レンガで道、木材＋レンガ＋羊毛＋小麦で家、小麦2＋鉱石3で都市を建てられます。"
	},
	{
		"title": "5. 交換する",
		"body": "足りない資源は銀行交換やプレイヤー交渉で集めます。港を使うと有利な交換ができます。"
	},
	{
		"title": "6. 発展カード",
		"body": "小麦＋羊毛＋鉱石で発展カードを引けます。騎士や街道建設などで有利に進められます。"
	},
	{
		"title": "7. 10点を目指す",
		"body": "家は1点、都市は2点です。最長交易路や最大騎士力でも点が入ります。合計10点で勝利です。"
	},
	{
		"title": "練習ゲームを開始",
		"body": "ここから実際の盤面で操作を試します。チュートリアル用に1人プレイのホストとして開始します。"
	}
]

func _ready():
	prev_btn.pressed.connect(_on_prev_pressed)
	next_btn.pressed.connect(_on_next_pressed)
	start_btn.pressed.connect(_on_start_pressed)
	back_btn.pressed.connect(_on_back_pressed)
	_show_step()

func _show_step():
	var step = steps[current_step]

	title_label.text = step["title"]
	body_label.text = step["body"]
	progress_label.text = str(current_step + 1) + " / " + str(steps.size())

	prev_btn.disabled = current_step == 0
	next_btn.visible = current_step < steps.size() - 1
	start_btn.visible = current_step == steps.size() - 1

func _on_prev_pressed():
	current_step -= 1
	if current_step < 0:
		current_step = 0
	_show_step()

func _on_next_pressed():
	current_step += 1
	if current_step >= steps.size():
		current_step = steps.size() - 1
	_show_step()

func _on_start_pressed():
	GameManager.set_meta("tutorial_mode", true)
	GameManager.host_game()
	GameManager.register_player_name("Tutorial")
	GameManager.start_game()

func _on_back_pressed():
	GameManager.remove_meta("tutorial_mode")
	get_tree().change_scene_to_file("res://scenes/title.tscn")
