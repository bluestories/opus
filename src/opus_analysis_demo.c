/*
   add by xavier @ 2018-10-26
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "opus.h"
#include "debug.h"
#include "opus_types.h"
#include "opus_private.h"
#include "opus_multistream.h"

#define MAX_PACKET 1500

void print_usage( char* argv[] )
{
    fprintf(stderr, "Usage: %s <sampling rate (Hz)> <channels (1/2)> <input>\n", argv[0]);
}

int main(int argc, char *argv[])
{
    if (argc < 4 )
    {
       print_usage( argv );
	   
	   return 0;
    }

    fprintf(stderr, "%s\n", opus_get_version_string());

    int args = 1;
    
    int sampling_rate = (opus_int32)atol(argv[args]);
    if (sampling_rate != 8000 && sampling_rate != 12000
     && sampling_rate != 16000 && sampling_rate != 24000
     && sampling_rate != 48000)
    {
        fprintf(stderr, "Supported sampling rates are 8000, 12000, "
                "16000, 24000 and 48000.\n");
        
		return 0;
    }
    int frame_size = sampling_rate/50;
	args++;

    int channels = atoi(argv[args]);
    args++;

    if (channels < 1 || channels > 2)
    {
        fprintf(stderr, "Opus_demo supports only 1 or 2 channels.\n");
        
		return 0;
    }

    char* inFile = argv[argc-1];
    FILE* fin = fopen(inFile, "rb");
    if (!fin)
    {
        fprintf (stderr, "Could not open input file %s\n", argv[argc-2]);
		
		return 0;
    }

	int err = 0;
	OpusEncoder* enc = opus_encoder_create(sampling_rate, channels, OPUS_APPLICATION_AUDIO, &err);
    if (err != OPUS_OK)
    {
        fprintf(stderr, "Cannot create encoder: %s\n", opus_strerror(err));
        return 0;
    }

    opus_encoder_ctl(enc, OPUS_SET_COMPLEXITY(10));
    opus_encoder_ctl(enc, OPUS_SET_LSB_DEPTH(16));

	unsigned char* fbytes = (unsigned char*)malloc(48000*2*channels*sizeof(short));
	short* in = (short*)malloc(48000 * 2 * channels * sizeof(short));

	int count = 0;
	while (1)
	{
		int curr_read = fread(fbytes, sizeof(short)*channels, frame_size, fin);
		if (curr_read == frame_size)
		{
			for (int i = 0; i < curr_read * channels; i++)
			{
				opus_int32 s;
				s = fbytes[2 * i + 1] << 8 | fbytes[2 * i];
				s = ((s & 0xFFFF) ^ 0x8000) - 0x8000;
				in[i] = s;
			}
		}
		else
		{
			fseek(fin, 0, SEEK_SET);
			//break;
		}

		float activity = 0.0;
		float prob = 0.0;
		float fft[960] = { 0 };
		int anaret = opus_encode_analysis(enc, in, frame_size, &activity, &prob,NULL);
  		printf("[%d] anaret:%d - active = %0.2f, prob = %0.2f\n", count++, anaret, activity, prob);
	}

	free(in);
	free(fbytes);
	fclose(fin);
    opus_encoder_destroy(enc);

	return 0;
}
