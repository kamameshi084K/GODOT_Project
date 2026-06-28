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
		"body": "このモードでは、カタンの基本的な流れを順番に確認できます。まずは『資源を集める → 建築する → 10点を目指す』という全体像を覚えましょう。"
	},
	{
		"title": "1. 盤面と数字を見る",
		"body": "六角形の土地には資源の種類と数字があります。サイコロの合計が土地の数字と同じなら、その土地に隣接している家・都市から資源がもらえます。"
	},
	{
		"title": "2. 初期配置をする",
		"body": "ゲーム開始時は、家（開拓地）と道を置きます。家を置く場所によって、序盤にもらえる資源が大きく変わります。まずは数字が出やすい土地の近くを狙いましょう。"
	},
	{
		"title": "3. サイコロを振る",
		"body": "自分のターンになったら最初にサイコロを振ります。7が出ると盗賊が動き、手札が多いプレイヤーは資源を捨てることがあります。"
	},
	{
		"title": "4. 建築する",
		"body": "木材＋レンガで道、木材＋レンガ＋羊毛＋小麦で家、小麦2＋鉱石3で都市を作れます。建築で勝利点を増やしていきます。"
	},
	{
		"title": "5. 交換する",
		"body": "足りない資源は銀行交換やプレイヤー交渉で集めます。港を使える場所に家を建てると、より有利なレートで交換できます。"
	},
	{
		"title": "6. 発展カードを使う",
		"body": "小麦＋羊毛＋鉱石で発展カードを引けます。騎士、街道建設、独占、収穫、勝利点などがあり、逆転のきっかけになります。"
	},
	{
		"title": "7. 10点を目指す",
		"body": "家は1点、都市は2点です。最長交易路や最大騎士力でも2点を獲得できます。合計10点に到達したプレイヤーが勝利です。"
	},
	{
		"title": "練習ゲームを開始",
		"body": "準備ができたら練習ゲームを開始してください。チュートリアル用のローカルホストとして開始し、実際の盤面で操作を試せます。"
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
	current_step = max(current_step - 1, 0)
	_show_step()

func _on_next_pressed():
	current_step = min(current_step + 1, steps.size() - 1)
	_show_step()

func _on_start_pressed():
	GameManager.set_meta("tutorial_mode", true)
	GameManager.host_game()
	GameManager.register_player_name("Tutorial")
	GameManager.start_game()

func _on_back_pressed():
	get_tree().change_scene_to_file("res://scenes/title.tscn")