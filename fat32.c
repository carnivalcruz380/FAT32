#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ATTR_READ_ONLY 0x01;	// file cannot be modified 
#define ATTR_HIDDEN 0x02;		// file/sub dir must not be listed 
#define ATTR_SYSTEM 0x04;		// file is tagged as a componenet of the OS
#define ATTR_VOLUME_ID 0x08;	// entry containe the volume label (DIR_FatClusHI and DIR_FatClusLO must be 0)
#define ATTR_DIRECTORY 0x10;	// database entry
#define ATTR_ARCHIVE 0x20;		//indicates that properties of the associated file have been modified 

// Boot sector and BPB(BIOS parameter block) structure
struct BS_BPB{
	unsigned char BS_jmpBoot[3];	// jump instructions to boot code: bytes 0-3, length 3
	unsigned char BS_OEMName[8];	// OEM name identifier: bytes 4-11, length 8
	unsigned short BPB_BytsPerSec;	// bytes per sector: bytes 12-13, length 2
	unsigned char BPB_SecPerClus;	// sectors per allocated unit: byte 14
	unsigned short BPB_RsvdSecCnt;	// # of reserved sectors in the reseved region: bytes 15-16, length 2
	unsigned char BPB_NumFATs;		// # of FATs: byte 17
	unsigned short BPB_RootEntCnt;	// # of dir entries(0 for FAT32): bytes 18-19, length 2
	unsigned short BPB_TotSec16;	// 16 bit total # of sectors: bytes 20-21, length 2
	unsigned char BPB_Media;		// byte 22
	unsigned short BPB_FATSz16;		// FAT size count(0 for FAT32): bytes 23-24, length 2
	unsigned short BPB_SecPerTrk;	// sector per track: bytes 25-26, length 2
	unsigned short BPB_NumHeads;	// number of heads: bytes 27-28, length 2
	unsigned int BPB_HiddSec;		// # of hidden sectors preceding the partition: bytes 29-32, length 4
	unsigned int BPB_TotSec32;		// 32 bit sector count: bytes 33-36, length 4
	unsigned int BPB_FATSz32;		// 32 bit count of sectors occupied by 1 fat: bytes 37-40, length 4
	unsigned short BPB_ExtFlags;	// bytes 41-42, length 2
	unsigned short BPB_FSVer;		// version # of volume: bytes 43-44, length 2
	unsigned int BPB_RootClus;		// cluster # of the first cluster of the root dir: bytes 45-48, length 4
	unsigned short BPB_FSInfo;		// sector # of FSInfo struct: bytes 49-50, length 2
	unsigned short BPB_BkBootSec;	// (set to 0 or 6): bytes 51-52, length 2
	unsigned char BPB_Reserved[12];	// (set to 0x0): bytes 53-64, length 12
	unsigned char BS_DrvNum;		// drive #: byte 65
	unsigned char BS_Reserved1;		// byte 66
	unsigned char BS_BootSig;		// extended boot sig: byte 67
	unsigned int BS_VolID;			// volume serial #: bytes 68-71, length 4
	unsigned char BS_VolLab[11];	// volume label: bytes 72-82, length 11
	unsigned char BS_FilSysType[8];	// (set to string "FAT32"): bytes 83-90, length 8
}__attribute__((packed));

// file system information structure
struct FSInfo{
	unsigned int FSI_LeadSig;		// lead signature(0x41615252): bytes 0-3, length 4
	unsigned char FSI_Reserved1[480];// (must be 0) bytes 4-483, length 480
	unsigned int FSI_StrucSig;		// additional signature(0x61417272): bytes 484-487, length 4
	unsigned int FSI_Free_Count;	// last known free cluster count on the volume: bytes 488-491, length 4
	unsigned int FSI_Nxt_Free;		// cluster # of first free cluster on the volume: bytes 492-495, length 4
	unsigned char FSI_Reserved2[12];// bytes 496-507, length 12
	unsigned int FSI_TrailSig;		// trail signature(0xAA550000): bytes 508-511, length 4
}__attribute__((packed));

// directory entry structure
struct DIRENTRY{
	unsigned char DIR_Name[11];		// file name: bytes 0-10, length 11
	unsigned char DIR_Attr;			// file attributes: byte 11, inititated with defined attributes above
	unsigned char DIR_NTRes;		// reserved(must be 0): byte 12
	unsigned char DIR_CrtTimeTenth;	// componenet of file creation time(0 <= CrtTimeTenth <= 199): byte 13
	unsigned short DIR_CrtTime;		// creation time: bytes 14-15, length 2
	unsigned short DIR_CrtDate;		// creation date: bytes 16-17, length 2
	unsigned short DIR_LstAccDate;	// last access date(must be equal to DIR_WrtDate): bytes 18-19, length 2
	unsigned short FstSlusHI;		// high word of first data clustrer #: bytes 20-21, length 2
	unsigned short DIR_WrtTime;		// last modified(write) time: bytes 22-23, length 2
	unsigned short DIR_WrtDate;		// last modified(write) date: bytes 24-25, length 2
	unsigned short DIR_FstClusLO;	// low word of first data cluster #: bytes 26-27, length 2
	unsigned int DIR_FileSize;		// file/dir size: bytes 28-31, length 4
}__attribute__((packed));

typedef struct {
	int size;
	char **items;
} tokenlist;

char *get_input(void);
tokenlist *get_tokens(char *input);

tokenlist *new_tokenlist(void);
void add_token(tokenlist *tokens, char *item);
void free_tokens(tokenlist *tokens);

int main()
{
	while (1) {
		printf("> ");

		/* input contains the whole command
		 * tokens contains substrings from input split by spaces
		 */

		char *input = get_input();
		printf("whole input: %s\n", input);

		tokenlist *tokens = get_tokens(input);
		for (int i = 0; i < tokens->size; i++) {
			printf("token %d: (%s)\n", i, tokens->items[i]);
		}

		free(input);
		free_tokens(tokens);
	}

	return 0;
}

tokenlist *new_tokenlist(void)
{
	tokenlist *tokens = (tokenlist *) malloc(sizeof(tokenlist));
	tokens->size = 0;
	tokens->items = (char **) malloc(sizeof(char *));
	tokens->items[0] = NULL; /* make NULL terminated */
	return tokens;
}

void add_token(tokenlist *tokens, char *item)
{
	int i = tokens->size;

	tokens->items = (char **) realloc(tokens->items, (i + 2) * sizeof(char *));
	tokens->items[i] = (char *) malloc(strlen(item) + 1);
	tokens->items[i + 1] = NULL;
	strcpy(tokens->items[i], item);

	tokens->size += 1;
}

char *get_input(void)
{
	char *buffer = NULL;
	int bufsize = 0;

	char line[5];
	while (fgets(line, 5, stdin) != NULL) {
		int addby = 0;
		char *newln = strchr(line, '\n');
		if (newln != NULL)
			addby = newln - line;
		else
			addby = 5 - 1;

		buffer = (char *) realloc(buffer, bufsize + addby);
		memcpy(&buffer[bufsize], line, addby);
		bufsize += addby;

		if (newln != NULL)
			break;
	}

	buffer = (char *) realloc(buffer, bufsize + 1);
	buffer[bufsize] = 0;

	return buffer;
}

tokenlist *get_tokens(char *input)
{
	char *buf = (char *) malloc(strlen(input) + 1);
	strcpy(buf, input);

	tokenlist *tokens = new_tokenlist();

	char *tok = strtok(buf, " ");
	while (tok != NULL) {
		add_token(tokens, tok);
		tok = strtok(NULL, " ");
	}

	free(buf);
	return tokens;
}

void free_tokens(tokenlist *tokens)
{
	for (int i = 0; i < tokens->size; i++)
		free(tokens->items[i]);
	free(tokens->items);
	free(tokens);
}


