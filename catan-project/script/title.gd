extends Control

@onready var host_btn = $VBoxContainer/HostButton
@onready var join_btn = $VBoxContainer/JoinButton
@onready var ip_input = $VBoxContainer/AddressInput
@onready var status_lbl = $VBoxContainer/StatusLabel
@onready var start_btn = $VBoxContainer/StartButton
@onready var tutorial_btn = $VBoxContainer/TutorialButton

# ★追加：名前入力用のノードを取得
@onready var name_input = $VBoxContainer/NameInput

func _ready():
	host_btn.pressed.connect(_on_host_pressed)
	join_btn.pressed.connect(_on_join_pressed)
	start_btn.pressed.connect(_on_start_pressed)
	tutorial_btn.pressed.connect(_on_tutorial_pressed)
	
	multiplayer.peer_connected.connect(_update_status)
	multiplayer.peer_disconnected.connect(_update_status)
	
	# ★変更：クライアントが接続成功した時に、名前を送信する関数を呼ぶ
	multiplayer.connected_to_server.connect(_on_connected_ok)
	
	$VBoxContainer/ReconnectButton.pressed.connect(_on_reconnect_button_pressed)

func _on_host_pressed():
	GameManager.host_game()
	_disable_buttons()
	status_lbl.text = "Hosting... Players: 1"
	
	# ★追加：ホスト自身も名前を登録する
	var my_name = name_input.text
	if my_name == "": my_name = "Host"
	GameManager.register_player_name(my_name)

func _on_join_pressed():
	var ip = ip_input.text
	if ip == "":
		ip = "127.0.0.1"
	GameManager.join_game(ip)
	_disable_buttons()
	status_lbl.text = "Connecting to " + ip + "..."

func _on_tutorial_pressed():
	GameManager.set_meta("tutorial_mode", true)
	get_tree().change_scene_to_file("res://scenes/tutorial_mode.tscn")

# ★追加：クライアント側の接続成功処理
func _on_connected_ok():
	status_lbl.text = "Connected! Waiting for host..."
	
	var my_name = name_input.text
	if my_name == "": my_name = "Client"
	
	# サーバー（ID: 1）に対して自分の名前を送信する
	GameManager.rpc_id(1, "register_player_name", my_name)

func _on_start_pressed():
	GameManager.start_game()

func _disable_buttons():
	host_btn.disabled = true
	join_btn.disabled = true
	tutorial_btn.disabled = true
	ip_input.editable = false
	name_input.editable = false # ★追加：接続中は名前も変更不可にする

func _update_status(_id):
	var count = multiplayer.get_peers().size() + 1
	status_lbl.text = "Players: " + str(count)
	
	if multiplayer.is_server() and count >= 2:
		start_btn.disabled = false

func _on_reconnect_button_pressed():
	var player_name = name_input.text
	var ip = ip_input.text
	
	if ip == "":
		ip = "127.0.0.1"
		
	if player_name == "":
		print("名前を入力してください！")
		return
		
	# 1. サーバーに接続する
	GameManager.join_game(ip, 53000)
	
	# 2. 通信が安定するまで待つ
	await get_tree().create_timer(1.0).timeout 
	
	# ▼▼▼ 修正：ずっと消えない GameManager に名前を「メモ」しておく！ ▼▼▼
	GameManager.set_meta("reconnect_name", player_name)
	
	# 3. メイン画面へ移動（※これ以降のコードは書かない）
	get_tree().change_scene_to_file("res://scenes/main.tscn")