#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "structures.h"

typedef struct {
	int size;
	char **items;
} tokenlist;

char *get_input(void);
tokenlist *get_tokens(char *input);

tokenlist *new_tokenlist(void);
void add_token(tokenlist *tokens, char *item);
void free_tokens(tokenlist *tokens);

void print_info(BS_BPB * bpb);	//print function for the info command 
BS_BPB* get_BPB();	// reads and parses BPB object from img file
unsigned int get_clusterOffset(BS_BPB* bpb, unsigned int clusNum);
DIRENTRY* get_Entry(unsigned int clusNum, BS_BPB* bpb);

int main(void){
	BS_BPB* bs_bpb = get_BPB();
	//unsigned int offset = 
	DIRENTRY* entry = get_Entry(2, bs_bpb);
	while (1) {
		printf("> ");

		/* input contains the whole command
		 * tokens contains substrings from input split by spaces
		 */

		char *input = get_input();

		tokenlist *tokens = get_tokens(input);

		if (tokens->items[0] == NULL){
			continue;
		}

		// exit command
		if (strcmp(tokens->items[0], "exit") == 0){
			free(input);
			free_tokens(tokens);
			free(bs_bpb);
			free(entry);
			return 0;
		}

		// info command 
		if (strcmp(tokens->items[0], "info") == 0){
			print_info(bs_bpb);
		}
		
	}

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

// info command function 
void print_info(BS_BPB * bpb){
	printf("Bytes per sector: %d\n", bpb->BPB_BytsPerSec);
	printf("Sector per cluster: %d\n", bpb->BPB_SecPerClus);
	printf("Reserved sector count: %d\n", bpb->BPB_RsvdSecCnt);
	printf("Number of fats: %d\n", bpb->BPB_NumFATs);
	printf("Total sectors: %d\n", bpb->BPB_TotSec32);
	printf("Root cluster: %d\n", bpb->BPB_RootClus);
}

// BSB object parser
BS_BPB* get_BPB(void){
	int fd = open("fat32.img", O_RDONLY);
	BS_BPB* bpb = (BS_BPB *) malloc(sizeof(BS_BPB));
	unsigned char buffer[512];
	if (fd){
		read(fd, bpb, 90);
		close(fd);
	}

	return bpb;
}

unsigned int get_clusterOffset(BS_BPB* bpb, unsigned int clusNum){
		unsigned int FAT_offset = bpb->BPB_RsvdSecCnt;
		unsigned int clus_offset = FAT_offset * bpb->BPB_BytsPerSec + clusNum * 4;
		return clus_offset;
}

DIRENTRY* get_Entry(unsigned int clusNum, BS_BPB* bpb){
		unsigned int firstDataSector = bpb->BPB_RsvdSecCnt + (bpb->BPB_NumFATs * bpb->BPB_FATSz32);
		unsigned int clusOffset = firstDataSector + (clusNum - 2) * bpb->BPB_SecPerClus;
		int fd = open("fat32.img",O_RDONLY);
		DIRENTRY* entry = (DIRENTRY*) malloc(sizeof(DIRENTRY));
		unsigned char buffer[32];
		if (fd){
		lseek(fd, clusOffset, SEEK_SET);
		read(fd, entry, 32);

		for (int i = 0; i < 11; i++){
			char c = entry->DIR_Name[i];
			printf("%c", c);
		}
		printf("\n");
	
		close(fd);
	}

	return entry;
	
}
