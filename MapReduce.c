//for i in $( ls ); do awk '{print length}' $i |sort -nr|head -1; done | sort -nr| head -1

#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include "mpi.h"

char rootInputFolderPath[256], rootOutputFolderPath[256], sysCmd[256];
int mpiId, mpiProcesses, files_count;
char files[100][256], filesCopy[100][256];
char* _outputFolder, * _fileName;

void countWordsForAllFiles();
void combineAll();
void writeInvertedIndexForAllWords();
int nextSlave();
void releaseSlaves();
void joinAll();

void combine(char* firstFolder, char* secondFolder, char* outputFolder);
void countWords(char* inputFilename, char* outputFolder, char* fileName);
void cleanLine(char* line);
void splitLine(char* line);
void increaseWordCount(char* word);

void printTimestamp();

int main(int argc, char** argv){
	MPI_Init(NULL, NULL);
	
	MPI_Comm_rank(MPI_COMM_WORLD, &mpiId);
	MPI_Comm_size(MPI_COMM_WORLD, &mpiProcesses);
	
	strcpy(rootInputFolderPath, argv[1]);
	strcpy(rootOutputFolderPath, argv[2]);

	if( mpiId == 0 ) {
		FILE* file;
		char aux[256];
		
		printTimestamp(); printf("[0-master] Preparing folder %s and reading file names\r\n", rootOutputFolderPath);
		//not sure if files will always be printed alphabetically
		sprintf(sysCmd, "rm -rf %s && mkdir %s && cd %s && ls -1 *.txt | sort -g > ../%s/p0_files.txt && cd ..", rootOutputFolderPath, rootOutputFolderPath, rootInputFolderPath, rootOutputFolderPath);
		system(sysCmd);
		
		sprintf(aux, "%s/p0_files.txt", rootOutputFolderPath);
		file = fopen(aux, "r");
		while( fgets(aux, 256, file) != NULL ) {
			aux[strlen(aux)-1] = '\0';
			strcpy(filesCopy[files_count], aux);
			aux[strlen(aux)-4] = '\0';
			strcpy(files[files_count++], aux);
		}
		fclose(file);
		printTimestamp(); printf("[0-master] Done preparing folder %s and reading file names\r\n", rootOutputFolderPath);
		
		printTimestamp(); printf("[0-master] Calling slaves to count words from files\r\n");
		countWordsForAllFiles();
		
		printTimestamp(); printf("[0-master] Calling slaves to combine all folders into one\r\n");
		combineAll();
		
		releaseSlaves();
		
		printTimestamp(); printf("[0-master] Writing inverted indexes to file\r\n");
		writeInvertedIndexForAllWords();
		printTimestamp(); printf("[0-master] Done writing inverted indexes to file\r\n");
	} else {
		int task;
		char fileName[256], inputFilePath[256], outputFolder[256], firstFolder[256], secondFolder[256], firstFoldername[256], secondFoldername[256];
		do {
			MPI_Send(&task, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
			MPI_Recv(&task, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			if(task==1) {
				MPI_Recv(inputFilePath, 256, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				MPI_Recv(outputFolder, 256, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				MPI_Recv(fileName, 256, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				countWords(inputFilePath, outputFolder, fileName);
				printTimestamp(); printf("[%d-slave] Finished counting words from file %s to folder %s\r\n", mpiId, inputFilePath, outputFolder);
			} else if(task==2) {
				MPI_Recv(firstFolder, 256, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				MPI_Recv(secondFolder, 256, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				MPI_Recv(outputFolder, 256, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				combine(firstFolder, secondFolder, outputFolder);
				printTimestamp(); printf("[%d-slave] Finished combining folders %s and %s into %s\r\n", mpiId, firstFolder, secondFolder, outputFolder);
			}
		}while(task!=0);
	}
	
	MPI_Finalize();
	
	return 0;
}

void countWordsForAllFiles() {
	int i, task, worker;
	char inputFilePath[256], outputFolder[256], fileName[256];
	
	for(i=0; i<files_count; i++) {
		sprintf(inputFilePath, "%s/%s.txt", rootInputFolderPath, files[i]);
		sprintf(outputFolder, "%s/%s", rootOutputFolderPath, files[i]);
		sprintf(fileName, "%s.txt", files[i]);
		sprintf(sysCmd, "mkdir %s", outputFolder);
		system(sysCmd);
		
		worker = nextSlave();
		task = 1;
		MPI_Send(&task, 1, MPI_INT, worker, 0, MPI_COMM_WORLD);
		MPI_Send(inputFilePath, strlen(inputFilePath)+1, MPI_CHAR, worker, 0, MPI_COMM_WORLD);
		MPI_Send(outputFolder, strlen(outputFolder)+1, MPI_CHAR, worker, 0, MPI_COMM_WORLD);
		MPI_Send(fileName, strlen(fileName)+1, MPI_CHAR, worker, 0, MPI_COMM_WORLD);
	}
	
	joinAll();
}

void combineAll() {
	char firstFile[256], secondFile[256], outputFile[256];
	char firstFolder[256], secondFolder[256], outputFolder[256];
	int files_left = files_count, i, i1, i2, j, worker, task;
	do {
		printTimestamp(); printf("[0-master] %d folders left: ", files_left); for(j=0; j<files_count; j++) printf("%s ", (strlen(files[j])>0?files[j]:"*")); printf("\r\n");
		i1 =  -1;
		for(i=0; i<files_count; i++) {
			if(strlen(files[i]) > 0) {
				if(i1==-1) {
					i1 = i;
				} else {
					i2 = i;
					
					worker = nextSlave();
					
					strcpy(firstFile, files[i1]);
					strcpy(secondFile, files[i2]);
					sprintf(outputFile, "p%d_f%d", worker, files_left);
					sprintf(firstFolder, "%s/%s", rootOutputFolderPath, firstFile);
					sprintf(secondFolder, "%s/%s", rootOutputFolderPath, secondFile);
					sprintf(outputFolder, "%s/%s", rootOutputFolderPath, outputFile);
					
					task = 2;
					MPI_Send(&task, 1, MPI_INT, worker, 0, MPI_COMM_WORLD);
					MPI_Send(firstFolder, strlen(firstFolder)+1, MPI_CHAR, worker, 0, MPI_COMM_WORLD);
					MPI_Send(secondFolder, strlen(secondFolder)+1, MPI_CHAR, worker, 0, MPI_COMM_WORLD);
					MPI_Send(outputFolder, strlen(outputFolder)+1, MPI_CHAR, worker, 0, MPI_COMM_WORLD);
					
					strcpy(files[i1], outputFile);
					files[i2][0] = '\0';
					files_left--;
					i1 = -1;
				}
			}
		}
		joinAll();
	} while(files_left > 1);
}

void writeInvertedIndexForAllWords() {
	DIR *dir;
	struct dirent *ent;
	FILE * inFile, * outFile;
	char inputFolderPath[256], inputFilePath[256], outputFilePath[256];
	char noExtFileName[256], doc[256], count[256];
	int fileTrack;
	int printDocWithCount0WhereNeeded = 1;
	
	sprintf(inputFolderPath, "%s/%s", rootOutputFolderPath, files[0]);
	sprintf(outputFilePath, "%s/%s", rootOutputFolderPath, "invIndex.txt");
	
	outFile = fopen(outputFilePath, "w");
	
	if ((dir = opendir (inputFolderPath)) != NULL) {
		while ((ent = readdir (dir)) != NULL) {
			if(ent->d_type == DT_REG) {
				strcpy(noExtFileName, ent->d_name);
				noExtFileName[strlen(noExtFileName)-4] = '\0';
				sprintf(inputFilePath, "%s/%s", inputFolderPath, ent->d_name);
				fileTrack = 0;
				
				inFile = fopen(inputFilePath, "r");
				
				fprintf(outFile, "%s", noExtFileName);
				while(fscanf(inFile, "%s %s", doc, count)==2) {
					while( strcmp(filesCopy[fileTrack], doc) != 0 && fileTrack < files_count && printDocWithCount0WhereNeeded ) {
						fprintf(outFile, " %s %s", filesCopy[fileTrack], "0");
						fileTrack++;
					}
					fprintf(outFile, " %s %s", doc, count);
					fileTrack++;
				}
				while( fileTrack < files_count && printDocWithCount0WhereNeeded ) {
					fprintf(outFile, " %s %s", filesCopy[fileTrack], "0");
					fileTrack++;
				}
				fprintf(outFile, "\r\n");
				
				fclose(inFile);
			}
		}
	}
	
	fclose(outFile);
}

int nextSlave() {
	int ok;
	MPI_Status status;
	
	MPI_Recv(&ok, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
	
	return status.MPI_SOURCE;
}

void releaseSlaves() {
	int i, ok=0;
	
	for(i=1; i<mpiProcesses; i++) {
		MPI_Send(&ok, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
	}
}

void joinAll(){
	int i, ok;
	
	for(i=1; i<mpiProcesses; i++) {
		MPI_Recv(&ok, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		ok = 3;
		MPI_Send(&ok, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
	}
}

void combine(char* firstFolderPath, char* secondFolderPath, char* outputFolderPath) {
	DIR *dir;
	struct dirent *ent;
	char firstFilePath[256], secondFilePath[256], outputFilePath[256], line[3250];
	FILE * firstFile, * secondFile, * outputFile;
	
	sprintf(sysCmd, "mkdir %s", outputFolderPath);
	system(sysCmd);
	
	if ((dir = opendir (firstFolderPath)) != NULL) {
		while ((ent = readdir (dir)) != NULL) {
			sprintf(firstFilePath, "%s/%s", firstFolderPath, ent->d_name);
			sprintf(secondFilePath, "%s/%s", secondFolderPath, ent->d_name);
			sprintf(outputFilePath, "%s/%s", outputFolderPath, ent->d_name);
			
			if(ent->d_type == DT_REG)
			if( access( secondFilePath, F_OK ) != -1 ) {
				firstFile = fopen(firstFilePath, "r");
				secondFile = fopen(secondFilePath, "r");
				outputFile = fopen(outputFilePath, "w");

				while( fgets(line, 3250, firstFile) != NULL ) {
					fputs(line, outputFile);
				}
				while( fgets(line, 3250, secondFile) != NULL ) {
					fputs(line, outputFile);
				}
				
				fclose(firstFile);
				fclose(secondFile);
				fclose(outputFile);
			} else {
				sprintf(sysCmd, "cp %s %s", firstFilePath, outputFilePath);
				system(sysCmd);
			}
		}
	}
	
	if ((dir = opendir (secondFolderPath)) != NULL) {
		while ((ent = readdir (dir)) != NULL) {
			sprintf(firstFilePath, "%s/%s", firstFolderPath, ent->d_name);
			sprintf(secondFilePath, "%s/%s", secondFolderPath, ent->d_name);
			sprintf(outputFilePath, "%s/%s", outputFolderPath, ent->d_name);
			
			if(ent->d_type == DT_REG)
			if( access( firstFilePath, F_OK ) == -1 ) {				
				sprintf(sysCmd, "cp %s %s", secondFilePath, outputFilePath);
				system(sysCmd);
			}
		}
	}
}

void countWords(char* inputFilePath, char* outputFolder, char* fileName){
	FILE* file;
	char line[3250];
	int debugMaxLines = 0;
	
	_outputFolder = outputFolder;
	_fileName = fileName;
	file = fopen(inputFilePath, "r");
	while( fgets(line, 3250, file) != NULL /*&& debugMaxLines++ < 20*/ ) {
		cleanLine(line);
		splitLine(line);
	}
	fclose(file);
}

void cleanLine(char* line){
	int i;
	for(i=0; i<strlen(line); i++){
		if( line[i]>='A' && line[i]<='Z' ){
			line[i] += 'a'-'A';
		}else if( line[i]<'a' || line[i]>'z' ){
			line[i] = ' ';
		}
	}
}

void splitLine(char* line){
	char* nextWord;
	
	nextWord = strtok (line, " ");
	
	while (nextWord != NULL) {
		increaseWordCount(nextWord);
		nextWord = strtok (NULL, " ");
	}
}

void increaseWordCount(char* word){
	struct stat st = {0};
	FILE *file;
	char path[256];
	int c;
	
	strcpy(path, _outputFolder);
	strcat(path, "/");
	strcat(path, word);
	strcat(path, ".txt");
	
	if( access( path, F_OK ) != -1 ) {
		file = fopen(path,"r");
		fscanf(file, "%s %d", _fileName, &c);
		c++;
		fclose(file);
		file = fopen(path,"w");
		fprintf(file, "%s %d\r\n", _fileName, c);
		fclose(file);
	} else {
		file = fopen(path,"w");
		fprintf(file, "%s %s\r\n", _fileName, "1");
		fclose(file);
	}
}

void printTimestamp() {
	time_t epoch = time(NULL);
	struct tm * now = localtime(&epoch);
	printf("[%02d:%02d:%02d] ", now->tm_hour, now->tm_min, now->tm_sec);
}