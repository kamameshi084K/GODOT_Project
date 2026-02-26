extends Control

@onready var buy_btn = $BuyButton
@onready var close_btn = $CloseButton

@onready var knight_label = $KnightLabel
@onready var vp_label = $VPLabel
@onready var road_label = $RoadLabel
@onready var plenty_label = $PlentyLabel
@onready var mono_label = $MonoLabel

func _ready():
	# 購入ボタンと閉じるボタンの設定
	buy_btn.pressed.connect(func(): GameManager.request_buy_dev_card())
	close_btn.pressed.connect(func(): hide())
	
	# ★ サーバーから自分だけに送られてくる「最新の枚数」を受信！
	GameManager.private_dev_cards_synced.connect(_on_private_dev_cards_synced)

func _on_private_dev_cards_synced(knight: int, vp: int, road: int, plenty: int, mono: int):
	# 届いた数字をそのままラベルに上書きするだけ
	knight_label.text = "騎士: " + str(knight)
	vp_label.text = "VP: " + str(vp)
	road_label.text = "街道: " + str(road)
	plenty_label.text = "収穫: " + str(plenty)
	mono_label.text = "独占: " + str(mono)
