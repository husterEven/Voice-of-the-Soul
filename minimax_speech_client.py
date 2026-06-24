import os
import json
import base64
import requests


class MiniMaxSpeechClient:
    """
    MiniMax Speech 2.8 HD 同步语音合成客户端
    API 文档参考: https://ppio.com/docs/models/reference-minimax-speech-2.8-hd.md
    """

    ENDPOINT = "https://api.ppio.com/v3/minimax-speech-2.8-hd"

    # 系统音色列表
    VOICE_IDS = [
        # 男性音色
        "male-qn-qingse", "male-qn-jingying", "male-qn-badao", "male-qn-daxuesheng",
        "male-qn-qingse-jingpin", "male-qn-jingying-jingpin", "male-qn-badao-jingpin",
        "male-qn-daxuesheng-jingpin",
        "presenter_male", "audiobook_male_1", "audiobook_male_2",
        "clever_boy", "cute_boy", "bingjiao_didi", "junlang_nanyou",
        "chunzhen_xuedi", "lengdan_xiongzhang", "badao_shaoye",
        "Santa_Claus", "Grinch", "Rudolph", "Arnold", "Charming_Santa",
        # 女性音色
        "female-shaonv", "female-yujie", "female-chengshu", "female-tianmei",
        "female-shaonv-jingpin", "female-yujie-jingpin", "female-chengshu-jingpin",
        "female-tianmei-jingpin",
        "presenter_female", "audiobook_female_1", "audiobook_female_2",
        "lovely_girl", "cartoon_pig", "tianxin_xiaoling", "qiaopi_mengmei",
        "wumei_yujie", "diadia_xuemei", "danya_xuejie",
        "Charming_Lady", "Sweet_Girl", "Cute_Elf", "Attractive_Girl", "Serene_Woman",
    ]

    # 情绪列表
    EMOTIONS = ["happy", "sad", "angry", "fearful", "disgusted", "surprised", "calm", "fluent", "whisper"]

    # 音频格式
    AUDIO_FORMATS = ["mp3", "pcm", "flac", "wav"]

    # 比特率
    BITRATES = [32000, 64000, 128000, 256000]

    # 采样率
    SAMPLE_RATES = [8000, 16000, 22050, 24000, 32000, 44100]

    # 音效
    SOUND_EFFECTS = ["spacious_echo", "auditorium_echo", "lofi_telephone", "robotic"]

    def __init__(self, api_key: str = None):
        self.api_key = api_key or os.environ.get("PPIO_API_KEY", "")
        if not self.api_key:
            raise ValueError("API 密钥不能为空，请设置 PPIO_API_KEY 环境变量或传入 api_key 参数")

    def _build_headers(self) -> dict:
        return {
            "Content-Type": "application/json",
            "Authorization": f"Bearer {self.api_key}",
        }

    def _build_payload(
        self,
        text: str,
        voice_id: str = "male-qn-jingying",
        speed: float = 1.0,
        vol: float = 1.0,
        pitch: int = 0,
        emotion: str = None,
        audio_format: str = "mp3",
        sample_rate: int = 32000,
        bitrate: int = 128000,
        channel: int = 1,
        stream: bool = False,
        output_format: str = "hex",
        language_boost: str = None,
        voice_modify: dict = None,
        timber_weights: list = None,
        continuous_sound: bool = False,
        text_normalization: bool = False,
        latex_read: bool = False,
        pronunciation_dict: dict = None,
        aigc_watermark: bool = False,
    ) -> dict:
        payload = {
            "text": text,
            "stream": stream,
            "voice_setting": {
                "voice_id": voice_id,
                "speed": speed,
                "vol": vol,
                "pitch": pitch,
            },
            "audio_setting": {
                "format": audio_format,
                "sample_rate": sample_rate,
                "bitrate": bitrate,
                "channel": channel,
            },
        }

        if emotion:
            payload["voice_setting"]["emotion"] = emotion

        if output_format != "hex":
            payload["output_format"] = output_format

        if language_boost:
            payload["language_boost"] = language_boost

        if voice_modify:
            payload["voice_modify"] = voice_modify

        if timber_weights:
            payload["timber_weights"] = timber_weights
            payload["voice_setting"]["voice_id"] = ""

        if continuous_sound:
            payload["continuous_sound"] = True

        if text_normalization:
            payload["voice_setting"]["text_normalization"] = True

        if latex_read:
            payload["voice_setting"]["latex_read"] = True

        if pronunciation_dict:
            payload["pronunciation_dict"] = pronunciation_dict

        if aigc_watermark:
            payload["aigc_watermark"] = True

        return payload

    def _handle_response(self, response: requests.Response) -> dict:
        if response.status_code != 200:
            raise RuntimeError(f"请求失败 (HTTP {response.status_code}): {response.text}")

        result = response.json()

        base_resp = result.get("base_resp", {})
        status_code = base_resp.get("status_code", 0)
        if status_code != 0:
            status_msg = base_resp.get("status_msg", "未知错误")
            raise RuntimeError(f"API 错误 (code={status_code}): {status_msg}")

        return result

    def synthesize(
        self,
        text: str,
        voice_id: str = "male-qn-jingying",
        speed: float = 1.0,
        vol: float = 1.0,
        pitch: int = 0,
        emotion: str = None,
        audio_format: str = "mp3",
        sample_rate: int = 32000,
        bitrate: int = 128000,
        channel: int = 1,
        output_format: str = "hex",
        language_boost: str = None,
        voice_modify: dict = None,
        timber_weights: list = None,
        continuous_sound: bool = False,
        text_normalization: bool = False,
        latex_read: bool = False,
        pronunciation_dict: dict = None,
        aigc_watermark: bool = False,
    ) -> dict:
        """
        同步语音合成（非流式）

        返回:
            dict: {
                "audio": str,          # hex 编码的音频数据或音频下载 URL
                "trace_id": str,       # 会话 ID
                "base_resp": dict,     # 状态信息
                "extra_info": dict,    # 附加信息 (bitrate, audio_size, word_count 等)
            }
        """
        payload = self._build_payload(
            text=text,
            voice_id=voice_id,
            speed=speed,
            vol=vol,
            pitch=pitch,
            emotion=emotion,
            audio_format=audio_format,
            sample_rate=sample_rate,
            bitrate=bitrate,
            channel=channel,
            stream=False,
            output_format=output_format,
            language_boost=language_boost,
            voice_modify=voice_modify,
            timber_weights=timber_weights,
            continuous_sound=continuous_sound,
            text_normalization=text_normalization,
            latex_read=latex_read,
            pronunciation_dict=pronunciation_dict,
            aigc_watermark=aigc_watermark,
        )

        response = requests.post(
            self.ENDPOINT,
            headers=self._build_headers(),
            json=payload,
            timeout=120,
        )

        result = self._handle_response(response)
        data = result.get("data", {})
        return {
            "audio": data.get("audio", ""),
            "trace_id": result.get("trace_id", ""),
            "base_resp": result.get("base_resp", {}),
            "extra_info": result.get("extra_info", {}),
        }

    def synthesize_stream(
        self,
        text: str,
        voice_id: str = "male-qn-jingying",
        speed: float = 1.0,
        vol: float = 1.0,
        pitch: int = 0,
        emotion: str = None,
        audio_format: str = "mp3",
        sample_rate: int = 32000,
        bitrate: int = 128000,
        channel: int = 1,
        language_boost: str = None,
        voice_modify: dict = None,
        timber_weights: list = None,
        continuous_sound: bool = False,
    ):
        """
        流式语音合成

        返回:
            generator: 每次 yield 一个 dict:
                {"audio": str, "status": int}  # status: 1=合成中, 2=合成结束
        """
        payload = self._build_payload(
            text=text,
            voice_id=voice_id,
            speed=speed,
            vol=vol,
            pitch=pitch,
            emotion=emotion,
            audio_format=audio_format,
            sample_rate=sample_rate,
            bitrate=bitrate,
            channel=channel,
            stream=True,
            language_boost=language_boost,
            voice_modify=voice_modify,
            timber_weights=timber_weights,
            continuous_sound=continuous_sound,
        )

        response = requests.post(
            self.ENDPOINT,
            headers=self._build_headers(),
            json=payload,
            stream=True,
            timeout=120,
        )

        if response.status_code != 200:
            raise RuntimeError(f"请求失败 (HTTP {response.status_code}): {response.text}")

        for line in response.iter_lines(decode_unicode=True):
            if line and line.startswith("data: "):
                data = json.loads(line[6:])
                yield data

    def save_audio(self, audio_hex: str, filepath: str):
        """将 hex 编码的音频数据保存为文件"""
        audio_bytes = bytes.fromhex(audio_hex)
        os.makedirs(os.path.dirname(filepath) or ".", exist_ok=True)
        with open(filepath, "wb") as f:
            f.write(audio_bytes)
        return filepath

    def download_audio_url(self, url: str, filepath: str):
        """从返回的 URL 下载音频文件"""
        resp = requests.get(url, timeout=60)
        resp.raise_for_status()
        os.makedirs(os.path.dirname(filepath) or ".", exist_ok=True)
        with open(filepath, "wb") as f:
            f.write(resp.content)
        return filepath

    def text_to_speech_file(
        self,
        text: str,
        output_path: str = "output.mp3",
        voice_id: str = "male-qn-jingying",
        speed: float = 1.0,
        vol: float = 1.0,
        pitch: int = 0,
        emotion: str = None,
        audio_format: str = "mp3",
        sample_rate: int = 32000,
        bitrate: int = 128000,
        **kwargs,
    ) -> str:
        """
        一步完成：文本 -> 语音文件

        返回:
            str: 音频文件路径
        """
        print(f"正在合成语音: {text[:30]}...")
        result = self.synthesize(
            text=text,
            voice_id=voice_id,
            speed=speed,
            vol=vol,
            pitch=pitch,
            emotion=emotion,
            audio_format=audio_format,
            sample_rate=sample_rate,
            bitrate=bitrate,
            **kwargs,
        )

        audio = result["audio"]
        if not audio:
            raise RuntimeError("未获取到音频数据")

        extra_info = result.get("extra_info", {})
        print(f"  字数: {extra_info.get('word_count', 'N/A')}")
        print(f"  时长: {extra_info.get('audio_length', 'N/A')}ms")
        print(f"  大小: {extra_info.get('audio_size', 'N/A')} bytes")
        print(f"  trace_id: {result['trace_id']}")

        if audio.startswith("http://") or audio.startswith("https://"):
            return self.download_audio_url(audio, output_path)
        else:
            return self.save_audio(audio, output_path)


def main():
    api_key = os.environ.get("PPIO_API_KEY", "your-api-key-here")

    client = MiniMaxSpeechClient(api_key=api_key)

    # ========= 示例 1: 基础合成 =========
    print("=" * 50)
    print("示例 1: 基础非流式合成 (输出 mp3 文件)")
    print("=" * 50)
    try:
        filepath = client.text_to_speech_file(
            text="近年来，人工智能在国内迎来高速发展期，"
                 "技术创新与产业应用齐头并进。",
            output_path="output_basic.mp3",
            voice_id="female-shaonv",
        )
        print(f"音频已保存到: {filepath}\n")
    except Exception as e:
        print(f"错误: {e}\n")

    # ========= 示例 2: 带情绪和音效 =========
    print("=" * 50)
    print("示例 2: 指定情绪 + 音效 + 语调/语速")
    print("=" * 50)
    try:
        client.text_to_speech_file(
            text="生日快乐！愿你每天都有好心情，天天开心！",
            output_path="output_happy.wav",
            voice_id="female-tianmei",
            emotion="happy",
            speed=1.2,
            pitch=3,
            audio_format="wav",
            voice_modify={"sound_effects": "auditorium_echo"},
            output_format="hex",
        )
        print("音频已保存到: output_happy.wav\n")
    except Exception as e:
        print(f"错误: {e}\n")

    # ========= 示例 3: 混合音色 =========
    print("=" * 50)
    print("示例 3: 混合音色合成")
    print("=" * 50)
    try:
        client.text_to_speech_file(
            text="欢迎使用 MiniMax 语音合成服务，"
                 "这里支持多种音色混合。",
            output_path="output_mixed.mp3",
            timber_weights=[
                {"voice_id": "male-qn-jingying", "weight": 60},
                {"voice_id": "female-shaonv", "weight": 40},
            ],
        )
        print("音频已保存到: output_mixed.mp3\n")
    except Exception as e:
        print(f"错误: {e}\n")

    # ========= 示例 4: 流式合成 =========
    print("=" * 50)
    print("示例 4: 流式合成")
    print("=" * 50)
    try:
        all_audio_chunks = []
        for chunk in client.synthesize_stream(
            text="流式语音合成，适合长文本场景，可以边生成边播放。",
            voice_id="audiobook_male_1",
            speed=1.0,
        ):
            status = chunk.get("status", 0)
            audio_hex = chunk.get("audio", "")
            if audio_hex:
                all_audio_chunks.append(audio_hex)
                print(f"  收到音频块 ({len(audio_hex)} hex 字符), status={status}")
            else:
                print(f"  合成结束, status={status}")

        full_audio_hex = "".join(all_audio_chunks)
        if full_audio_hex:
            client.save_audio(full_audio_hex, "output_stream.mp3")
            print(f"音频已保存到: output_stream.mp3\n")
    except Exception as e:
        print(f"错误: {e}\n")

    # ========= 示例 5: URL 输出格式 =========
    print("=" * 50)
    print("示例 5: URL 输出格式 (返回下载链接)")
    print("=" * 50)
    try:
        result = client.synthesize(
            text="URL 格式返回，适合需要下载链接的场景。",
            voice_id="presenter_male",
            output_format="url",
        )
        print(f"音频 URL: {result['audio']}")
        print(f"trace_id: {result['trace_id']}\n")

        client.download_audio_url(result["audio"], "output_url.mp3")
        print("音频已下载到: output_url.mp3\n")
    except Exception as e:
        print(f"错误: {e}\n")


if __name__ == "__main__":
    main()
