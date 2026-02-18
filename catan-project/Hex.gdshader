shader_type canvas_item;

void fragment() {
    // 中心を基準にする (-0.5 〜 0.5)
    vec2 uv = UV - 0.5;
    vec2 p = abs(uv);
    
    // --- 90度回転（Flat Top型に変更） ---
    
    // 1. 縦幅の制限（以前の横幅制限をY軸に適用）
    //    上下の水平な辺を作ります
    float d_height = p.y - 0.433; // 0.433 = sqrt(3)/4
    
    // 2. 斜めの制限（XとYを入れ替え）
    //    斜めの辺を作ります
    float d_diag = p.x + p.y * 0.57735 - 0.5; // 0.57735 = 1/sqrt(3)
    
    // どちらか一方でもはみ出していたらアウト
    float dist = max(d_height, d_diag);
    
    // 境界を少し滑らかにしてマスクを作成
    float mask = 1.0 - smoothstep(0.0, 0.01, dist);
    
    vec4 color = texture(TEXTURE, UV);
    COLOR = vec4(color.rgb, color.a * mask);
}