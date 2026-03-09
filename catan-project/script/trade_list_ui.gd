extends Control

# ▼ ツリーに合わせて「Panel/」を追加しました ▼
@onready var list_container = $Panel/ListContainer
@onready var cancel_btn = $Panel/CancelButton

func _ready():
	cancel_btn.pressed.connect(_on_cancel)

func setup():
	# リストを空にして表示
	for child in list_container.get_children():
		child.queue_free()
	show()

func add_accepter(accepter_id: int, accepter_name: String):
	var btn = Button.new()
	btn.text = accepter_name + " と取引する"
	btn.pressed.connect(func(): _on_accepter_chosen(accepter_id))
	list_container.add_child(btn)

func _on_accepter_chosen(target_id: int):
	# この人と取引確定！
	GameManager.request_execute_trade(target_id)
	hide()

func _on_cancel():
	GameManager.request_cancel_trade()
	hide()
