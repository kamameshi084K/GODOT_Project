#pragma once

#include <godot_cpp/classes/canvas_layer.hpp>
#include <godot_cpp/classes/progress_bar.hpp>
#include <godot_cpp/classes/label.hpp>

namespace godot
{
    class HUD : public CanvasLayer
    {
        GDCLASS(HUD, CanvasLayer)

    private:
        // UI部品へのパス（エディタで設定）
        NodePath hp_bar_path;
        NodePath level_label_path;
        NodePath exp_label_path;

        // 実際のノードへのポインタ
        ProgressBar* hp_bar;
        Label* level_label;
        Label* exp_label;

    protected:
        /**
         * @brief Godotのメソッドバインディング登録
         * 
         */
        static void _bind_methods();

    public:
        HUD();
        ~HUD();

        /**
         * @brief ノードがシーンツリーに追加されたときに呼ばれる初期化メソッド
         * 
         */
        virtual void _ready() override;
        /**
         * @brief 毎フレーム呼ばれる更新メソッド
         * 
         * @param delta 前のフレームからの経過時間（秒）
         */
        virtual void _process(double delta) override; 

        // セッター・ゲッター
        /**
         * @brief HPバーのノードパスを設定する
         * @param path ノードパス
         */
        void set_hp_bar_path(const NodePath& path);
        /**
         * @brief HPバーのノードパスを取得する
         * @return NodePath ノードパス
         */
        NodePath get_hp_bar_path() const;

        /**
         * @brief レベルラベルのノードパスを設定する
         * @param path ノードパス
         */
        void set_level_label_path(const NodePath& path);
        /**
         * @brief レベルラベルのノードパスを取得する
         * @return NodePath ノードパス
         */
        NodePath get_level_label_path() const;

        /**
         * @brief 経験値ラベルのノードパスを設定する
         * @param path ノードパス
         */
        void set_exp_label_path(const NodePath& path);
        /**
         * @brief 経験値ラベルのノードパスを取得する
         * @return NodePath ノードパス
         */
        NodePath get_exp_label_path() const;
    };
}