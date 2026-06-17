

#include <string>
#include "DTLN_AEC.h"
#include "wav_reader.h"
#include <cstring>

int main(int argc, char *argv[]){
    //lpszInputRefWave is a reference data(far end)
    //lpszInputRecWave is a recording data(near end recording)

    std::string lpszInputRefWave = std::string(argv[1]);
    std::string lpszInputRecWave = std::string(argv[2]);
    std::string lpszOutputWave = std::string(argv[3]);
    std::string model1 = std::string(argv[4]);
    std::string model2 = std::string(argv[5]);

    FILE *lpoInputRefFile = NULL;
    FILE *lpoInputRecFile = NULL;
    FILE *lpoOutputFile = NULL;

    short *lpsInputRefSample = NULL;
    short *lpsInputRecSample = NULL;
    short *lpsOutputSample = NULL;

    int nReadSize, nFrameSize;

    lpoInputRefFile = fopen(lpszInputRefWave.c_str(), "rb");
    lpoInputRecFile = fopen(lpszInputRecWave.c_str(), "rb");
    lpoOutputFile = fopen(lpszOutputWave.c_str(), "wb+");

    DTLN_AEC oDtlnAec;

    nFrameSize = oDtlnAec.Init(model1.c_str(), model2.c_str());

    lpsInputRefSample = new short[nFrameSize];
    lpsInputRecSample = new short[nFrameSize];
    lpsOutputSample = new short[nFrameSize];

    //read wav header
    wav_reader::WavHeader header_ref;
    wav_reader::WavHeader header_rec;
    fread(&header_ref, 1, 44, lpoInputRefFile);
    fread(&header_rec, 1, 44, lpoInputRecFile);

    uint8_t* output_buffer = (uint8_t*)malloc(header_rec.data_chunk_size);
    int tot_frames = 0;
    while (true){
        nReadSize = fread(lpsInputRefSample, 1, nFrameSize * sizeof(short), lpoInputRefFile);
        if (nReadSize <= 0)
            break;

        nReadSize = fread(lpsInputRecSample, 1, nFrameSize * sizeof(short), lpoInputRecFile);
        if (nReadSize <= 0)
            break;

        oDtlnAec.Process(lpsInputRefSample, lpsInputRecSample, lpsOutputSample);

        //write PCM
        //fwrite(lpsOutputSample, 1, nFrameSize * sizeof(short), lpoOutputFile);
        memcpy(output_buffer + tot_frames * sizeof(short), lpsOutputSample, nFrameSize * sizeof(short));
        tot_frames += nFrameSize;
    }

    wav_reader::WritePCM2WavFile(lpoOutputFile, 
                        output_buffer, 
                        tot_frames, 
                        header_rec.sample_rate,
                        header_rec.bits_per_sample,
                        header_rec.num_channels
                    );
    
    free(output_buffer);
    fclose(lpoInputRefFile);
    fclose(lpoInputRecFile);
    fclose(lpoOutputFile);

    if (lpsInputRefSample != NULL)
        delete[] lpsInputRefSample;

    if (lpsInputRecSample != NULL)
        delete[] lpsInputRecSample;

    if (lpsOutputSample != NULL)
        delete[] lpsOutputSample;

    return 0;

}