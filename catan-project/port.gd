extends Node2D

# エディタのインスペクターでプルダウンから選べるようにする魔法！
@export_enum("3:1", "wood", "brick", "sheep", "wheat", "ore") var port_type: String = "3:1"

# ※ラベルを付けている場合は、_readyなどで port_type の文字を表示するようにしておくと便利です
