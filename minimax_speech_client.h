#ifndef MINIMAX_SPEECH_CLIENT_H
#define MINIMAX_SPEECH_CLIENT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MINIMAX_ENDPOINT "https://api.ppio.com/v3/minimax-speech-2.8-hd"

/* ---------- error codes ---------- */
#define MINIMAX_OK                0
#define MINIMAX_ERR_API_KEY      -1
#define MINIMAX_ERR_NETWORK      -2
#define MINIMAX_ERR_JSON         -3
#define MINIMAX_ERR_API          -4
#define MINIMAX_ERR_NO_AUDIO     -5
#define MINIMAX_ERR_FILE         -6

/* ---------- audio format ---------- */
typedef enum {
    MINIMAX_FMT_MP3,
    MINIMAX_FMT_PCM,
    MINIMAX_FMT_FLAC,
    MINIMAX_FMT_WAV
} MiniMaxAudioFormat;

/* ---------- emotion ---------- */
typedef enum {
    MINIMAX_EMO_NONE,
    MINIMAX_EMO_HAPPY,
    MINIMAX_EMO_SAD,
    MINIMAX_EMO_ANGRY,
    MINIMAX_EMO_FEARFUL,
    MINIMAX_EMO_DISGUSTED,
    MINIMAX_EMO_SURPRISED,
    MINIMAX_EMO_CALM,
    MINIMAX_EMO_FLUENT,
    MINIMAX_EMO_WHISPER
} MiniMaxEmotion;

/* ---------- timber weight (for voice mixing) ---------- */
typedef struct {
    const char* voice_id;
    int         weight;
} MiniMaxTimberWeight;

/* ---------- pronunciation dict entry ---------- */
typedef struct {
    const char* word;
    const char* pronunciation;
} MiniMaxPronunciationEntry;

/* ---------- synthesis config ---------- */
typedef struct {
    const char*              voice_id;
    float                    speed;
    float                    vol;
    int                      pitch;
    MiniMaxEmotion           emotion;
    MiniMaxAudioFormat       audio_format;
    int                      sample_rate;
    int                      bitrate;
    int                      channel;
    const char*              output_format;       /* "hex" or "url" */
    const char*              language_boost;
    const char*              sound_effects;       /* NULL or one of SOUND_EFFECTS */
    MiniMaxTimberWeight*     timber_weights;
    int                      timber_weights_count;
    bool                     continuous_sound;
    bool                     text_normalization;
    bool                     latex_read;
    MiniMaxPronunciationEntry* pronunciation_dict;
    int                      pronunciation_dict_count;
    bool                     aigc_watermark;
} MiniMaxConfig;

/* ---------- synthesis result ---------- */
typedef struct {
    char*  audio;          /* hex-encoded audio or download URL */
    char*  trace_id;
    int    status_code;
    char*  status_msg;
    /* extra_info */
    int    bitrate;
    int    audio_size;
    int    word_count;
    double audio_length;   /* ms */
} MiniMaxResult;

/* ---------- API ---------- */

/* initialise the client (must be called once) */
int minimax_init(const char* api_key);

/* release all resources */
void minimax_cleanup(void);

/* fill config with defaults */
MiniMaxConfig minimax_config_default(void);

/* non-streaming synthesis – caller must free result with minimax_result_free() */
MiniMaxResult* minimax_synthesize(const char* text, const MiniMaxConfig* config);

/* one-step: text -> audio file. Returns 0 on success */
int minimax_text_to_speech_file(
    const char* text,
    const char* output_path,
    const MiniMaxConfig* config
);

/* save hex-encoded audio to file. Returns 0 on success */
int minimax_save_audio(const char* audio_hex, const char* filepath);

/* download audio from URL. Returns 0 on success */
int minimax_download_audio_url(const char* url, const char* filepath);

/* free a MiniMaxResult */
void minimax_result_free(MiniMaxResult* result);

/* convert enum to string */
const char* minimax_audio_format_str(MiniMaxAudioFormat fmt);
const char* minimax_emotion_str(MiniMaxEmotion emo);

/* ---------- streaming ---------- */

/* callback type for streaming chunks.
   audio_hex may be NULL when the stream ends; status is 1=synthesising, 2=done */
typedef void (*MiniMaxStreamCallback)(
    const char* audio_hex,
    int         status,
    void*       user_data
);

/* streaming synthesis. Returns 0 on success */
int minimax_synthesize_stream(
    const char*              text,
    const MiniMaxConfig*     config,
    MiniMaxStreamCallback    callback,
    void*                    user_data
);

/* ---------- built-in voice ids ---------- */
extern const char* MINIMAX_VOICE_IDS[];        /* NULL-terminated */
extern const int   MINIMAX_VOICE_IDS_COUNT;

extern const char* MINIMAX_MALE_VOICE_IDS[];   /* NULL-terminated */
extern const int   MINIMAX_MALE_VOICE_IDS_COUNT;

extern const char* MINIMAX_FEMALE_VOICE_IDS[]; /* NULL-terminated */
extern const int   MINIMAX_FEMALE_VOICE_IDS_COUNT;

extern const char* MINIMAX_SOUND_EFFECTS[];

#ifdef __cplusplus
}
#endif

#endif /* MINIMAX_SPEECH_CLIENT_H */
