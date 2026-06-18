#!/bin/bash

./sample_test \
    -r wav/test2_ref.wav \
    -i wav/test2_rec.wav \
    -o test2_out_512.wav \
    -m ../models/dtln_aec_512_1.tflite \
    -n ../models/dtln_aec_512_2.tflite