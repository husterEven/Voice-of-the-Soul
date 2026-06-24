> ## Documentation Index
> Fetch the complete documentation index at: https://ppio.com/docs/llms.txt
> Use this file to discover all available pages before exploring further.

# MiniMax Speech 2.8 HD 同步语音合成

MiniMax 同步语音合成 API，使用 HTTP 网络通信协议进行同步语音合成。支持多种音色、情绪、语速等参数设置。

## 请求头

<ParamField header="Content-Type" type="string" required={true}>
  枚举值: `application/json`
</ParamField>

<ParamField header="Authorization" type="string" required={true}>
  Bearer 身份验证格式，例如：Bearer \{\{API 密钥}}。
</ParamField>

## 请求体

<ParamField body="text" type="string" required={true}>
  需要合成语音的文本，长度限制小于 10000 字符，若文本长度大于 3000 字符，推荐使用流式输出。支持段落切换（换行符）、停顿控制（`&lt;#x#&gt;`标记）、语气词标签（如(laughs)、(coughs)等，仅 speech-2.8-hd/turbo 支持）
</ParamField>

<ParamField body="stream" type="boolean" default={false}>
  控制是否流式输出。默认 false，即不开启流式
</ParamField>

<ParamField body="voice_modify" type="object">
  <Expandable title="properties" defaultOpen={true}>
    <ParamField body="pitch" type="integer">
      音高调整（低沉/明亮），范围 \[-100, 100]，数值接近 -100，声音更低沉；接近 100，声音更明亮

      取值范围：\[-100, 100]
    </ParamField>

    <ParamField body="timbre" type="integer">
      音色调整（磁性/清脆），范围 \[-100, 100]，数值接近 -100，声音更浑厚；数值接近 100，声音更清脆

      取值范围：\[-100, 100]
    </ParamField>

    <ParamField body="intensity" type="integer">
      强度调整（力量感/柔和），范围 \[-100, 100]，数值接近 -100，声音更刚劲；接近 100，声音更轻柔

      取值范围：\[-100, 100]
    </ParamField>

    <ParamField body="sound_effects" type="string">
      音效设置，单次仅能选择一种，可选值：spacious\_echo（空旷回音）、auditorium\_echo（礼堂广播）、lofi\_telephone（电话失真）、robotic（电音）

      可选值：`spacious_echo`, `auditorium_echo`, `lofi_telephone`, `robotic`
    </ParamField>
  </Expandable>
</ParamField>

<ParamField body="audio_setting" type="object">
  <Expandable title="properties" defaultOpen={true}>
    <ParamField body="format" type="string" default="mp3">
      生成音频的格式，wav 仅在非流式输出下支持

      可选值：`mp3`, `pcm`, `flac`, `wav`
    </ParamField>

    <ParamField body="bitrate" type="integer" default={128000}>
      生成音频的比特率。可选范围 \[32000, 64000, 128000, 256000]，默认值为 128000。该参数仅对 mp3 格式的音频生效

      可选值：`32000`, `64000`, `128000`, `256000`
    </ParamField>

    <ParamField body="channel" type="integer" default={1}>
      生成音频的声道数。可选范围：\[1, 2]，其中 1 为单声道，2 为双声道，默认值为 1

      可选值：`1`, `2`
    </ParamField>

    <ParamField body="force_cbr" type="boolean" default={false}>
      对于音频恒定比特率（cbr）控制，可选 false、true。当此参数设置为 true，将以恒定比特率方式进行音频编码。注意：本参数仅当音频设置为流式输出，且音频格式为 mp3 时生效
    </ParamField>

    <ParamField body="sample_rate" type="integer" default={32000}>
      生成音频的采样率。可选范围 \[8000, 16000, 22050, 24000, 32000, 44100]，默认为 32000

      可选值：`8000`, `16000`, `22050`, `24000`, `32000`, `44100`
    </ParamField>
  </Expandable>
</ParamField>

<ParamField body="output_format" type="string" default="hex">
  控制输出结果形式的参数，可选值范围为 url、hex，默认值为 hex。该参数仅在非流式场景生效，流式场景仅支持返回 hex 形式。返回的 url 有效期为 24 小时

  可选值：`url`, `hex`
</ParamField>

<ParamField body="voice_setting" type="object">
  <Expandable title="properties" defaultOpen={true}>
    <ParamField body="vol" type="number" default={1}>
      合成音频的音量，取值越大，音量越高。取值范围 (0, 10]，默认值为 1.0

      取值范围：\[0, 10]
    </ParamField>

    <ParamField body="pitch" type="integer" default={0}>
      合成音频的语调，取值范围 \[-12, 12]，默认值为 0，其中 0 为原音色输出

      取值范围：\[-12, 12]
    </ParamField>

    <ParamField body="speed" type="number" default={1}>
      合成音频的语速，取值越大，语速越快。取值范围 \[0.5, 2]，默认值为 1.0

      取值范围：\[0.5, 2]
    </ParamField>

    <ParamField body="emotion" type="string">
      控制合成语音的情绪，参数范围分别对应 8 种情绪：高兴(happy)，悲伤(sad)，愤怒(angry)，害怕(fearful)，厌恶(disgusted)，惊讶(surprised)，中性(calm)，生动(fluent)，低语(whisper)。模型会根据输入文本自动匹配合适的情绪，一般无需手动指定

      可选值：`happy`, `sad`, `angry`, `fearful`, `disgusted`, `surprised`, `calm`, `fluent`, `whisper`
    </ParamField>

    <ParamField body="voice_id" type="string" required={true}>
      合成音频的音色编号。若需要设置混合音色，请设置 timber\_weights 参数，本参数设置为空值。支持系统音色、复刻音色以及文生音色三种类型
    </ParamField>

    <ParamField body="latex_read" type="boolean" default={false}>
      控制是否朗读 latex 公式，默认为 false。仅支持中文，开启该参数后，language\_boost 参数会被设置为 Chinese
    </ParamField>

    <ParamField body="text_normalization" type="boolean" default={false}>
      是否启用中文、英语文本规范化，开启后可提升数字阅读场景的性能，但会略微增加延迟，默认值为 false
    </ParamField>
  </Expandable>
</ParamField>

<ParamField body="aigc_watermark" type="boolean" default={false}>
  控制在合成音频的末尾添加音频节奏标识，默认值为 false。该参数仅对非流式合成生效
</ParamField>

<ParamField body="language_boost" type="string">
  是否增强对指定的小语种和方言的识别能力。默认值为 null，可设置为 auto 让模型自主判断

  可选值：`Chinese`, `Chinese,Yue`, `English`, `Arabic`, `Russian`, `Spanish`, `French`, `Portuguese`, `German`, `Turkish`, `Dutch`, `Ukrainian`, `Vietnamese`, `Indonesian`, `Japanese`, `Italian`, `Korean`, `Thai`, `Polish`, `Romanian`, `Greek`, `Czech`, `Finnish`, `Hindi`, `Bulgarian`, `Danish`, `Hebrew`, `Malay`, `Persian`, `Slovak`, `Swedish`, `Croatian`, `Filipino`, `Hungarian`, `Norwegian`, `Slovenian`, `Catalan`, `Nynorsk`, `Tamil`, `Afrikaans`, `auto`
</ParamField>

<ParamField body="stream_options" type="object">
  <Expandable title="properties" defaultOpen={true}>
    <ParamField body="exclude_aggregated_audio" type="boolean" default={false}>
      设置最后一个 chunk 是否包含拼接后的语音 hex 数据。默认值为 false，即最后一个 chunk 中包含拼接后的完整语音 hex 数据
    </ParamField>
  </Expandable>
</ParamField>

<ParamField body="timber_weights" type="array">
  混合音色设置，最多支持 4 种音色混合

  <Expandable title="properties" defaultOpen={true}>
    <ParamField body="weight" type="integer" required={true}>
      合成音频各音色所占的权重，须与 voice\_id 同步填写。可选值范围为 \[1, 100]，最多支持 4 种音色混合，单一音色取值占比越高，合成音色与该音色相似度越高

      取值范围：\[1, 100]
    </ParamField>

    <ParamField body="voice_id" type="string" required={true}>
      合成音频的音色编号，须和 weight 参数同步填写。支持系统音色、复刻音色以及文生音色三种类型
    </ParamField>
  </Expandable>
</ParamField>

<ParamField body="subtitle_enable" type="boolean" default={false}>
  控制是否开启字幕服务，默认值为 false。此参数仅在非流式输出场景下有效，且仅对 speech-2.6-hd, speech-2.6-turbo, speech-02-turbo, speech-02-hd, speech-01-turbo, speech-01-hd 模型有效
</ParamField>

<ParamField body="continuous_sound" type="boolean" default={false}>
  启用该参数，使得子句衔接处更自然，仅支持 speech-2.8-hd 和 speech-2.8-turbo 模型
</ParamField>

<ParamField body="pronunciation_dict" type="object">
  <Expandable title="properties" defaultOpen={true}>
    <ParamField body="tone" type="array">
      定义需要特殊标注的文字或符号对应的注音或发音替换规则。在中文文本中，声调用数字表示：一声为 1，二声为 2，三声为 3，四声为 4，轻声为 5。示例：\["燕少飞/(yan4)(shao3)(fei1)", "omg/oh my god"]
    </ParamField>
  </Expandable>
</ParamField>

## 响应

<ResponseField name="data" type="object">
  返回的合成数据对象，可能为 null，需进行非空判断

  <Expandable title="properties" defaultOpen={true}>
    <ResponseField name="audio" type="string">
      合成后的音频数据，采用 hex 编码，格式与请求中指定的输出格式一致
    </ResponseField>

    <ResponseField name="status" type="integer">
      当前音频流状态：1 表示合成中，2 表示合成结束
    </ResponseField>

    <ResponseField name="subtitle_file" type="string">
      合成的字幕下载链接。音频文件对应的字幕，精确到句（不超过 50 字），单位为毫秒，格式为 json
    </ResponseField>
  </Expandable>
</ResponseField>

<ResponseField name="trace_id" type="string">
  本次会话的 id，用于在咨询/反馈时帮助定位问题
</ResponseField>

<ResponseField name="base_resp" type="object">
  本次请求的状态码和详情

  <Expandable title="properties" defaultOpen={true}>
    <ResponseField name="status_msg" type="string">
      状态详情
    </ResponseField>

    <ResponseField name="status_code" type="integer">
      状态码。0: 请求结果正常, 1000: 未知错误, 1001: 超时, 1002: 触发限流, 1004: 鉴权失败, 1039: 触发 TPM 限流, 1042: 非法字符超过 10%, 2013: 输入参数信息不正常
    </ResponseField>
  </Expandable>
</ResponseField>

<ResponseField name="extra_info" type="object">
  音频的附加信息

  <Expandable title="properties" defaultOpen={true}>
    <ResponseField name="bitrate" type="integer">
      音频比特率
    </ResponseField>

    <ResponseField name="audio_size" type="integer">
      音频文件大小（字节）
    </ResponseField>

    <ResponseField name="word_count" type="integer">
      已发音的字数统计，包含汉字、数字、字母，不包含标点符号
    </ResponseField>

    <ResponseField name="audio_format" type="string">
      生成音频文件的格式。取值范围 \[mp3, pcm, flac]

      可选值：`mp3`, `pcm`, `flac`
    </ResponseField>

    <ResponseField name="audio_length" type="integer">
      音频时长（毫秒）
    </ResponseField>

    <ResponseField name="audio_channel" type="integer">
      生成音频声道数，1：单声道，2：双声道
    </ResponseField>

    <ResponseField name="usage_characters" type="integer">
      计费字符数
    </ResponseField>

    <ResponseField name="audio_sample_rate" type="integer">
      音频采样率
    </ResponseField>

    <ResponseField name="invisible_character_ratio" type="number">
      非法字符占比。非法字符不超过 10%（包含 10%），音频会正常生成，并返回非法字符占比数据；如超过 10% 将进行报错
    </ResponseField>
  </Expandable>
</ResponseField>
