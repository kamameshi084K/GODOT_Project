#pragma once

#include <godot_cpp/classes/canvas_layer.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/panel.hpp>
#include <godot_cpp/classes/input_event.hpp>

namespace godot
{
    class DialogueBox : public CanvasLayer
    {
        GDCLASS(DialogueBox, CanvasLayer)

    private:
        NodePath label_path; // Labelノードへのパス
        Label* text_label; // メッセージ表示用のLabelノード
        
        NodePath panel_path; // Panelノードへのパス
        Control* background_panel; // 背景用のPanelノード

        bool is_active; // ダイアログがアクティブかどうかのフラグ

    protected:
        static void _bind_methods();

    public:
        // コンストラクタとデストラクタ

        DialogueBox();
        ~DialogueBox();

        /**
         * @brief ノードが準備完了したときに呼ばれる
         * 
         */
        virtual void _ready() override;
        /**
         * @brief 入力イベントを処理する
         * 
         * @param event 入力イベントの参照
         */
        virtual void _input(const Ref<InputEvent>& event) override;

        // 外部から呼ばれる関数
        /**
         * @brief メッセージを表示する
         * 
         * @param message 表示するメッセージ文字列
         */
        void show_message(const String& message);
        /**
         * @brief メッセージを閉じる
         * 
         */
        void close_message();

        // エディタ設定用
        /**
         * @brief Labelノードへのパスを設定する
         * 
         * @param path LabelノードへのNodePath
         */
        void set_label_path(const NodePath& path);
        /**
         * @brief Labelノードへのパスを取得する
         * 
         * @return NodePath LabelノードへのNodePath
         */
        NodePath get_label_path() const;

        /**
         * @brief Panelノードへのパスを設定する
         * 
         * @param path PanelノードへのNodePath
         */
        void set_panel_path(const NodePath& path);
        /**
         * @brief Panelノードへのパスを取得する
         * 
         * @return NodePath PanelノードへのNodePath
         */
        NodePath get_panel_path() const;
    };
}