#pragma once

#include "monster_data.hpp"

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/typed_array.hpp>

#include <godot_cpp/classes/e_net_multiplayer_peer.hpp>
#include <godot_cpp/classes/multiplayer_api.hpp>

namespace godot
{
    // ゲームの進行状態を定義
    enum GameState {
        STATE_LOBBY,      // ロビー（タイトルなど）
        STATE_COLLECTION, // モンスター集め（制限時間あり）
        STATE_TOWN,       // 町（対戦準備）
        STATE_BATTLE      // 対戦中
    };

    class GameManager : public Node
    {
        GDCLASS(GameManager, Node)

    private:
        static GameManager *singleton; // 自分自身（シングルトン）へのポインタ
        String last_scene_path;        // 最後にいたシーンのパス
        
        Vector3 last_player_position;       // プレイヤーの座標
        bool is_returning_from_battle;      // バトルから帰ってきたかフラグ
        String next_enemy_scene_path;       // 次に戦う敵のシーンパス
        String current_enemy_id;            // 今戦っている敵のID
        PackedStringArray defeated_enemies; // 倒された敵のIDリスト
        Ref<MonsterData> next_enemy_data;      // --- 敵ステータス（バトル時） ---

        String next_enemy_name;     // 次に戦う敵の名前
        int next_enemy_max_hp;      // 次に戦う敵の最大HP
        int next_enemy_attack;      // 次に戦う敵の攻撃力
        int next_enemy_defense;     // 次に戦う敵の防御力
        int next_enemy_speed;       // 次に戦う敵の素早さ
        int next_enemy_exp_reward;  // 次に戦う敵の経験値報酬

        // --- プレイヤー（バトル時）ステータス ---
        // ※パーティ先頭のモンスターのステータスがここにコピーされます
        int player_max_hp;      // 最大HP
        int player_current_hp;  // 現在HP      
        int player_level;       // レベル
        int player_exp;         // 現在の経験値
        int player_next_exp;    // 次のレベルまでの必要経験
        int player_attack;      // プレイヤー攻撃力
        int player_defense;     // プレイヤー防御力
        int player_speed;       // プレイヤー素早さ

        TypedArray<MonsterData> party_monsters;   // 現在のパーティ（最大3体）
        TypedArray<MonsterData> standby_monsters; // 控え（倉庫）

        Ref<ENetMultiplayerPeer> peer; // ネットワークピア

        // --- ゲーム進行管理用 ---
        GameState current_state;
        float time_remaining;      // 残り時間（秒）
        bool is_timer_active;      // タイマーが動いているか

        // プレイヤーの準備完了状態（IDごとのマップ）
        // ※今回は簡易的に「準備完了した人数」で管理します
        int ready_player_count;

        Dictionary p1_data; // ホストの情報
        Dictionary p2_data; // クライアントの情報

    protected:
        /**
         * @brief Godot にメソッドを登録する
         * @return void
         * @note _bind_methods は protected メソッドとして宣言されている必要があります
         */
        static void _bind_methods();

        Ref<MonsterData> starter_option_1; // Speed (0)
        Ref<MonsterData> starter_option_2; // Tank (1)
        Ref<MonsterData> starter_option_3; // Balance (2)

    public:
        GameManager();
        ~GameManager();

        // どこからでも GameManager を呼べるようにする
        static GameManager *get_singleton();

        virtual void _process(double delta) override; // タイマー更新用

        /**
         * @brief サーバー（ホスト）としてゲームを開始する
         * @param port ポート番号（デフォルト8910）
         */
        void host_game(int port = 8910);

        /**
         * @brief クライアントとしてサーバーに参加する
         * @param address サーバーのIPアドレス（"127.0.0.1"など）
         * @param port ポート番号
         */
        void join_game(const String& address, int port = 8910);

        // --- モンスター操作 ---
        
        /**
         * @brief モンスターをコレクションに追加する
         * パーティが満員なら倉庫へ送る
         * @param monster 追加するモンスターのデータ
         */
        void add_monster(const Ref<MonsterData>& monster);

        /**
         * @brief ゲーム開始時に初期モンスターを選択して生成する
         * * @param type_index 選択したタイプ（0:炎, 1:水, 2:草など）
         */
        void select_starter_monster(int type_index);

        /**
         * @brief バトル開始直前に呼ぶセットアップ関数
         * パーティの先頭モンスターのステータスを、戦闘用変数にコピーする
         */
        void prepare_battle_stats();

        /**
         * @brief 現在のパーティリストを取得する
         * @return TypedArray<MonsterData> パーティモンスターの配列
         */
        TypedArray<MonsterData> get_party() const;

        void set_party(const TypedArray<MonsterData>& p_party);

        /**
         * @brief 控え（倉庫）のモンスターリストを取得する
         * @return TypedArray<MonsterData> 控えモンスターの配列
         */
        TypedArray<MonsterData> get_standby() const;

        // --- 既存機能 ---

        /**
         * @brief プレイヤーの全ステータスを初期化・設定する
         * (ゲーム開始時にPlayerクラスから呼ばれる想定)
         * @param max_hp 最大HP
         * @param attack 攻撃力
         * @param defense 防御力
         * @param level レベル
         * @param exp 現在の経験値
         * @param next_exp 次のレベルまでの必要経験値
         */
        void init_player_stats(int max_hp, int attack, int defense, int level, int exp, int next_exp);

        int get_player_speed() const; // プレイヤーの素早さを取得する

        // データのセット・ゲット
        /**
         * @brief プレイヤーの最後の座標を設定する
         * * @param pos 設定する座標
         * @note const reference を使って効率的に引数を渡す
         */
        void set_last_player_position(const Vector3 &pos);

        /**
         * @brief プレイヤーの最後の座標を取得する
         * * @return Vector3 最後の座標
         */
        Vector3 get_last_player_position() const;

        /**
         * @brief バトルから帰ってきたかどうかのフラグを設定する
         * * @param value 設定するフラグの値
         */
        void set_is_returning_from_battle(bool value);

        /**
         * @brief バトルから帰ってきたかどうかのフラグを取得する
         * * @return true バトルから帰ってきた
         * @return false バトルから帰ってきていない
         */
        bool get_is_returning_from_battle() const;

        /**
         * @brief 次に戦う敵のシーンパスを設定する
         * * @param path シーンパス
         */
        void set_next_enemy_scene_path(const String &path);

        /**
         * @brief 次に戦う敵のシーンパスを取得する
         * * @return String シーンパス
         */
        String get_next_enemy_scene_path() const;

        /**
         * @brief 現在戦っている敵のIDを設定する
         * * @param id 敵のID
         */
        void set_current_enemy_id(const String &id);
        /**
         * @brief 現在戦っている敵のIDを取得する
         * * @return String 敵のID
         */
        String get_current_enemy_id() const;

        /**
         * @brief 倒した敵のIDをリストに追加する
         * * @param id 
         * @note 同じIDが複数回追加されることはないようにする
         */
        void add_defeated_enemy(const String &id);
        /**
         * @brief 敵が倒されたかどうかを確認する
         * * @param id 敵のID
         * @return - true: 敵は倒された
         * @return - false: 敵はまだ生きている
         */
        bool is_enemy_defeated(const String &id) const;

        /**
         * @brief 次に戦う敵のステータスを設定する
         * * @param name 敵の名前
         * @param hp 敵の最大HP
         * @param attack 敵の攻撃力
         */
        void set_next_enemy_stats(const String &name, int hp, int attack);
        
        /**
         * @brief 次に戦う敵の名前を取得する
         * * @return String 敵の名前
         */
        String get_next_enemy_name() const;
        /**
         * @brief 次に戦う敵の最大HPを取得する
         * * @return int 敵の最大HP
         */
        int get_next_enemy_max_hp() const;
        /**
         * @brief 次に戦う敵の攻撃力を取得する
         * * @return int 敵の攻撃力
         */
        int get_next_enemy_attack() const;

        /**
         * @brief 次に戦う敵のデータを設定する
         * * @param data 敵のモンスターリソースデータ
         */
        void set_next_enemy_data(const Ref<MonsterData>& data);
        /**
         * @brief 次に戦う敵のデータを取得する
         * * @return Ref<MonsterData> 敵のモンスターリソースデータ
         */
        Ref<MonsterData> get_next_enemy_data() const;

        /**
         * @brief プレイヤーのステータスを設定する
         * * @param attack 攻撃力
         * @param defense 防御力
         */
        void set_player_stats(int attack, int defense); 
        /**
         * @brief プレイヤーの攻撃力を取得する
         * * @return int 攻撃力
         */
        int get_player_attack() const;
        /**
         * @brief プレイヤーの防御力を取得する
         * * @return int 防御力
         */
        int get_player_defense() const;

        /**
         * @brief 次に戦う敵の防御力を設定する
         * * @param def 敵の防御力
         */
        void set_next_enemy_defense(int def);
        /**
         * @brief 次に戦う敵の防御力を取得する
         * * @return int 敵の防御力
         */
        int get_next_enemy_defense() const;

        /**
         * @brief 次に戦う敵の素早さを設定する
         * * @param spd 敵の素早さ
         */
        void set_next_enemy_speed(int spd);
        int get_next_enemy_speed() const; // 次に戦う敵の素早さを取得する
        
        /**
         * @brief 次に戦う敵の経験値報酬を設定する
         * * @param value 敵の経験値報酬
         */
        void set_next_enemy_exp_reward(int value);
        /**
         * @brief 次に戦う敵の経験値報酬を取得する
         * * @return int 敵の経験値報酬
         */
        int get_next_enemy_exp_reward() const;

        /**
         * @brief プレイヤーの最大HPを取得する
         * * @return int 最大HP
         */
        int get_player_max_hp() const;
        /**
         * @brief プレイヤーの現在HPを取得する
         * * @return int 現在HP
         */
        int get_player_current_hp() const;
        /**
         * @brief プレイヤーの現在HPを設定する
         * * @param hp 現在HP
         * @note ダメージ計算後や回復処理後に呼ばれる想定
         */
        void set_player_current_hp(int hp);

        /**
         * @brief プレイヤーのレベルを取得する
         * * @return int レベル
         */
        int get_player_level() const;
        
        /**
         * @brief プレイヤーの現在の経験値を取得する
         * * @return int 現在の経験値
         */
        int get_player_exp() const;
        
        /**
         * @brief プレイヤーの次のレベルまでの必要経験値を取得する
         * * @return int 次のレベルまでの必要経験値
         */
        int get_player_next_exp() const;

        /**
         * @brief 経験値を獲得する処理
         * @param amount 獲得量
         */
        void gain_experience(int amount);

        /**
         * @brief プレイヤーが最後にいたシーンのパスを設定する
         * * @param path シーンのパス
         */
        void set_last_scene_path(const String &path);
        /**
         * @brief プレイヤーが最後にいたシーンのパスを取得する
         * * @return String シーンのパス
         */
        String get_last_scene_path() const;

        /**
         * @deprecated 以前の仕様。互換性のために残していますが、add_monsterを使用してください。
         */
        void add_collected_monster(const Ref<MonsterData>& monster);
        
        /**
         * @deprecated 以前の仕様。get_party または get_standby を使用してください。
         */
        TypedArray<MonsterData> get_collected_monsters() const;

        // ゲームサイクルを開始する（ホストが呼ぶ）
        void start_collection_phase();

        // [RPC] 全員に「収集フェーズ開始」を伝える
        void _rpc_start_collection();

        /**
         * @brief タイマーを同期する（ホストが呼ぶ）
         * 
         * @param time 残り時間（秒）
         */
        void _rpc_sync_timer(float time);

        // [RPC] 全員に「町へ移動」を命令する
        void _rpc_go_to_town();

        // プレイヤーが「準備完了」ボタンを押したときに呼ぶ
        void set_player_ready();

        // [RPC] サーバーに「準備できたよ」と伝える
        void _rpc_notify_ready();

        // [RPC] 全員に「バトル開始」を命令する
        void _rpc_start_battle();

        /**
         * @brief サーバーに自分のモンスター情報を登録するRPC
         * 
         * @param peer_id プレイヤーのピアID
         * @param monster_data_path モンスターデータのリソースパス
         * @param model_path モデルのリソースパス
         * @param hp モンスターのHP
         * @param speed モンスターの素早さ
         * 
         */
        void _rpc_register_battle_ready(int peer_id, const String& monster_data_path, const String& model_path, int hp, int speed);

        // 全員の準備が整ったか確認し、整っていればバトル開始RPCを呼ぶ
        void _check_and_start_battle();
        
        // 現在の残り時間を取得（UI表示用）
        float get_time_remaining() const;

        void set_starter_option_1(const Ref<MonsterData>& data);
        Ref<MonsterData> get_starter_option_1() const;

        void set_starter_option_2(const Ref<MonsterData>& data);
        Ref<MonsterData> get_starter_option_2() const;

        void set_starter_option_3(const Ref<MonsterData>& data);
        Ref<MonsterData> get_starter_option_3() const;
    };
}