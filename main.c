
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libavcodec\avcodec.h>
#include <libavutil\frame.h>
#include <libavformat\avformat.h>
#include <libswresample\swresample.h>

#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "swresample.lib")

const char *in_file = "D:\\audioTest\\input.aac";
const char *out_file = "D:\\audioTest\\out.pcm";
int main()
{
	av_register_all();

	AVFormatContext *fmt_ctx = NULL;
	AVCodecContext  *cod_ctx = NULL;
	AVCodec         *cod = NULL;

	//分配一个avformat
	fmt_ctx = avformat_alloc_context();
	if (fmt_ctx == NULL)
		printf("alloc fail");

	//打开文件，解封装
	if (avformat_open_input(&fmt_ctx, in_file, NULL, NULL) != 0)
		printf("open fail");

	//查找文件的相关流信息
	if (avformat_find_stream_info(fmt_ctx, NULL) < 0)
		printf("find stream fail");

	//输出格式信息
	av_dump_format(fmt_ctx, 0, in_file, 0);

	cod = avcodec_find_decoder(AV_CODEC_ID_AAC);
	if (cod == NULL)
		printf("find codec fail");
	AVCodecParserContext *parser = av_parser_init(cod->id);
	if (!parser) {
		printf(stderr, "Parser not found\n");
	}

	cod_ctx = avcodec_alloc_context3(cod);
	if (cod_ctx ==  NULL)
		printf("alloc context fail");
	else {
		cod_ctx->channels = 2;
		cod_ctx->sample_rate = 48000;
	}

	if (avcodec_open2(cod_ctx, cod, NULL) < 0)
		printf("can't open codec");

	FILE *out_fb = NULL;
	out_fb = fopen(out_file, "wb+");

	//创建packet,用于存储解码前的数据
	AVPacket *packet = (AVPacket *)malloc(sizeof(AVPacket));
	av_init_packet(packet);

	//设置转码后输出相关参数
	uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO;
	//采样个数
	int out_nb_samples = 1024;
	//采样格式
	enum AVSampleFormat  out_sample_fmt = AV_SAMPLE_FMT_S16;
	//采样率
	int out_sample_rate = 48000;
	//通道数
	int out_channels = av_get_channel_layout_nb_channels(out_channel_layout);
	printf("%d\n", out_channels);

	//创建buffer
	int buffer_size = av_samples_get_buffer_size(NULL, out_channels, out_nb_samples, AV_SAMPLE_FMT_S16, 1);

	//注意要用av_malloc
	uint8_t *buffer = (uint8_t *)av_malloc(buffer_size);

	//创建Frame，用于存储解码后的数据
	AVFrame *frame = av_frame_alloc();

	int got_picture;
	int64_t in_channel_layout = av_get_default_channel_layout(cod_ctx->channels);
	//打开转码器

	SwrContext *convert_ctx = swr_alloc();
	//设置转码参数
	swr_alloc_set_opts(convert_ctx, out_channel_layout, out_sample_fmt, out_sample_rate, \
		in_channel_layout, AV_SAMPLE_FMT_FLTP, 48000, 0, NULL);
	//初始化转码器
	swr_init(convert_ctx);

	//while循环，每次读取一帧，并转码
	while (av_read_frame(fmt_ctx, packet) >= 0) {
		//解码声音，老接口
		//if (avcodec_decode_audio4(cod_ctx, frame, &got_picture, packet) < 0) {
		//	printf("decode error");
		//	return -1;
		//}
		
		int ret = avcodec_send_packet(cod_ctx, packet);
		if (ret < 0) {
			printf(stderr, "Error submitting the packet to the decoder\n");
			return -1;
		}

		ret = avcodec_receive_frame(cod_ctx, frame);
		if (ret >= 0) {
			//转码
			swr_convert(convert_ctx, &buffer, buffer_size, frame->data, frame->nb_samples);
			printf("pts:%10lld\t packet size:%d\n", packet->pts, packet->size);
			fwrite(buffer, 1, buffer_size, out_fb);
		}
	}
	
	avformat_close_input(&fmt_ctx);
	avcodec_close(cod_ctx);
	av_frame_free(&frame);

	av_free_packet(packet);
	swr_free(&convert_ctx);
	fclose(out_fb);


	return 0;
}



