extends Control

@onready var host_btn = $VBoxContainer/HostButton
@onready var join_btn = $VBoxContainer/JoinButton
@onready var ip_input = $VBoxContainer/AddressInput
@onready var status_lbl = $VBoxContainer/StatusLabel
@onready var start_btn = $VBoxContainer/StartButton

func _ready():
	# ボタンが押されたときの処理を紐付け
	host_btn.pressed.connect(_on_host_pressed)
	join_btn.pressed.connect(_on_join_pressed)
	start_btn.pressed.connect(_on_start_pressed)
	
	# 誰かが接続・切断したときに人数を更新する
	multiplayer.peer_connected.connect(_update_status)
	multiplayer.peer_disconnected.connect(_update_status)
	
	# クライアントがサーバーへの接続に成功したとき
	multiplayer.connected_to_server.connect(func(): status_lbl.text = "Connected! Waiting for host...")

func _on_host_pressed():
	# C++の host_game() を呼び出す
	GameManager.host_game()
	_disable_buttons()
	status_lbl.text = "Hosting... Players: 1"

func _on_join_pressed():
	var ip = ip_input.text
	if ip == "":
		ip = "127.0.0.1"
	# C++の join_game() を呼び出す
	GameManager.join_game(ip)
	_disable_buttons()
	status_lbl.text = "Connecting to " + ip + "..."

func _on_start_pressed():
	# C++の start_game() を呼び出す (全員のシーンをMain.tscnに切り替えるRPC)
	GameManager.start_game()

func _disable_buttons():
	host_btn.disabled = true
	join_btn.disabled = true
	ip_input.editable = false

func _update_status(_id):
	# 自分を含めた接続人数を計算
	var count = multiplayer.get_peers().size() + 1
	status_lbl.text = "Players: " + str(count)
	
	# 自分がサーバー（ホスト）で、かつ2人以上いればスタートボタンを押せるようにする
	if multiplayer.is_server() and count >= 2:
		start_btn.disabled = false
