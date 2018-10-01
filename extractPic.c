#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "extractPic.h"

char *ExtractPic_getExtension(char *fileName)
{
	int len = strlen(fileName);
	int i = len;

	while (fileName[i]!='.' && i > 0) 
		i--;

	if (fileName[i] == '.') {
		return &fileName[i+1];
	} else {
		return &fileName[len];
	}
}

void ExtractPic_SaveJpeg(FILE *fp, int availframe)
{
	unsigned char *pPicData;
	int blockData;
	FILE *fp_pic = fopen("cover.jpg", "wb");

	while (availframe > 0) {
		blockData = (availframe > 1024) ? 1024 : availframe;
		pPicData = (unsigned char*)malloc(blockData);
		memset(pPicData, 0, blockData);
		fread(pPicData, blockData, 1, fp);
		fwrite(pPicData, blockData, 1, fp_pic);
		free(pPicData);
		availframe -= blockData;
	}

	fclose(fp_pic);
}

/*
 * @Brief:
 *   extracting picture from various music file
 * @param:
 *   char *filename: music file path
 * @return:
 *   0: fail
 *   1: success
 */
int main(int argc, char *argv[])
{
	FILE *fp = NULL;
	char *type;
	int pic_size = 0;

	char *filename = argv[1];

	if (!filename) {
		printf("[extract_pic] Error: no input file!\n");
		return 0;
	}

	if ((fp = fopen(filename, "rb")) == NULL) {
		printf("[extract_pic] Error: open \"%s\" fail!\n", filename);
		return 0;
	}

	type = ExtractPic_getExtension(filename);

	if (0 == strcasecmp(type, "mp3")) {
		printf("[extract_pic] type: mp3\n");
		pic_size = Mp3ToPic(fp);
	}
	else if (0 == strcasecmp(type, "aac")) {
		printf("[extract_pic] type: aac\n");
		pic_size = AacToPic(fp);
	}
	else if (0 == strcasecmp(type, "m4a")) {
		printf("[extract_pic] type: m4a\n");
		pic_size = M4aToPic(fp);
	}
	else if (0 == strcasecmp(type, "wma")) {
		printf("[extract_pic] type: wma\n");
		pic_size = WmaToPic(fp);
	}
	else if (0 == strcasecmp(type, "flac")) {
		printf("[extract_pic] type: flac\n");
		pic_size = FlacToPic(fp);
	}
	else {
		printf("[extract_pic] Error: unsupported file!\n");
		fclose(fp);
		return 0;
	}

	if (pic_size == 0) {
		printf("[extract_pic] Error: get pic fail!\n");
		fclose(fp);
		return 0;
	}

	printf("[extract_pic] pic size = %d\n", pic_size);
	// TODO: ExtractPic_compress

	if (pic_size > 1024*150) {
		printf("[extract_pic] Error: pic size(%dk) > 150k, skip saving pic!\n", pic_size/1024);
		fclose(fp);
		return 0;
	}

	ExtractPic_SaveJpeg(fp, pic_size);
	fclose(fp);

	return 1;
}
