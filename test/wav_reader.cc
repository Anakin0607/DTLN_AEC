// Only Support Little endian encoding
#include "wav_reader.h"

#include <cstring>
#include <iostream>
#include <vector>

namespace wav_reader{
void ShowWavHead(WavHeader header) {
	WAV_READER_OUTPUT("ChunkID: 0x%08x\t", header.chunk_id);
	WAV_READER_OUTPUT("ChunkSize: %u\t", header.chunk_size);
	WAV_READER_OUTPUT("Format: 0x%08x\n", header.format);
	WAV_READER_OUTPUT("FmtChunkID: 0x%08x\t", header.fmt_chunk_id);
	WAV_READER_OUTPUT("FmtChunkSize: %u\t", header.fmt_chunk_size);
	WAV_READER_OUTPUT("AudioFormat: %d\t", header.audio_format);
	WAV_READER_OUTPUT("NumChannels: %d\t", header.num_channels);
	WAV_READER_OUTPUT("SampleRate: %u\t", header.sample_rate);
	WAV_READER_OUTPUT("ByteRate: %u\t", header.byte_rate);
	WAV_READER_OUTPUT("BlockAlign: %d\t", header.block_align);
	WAV_READER_OUTPUT("BitsPerSample: %d\n", header.bits_per_sample);
	WAV_READER_OUTPUT("DataChunkID: 0x%08x\t", header.data_chunk_id);
	WAV_READER_OUTPUT("DataChunkSize: %u\n", header.data_chunk_size);
}

inline bool CheckWavHeader(WavHeader header){
    if(header.chunk_id != HEX_RIFF){
        WAV_READER_ERROR("Expected chunk_id RIFF. Given: 0x%08x\n", header.chunk_id);
        return false;
    }
    if(header.format != HEX_WAVE){
        WAV_READER_ERROR("Expected chunk_id WAVE. Given: 0x%08x\n", header.format);
        return false;
    }
    
    if(header.fmt_chunk_id != HEX_fmt){
        WAV_READER_ERROR("Expected chunk_id fmt . Given: 0x%08x\n", header.fmt_chunk_id);
        return false;
    }

    if(header.fmt_chunk_size != 16 &&
       header.fmt_chunk_size != 18) {
        // 16 for sample pcm
        // 18 for Microsoft ADPCM or IEEE floating point PCM
        WAV_READER_ERROR("Expected fmt_chunk_size to be 16 or 18. Given: %d\n", header.fmt_chunk_size);
        return false;
    }

    if(header.audio_format != 1 && header.audio_format != 3){
        // 1 for integer PCM
        // 3 for floating point PCM
        WAV_READER_ERROR("Expected audio_format to be 1 or 3. Given: %d\n", header.audio_format);
        return false;
    }

    if(header.byte_rate != header.num_channels * header.sample_rate * (header.bits_per_sample / 8)) {
        WAV_READER_ERROR("Incorrect byte rate: %d. Expected: %d", header.byte_rate,
            header.num_channels * header.sample_rate * (header.bits_per_sample / 8));
        return false;
    }

    if(header.block_align != header.num_channels * (header.bits_per_sample / 8)){
        WAV_READER_ERROR("Incorrect block align: %d. Expected: %d\n", header.block_align, 
            header.num_channels * (header.bits_per_sample / 8));
        return false;
    }

    if(header.bits_per_sample != 8 && header.bits_per_sample != 16 &&
        header.bits_per_sample != 32){
        WAV_READER_ERROR("Expected bits_per_sample 8, 16 or 32. Given: %d\n",
              header.bits_per_sample);
        return false;

    }
    return true;
}

int ReadWaveFile(FILE* Ifp, WavHeader* header, void* data){
    fseek(Ifp, 0L, SEEK_END);  
    int FileEnd = ftell(Ifp);   
    WAV_READER_LOG("Total file size: %d bytes \n", FileEnd);   
    rewind(Ifp);

    fread(header, 1, sizeof(WavHeader), Ifp); //从Ifp处开始读文件，从header处地址开始放，每次读一字节， 一共sizeof(WAVHEADER)字节，
    
    if(! CheckWavHeader(*header)){
        return -1;
    }

    if(header->data_chunk_size != (FileEnd - 44)){
        WAV_READER_ERROR("Invalid data_chunk_size. Expected %d, got %d", header->data_chunk_size, FileEnd - 44);
        return -1;
    }

    int _byte_per_sample = header->bits_per_sample / 8; //每个样例字节数=采样位深/8
    int _samples_num = header->data_chunk_size / _byte_per_sample; //音频采样数 计算方法为：(文件总字节数 - 44) / (每个样例的字节数)
    return fread(data, _byte_per_sample, _samples_num, Ifp);
}

int WritePCM2WavFile(FILE* Ofp, void *samples, uint32_t samples_num, uint32_t sample_rate, uint32_t bits_per_sample, uint32_t num_channels){
    WavHeader wav_header;
    int bytes_per_sample = bits_per_sample / 8;

    wav_header.chunk_id = HEX_RIFF;
    wav_header.chunk_size = 44 + samples_num * bytes_per_sample - 8;
    wav_header.format = HEX_WAVE;

    wav_header.fmt_chunk_id = HEX_fmt;
    wav_header.fmt_chunk_size = 16;
    wav_header.audio_format = 1;
    wav_header.num_channels = num_channels;
    wav_header.sample_rate = sample_rate;
    wav_header.byte_rate = num_channels * sample_rate * bytes_per_sample;
    wav_header.block_align = num_channels * bytes_per_sample;
    wav_header.bits_per_sample = bits_per_sample;

    wav_header.data_chunk_id = HEX_data;
    wav_header.data_chunk_size = samples_num * bytes_per_sample;

    int ret = fwrite(&wav_header, sizeof(wav_header), 1, Ofp);
    if(ret < 0){
        WAV_READER_ERROR("Write wave header failed\n");
        return -1;
    }

    ret += fwrite(samples, bytes_per_sample, samples_num, Ofp);
    return ret;
}
int AddPCM2WavFile(FILE* Ofp, void *samples, uint32_t samples_num, uint32_t sample_rate, uint32_t bits_per_sample, uint32_t num_channels){
    return 0;
}

Wave::Wave(){}
Wave::Wave(void* samples, uint32_t samples_num, uint32_t sample_rate, uint32_t bits_per_sample, uint32_t num_channels){
    /*------------Write wave header------------------*/
    header_.chunk_id = HEX_RIFF;
    header_.chunk_size = 44 + samples_num * bits_per_sample - 8;
    header_.format = HEX_WAVE;

    header_.fmt_chunk_id = HEX_fmt;
    header_.fmt_chunk_size = 16;
    header_.audio_format = 1;
    header_.num_channels = num_channels;
    header_.sample_rate = sample_rate;
    header_.byte_rate = num_channels * sample_rate * bits_per_sample / 8;
    header_.block_align = num_channels * bits_per_sample / 8;
    header_.bits_per_sample = bits_per_sample;

    header_.data_chunk_id = HEX_data;
    header_.data_chunk_size = samples_num * bits_per_sample / 8;

    if(num_channels > 1){
        WAV_READER_OUTPUT("Warning: %d channels are found. We only use the first channel.\n", header_.num_channels);
    }

    samples_.resize(samples_num / num_channels);

    if(bits_per_sample == 8){
        format = S8_LE;
        uint8_t* _samples = (uint8_t*)samples;
        for(int i = 0; i < samples_num / num_channels; i++){
            samples_[i] = (float)_samples[i * header_.num_channels] / 128 - 1;
        }
    }
    else if(bits_per_sample == 16){
        format = S16_LE;
        uint16_t* _samples = (uint16_t*)samples;
        for(int i = 0; i < samples_num / num_channels; i++){
            samples_[i] = (float)_samples[i * header_.num_channels] / 32768;
        }
    }
    else if(bits_per_sample == 32){
        format = S32_LE;
        uint32_t* _samples = (uint32_t*)samples;
        for(int i = 0; i < samples_num / num_channels; i++){
            samples_[i] = (float)_samples[i * header_.num_channels] / (1 << 31);
        }
    }
    else{
        WAV_READER_ERROR("Unsupported bits_per_sample: %d\n", bits_per_sample);
        return;
    }
}
Wave::~Wave(){
    // nothing to do
}

uint32_t Wave::GetSamples(void* samples){
    switch(format){
        case S8_LE:{
            int8_t* write_pcm = (int8_t*)samples;
            for(int i = 0; i < samples_.size(); i++){
                write_pcm[i] = samples_[i] * 128;
            }
            break;
        }
        case S16_LE:{
            int16_t* write_pcm = (int16_t*)samples;
            for(int i = 0; i < samples_.size(); i++){
                write_pcm[i] = samples_[i] * 32768;
            }
            break;
        }
        case S32_LE:{
            int32_t* write_pcm = (int32_t*)samples;
            for(int i = 0; i < samples_.size(); i++){
                write_pcm[i] = samples_[i] * (1 << 31);
            }
            break;
        }
    }
    return samples_.size() * (header_.bits_per_sample / 8);
}
int Wave::GetSampleRate(){
    return header_.sample_rate;
}

int Wave::GetSamplesLen(){
    return samples_.size();
}

int Wave::GetFormat(){
    return (int)format;
}

int Wave::ReadWaveNormalization_(FILE* Ifp){
    /* 获得文件字节数,fseek函数将文件内部指针指向文件末尾，
    ftell函数获取文件内部指针相对于文件头的偏移量，rewind函数将文件内部指针指向文件头 */
    fseek(Ifp, 0L, SEEK_END);  
    int FileEnd = ftell(Ifp);   
    WAV_READER_LOG("Total file size: %d bytes \n", FileEnd);   
    rewind(Ifp); 

    fread(&header_, 1, sizeof(WavHeader), Ifp); //从Ifp处开始读文件，从header处地址开始放，每次读一字节， 一共sizeof(WAVHEADER)字节，

    if(!CheckWavHeader(header_)){
        return -2;
    }

    if(header_.num_channels > 1){
        WAV_READER_OUTPUT("Warning: %d channels are found. We only use the first channel.\n", header_.num_channels);
    }

    int _byte_per_sample = header_.bits_per_sample / 8; //每个样例字节数=采样位深/8
    int _samples_num = (FileEnd - 44) / _byte_per_sample; //音频采样数 计算方法为：(文件总字节数 - 44) / (每个样例的字节数)

    samples_.resize(_samples_num / header_.num_channels);
    
    if(header_.bits_per_sample == 8 && header_.audio_format == 1){ //线性pcm编码，8位深采样
        format = S8_LE;
        int8_t* _samples = (int8_t*)malloc(_samples_num * sizeof(int8_t)); 
        fread(_samples, _byte_per_sample, _samples_num, Ifp);

        for(int i = 0; i < samples_.size(); ++i){
            samples_[i] = (float)_samples[i * header_.num_channels] / 128;
        }
        free(_samples);
    }
    else if(header_.bits_per_sample == 16 && header_.audio_format == 1){ //线性pcm编码，16位深采样
        format = S16_LE;
        int16_t* _samples = (int16_t*)malloc(_samples_num * sizeof(int16_t)); 
        fread(_samples, _byte_per_sample, _samples_num, Ifp);

        for(int i = 0; i < samples_.size(); ++i){
            samples_[i] = (float)_samples[i * header_.num_channels] / 32768;
        }
        free(_samples);
    }
    else if(header_.bits_per_sample == 32 && header_.audio_format == 1){ //线性pcm编码，32位深采样
        format = S32_LE;
        int32_t* _samples = (int32_t*)malloc(_samples_num * sizeof(int32_t)); 
        fread(_samples, _byte_per_sample, _samples_num, Ifp);

        for(int i = 0; i < samples_.size(); ++i){
            samples_[i] = (float)_samples[i * header_.num_channels] / (1 << 31);
        }
        free(_samples);
    }
    else if(header_.bits_per_sample == 32 && header_.audio_format == 3){//ieee float编码
        format = F32_LE;
        float* _samples = (float*)malloc(_samples_num * sizeof(float));
        fread(_samples, _byte_per_sample, _samples_num, Ifp);

        for(int i = 0; i < samples_.size(); ++i){
            samples_[i] = _samples[i * header_.num_channels];
        }
        free(_samples);
    }
    else{
        WAV_READER_ERROR("Unsupported audio format or bits per sample");
        return -3;
    }
    return _samples_num;
}

int Wave::ReadFromFile(std::string filename){
    FILE *Ifp;
    Ifp = fopen(filename.c_str(), "rb");

    if(Ifp == NULL){
        WAV_READER_ERROR("Can't open the file: %s \n", filename.c_str());
        return -1;
    }

    int ret = ReadWaveNormalization_(Ifp);
    fclose(Ifp);
    return ret;
}

int Wave::WritePCM(std::string filename){ //还原成16位
    FILE* pcm = fopen(filename.c_str(), "wb");
    if(pcm == NULL){
        WAV_READER_ERROR("Can't open the file: %s \n", filename.c_str());
        return -1;
    }

    uint32_t samples_bytes = samples_.size() * (header_.bits_per_sample / 8);
    char* write_pcm = new char[samples_bytes];
    uint32_t ret_get_samples = GetSamples(write_pcm);
    if(ret_get_samples != samples_bytes){
        WAV_READER_ERROR("To many or too few data, expected %d bytes, get %d bytes\n", samples_bytes, ret_get_samples);
        return -1;
    }

    int ret = fwrite(write_pcm, sizeof(char), samples_bytes, pcm);
    delete write_pcm;

    fclose(pcm);
    return ret;
}

int Wave::WriteWav(std::string filename){
    FILE* pcm = fopen(filename.c_str(), "wb");
    if(pcm == NULL){
        WAV_READER_ERROR("Can't open the file: %s \n", filename.c_str());
        return -1;
    }

    fwrite(&header_, 1, sizeof(WavHeader), pcm);

    uint32_t samples_bytes = samples_.size() * (header_.bits_per_sample / 8);
    char* write_pcm = new char[samples_bytes];
    uint32_t ret_get_samples = GetSamples(write_pcm);
    if(ret_get_samples != samples_bytes){
        WAV_READER_ERROR("To many or too few data, expected %d bytes, get %d bytes\n", samples_bytes, ret_get_samples);
        return -1;
    }

    int ret = fwrite(write_pcm, sizeof(char), samples_bytes, pcm);
    delete write_pcm;

    fclose(pcm);
    return ret;
}

} // namespace wav_reader