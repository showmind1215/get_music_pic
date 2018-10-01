/*
 * @Brief:
 *   This file is for extracting picture from flac file.
 * @References:
 *   http://xiph.org/flac/format.html
 *   http://blog.csdn.net/ffgamelife/article/details/7893747
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "extractPic.h"

int FlacToPic(FILE *fp)
{
	unsigned char marker[4];
	unsigned char mbh[4]; // metadata block header. 0:type 1~3:size
	unsigned char img_size[4];
	char img_type[5];
	int len;

	fread(marker, 4, 1, fp);

	if (strncmp(marker, "fLaC", 4) != 0) {
		printf("[flac_pic] Error: no flac tag!\n");
		return 0;
	}

	fseek(fp, 38, SEEK_CUR); // skip stream info
	fread(mbh, 4, 1, fp);

	while (1) {
		if ((mbh[0] >> 7) == 1) { // last metadata block
			printf("[flac_pic] Error: no image in file!\n");
			return 0;
		}
		if (mbh[0] == 6) { // type 6: PICTURE
			break;
		}
		len = (mbh[1] << 16) + (mbh[2] << 8) + mbh[3];
		fseek(fp, len, SEEK_CUR);
		fread(mbh, 4, 1, fp);
	}

	fseek(fp, 14, SEEK_CUR);
	fread(img_type, 4, 1, fp);
	if (strncmp(img_type, "jpeg", 4) == 0) {
		fseek(fp, 20, SEEK_CUR);
	} else if (strncmp(img_type, "png", 3) == 0) {
		fseek(fp, 19, SEEK_CUR);
	} else {
		printf("[flac_pic] Error: unsupported image format!\n");
		return 0;
	}

	fread(img_size, 4, 1, fp);
	len = (img_size[1] << 16) + (img_size[2] << 8) + img_size[3];

	return len;
}