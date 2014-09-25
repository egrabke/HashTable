#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "HashTable.h"

#define GET_COLLIDE 1
#define GET_EXISTS 2
#define GET_NONE 0

#define NOT_A_HASH_TABLE -1
#define ENTRY_NOT_FOUND -2
#define OUT_OF_RAM -2

// Created by Zhuowei

static int mycompare(const void* a, const void* b) {
	int retval = strcmp(*((const char**) a), *((const char**) b));
	return retval;
}

void printHashTable(HashTablePTR table) {
	if (table == NULL) return;
	char** keys;
	unsigned int keyCount;
	int status = GetKeys(table, &keys, &keyCount);
	if (status == NOT_A_HASH_TABLE || status == OUT_OF_RAM) return;
	/* sort the keys */
	qsort(keys, keyCount, sizeof(char*), mycompare);
	for (unsigned int i = 0; i < keyCount; i++) {
		char* dat;
		char* key = keys[i];
		FindEntry(table, key, (void**) &dat);
		printf("%s:%s\n", key, dat);
		free(key);
	}
	free(keys);
}

void freeHashTableContents(HashTablePTR table) {
	char** keys;
	unsigned int keyCount;
	GetKeys(table, &keys, &keyCount);
	for (unsigned int i = 0; i < keyCount; i++) {
		char* dat;
		char* key = keys[i];
		FindEntry(table, key, (void**) &dat);
		free(dat);
		free(key);
	}
	free(keys);
}

int main() {
	HashTablePTR table = NULL;
	for(;;) {
		char cmd[80];
		printf("Command: ");
		int retval = scanf("%79s", cmd);
		if (retval <= 0) break;
		if (strcmp(cmd, "create") == 0) {
			printf("Size: ");
			int size;
			scanf("%d", &size);
			
			if (size < 0) {
				printf("Size cannot be negative, clamping to 0.\n");
				size = 0;
			}
			if (table != NULL) DestroyHashTable(&table);
			CreateHashTable(&table, (unsigned int) size);
			
			if (table == NULL) {
				printf("create returned NULL\n");
			} else {
				printf("create returned non-NULL\n");
			}
		} else if (strcmp(cmd, "destroy") == 0) {
			freeHashTableContents(table);
			DestroyHashTable(&table);
			if (table != NULL) {
				printf("FAIL: destroy did not set NULL");
			}
		} else if (strcmp(cmd, "print") == 0) {
			printHashTable(table);
		} else if (strcmp(cmd, "set") == 0) {
			printf("Key: ");
			char key[81];
			scanf(" %80s", key);
			char* value = malloc(81 * sizeof(char));
			scanf(" %80s", value);
			char* existingData = NULL;
			int success = InsertEntry(table, key, value, (void**) &existingData);
			switch(success){
				case 0:
					printf("Inserted into blank space\n");
					break;
				case 1:
					printf("Inserted after resolving hash collision\n");
					break;
				case 2:
					printf("Inserted after removing existing data: %s\n", existingData);
					free(existingData);
					break;
				default:
					printf("InsertEntry failed\n");
					free(value);
					break;
			}
		} else if (strcmp(cmd, "read") == 0) {
			printf("Key: ");
			char key[81];
			scanf(" %80s", key);
			char* value;
			int success = FindEntry(table, key, (void**) &value);
			if (success == 0) {
				printf("%s\n", value);
			} else {
				printf("readPosition returned failure\n");
			}
		} else if (strcmp(cmd, "delete") == 0) {
			printf("Key: ");
			char key[81];
			scanf(" %80s", key);
			char* value;
			int success = DeleteEntry(table, key, (void**) &value);
			if (success == 0) {
				printf("Deleted (was %s)\n", value);
				free(value);
			} else {
				printf("DeleteValue returned failure\n");
			}
		} else if (strcmp(cmd, "info") == 0) {
			HashTableInfo info;
			int status = GetHashTableInfo(table, &info);
			if (status == 0) {
				printf(
					"Bucket count: %u\n"
					"Load factor: %f\n"
					"Use factor: %f\n"
					"Largest bucket size: %u\n"
					"Dynamic behaviour: %d\n"
					"Expand use factor: %f\n"
					"Contract use factor: %f\n",
					info.bucketCount, info.loadFactor, info.useFactor, info.largestBucketSize,
					info.dynamicBehaviour, info.expandUseFactor, info.contractUseFactor);
			} else {
				printf("GetHashTableInfo returned failure\n");
			}
		} else if (strcmp(cmd, "setresize") == 0) {
			int resize;
			float expand, contract;
			printf("Resize: (0/1) ");
			scanf("%d", &resize);
			printf("Expand: ");
			scanf("%f", &expand);
			printf("Contract: ");
			scanf("%f", &contract);
			int status = SetResizeBehaviour(table, resize, expand, contract);
			if (status != 0) {
				printf("Set resize returned failure\n");
			}
		} else if (strcmp(cmd, "quit") == 0) {
			break;
		} else {
			printf("Invalid command\n");
		}
	}
	if (table != NULL) {
		freeHashTableContents(table);
		DestroyHashTable(&table);
	}
	return 0;
}
