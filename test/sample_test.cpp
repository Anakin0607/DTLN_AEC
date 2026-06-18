

#include <string>
#include <cstring>
#include <sys/time.h>
#include <bits/getopt_core.h>

#include "DTLN_AEC.h"
#include "wav_reader.h"

int main(int argc, char *argv[]) {
    //RefWave is a reference data(far end)
    //RecWave is a recording data(near end recording)

    const char *usage = "Usage: %s -r <ref.wav> -i <rec.wav> -o <out.wav> -m <model1> -n <model2>\n";

    std::string ref_wave;
    std::string rec_wave;
    std::string out_wave;
    std::string model1;
    std::string model2;

    int opt;
    opterr = 0;
    while ((opt = getopt(argc, argv, "r:i:o:m:n:")) != -1) {
        switch (opt) {
            case 'r':
                ref_wave = optarg;
                break;
            case 'i':
                rec_wave = optarg;
                break;
            case 'o':
                out_wave = optarg;
                break;
            case 'm':
                model1 = optarg;
                break;
            case 'n':
                model2 = optarg;
                break;
            default:
                fprintf(stderr, "Unknown option or missing argument: -%c\n", (char)opt);
                fprintf(stderr, usage, argv[0]);
                return 1;
        }
    }

    // check params
    if (ref_wave.empty() || rec_wave.empty() || out_wave.empty() || model1.empty() || model2.empty()) {
        fprintf(stderr, "Error: all five arguments are required.\n");
        fprintf(stderr, usage, argv[0]);
        return 1;
    }

    FILE *input_ref_file = NULL;
    FILE *input_rec_file = NULL;
    FILE *output_file = NULL;

    short *input_ref_sample = NULL;
    short *input_rec_sample = NULL;
    short *output_sample = NULL;

    int read_size, frame_size;

    input_ref_file = fopen(ref_wave.c_str(), "rb");
    input_rec_file = fopen(rec_wave.c_str(), "rb");
    output_file = fopen(out_wave.c_str(), "wb+");

    if (!input_ref_file || !input_rec_file || !output_file) {
        fprintf(stderr, "Error opening files.\n");
        if (input_ref_file) fclose(input_ref_file);
        if (input_rec_file) fclose(input_rec_file);
        if (output_file) fclose(output_file);
        return 1;
    }

    DTLN_AEC dtln_aec;
    frame_size = dtln_aec.Init(model1.c_str(), model2.c_str());

    input_ref_sample = new short[frame_size];
    input_rec_sample = new short[frame_size];
    output_sample = new short[frame_size];

    // Read wav header
    wav_reader::WavHeader header_ref;
    wav_reader::WavHeader header_rec;
    fread(&header_ref, 1, 44, input_ref_file);
    fread(&header_rec, 1, 44, input_rec_file);

    uint8_t *output_buffer = (uint8_t *)malloc(header_rec.data_chunk_size);
    int tot_frames = 0;

    double total_inference_time_s = 0.0;
    struct timeval start_time, end_time;

    while (true) {
        read_size = fread(input_ref_sample, 1, frame_size * sizeof(short), input_ref_file);
        if (read_size <= 0)
            break;

        read_size = fread(input_rec_sample, 1, frame_size * sizeof(short), input_rec_file);
        if (read_size <= 0)
            break;

        gettimeofday(&start_time, NULL);
        dtln_aec.Process(input_ref_sample, input_rec_sample, output_sample);
        gettimeofday(&end_time, NULL);

        // Accumulate elapsed time in seconds
        total_inference_time_s += (end_time.tv_sec - start_time.tv_sec) + 
                                  (end_time.tv_usec - start_time.tv_usec) / 1000000.0;

        // cache pcm to output
        memcpy(output_buffer + tot_frames * sizeof(short), output_sample, frame_size * sizeof(short));
        tot_frames += frame_size;
    }

    // Calculate RTF (Real-Time Factor)
    double total_audio_duration_s = (double)tot_frames / header_rec.sample_rate;
    double rtf = total_audio_duration_s > 0.0 ? (total_inference_time_s / total_audio_duration_s) : 0.0;

    fprintf(stdout, "\n========== Performance Metrics ==========\n");
    fprintf(stdout, "Total Audio Duration : %.3f s\n", total_audio_duration_s);
    fprintf(stdout, "Total Inference Time : %.3f s\n", total_inference_time_s);
    fprintf(stdout, "Real-Time Factor(RTF): %.4f\n", rtf);
    fprintf(stdout, "=========================================\n\n");
    
    wav_reader::WritePCM2WavFile(output_file,
                                output_buffer,
                                tot_frames,
                                header_rec.sample_rate,
                                header_rec.bits_per_sample,
                                header_rec.num_channels);

    free(output_buffer);
    fclose(input_ref_file);
    fclose(input_rec_file);
    fclose(output_file);

    if (input_ref_sample != nullptr)
        delete[] input_ref_sample;
    if (input_rec_sample != nullptr)
        delete[] input_rec_sample;
    if (output_sample != nullptr)
        delete[] output_sample;

    return 0;
}