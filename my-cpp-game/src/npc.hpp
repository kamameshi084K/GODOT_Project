#pragma once

#include <godot_cpp/classes/character_body3d.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot
{
    class NPC : public CharacterBody3D
    {
        GDCLASS(NPC, CharacterBody3D)

    private:
        // 村人のセリフ
        String dialogue_text;

    protected:
        static void _bind_methods();

    public:
        NPC();
        ~NPC();

        /**
         * @brief ノードがシーンツリーに追加されたときに呼ばれる初期化メソッド
         * 
         */
        virtual void _ready() override;
        
        /**
         * @brief プレイヤーがNPCと対話するためのメソッド
         * 
         */
        void interact();

        // セッター・ゲッター
        /**
         * @brief セリフを設定する
         * 
         * @param p_text セリフのテキスト
         */
        void set_dialogue_text(const String& p_text);
        /**
         * @brief セリフを取得する
         * 
         * @return String セリフのテキスト
         * @note セリフが設定されていない場合は空文字列を返す
         * @note なぜconstなのか: セリフを取得するだけでオブジェクトの状態を変更しないため、constメソッドとして定義しています。
         */
        String get_dialogue_text() const;
    };
}