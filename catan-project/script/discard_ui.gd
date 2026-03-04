extends Control

@onready var info_label = $InfoLabel
@onready var confirm_btn = $ConfirmButton

@onready var counts = {
	"wood": $WoodCount, "brick": $BrickCount, "sheep": $SheepCount,
	"wheat": $WheatCount, "ore": $OreCount
}

var target_discard_amount = 0
var current_discard = {"wood": 0, "brick": 0, "sheep": 0, "wheat": 0, "ore": 0}

# ★追加: 自分が現在持っている最大枚数を記憶する
var max_resources = {"wood": 0, "brick": 0, "sheep": 0, "wheat": 0, "ore": 0}

# ★変更: setupで今の所持数も受け取って保存する
func setup(amount: int, w: int, b: int, s: int, wh: int, o: int):
	target_discard_amount = amount
	max_resources["wood"] = w
	max_resources["brick"] = b
	max_resources["sheep"] = s
	max_resources["wheat"] = wh
	max_resources["ore"] = o
	
	for res in current_discard.keys():
		current_discard[res] = 0
	_update_ui()
	show()

func _ready():
	confirm_btn.pressed.connect(_on_confirm)
	
	# 各＋ーボタンの処理を安全な bind 方式で接続
	for res in ["wood", "brick", "sheep", "wheat", "ore"]:
		var plus_btn = get_node(res.capitalize() + "Plus")
		var minus_btn = get_node(res.capitalize() + "Minus")
		plus_btn.pressed.connect(_change_count.bind(res, 1))
		minus_btn.pressed.connect(_change_count.bind(res, -1))

func _change_count(res: String, amount: int):
	var new_val = current_discard[res] + amount
	var total_selected = _get_total_selected()
	
	# ★変更: 持っている枚数(max_resources[res])より多くは選べないようにする！
	if new_val >= 0 and new_val <= max_resources[res] and total_selected + amount <= target_discard_amount:
		current_discard[res] = new_val
		_update_ui()

func _get_total_selected() -> int:
	var total = 0
	for val in current_discard.values():
		total += val
	return total

func _update_ui():
	var remain = target_discard_amount - _get_total_selected()
	info_label.text = "あと " + str(remain) + " 枚捨ててください"
	
	for res in current_discard.keys():
		counts[res].text = str(current_discard[res])
		
	confirm_btn.disabled = (remain != 0)

func _on_confirm():
	GameManager.request_discard(
		current_discard["wood"], current_discard["brick"], 
		current_discard["sheep"], current_discard["wheat"], current_discard["ore"]
	)
	hide()
