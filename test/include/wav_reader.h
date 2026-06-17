#ifndef _WAV_READER_H
#define _WAV_READER_H  

#include <vector>
#include <stdint.h>
#include <cstdio>
#include <string>

#define WAV_READER_LOG(...) do { \
    fprintf(stderr, "Wav Reader: "); fprintf(stderr, ##__VA_ARGS__); fprintf(stderr, "\n"); } while(0)
#define WAV_READER_ERROR(...) do { \
    fprintf(stderr, "Wav Reader: "); fprintf(stderr, ##__VA_ARGS__); fprintf(stderr, "\n"); } while(0)
#define WAV_READER_OUTPUT(...) do { \
    fprintf(stdout, ##__VA_ARGS__); } while(0)

//                 F F I R
#define HEX_RIFF 0x46464952
//                 E V A W
#define HEX_WAVE 0x45564157
//                   t m f
#define HEX_fmt  0x20746d66
//                 a t a d
#define HEX_data 0x61746164

namespace wav_reader{
/*---------------------- Wave header --------------------------- */
struct WavHeader {
	uint32_t   chunk_id;        // 文档标识。       大写字符串"RIFF",标明该文件为有效的 RIFF 格式文档。
	uint32_t   chunk_size;      // 文件数据长度。    从下一个字段首地址开始到文件末尾的总字节数。该字段的数值加 8 为当前文件的实际长度。
	uint32_t   format;          // 文件格式类型。    所有 WAV 格式的文件此处为字符串"WAVE",标明该文件是 WAV 格式文件。

	uint32_t   fmt_chunk_id;    // 格式块标识。      小写字符串,"fmt "。
	uint32_t  fmt_chunk_size;   // 格式块长度。      其数值不确定,取决于编码格式。可以是 16、 18 、20、40 等。
	uint16_t  audio_format;     // 编码格式代码。    常见的 WAV 文件使用 PCM 脉冲编码调制格式,该数值通常为 1。
	uint16_t  num_channels;     // 声道个数。        单声道为 1,立体声或双声道为 2。
	uint32_t  sample_rate;      // 采样频率。        每个声道单位时间采样次数。常用的采样频率有 11025, 22050 和 44100 kHz。
	uint32_t  byte_rate;        // 数据传输速率。     该数值为:声道数×采样频率×每样本的数据位数/8。播放软件利用此值可以估计缓冲区的大小。
	uint16_t  block_align;      // 数据块对齐单位。    采样帧大小。该数值为:声道数×位数/8。播放软件需要一次处理多个该值大小的字节数据,用该数值调整缓冲区。
	uint16_t  bits_per_sample;  // 采样位数。         存储每个采样值所用的二进制数位数。常见的位数有 4、8、12、16、24、32。

	uint32_t   data_chunk_id;   // 数据块标识。       小写字符串,"data"。
	uint32_t  data_chunk_size;  // 数据块大小。       一个32位无符号整数，代表数据大小
};

enum pcm_format{
    S8_LE, //signed 8 bit little endian 有符号8位小端
    S16_LE, 
    S24_LE,
    S32_LE,
    F32_LE
};

/**
 * @brief 输出文件头
 * 
 * @param header Wave文件头
 */
void ShowWavHead(WavHeader header);
int ReadWaveFile(FILE* Ifp, WavHeader* header, void* data);
int WritePCM2WavFile(FILE* Ofp, void *samples, uint32_t samples_num, uint32_t sample_rate, uint32_t bits_per_sample, uint32_t num_channels);

//todo: 修改支持不同位深，8bit，16bit，24bit，32bit
/*--------------------class wave----------------------------*/
class Wave{
public:
    Wave();
    Wave(void* samples, uint32_t samples_num, uint32_t sample_rate, uint32_t bits_per_sample, uint32_t num_channels);
    ~Wave();
    /**
    从名称为filename的音频文件中读取音频，结果以归一化的格式存到samples_vector中

        @param [in] filename：文件名

        @return >0为读取到的音频sample数，<=0为读取失败
            -1: 打开文件失败
            -2: 不是合法的wav文件
            -3: 不支持的格式        
    */
    int ReadFromFile(std::string filename);

    /**
    将samples数据还原到原本的格式，存储到samples开始的地址处

        @param [in] samples：存放音频数据的起始地址

        @return samples的字节数       
    */
    uint32_t GetSamples(void* samples);
    int GetFormat();
    int GetSampleRate();
    int GetSamplesLen();
    /**
    将归一化的samples_vector中音频转换为int16后，写入名称为filename的文件中

        @param [in] filename：文件名
    */
    int WritePCM(std::string filename);

    /**
    将归一化的samples_vector中音频转换为int16后，写入名称为filename的文件中

        @param [in] filename：文件名
    */
    int WriteWav(std::string filename);

private:
    struct WavHeader header_;
    std::vector<float> samples_;
    /**
    从文件指针Ifp处开始读取wav音频，结果为归一化到[-1, 1)后的音频样例存储位置起始地址，放到samples中

        @param [in] Ifp：输入参数，wav文件开头的文件指针

        @return >0为读取到的音频sample数，<=0为读取失败
    */ 
    int ReadWaveNormalization_(FILE* Ifp);  
    pcm_format format;
};

} //namespace wav_reader

#endif  // _WAV_READER_H