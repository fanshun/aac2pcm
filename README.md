根据实际文件情况，设置如下几个参数：

//设置转码后输出相关参数
uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO;

//采样格式
enum AVSampleFormat  out_sample_fmt = AV_SAMPLE_FMT_S16;

//采样率
	int out_sample_rate = 48000;

