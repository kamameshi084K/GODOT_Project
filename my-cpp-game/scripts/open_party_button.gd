extends Button

# さっき作ったUIシーンを読み込んでおく
# ※パスは実際の保存場所に合わせること！(例: res://scenes/ui/party_manager_ui.tscn)
var party_ui_scene = preload("res://scenes/party_manager_ui.tscn")

func _pressed():
	# UIのインスタンスを作成
	var ui = party_ui_scene.instantiate()
	
	# 画面の一番手前（CanvasLayer）に追加する
	# get_parent() は CanvasLayer、その親など適切な場所に追加
	# 一番簡単なのは get_tree().root に追加して最前面に出す方法です
	get_parent().add_child(ui)
	
	Input.set_mouse_mode(Input.MOUSE_MODE_VISIBLE)
