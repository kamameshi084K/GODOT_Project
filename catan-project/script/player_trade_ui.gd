extends Panel # （もしルートがControlなら extends Control にしてください）

@onready var give_box = $GiveBox
@onready var want_box = $WantBox
@onready var trade_btn = $TradeButton
@onready var close_btn = $CloseButton

func _ready():
	trade_btn.pressed.connect(_on_trade_pressed)
	close_btn.pressed.connect(func(): hide())

func _on_trade_pressed():
	# HBoxContainerの中にある "val" (SpinBox) の値を取得する
	var gw = int(give_box.get_node("Wood/val").value)
	var gb = int(give_box.get_node("Brick/val").value)
	var gs = int(give_box.get_node("Sheep/val").value)
	var gwh = int(give_box.get_node("Wheat/val").value)
	var go = int(give_box.get_node("Ore/val").value)

	var ww = int(want_box.get_node("Wood/val").value)
	var wb = int(want_box.get_node("Brick/val").value)
	var ws = int(want_box.get_node("Sheep/val").value)
	var wwh = int(want_box.get_node("Wheat/val").value)
	var wo = int(want_box.get_node("Ore/val").value)

	var total_give = gw + gb + gs + gwh + go
	var total_want = ww + wb + ws + wwh + wo

	# 最低でも「1枚あげて、1枚もらう」条件になっていないと弾く
	if total_give == 0 or total_want == 0:
		print("出す資源と欲しい資源を両方とも1つ以上設定してください！")
		return

	# C++(サーバー)にトレードを提案！
	GameManager.request_propose_trade(gw, gb, gs, gwh, go, ww, wb, ws, wwh, wo)
	hide()
