/*
 * minimax_speech_client.c
 *
 * MiniMax Speech 2.8 HD TTS API – C language implementation.
 *
 * Dependencies:
 *   libcurl   (https://curl.se/libcurl/)
 *   cJSON     (https://github.com/DaveGamble/cJSON)
 *
 * Compile (MSVC):
 *   cl minimax_speech_client.c /I<curl_include> /I<cjson_include>
 *      /link libcurl.lib cjson.lib ws2_32.lib
 *
 * Compile (GCC / MinGW):
 *   gcc -o minimax_speech minimax_speech_client.c
 *       -I/usr/include -I/usr/local/include
 *       -lcurl -lcjson -lws2_32
 *
 * Compile (Linux):
 *   gcc -o minimax_speech minimax_speech_client.c -lcurl -lcjson
 */

#include "minimax_speech_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <curl/curl.h>
#include <cjson/cJSON.h>

/* ===================================================================
 *  Internal state
 * =================================================================== */

static char* g_api_key = NULL;

/* ===================================================================
 *  Voice / effect tables
 * =================================================================== */

const char* MINIMAX_VOICE_IDS[] = {
    "male-qn-qingse", "male-qn-jingying", "male-qn-badao", "male-qn-daxuesheng",
    "male-qn-qingse-jingpin", "male-qn-jingying-jingpin", "male-qn-badao-jingpin",
    "male-qn-daxuesheng-jingpin",
    "presenter_male", "audiobook_male_1", "audiobook_male_2",
    "clever_boy", "cute_boy", "bingjiao_didi", "junlang_nanyou",
    "chunzhen_xuedi", "lengdan_xiongzhang", "badao_shaoye",
    "Santa_Claus", "Grinch", "Rudolph", "Arnold", "Charming_Santa",
    "female-shaonv", "female-yujie", "female-chengshu", "female-tianmei",
    "female-shaonv-jingpin", "female-yujie-jingpin", "female-chengshu-jingpin",
    "female-tianmei-jingpin",
    "presenter_female", "audiobook_female_1", "audiobook_female_2",
    "lovely_girl", "cartoon_pig", "tianxin_xiaoling", "qiaopi_mengmei",
    "wumei_yujie", "diadia_xuemei", "danya_xuejie",
    "Charming_Lady", "Sweet_Girl", "Cute_Elf", "Attractive_Girl", "Serene_Woman",
    NULL
};
const int MINIMAX_VOICE_IDS_COUNT = 48;

const char* MINIMAX_MALE_VOICE_IDS[] = {
    "male-qn-qingse", "male-qn-jingying", "male-qn-badao", "male-qn-daxuesheng",
    "male-qn-qingse-jingpin", "male-qn-jingying-jingpin", "male-qn-badao-jingpin",
    "male-qn-daxuesheng-jingpin",
    "presenter_male", "audiobook_male_1", "audiobook_male_2",
    "clever_boy", "cute_boy", "bingjiao_didi", "junlang_nanyou",
    "chunzhen_xuedi", "lengdan_xiongzhang", "badao_shaoye",
    "Santa_Claus", "Grinch", "Rudolph", "Arnold", "Charming_Santa",
    NULL
};
const int MINIMAX_MALE_VOICE_IDS_COUNT = 23;

const char* MINIMAX_FEMALE_VOICE_IDS[] = {
    "female-shaonv", "female-yujie", "female-chengshu", "female-tianmei",
    "female-shaonv-jingpin", "female-yujie-jingpin", "female-chengshu-jingpin",
    "female-tianmei-jingpin",
    "presenter_female", "audiobook_female_1", "audiobook_female_2",
    "lovely_girl", "cartoon_pig", "tianxin_xiaoling", "qiaopi_mengmei",
    "wumei_yujie", "diadia_xuemei", "danya_xuejie",
    "Charming_Lady", "Sweet_Girl", "Cute_Elf", "Attractive_Girl", "Serene_Woman",
    NULL
};
const int MINIMAX_FEMALE_VOICE_IDS_COUNT = 23;

const char* MINIMAX_SOUND_EFFECTS[] = {
    "spacious_echo", "auditorium_echo", "lofi_telephone", "robotic", NULL
};

/* ===================================================================
 *  String utilities
 * =================================================================== */

const char* minimax_audio_format_str(MiniMaxAudioFormat fmt)
{
    switch (fmt) {
    case MINIMAX_FMT_MP3:  return "mp3";
    case MINIMAX_FMT_PCM:  return "pcm";
    case MINIMAX_FMT_FLAC: return "flac";
    case MINIMAX_FMT_WAV:  return "wav";
    default:               return "mp3";
    }
}

const char* minimax_emotion_str(MiniMaxEmotion emo)
{
    switch (emo) {
    case MINIMAX_EMO_HAPPY:     return "happy";
    case MINIMAX_EMO_SAD:       return "sad";
    case MINIMAX_EMO_ANGRY:     return "angry";
    case MINIMAX_EMO_FEARFUL:   return "fearful";
    case MINIMAX_EMO_DISGUSTED: return "disgusted";
    case MINIMAX_EMO_SURPRISED: return "surprised";
    case MINIMAX_EMO_CALM:      return "calm";
    case MINIMAX_EMO_FLUENT:    return "fluent";
    case MINIMAX_EMO_WHISPER:   return "whisper";
    default:                    return NULL;
    }
}

/* ===================================================================
 *  Life-cycle
 * =================================================================== */

int minimax_init(const char* api_key)
{
    if (!api_key || !*api_key) {
        fprintf(stderr, "MiniMax: api_key is empty\n");
        return MINIMAX_ERR_API_KEY;
    }

    free(g_api_key);
    g_api_key = _strdup(api_key);        /* _strdup / strdup */
    if (!g_api_key) return MINIMAX_ERR_API_KEY;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    return MINIMAX_OK;
}

void minimax_cleanup(void)
{
    free(g_api_key);
    g_api_key = NULL;
    curl_global_cleanup();
}

/* ===================================================================
 *  Default config
 * =================================================================== */

MiniMaxConfig minimax_config_default(void)
{
    MiniMaxConfig cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.voice_id        = "male-qn-jingying";
    cfg.speed           = 1.0f;
    cfg.vol             = 1.0f;
    cfg.pitch           = 0;
    cfg.emotion         = MINIMAX_EMO_NONE;
    cfg.audio_format    = MINIMAX_FMT_MP3;
    cfg.sample_rate     = 32000;
    cfg.bitrate         = 128000;
    cfg.channel         = 1;
    cfg.output_format   = "hex";
    return cfg;
}

/* ===================================================================
 *  Result allocation / free
 * =================================================================== */

static MiniMaxResult* minimax_result_alloc(void)
{
    MiniMaxResult* r = (MiniMaxResult*)calloc(1, sizeof(MiniMaxResult));
    return r;
}

void minimax_result_free(MiniMaxResult* result)
{
    if (!result) return;
    free(result->audio);
    free(result->trace_id);
    free(result->status_msg);
    free(result);
}

/* ===================================================================
 *  libcurl write callback
 * =================================================================== */

struct memory_buffer {
    char* data;
    size_t size;
};

static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t realsize = size * nmemb;
    struct memory_buffer* mem = (struct memory_buffer*)userp;

    char* ptr = (char*)realloc(mem->data, mem->size + realsize + 1);
    if (!ptr) return 0;   /* out of memory */

    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = '\0';
    return realsize;
}

/* ===================================================================
 *  Build JSON payload
 * =================================================================== */

static char* build_payload_json(
    const char*         text,
    const MiniMaxConfig* cfg,
    bool                stream)
{
    cJSON* root = cJSON_CreateObject();
    cJSON* voice = cJSON_CreateObject();
    cJSON* audio = cJSON_CreateObject();

    /* ---- root fields ---- */
    cJSON_AddStringToObject(root, "text", text);
    cJSON_AddBoolToObject(root, "stream", stream);

    /* ---- voice_setting ---- */
    cJSON_AddStringToObject(voice, "voice_id",
        cfg->timber_weights ? "" : cfg->voice_id);
    cJSON_AddNumberToObject(voice, "speed", cfg->speed);
    cJSON_AddNumberToObject(voice, "vol", cfg->vol);
    cJSON_AddNumberToObject(voice, "pitch", cfg->pitch);

    if (cfg->emotion != MINIMAX_EMO_NONE) {
        cJSON_AddStringToObject(voice, "emotion",
            minimax_emotion_str(cfg->emotion));
    }
    if (cfg->text_normalization) {
        cJSON_AddBoolToObject(voice, "text_normalization", true);
    }
    if (cfg->latex_read) {
        cJSON_AddBoolToObject(voice, "latex_read", true);
    }
    cJSON_AddItemToObject(root, "voice_setting", voice);

    /* ---- audio_setting ---- */
    cJSON_AddStringToObject(audio, "format",
        minimax_audio_format_str(cfg->audio_format));
    cJSON_AddNumberToObject(audio, "sample_rate", cfg->sample_rate);
    cJSON_AddNumberToObject(audio, "bitrate", cfg->bitrate);
    cJSON_AddNumberToObject(audio, "channel", cfg->channel);
    cJSON_AddItemToObject(root, "audio_setting", audio);

    /* ---- optional settings ---- */

    if (cfg->output_format && strcmp(cfg->output_format, "hex") != 0) {
        cJSON_AddStringToObject(root, "output_format", cfg->output_format);
    }

    if (cfg->language_boost && cfg->language_boost[0]) {
        cJSON_AddStringToObject(root, "language_boost", cfg->language_boost);
    }

    if (cfg->sound_effects) {
        cJSON* vm = cJSON_CreateObject();
        cJSON_AddStringToObject(vm, "sound_effects", cfg->sound_effects);
        cJSON_AddItemToObject(root, "voice_modify", vm);
    }

    if (cfg->timber_weights && cfg->timber_weights_count > 0) {
        cJSON* tw = cJSON_CreateArray();
        for (int i = 0; i < cfg->timber_weights_count; i++) {
            cJSON* item = cJSON_CreateObject();
            cJSON_AddStringToObject(item, "voice_id",
                cfg->timber_weights[i].voice_id);
            cJSON_AddNumberToObject(item, "weight",
                cfg->timber_weights[i].weight);
            cJSON_AddItemToArray(tw, item);
        }
        cJSON_AddItemToObject(root, "timber_weights", tw);
    }

    if (cfg->continuous_sound) {
        cJSON_AddBoolToObject(root, "continuous_sound", true);
    }

    if (cfg->pronunciation_dict && cfg->pronunciation_dict_count > 0) {
        cJSON* pd = cJSON_CreateObject();
        for (int i = 0; i < cfg->pronunciation_dict_count; i++) {
            cJSON_AddStringToObject(pd,
                cfg->pronunciation_dict[i].word,
                cfg->pronunciation_dict[i].pronunciation);
        }
        cJSON_AddItemToObject(root, "pronunciation_dict", pd);
    }

    if (cfg->aigc_watermark) {
        cJSON_AddBoolToObject(root, "aigc_watermark", true);
    }

    char* json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return json_str;
}

/* ===================================================================
 *  HTTP helpers
 * =================================================================== */

static int http_post(
    const char*  json_payload,
    const char*  auth_header,
    struct memory_buffer* resp_buf,
    long*        http_code)
{
    CURL* curl = curl_easy_init();
    if (!curl) return MINIMAX_ERR_NETWORK;

    struct curl_slist* headers = NULL;
    char auth[1024];

    memset(resp_buf, 0, sizeof(*resp_buf));

    snprintf(auth, sizeof(auth), "Authorization: Bearer %s", auth_header);
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, auth);

    curl_easy_setopt(curl, CURLOPT_URL, MINIMAX_ENDPOINT);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(json_payload));
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)resp_buf);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);
#ifdef _WIN32
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
#endif

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        return MINIMAX_ERR_NETWORK;
    }

    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, http_code);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    return MINIMAX_OK;
}

/* ===================================================================
 *  Parse API response
 * =================================================================== */

static MiniMaxResult* parse_response(const char* response_text)
{
    MiniMaxResult* result = minimax_result_alloc();
    if (!result) return NULL;

    cJSON* root = cJSON_Parse(response_text);
    if (!root) {
        result->status_code = -1;
        result->status_msg  = _strdup("JSON parse error");
        return result;
    }

    /* base_resp */
    cJSON* base_resp = cJSON_GetObjectItem(root, "base_resp");
    if (base_resp) {
        cJSON* sc = cJSON_GetObjectItem(base_resp, "status_code");
        if (sc && cJSON_IsNumber(sc)) result->status_code = sc->valueint;

        cJSON* sm = cJSON_GetObjectItem(base_resp, "status_msg");
        if (sm && cJSON_IsString(sm)) result->status_msg = _strdup(sm->valuestring);
    }

    /* trace_id */
    cJSON* tid = cJSON_GetObjectItem(root, "trace_id");
    if (tid && cJSON_IsString(tid)) result->trace_id = _strdup(tid->valuestring);

    /* data.audio */
    cJSON* data = cJSON_GetObjectItem(root, "data");
    if (data) {
        cJSON* aud = cJSON_GetObjectItem(data, "audio");
        if (aud && cJSON_IsString(aud)) result->audio = _strdup(aud->valuestring);
    }

    /* extra_info */
    cJSON* ei = cJSON_GetObjectItem(root, "extra_info");
    if (ei) {
        cJSON* it;
        it = cJSON_GetObjectItem(ei, "bitrate");
        if (it && cJSON_IsNumber(it)) result->bitrate = it->valueint;
        it = cJSON_GetObjectItem(ei, "audio_size");
        if (it && cJSON_IsNumber(it)) result->audio_size = it->valueint;
        it = cJSON_GetObjectItem(ei, "word_count");
        if (it && cJSON_IsNumber(it)) result->word_count = it->valueint;
        it = cJSON_GetObjectItem(ei, "audio_length");
        if (it && cJSON_IsNumber(it)) result->audio_length = it->valuedouble;
    }

    cJSON_Delete(root);
    return result;
}

/* ===================================================================
 *  Public: non-streaming synthesis
 * =================================================================== */

MiniMaxResult* minimax_synthesize(const char* text, const MiniMaxConfig* config)
{
    if (!g_api_key) {
        MiniMaxResult* r = minimax_result_alloc();
        r->status_code = -1;
        r->status_msg  = _strdup("Client not initialised – call minimax_init() first");
        return r;
    }

    MiniMaxConfig default_cfg = minimax_config_default();
    if (!config) config = &default_cfg;

    char* payload = build_payload_json(text, config, false);
    struct memory_buffer resp_buf;
    long http_code = 0;

    int ret = http_post(payload, g_api_key, &resp_buf, &http_code);
    free(payload);

    if (ret != MINIMAX_OK) {
        MiniMaxResult* r = minimax_result_alloc();
        r->status_code = (int)http_code;
        r->status_msg  = _strdup("Network error");
        free(resp_buf.data);
        return r;
    }

    if (http_code != 200) {
        MiniMaxResult* r = minimax_result_alloc();
        r->status_code = (int)http_code;
        r->status_msg  = resp_buf.data;   /* transfer ownership */
        return r;
    }

    MiniMaxResult* result = parse_response(resp_buf.data);
    free(resp_buf.data);

    if (result->status_code != 0) {
        /* API returned an error */
    }
    return result;
}

/* ===================================================================
 *  Hex decode helper
 * =================================================================== */

static int hex_char_value(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static unsigned char* hex_decode(const char* hex, size_t* out_len)
{
    size_t hex_len = strlen(hex);
    if (hex_len % 2 != 0) return NULL;

    *out_len = hex_len / 2;
    unsigned char* bytes = (unsigned char*)malloc(*out_len);
    if (!bytes) return NULL;

    for (size_t i = 0; i < *out_len; i++) {
        int hi = hex_char_value(hex[2 * i]);
        int lo = hex_char_value(hex[2 * i + 1]);
        if (hi < 0 || lo < 0) {
            free(bytes);
            return NULL;
        }
        bytes[i] = (unsigned char)((hi << 4) | lo);
    }
    return bytes;
}

/* ===================================================================
 *  Public: save & download
 * =================================================================== */

int minimax_save_audio(const char* audio_hex, const char* filepath)
{
    if (!audio_hex || !*audio_hex) return MINIMAX_ERR_NO_AUDIO;

    size_t out_len = 0;
    unsigned char* bytes = hex_decode(audio_hex, &out_len);
    if (!bytes) return MINIMAX_ERR_JSON;

    FILE* fp = fopen(filepath, "wb");
    if (!fp) {
        free(bytes);
        fprintf(stderr, "Cannot open file for writing: %s\n", filepath);
        return MINIMAX_ERR_FILE;
    }

    fwrite(bytes, 1, out_len, fp);
    fclose(fp);
    free(bytes);
    return MINIMAX_OK;
}

int minimax_download_audio_url(const char* url, const char* filepath)
{
    CURL* curl = curl_easy_init();
    if (!curl) return MINIMAX_ERR_NETWORK;

    FILE* fp = fopen(filepath, "wb");
    if (!fp) {
        curl_easy_cleanup(curl);
        return MINIMAX_ERR_FILE;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)fp);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);

    CURLcode res = curl_easy_perform(curl);

    fclose(fp);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        fprintf(stderr, "Download failed: %s\n", curl_easy_strerror(res));
        return MINIMAX_ERR_NETWORK;
    }

    return MINIMAX_OK;
}

/* ===================================================================
 *  Public: one-step text -> file
 * =================================================================== */

int minimax_text_to_speech_file(
    const char*         text,
    const char*         output_path,
    const MiniMaxConfig* config)
{
    printf("Synthesising: %.30s...\n", text);

    MiniMaxResult* result = minimax_synthesize(text, config);
    if (!result) return MINIMAX_ERR_API;

    if (result->status_code != 0) {
        fprintf(stderr, "API error (code=%d): %s\n",
                result->status_code,
                result->status_msg ? result->status_msg : "unknown");
        minimax_result_free(result);
        return MINIMAX_ERR_API;
    }

    if (!result->audio || !result->audio[0]) {
        fprintf(stderr, "No audio data received\n");
        minimax_result_free(result);
        return MINIMAX_ERR_NO_AUDIO;
    }

    printf("  words: %d\n", result->word_count);
    printf("  length: %.0f ms\n", result->audio_length);
    printf("  size: %d bytes\n", result->audio_size);
    printf("  trace_id: %s\n", result->trace_id ? result->trace_id : "N/A");

    int ret;
    if (strncmp(result->audio, "http://",  7) == 0 ||
        strncmp(result->audio, "https://", 8) == 0) {
        ret = minimax_download_audio_url(result->audio, output_path);
    } else {
        ret = minimax_save_audio(result->audio, output_path);
    }

    if (ret == 0) {
        printf("  audio saved to: %s\n", output_path);
    }

    minimax_result_free(result);
    return ret;
}

/* ===================================================================
 *  Public: streaming synthesis
 * =================================================================== */

int minimax_synthesize_stream(
    const char*              text,
    const MiniMaxConfig*     config,
    MiniMaxStreamCallback    callback,
    void*                    user_data)
{
    if (!g_api_key) return MINIMAX_ERR_API_KEY;
    if (!callback) return MINIMAX_ERR_API;

    MiniMaxConfig default_cfg = minimax_config_default();
    if (!config) config = &default_cfg;

    char* payload = build_payload_json(text, config, true);

    CURL* curl = curl_easy_init();
    if (!curl) {
        free(payload);
        return MINIMAX_ERR_NETWORK;
    }

    char auth[1024];
    snprintf(auth, sizeof(auth), "Authorization: Bearer %s", g_api_key);

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, auth);

    curl_easy_setopt(curl, CURLOPT_URL, MINIMAX_ENDPOINT);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(payload));
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);

    /* write callback for streaming */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

    struct memory_buffer line_buf;
    memset(&line_buf, 0, sizeof(line_buf));

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&line_buf);

    CURLcode res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    free(payload);

    if (res != CURLE_OK) {
        fprintf(stderr, "Stream request failed: %s\n", curl_easy_strerror(res));
        free(line_buf.data);
        return MINIMAX_ERR_NETWORK;
    }

    /* Parse SSE lines from response.
       Each chunk looks like: data: {"audio":"...","status":1}\n */
    char* saveptr = NULL;
    char* line = strtok_s(line_buf.data, "\r\n", &saveptr);
    while (line) {
        if (strncmp(line, "data: ", 6) == 0) {
            const char* json_str = line + 6;
            cJSON* chunk = cJSON_Parse(json_str);
            if (chunk) {
                cJSON* stat = cJSON_GetObjectItem(chunk, "status");
                int status = stat && cJSON_IsNumber(stat) ? stat->valueint : 0;

                cJSON* aud = cJSON_GetObjectItem(chunk, "audio");
                const char* audio_hex = (aud && cJSON_IsString(aud)) ? aud->valuestring : NULL;

                callback(audio_hex, status, user_data);
                cJSON_Delete(chunk);
            }
        }
        line = strtok_s(NULL, "\r\n", &saveptr);
    }

    free(line_buf.data);
    return MINIMAX_OK;
}

/* ===================================================================
 *  main()  – 演示 / Demo
 * =================================================================== */

#ifdef MINIMAX_STANDALONE

#ifndef _WIN32
#define _strdup strdup
#define strtok_s strtok_r
#endif

int main(int argc, char** argv)
{
    const char* api_key = getenv("PPIO_API_KEY");
    if (!api_key) {
        fprintf(stderr,
            "Please set PPIO_API_KEY environment variable.\n"
            "Usage:\n"
            "  set PPIO_API_KEY=your-key-here   (Windows)\n"
            "  export PPIO_API_KEY=your-key-here (Linux/Mac)\n"
            "  %s\n", argv[0]);
        return 1;
    }

    int ret = minimax_init(api_key);
    if (ret != MINIMAX_OK) {
        fprintf(stderr, "init failed: %d\n", ret);
        return ret;
    }

    /* ===== Example 1: Basic synthesis ===== */
    printf("==================================================\n");
    printf("Example 1: Basic non-streaming (mp3 output)\n");
    printf("==================================================\n");

    MiniMaxConfig cfg1 = minimax_config_default();
    cfg1.voice_id = "female-shaonv";

    ret = minimax_text_to_speech_file(
        "近年来，人工智能在国内迎来高速发展期，"
        "技术创新与产业应用齐头并进。",
        "output_basic.mp3",
        &cfg1);
    if (ret != MINIMAX_OK) {
        fprintf(stderr, "Example 1 failed: %d\n", ret);
    } else {
        printf("Audio saved to: output_basic.mp3\n\n");
    }

    /* ===== Example 2: Emotion + effect ===== */
    printf("==================================================\n");
    printf("Example 2: Emotion + effect + pitch/speed\n");
    printf("==================================================\n");

    MiniMaxConfig cfg2 = minimax_config_default();
    cfg2.voice_id      = "female-tianmei";
    cfg2.emotion       = MINIMAX_EMO_HAPPY;
    cfg2.speed         = 1.2f;
    cfg2.pitch         = 3;
    cfg2.audio_format  = MINIMAX_FMT_WAV;
    cfg2.sound_effects = "auditorium_echo";
    cfg2.output_format = "hex";

    ret = minimax_text_to_speech_file(
        "生日快乐！愿你每天都有好心情，天天开心！",
        "output_happy.wav",
        &cfg2);
    if (ret != MINIMAX_OK) {
        fprintf(stderr, "Example 2 failed: %d\n", ret);
    } else {
        printf("Audio saved to: output_happy.wav\n\n");
    }

    /* ===== Example 3: Mixed timbre ===== */
    printf("==================================================\n");
    printf("Example 3: Mixed timbre synthesis\n");
    printf("==================================================\n");

    MiniMaxTimberWeight weights[2] = {
        { "male-qn-jingying", 60 },
        { "female-shaonv",    40 },
    };

    MiniMaxConfig cfg3 = minimax_config_default();
    cfg3.timber_weights       = weights;
    cfg3.timber_weights_count = 2;

    ret = minimax_text_to_speech_file(
        "欢迎使用 MiniMax 语音合成服务，"
        "这里支持多种音色混合。",
        "output_mixed.mp3",
        &cfg3);
    if (ret != MINIMAX_OK) {
        fprintf(stderr, "Example 3 failed: %d\n", ret);
    } else {
        printf("Audio saved to: output_mixed.mp3\n\n");
    }

    minimax_cleanup();
    return 0;
}

#endif /* MINIMAX_STANDALONE */
