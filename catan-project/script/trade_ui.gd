extends Control

@onready var give_option = $GiveOption
@onready var get_option = $GetOption
@onready var trade_btn = $TradeButton
@onready var close_btn = $CloseButton

func _ready():
	# ドロップダウンに資源の選択肢を追加
	var resources = ["wood", "brick", "sheep", "wheat", "ore"]
	for res in resources:
		give_option.add_item(res)
		get_option.add_item(res)
	
	# ボタンが押された時の処理
	trade_btn.pressed.connect(_on_trade_pressed)
	close_btn.pressed.connect(func(): hide()) # 閉じるボタンで非表示にする

func _on_trade_pressed():
	var give_res = give_option.get_item_text(give_option.selected)
	var get_res = get_option.get_item_text(get_option.selected)
	
	if give_res == get_res:
		print("同じ資源は交換できません！")
		return
		
	# C++のサーバーにトレードをリクエスト
	GameManager.request_bank_trade(give_res, get_res)
	
	# トレードを実行したらUIを閉じる（お好みで残してもOK）
	hide()
