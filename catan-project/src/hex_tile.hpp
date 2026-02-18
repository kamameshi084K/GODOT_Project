#pragma once

#include <godot_cpp/classes/sprite2d.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/label_settings.hpp>

namespace godot
{
    /**
     * @class HexTile
     * @brief 六角形グリッドのタイルを管理するクラス
     */
    class HexTile : public Sprite2D
    {
        GDCLASS(HexTile, Sprite2D)

    public:
        // タイルの種類（リソースタイプ）
        enum TileType
        {
            FOREST,   // 木
            HILL,     // レンガ
            PASTURE,  // 羊
            FIELD,    // 麦
            MOUNTAIN, // 鉄
            DESERT,   // 砂漠
            SEA       // 海
        };

    private:
        // 六角形座標 (Axial Coordinates) のQ
        int q_coord;
        // 六角形座標 (Axial Coordinates) のR
        int r_coord;
        // タイルに割り当てられた数字（2-12）
        int number;
        // タイルの種類
        TileType tile_type;

        // 内部の見た目更新処理
        void _update_visuals();

    protected:
        // Godotへのメソッド登録用
        static void _bind_methods();
    
    public:
        // コンストラクタ
        HexTile();
        // デストラクタ
        ~HexTile();

        // ノード準備完了時の処理
        void _ready() override;

        // --- セッター・ゲッター ---

        /**
         * @brief Q座標を設定
         * @param p_q 設定する値
         */
        void set_q(int p_q);
        int get_q() const;

        /**
         * @brief R座標を設定
         * @param p_r 設定する値
         */
        void set_r(int p_r);
        int get_r() const;

        /**
         * @brief タイルの数字を設定（見た目も更新）
         * @param p_number 設定する数字
         */
        void set_number(int p_number);
        int get_number() const;

        /**
         * @brief タイルの種類を設定（見た目も更新）
         * @param p_type 設定する種類
         */
        void set_tile_type(TileType p_type);
        TileType get_tile_type() const;
    };
}

VARIANT_ENUM_CAST(HexTile::TileType);