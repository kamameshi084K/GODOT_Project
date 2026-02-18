#include "hex_tile.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

HexTile::HexTile()
{
    // 初期値の設定
    q_coord = 0;
    r_coord = 0;
    number = 0;
    tile_type = FOREST;
}

HexTile::~HexTile()
{
}

void HexTile::_ready()
{
    // 初期化時に見た目を反映
    _update_visuals();
}

void HexTile::_bind_methods()
{
    // プロパティをインスペクターに登録
    ClassDB::bind_method(D_METHOD("set_q", "q"), &HexTile::set_q);
    ClassDB::bind_method(D_METHOD("get_q"), &HexTile::get_q);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "q"), "set_q", "get_q");

    ClassDB::bind_method(D_METHOD("set_r", "r"), &HexTile::set_r);
    ClassDB::bind_method(D_METHOD("get_r"), &HexTile::get_r);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "r"), "set_r", "get_r");

    ClassDB::bind_method(D_METHOD("set_tile_type", "type"), &HexTile::set_tile_type);
    ClassDB::bind_method(D_METHOD("get_tile_type"), &HexTile::get_tile_type);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "tile_type", PROPERTY_HINT_ENUM, "Forest,Hill,Pasture,Field,Mountain,Desert,Sea"), "set_tile_type", "get_tile_type");

    ClassDB::bind_method(D_METHOD("set_number", "number"), &HexTile::set_number);
    ClassDB::bind_method(D_METHOD("get_number"), &HexTile::get_number);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "number"), "set_number", "get_number");
}

void HexTile::_update_visuals()
{
    // 1. TileTypeに応じた画像のロード
    String path = "res://assets/images/forest.png";
    
    switch (tile_type)
    {
        case FOREST:   path = "res://assets/images/forest.png"; break;
        case HILL:     path = "res://assets/images/hill.png"; break;
        case PASTURE:  path = "res://assets/images/pasture.png"; break;
        case FIELD:    path = "res://assets/images/field.png"; break;
        case MOUNTAIN: path = "res://assets/images/mountain.png"; break;
        case DESERT:   path = "res://assets/images/desert.png"; break;
        case SEA:      path = "res://assets/images/sea.png"; break;
    }

    Ref<Texture2D> tex = ResourceLoader::get_singleton()->load(path);
    if (tex.is_valid())
    {
        set_texture(tex);
    }

    // 2. タイル上の数字ラベルの更新
    Node* node = get_node_or_null("NumberLabel");
    Label* label = Object::cast_to<Label>(node);

    if (label)
    {
        if (number > 0)
        {
            label->set_text(String::num_int64(number));
            label->set_visible(true);

            // カタンのルール準拠：確率の高い6と8は赤色で強調
            if (number == 6 || number == 8)
            {
                label->set_modulate(Color(1, 0, 0));
            }
            else
            {
                label->set_modulate(Color(1, 1, 1));
            }
        }
        else
        {
            // 数字がない（砂漠や海など）場合は非表示
            label->set_visible(false);
        }
    }
}

// --- セッター・ゲッターの実装 ---

void HexTile::set_q(int p_q)
{
    q_coord = p_q;
}

int HexTile::get_q() const
{
    return q_coord;
}

void HexTile::set_r(int p_r)
{
    r_coord = p_r;
}

int HexTile::get_r() const
{
    return r_coord;
}

void HexTile::set_number(int p_number)
{
    number = p_number;
    _update_visuals();
}

int HexTile::get_number() const
{
    return number;
}

void HexTile::set_tile_type(TileType p_type)
{
    tile_type = p_type;
    _update_visuals();
}

HexTile::TileType HexTile::get_tile_type() const
{
    return tile_type;
}