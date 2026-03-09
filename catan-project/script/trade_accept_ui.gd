extends Control

# ▼ ツリーに合わせて「Panel/」を追加しました ▼
@onready var msg_label = $Panel/MessageLabel
@onready var give_label = $Panel/GiveLabel
@onready var want_label = $Panel/WantLabel
@onready var accept_btn = $Panel/AcceptButton
@onready var decline_btn = $Panel/DeclineButton

func _ready():
	accept_btn.pressed.connect(_on_accept)
	decline_btn.pressed.connect(func(): hide())

# C++(サーバー)から提案を受信した時に呼ばれる関数
func setup(proposer_name: String, gw: int, gb: int, gs: int, gwh: int, go: int, ww: int, wb: int, ws: int, wwh: int, wo: int):
	msg_label.text = proposer_name + " さんからのトレード提案！"
	
	var text_map = ["木", "土", "羊", "麦", "鉄"]
	var give_arr = [gw, gb, gs, gwh, go]
	var want_arr = [ww, wb, ws, wwh, wo]
	
	var give_str = ""
	var want_str = ""
	
	for i in range(5):
		if give_arr[i] > 0: give_str += text_map[i] + str(give_arr[i]) + "個 "
		if want_arr[i] > 0: want_str += text_map[i] + str(want_arr[i]) + "個 "

	# 受信する側から見た視点なので「相手が出す＝自分がもらえる」になる
	give_label.text = "もらえる資源: " + give_str
	want_label.text = "あなたが払う資源: " + want_str
	
	show()

func _on_accept():
	# 承諾ボタンを押したらサーバーに送信
	GameManager.request_accept_trade()
	hide()
