/*
 * @Brief:
 *   This file is for extracting picture from wma file.
 * @References:
 *   http://uguisu.skr.jp/Windows/format_asf.html
 * @Todo:
 *   Try to improve brute-force attack part.
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "extractPic.h"

#define GUID_ASF_Header_Object		"3026B2758E66CF11A6D900AA0062CE6C"

unsigned char WmaToPic_GetHeaderInfo(FILE *fp, int *len)
{
	unsigned char head[16];
	unsigned char size[4];
	char guid_head_text[33];

	fseek(fp, 0, SEEK_SET);
	fread(head, 16, 1, fp);

	snprintf(guid_head_text, 33, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\0",
		head[0], head[1], head[2], head[3], head[4], head[5], head[6], head[7],
		head[8], head[9], head[10], head[11], head[12], head[13], head[14], head[15]);

	if (strncasecmp(guid_head_text, GUID_ASF_Header_Object, 32) != 0) {
		printf("[wma_pic] Error: wrong ASF Header!\n");
		return 0;
	}

	fread(size, 4, 1, fp);
	fseek(fp, 4, SEEK_CUR); // 8 bytes for size is too much, so just get 4 bytes and shift another 4

	*len = (size[3] << 24)
		+ (size[2] << 16)
		+ (size[1] << 8)
		+ (size[0]);

	return 1;
}

// use memcmp instead of strncmp since strncmp will return when comparing "\0" (0x00)
int WmaToPic_GetPicSize(FILE *fp, int header_len)
{
	unsigned char img_tag[12];
	unsigned char img_size[4];
	unsigned char img_type[8];
	int size;

	// brute-force attack
	while (1) {
		if (ftell(fp) > header_len) {
			printf("[wma_pic] Error: no image in file!\n");
			return 0;
		}
		fread(img_tag, 12, 1, fp);
		if (memcmp(img_tag, "i\0m\0a\0g\0e\0/\0", 12) == 0) // i.m.a.g.e./.
			break;
		fseek(fp, -11, SEEK_CUR);
	}

	// 4 bytes before "i.m.a.g.e./" is pic size
	fseek(fp, -16, SEEK_CUR);
	fread(img_size, 4, 1, fp);
	size = (img_size[3] << 24)
		+ (img_size[2] << 16)
		+ (img_size[1] << 8)
		+ img_size[0];

	// skip "i.m.a.g.e./."
	fseek(fp, 12, SEEK_CUR);

	// different offset between jpeg & png
	fread(img_type, 8, 1, fp);
	if (memcmp(img_type, "j\0p\0e\0g\0", 8) == 0) { // j.p.e.g.
		fseek(fp, 4, SEEK_CUR);
	} else if (memcmp(img_type, "p\0n\0g\0", 6) == 0) { // p.n.g.
		fseek(fp, 2, SEEK_CUR);
	}

	return size;
}

int WmaToPic(FILE *fp)
{
	int len;

	if (WmaToPic_GetHeaderInfo(fp, &len) == 0)
		return 0;

	return WmaToPic_GetPicSize(fp, len);
}