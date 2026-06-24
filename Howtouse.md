# MiniMax Speech Client (C 语言版) 使用说明

## 环境要求

- Windows 7+ (64-bit)
- [可选] MinGW-w64 (D:\MinGW 已配置)

## 快速开始

### 1. 设置 API 密钥

```cmd
set PPIO_API_KEY=你的API密钥
```

或在 PowerShell 中:

```powershell
$env:PPIO_API_KEY = "你的API密钥"
```

### 2. 运行

```cmd
"C:\Users\Even\Desktop\Professional Training\Program\minimax_speech.exe"
```

程序会自动运行 5 个示例并生成对应的音频文件:
- `output_basic.mp3` — 基础合成
- `output_happy.wav` — 带情绪和音效
- `output_mixed.mp3` — 混合音色
- `output_stream.mp3` — 流式合成 (需启用 MINIMAX_STANDALONE)
- `output_url.mp3` — URL 下载方式 (需启用 MINIMAX_STANDALONE)

## API 参考 (C 接口)

### 初始化

```c
#include "minimax_speech_client.h"

int ret = minimax_init("your-api-key");
if (ret != MINIMAX_OK) { /* 错误处理 */ }
```

### 基础合成

```c
MiniMaxConfig cfg = minimax_config_default();
cfg.voice_id = "female-shaonv";

MiniMaxResult* result = minimax_synthesize("你好，世界！", &cfg);
// 使用 result->audio (hex 编码音频)
minimax_result_free(result);
```

### 一步合成到文件

```c
MiniMaxConfig cfg = minimax_config_default();
cfg.voice_id = "female-tianmei";
cfg.emotion = MINIMAX_EMO_HAPPY;
cfg.speed = 1.2f;

int ret = minimax_text_to_speech_file("文本内容", "output.mp3", &cfg);
```

### 流式合成

```c
void on_chunk(const char* audio_hex, int status, void* user_data) {
    if (audio_hex) {
        printf("收到音频块, status=%d\n", status);
    } else {
        printf("合成结束, status=%d\n", status);
    }
}

MiniMaxConfig cfg = minimax_config_default();
minimax_synthesize_stream("长文本...", &cfg, on_chunk, NULL);
```

### 保存音频文件

```c
minimax_save_audio(audio_hex_string, "output.mp3");
```

### 清理

```c
minimax_cleanup();
```

## 主要配置项 (MiniMaxConfig)

| 字段 | 类型 | 默认值 | 说明 |
|---|---|---|---|
| `voice_id` | `const char*` | `"male-qn-jingying"` | 音色 ID |
| `speed` | `float` | `1.0` | 语速 (0.5-2.0) |
| `vol` | `float` | `1.0` | 音量 (0.0-2.0) |
| `pitch` | `int` | `0` | 语调 (-12 ~ 12) |
| `emotion` | `MiniMaxEmotion` | `MINIMAX_EMO_NONE` | 情绪 |
| `audio_format` | `MiniMaxAudioFormat` | `MINIMAX_FMT_MP3` | 音频格式 |
| `sample_rate` | `int` | `32000` | 采样率 |
| `bitrate` | `int` | `128000` | 比特率 |
| `channel` | `int` | `1` | 声道数 |
| `output_format` | `const char*` | `"hex"` | 输出格式 ("hex" 或 "url") |
| `sound_effects` | `const char*` | `NULL` | 音效 |
| `timber_weights` | `MiniMaxTimberWeight*` | `NULL` | 混合音色权重 |
| `continuous_sound` | `bool` | `false` | 连续发声 |
| `text_normalization` | `bool` | `false` | 文本归一化 |
| `latex_read` | `bool` | `false` | LaTeX 朗读 |
| `aigc_watermark` | `bool` | `false` | AIGC 水印 |

## 可选音效

- `spacious_echo` — 空间回声
- `auditorium_echo` — 礼堂回声
- `lofi_telephone` — 复古电话
- `robotic` — 机器人

## 可选情绪

| 枚举值 | 说明 |
|---|---|
| `MINIMAX_EMO_HAPPY` | 开心 |
| `MINIMAX_EMO_SAD` | 悲伤 |
| `MINIMAX_EMO_ANGRY` | 愤怒 |
| `MINIMAX_EMO_FEARFUL` | 恐惧 |
| `MINIMAX_EMO_DISGUSTED` | 厌恶 |
| `MINIMAX_EMO_SURPRISED` | 惊讶 |
| `MINIMAX_EMO_CALM` | 平静 |
| `MINIMAX_EMO_FLUENT` | 流畅 |
| `MINIMAX_EMO_WHISPER` | 耳语 |

## 自定义编译

```cmd
gcc -o minimax_speech.exe ^
    minimax_speech_client.c cJSON.c ^
    -I"D:\MinGW\include" ^
    -L"D:\MinGW\lib" ^
    -lcurl -lws2_32 ^
    -DMINIMAX_STANDALONE
```
