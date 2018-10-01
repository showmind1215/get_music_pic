/*
 * @Brief:
 *   This file is for extracting picture from mp3 file.
 * @References:
 *   http://blog.csdn.net/ydtbk/article/details/9258651
 *   http://magiclen.org/mp3-tag/
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "extractPic.h"

// id3v2 header tag structure
typedef struct {
	char identi[3];
	char version;
	char revision;
	char flags;
	unsigned char size[4];
} ID3V2Header;

// frame header structure
typedef struct {
	char frameId[4];
	unsigned char size[4];
	char flags[2];
} ID3V2Frameheader;

unsigned char Mp3ToPic_GetHeaderInfo(FILE *fp, int *len)
{
	ID3V2Header mp3ID3V2;

	// pointer to the mp3 at the beginning
	fseek(fp, 0, SEEK_SET);

	// load header tag
	memset(&mp3ID3V2, 0, 10);
	fread(&mp3ID3V2, 10, 1, fp);

	if (strncmp(mp3ID3V2.identi, "ID3", 3) != 0) {
		printf("[mp3_pic] Error: no ID3V2 tag!\n");
		return 0;
	}

	// calculate header tag size
	*len= ((mp3ID3V2.size[0]&0x7f) << 21)
		+ ((mp3ID3V2.size[1]&0x7f) << 14)
		+ ((mp3ID3V2.size[2]&0x7f) << 7)
		+ (mp3ID3V2.size[3]&0x7f);

	return 1;
}

int Mp3ToPic_GetPicSize(FILE *fp, int len)
{
	ID3V2Frameheader pFrameBuf;
	char image_tag[7] = {"0"};
	char pic_type[5] = {"0"};
	int frame_size = 0;
	int skip_data = 0;
	int i = 0;

	memset(&pFrameBuf, 0, 10);
	fread(&pFrameBuf, 10, 1, fp);

	// find pic tag location
    while (strncmp(pFrameBuf.frameId, "APIC", 4) != 0) {
		if (ftell(fp) > len) {
			printf("[mp3_pic] Error: no APIC tag!\n\n");
			return 0;
		}
		frame_size = (pFrameBuf.size[0] << 24)
			+ (pFrameBuf.size[1] << 16)
			+ (pFrameBuf.size[2] << 8)
			+ pFrameBuf.size[3];
		fseek(fp, frame_size, SEEK_CUR);
		memset(&pFrameBuf, 0, 10);
		fread(&pFrameBuf, 10, 1, fp);
	}

	// calculate pic tag size
	frame_size = (pFrameBuf.size[0] << 24)
		+ (pFrameBuf.size[1] << 16)
		+ (pFrameBuf.size[2] << 8)
		+ pFrameBuf.size[3];

	// check pic format
	fread(image_tag, 6, 1, fp);
	while (1) {
		if (i > frame_size) {
			printf("[mp3_pic] Error: cannot find image tag!\n");
			fclose(fp);
			return 0;
		}
		if (strcmp(image_tag, "image/") == 0) {
			skip_data += 6;
			fread(pic_type, 4, 1, fp);
			if (strncmp(pic_type, "jpeg", 4) == 0) {
				skip_data += 4;
				break;
			} else if (strncmp(pic_type, "png", 3) == 0) {
				skip_data += 3;
				fseek(fp, -1, SEEK_CUR);
				break;
			} else {
				printf("[mp3_pic] Error: unsupported image format!\n");
				return 0;
			}
		} else {
			i++;
			fseek(fp, -5, SEEK_CUR);
			fread(image_tag, 6, 1, fp);
			skip_data += 1;
			continue;
		}
	}

	// need to shift 3 bytes but don't know why
	fseek(fp, 3, SEEK_CUR);
	skip_data += 3;

	return (frame_size - skip_data);
}

int Mp3ToPic(FILE *fp)
{
	int tag_size = 0;

	if (Mp3ToPic_GetHeaderInfo(fp, &tag_size) == 0)
		return 0;

	return Mp3ToPic_GetPicSize(fp, tag_size);
}
