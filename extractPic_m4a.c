/*
 * @Brief:
 *   This file is for extracting picture from m4a file.
 * @References:
 *   http://blog.csdn.net/yue_huang/article/details/72812109
 *   http://developer.apple.com/library/content/documentation/QuickTime/QTFF/QTFFChap2/qtff2.html
 *   http://developer.apple.com/library/content/documentation/QuickTime/QTFF/Metadata/Metadata.html
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "extractPic.h"

int _byte2int(unsigned char *input)
{
	int output;

	output = (input[0] << 24)
		+ (input[1] << 16)
		+ (input[2] << 8)
		+ input[3];

	return output;
}

int _getTagSize(FILE *fp, char *tag)
{
	unsigned char atom_size[4];
	unsigned char atom_type[4];

	fread(atom_size, 4, 1, fp);
	fread(atom_type, 4, 1, fp);

	if (strncmp(atom_type, tag, 4) != 0) {
		printf("[m4a_pic] Error: no %s tag!\n", tag);
		return 0;
	}

	return _byte2int(atom_size);
}

int M4aToPic_CheckFileType(FILE *fp)
{
	unsigned char atom_size[4];
	unsigned char atom_type[7];
	int ftyp_size;

	fread(atom_size, 4, 1, fp);
	fread(atom_type, 7, 1, fp);

	if (strncmp(atom_type, "ftypM4A", 7) != 0) {
		printf("[m4a_pic] Error: wrong file type!\n");
		return 0;
	}

	ftyp_size = _byte2int(atom_size);
	fseek(fp, ftyp_size-11, SEEK_CUR);

	return ftyp_size;
}

int getMoovSize(FILE *fp)
{
	return _getTagSize(fp, "moov");
}

void skipMvhd(FILE *fp)
{
	fseek(fp, _getTagSize(fp, "mvhd")-8, SEEK_CUR);
	//printf("cur pos [0x%X] after skip mvhd\n", ftell(fp));
}

void skipTrak(FILE *fp)
{
	fseek(fp, _getTagSize(fp, "trak")-8, SEEK_CUR);
	//printf("cur pos [0x%X] after skip trak\n", ftell(fp));
}

void skipUdtaMeta(FILE *fp)
{
	fseek(fp, 20, SEEK_CUR); // unused container data, shift directly
	//printf("cur pos [0x%X] after skip udta & meta\n", ftell(fp));
}

void skipHdlr(FILE *fp)
{
	fseek(fp, _getTagSize(fp, "hdlr")-8, SEEK_CUR);
	//printf("cur pos [0x%X] after skip hdlr\n", ftell(fp));
}

void skipIlst(FILE *fp)
{
	fseek(fp, 8, SEEK_CUR); // unused container data, shift directly
	//printf("cur pos [0x%X] after skip ilst\n", ftell(fp));
}

int M4aToPic_GetPicSize(FILE *fp, int len)
{
	unsigned char item_size[4];
	unsigned char item_type[4];

	while (1) {
		if (ftell(fp) >= len) {
			printf("[m4a_pic] Error: no image in file!\n");
			return 0;
		}
		fread(item_size, 4, 1, fp);
		fread(item_type, 4, 1, fp);
		//printf("cur pos [0x%X] in finding cover loop", ftell(fp));
		//getchar();
		if (strncmp(item_type, "covr", 4) == 0) {
			break;
		}
		fseek(fp, _byte2int(item_size)-8, SEEK_CUR);
	}

	fseek(fp, 16, SEEK_CUR);
	return (_byte2int(item_size)-24);
}

int M4aToPic(FILE *fp)
{
	int ftyp_size;
	int len; // as whole tag size, can be a breaking condition

	if ((ftyp_size = M4aToPic_CheckFileType(fp)) == 0)
		return 0;

	len = getMoovSize(fp) + ftyp_size;
	skipMvhd(fp);
	skipTrak(fp);
	skipUdtaMeta(fp);
	skipHdlr(fp);
	skipIlst(fp);

	return M4aToPic_GetPicSize(fp, len);
}

// appendix: illustration of M4A structure
// ┌────────┐
// │  ftyp  │
// ├────────┤
// │  moov  │
// │┌──────┐│
// ││ mvhd ││
// │└──────┘│
// │┌──────┐│
// ││ trak ││
// ││      ││
// ││      ││
// ││      ││
// │└──────┘│
// │┌──────┐│
// ││ udta ││
// ││ meta ││
// ││┌────┐││
// │││hdlr│││
// ││└────┘││
// ││┌────┐││
// │││ilst│││
// ││└────┘││
// │└──────┘│
// └────────┘