/*
 * @Brief:
 *   This file is for extracting picture from aac file.
 * @References:
 *   http://wiki.hydrogenaud.io/index.php?title=APEv2_specification
 *   http://magiclen.org/mp3-tag/
 * @Todo:
 *   Improve brute-force attack part.
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "extractPic.h"

// APEv2 header/footer tag (32bytes)
typedef struct {
	unsigned char preamble[8];
	unsigned char version[4];
	unsigned char tag_size[4];
	unsigned char tag_count[4];
	unsigned char flags[4];
	long reserved;
} APEv2Header;

// (part of) item tag // discard uncertain size
typedef struct {
	unsigned char lenth[4];
	unsigned char flags[4];
	//char *key;
	//const char key_terminator = 0x00;
	//char *value;
} APEv3Item;

unsigned char AacToPic_GetHeaderInfo(FILE *fp, int *size, int *count)
{
	APEv2Header header;

	fseek(fp, -32, SEEK_END);
	memset(&header, 0, 32);
	fread(&header, 32, 1, fp);

	if (strncmp(header.preamble, "APETAGEX", 8) != 0) {
		printf("[aac_pic] Error: no APETAGEX tag!\n");
		return 0;
	}

	// version 1: 1000, version 2: 2000
	// 0x07D0 = 2000
	if (header.version[0] != 0xD0 || header.version[1] != 0x07) {
		printf("[aac_pic] Error: not version 2!\n");
		return 0;
	}

	*size = (header.tag_size[3] << 24)
		+ (header.tag_size[2] << 16)
		+ (header.tag_size[1] << 8)
		+ header.tag_size[0];

	*count = (header.tag_count[3] << 24)
		+ (header.tag_count[2] << 16)
		+ (header.tag_count[1] << 8)
		+ header.tag_count[0];

	return 1;
}

int AacToPic_GetPicSize(FILE *fp, int cnt)
{
	APEv3Item item;
	int item_len;
	unsigned char tag_key[4];
	unsigned char cur_byte = 0xff;

	while (1) {
		// 0. item count as break condition
		if (cnt == 0) {
			printf("[aac_pic] Error: no pic item!\n");
			return 0;
		}
		cnt--;
		
		// 1. get item lenth
		memset(&item, 0, 8);
		fread(&item, 8, 1, fp); // read 8 but ignore 5~8 item.flags
		item_len = (item.lenth[3] << 24)
			+ (item.lenth[2] << 16)
			+ (item.lenth[1] << 8)
			+ item.lenth[0];

		// 2. get item key, tag_key must < 5, since item "year" is only 4 bytes
		fread(tag_key, 4, 1, fp);
		if (strncmp(tag_key, "Cove", 4) == 0) // Cover
			break;

		// 3. shift 1, until find key terminator
		do {
			fread(&cur_byte, 1, 1, fp);
		} while (cur_byte != 0x00);

		// 4. shift item lenth
		fseek(fp, item_len, SEEK_CUR);
	}

	// brute-force attack to skip unnecessary data
	fseek(fp, 36, SEEK_CUR);
	item_len -= strlen("Cover Art (Front).jpg."); // or "Cover Art (Front).png."

	return item_len;
}

int AacToPic(FILE *fp)
{
	int size = 0;
	int count = 0;

	if (AacToPic_GetHeaderInfo(fp, &size, &count) == 0)
		return 0;

	// fp will be eof after getting header info
	// ┌────────┬───────────────────┬────────┐
	// │ header │       items       │ footer │
	// └────────┴───────────────────┴────────┘
	//          │←         TAG size         →│
	fseek(fp, size*(-1), SEEK_END);

	return AacToPic_GetPicSize(fp, count);
}