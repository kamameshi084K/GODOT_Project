#pragma once

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/v_box_container.hpp> 
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/line_edit.hpp>

namespace godot
{
    class TitleScreen : public Control
    {
        GDCLASS(TitleScreen, Control)

    private:
        // UI要素へのパス（エディタで設定）
        NodePath main_menu_path;
        NodePath selection_menu_path;

        NodePath network_menu_path; //通信メニュー用コンテナへのパス
        NodePath ip_input_path;     //IPアドレス入力欄へのパス

        // 実際のノード
        Control* main_menu;      // スタート・終了ボタンが入った箱
        Control* selection_menu; // 3択ボタンが入った箱
        Control* network_menu; // 通信メニュー用コンテナ
        LineEdit* ip_input;    // IPアドレス入力欄

        // ゲーム開始を試みる
        void _attempt_start_game();

    protected:
        static void _bind_methods();

    public:
        // コンストラクタ・デストラクタ
        TitleScreen();
        ~TitleScreen();

        virtual void _ready() override;

        // ボタンが押された時の関数
        /**
         * @brief スタートボタンが押された時の処理
         * 
         */
        void _on_start_button_pressed();
        /**
         * @brief 通信ボタンが押された時の処理
         * 
         */
        void _on_multiplayer_button_pressed();
        /**
         * @brief 終了ボタンが押された時の処理
         * 
         */
        void _on_quit_button_pressed();

        // --- ネットワークメニューのボタン ---

        /**
         * @brief ホストボタンが押された時の処理
         * 
         */
        void _on_host_button_pressed();
        /**
         * @brief ジョインボタンが押された時の処理
         * 
         */
        void _on_join_button_pressed();
        /**
         * @brief 戻るボタンが押された時の処理
         * 
         */
        void _on_back_button_pressed();

        // ネットワークコールバック
        /**
         * @brief サーバーが起動した時の処理
         * 
         */
        void _on_connected_to_server();
        /**
         * @brief サーバーへの接続に失敗した時の処理
         * 
         */
        void _on_connection_failed();

        // 3択ボタン用
        void _on_fire_button_pressed();
        void _on_water_button_pressed();
        void _on_grass_button_pressed();
        
        // セッター・ゲッター
        void set_main_menu_path(const NodePath& path);
        NodePath get_main_menu_path() const;
        
        void set_selection_menu_path(const NodePath& path);
        NodePath get_selection_menu_path() const;

        void set_network_menu_path(const NodePath& path);
        NodePath get_network_menu_path() const;

        void set_ip_input_path(const NodePath& path);
        NodePath get_ip_input_path() const;
    };
}