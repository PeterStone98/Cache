/**
  The concept is to:
1st: read the arguments from the command line and build a cache
2nd: read through the file and convert each hex value into a decimal and then into a char array as its bianry digits
3rd: for each address check for hits and misses in the cache
 **/
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <stdint.h>

struct Parameters{
	int M;//Address size
	int E;//(2^e lines per set)
	int e;//index bits
	int S;//(2^s set or columns)
	int s;//line bits
	int B;//(2^b bytes per cache block)
	int b;//block bits
	int hits;//number ot hits
	int misses;//number of misses
};

struct Line{
	int uses;//number of uses(for replacement)
	int valid;//if its used or not(0 means empty, 1 means full)
	char* tag;//tag number
};

struct Set{
	struct Line *lines;//every set has an array of lines
};

struct Cache{
	struct Set *sets;//every cache has an array of sets
};

//build new cache
struct Cache build(int totalSets, int totalLines, int blockSize){
	struct Cache newCache;//cache being made
	struct Set set;//set to be made
	struct Line line;//lines to be made

	//set the new cache's set to the size of the totalSets
	newCache.sets = (struct Set*)malloc(sizeof(struct Set)* totalSets);
	for(int i = 0; i< totalSets;i++){
		//for each set make lines
		set.lines = (struct Line*)malloc(sizeof(struct Line)* totalLines);
		//place the set in cache's set array
		newCache.sets[i] = set;
		for(int j = 0;j<totalLines;j++){
			//start everything at 0
			line.uses = 0;		
			line.valid = 0;
			line.tag =0;
			//place the line in set line array
			set.lines[j] = line;
		}
	}
	//return the new built cache
	return newCache;
};

//find least recently used
int findLRU(struct Set set, struct Parameters param){
	int totalLines = param.E;
	int minUses = set.lines[0].uses;
	int LRU = 0;
	//iterate through the lines to see which is used least
	for (int i = 0; i<totalLines;i++){
		if(set.lines[i].uses < minUses){
			//change lru
			LRU = i;
			minUses = set.lines[i].uses;
		}
	}
	//return the index of the least recently used line
	return LRU;
}

//see if the input address tag matches the line address tag
int checkHit(struct Line line,char* address){
	if(line.tag != NULL){
		return strcmp(line.tag, address);
	}	else{
		return 1;
	}
}

//checks to see if the set is full(need to replace)
int isSetFull(struct Set set, struct Parameters param){
	int totalLines = param.E;
	//iterate through lines and see if they are valid or not
	for(int i = 0;i < totalLines;i++){
		//if valid = 0(empty) return 1
		if(set.lines[i].valid == 0){
			return 1;
		}
	}
	//if all lines are unvalid(full) return 0
	return 0;
}

//find any emty line to input address
int findEmptyLine(struct Set set, struct Parameters param){
	int totalLines = param.E;
	struct Line line;
	//iterate through lines and return first one that is valid(empty)
	for(int i = 0; i<totalLines;i++){
		line = set.lines[i];
		if(line.valid == 0){
			return i;
		}
	}
}

//turn decimal into binary char array
char * dec2bin(unsigned long n,struct Parameters param){
	int  i = 0;
	char array[100];
	//create array binary array
	while(n > 0){
		if((n%2) == 1){
			array[i] = '1';
		}else{
			array[i] = '0';
		}		
		//decrease n
		n = n/2;
		i++;
	}
	//make new array size of memory address(M)
	int size = param.M-1;
	char* bin;
	bin = (char*)malloc(sizeof(char) * (size));
	int k=0;
	//fill it with 0's
	for (int h =0; h< size;h++){
		bin[h] = '0';
	}
	//place the binary digits at the end of the array
	for(int j = i-1 ;j >= 0; j--){
		bin[size-j] =array[j];
		k++;        
	}
	return bin;
}

//turns char array of binary digits into a decimal
int bin2dec(char* array){
	int i;
	int num = 0;
	//get the length of the array and iterate through it	
	int size = sizeof(array)/sizeof(char);
	for(i =(size-1); i>=0 ;i--){
		if(array[i] == '1'){
			num =+ (pow(2,i));
		}
	}
	return num;
}

//run the cache to see if hit or miss
struct Parameters Simulation(struct Cache cache, struct Parameters param, char* address){	
	int i;
	//the tag size is M-b-s
	int tagSize = param.M - (param.s+param.b);
	//get the tag from the address input
	char *tag;
	tag = (char*)malloc(sizeof(char)*(tagSize));
	for (i =0; i<tagSize;i++){
		tag[i] = address[i];
	}

	//get the set number from the address input
	char *setN;
	int k=0;
	setN = (char*)malloc(sizeof(char)*(param.s));
	for(i; i<param.M-param.b;i++){
		setN[k] = address[i];
		k++;
	}

	//get block(although it is unused for this simulation)
	char *block;
	block = (char*)malloc(sizeof(char)*param.b);
	k =0;	
	for(i;i<param.M;i++){
		block[k] = address[i];
		k++;
	}

	//turn set array into dec
	int setNum = bin2dec(setN);
	//get the correct set	
	struct Set set = cache.sets[setNum];
	int hit = 0;//to keep track if theres a hit as we iterate through set
	//iterate through all lines
	for(int i = 0; i< param.E;i++){
		struct Line line = set.lines[i];
		//if the address tag macthes the line tag call hit
		if(checkHit(line,tag ) == 0){
			param.hits++;
			hit = 1;
			printf(" H\n");
			//increment line use
			cache.sets[setNum].lines[i].uses =+ 1;
			break;
		}
	}


	//if there was a miss and cache is not empty then place
	int setFull = isSetFull(set, param);
	if(hit ==0 && setFull == 1){
		param.misses++;
		printf(" M\n");
		//find empty line and fill it
		int emptyLine = findEmptyLine(set, param);
		set.lines[emptyLine].tag = tag;
		set.lines[emptyLine].valid = 1;
		//increment use count
		cache.sets[setNum].lines[emptyLine].uses =+ 1;
	}
	//have to replace
	else if(hit ==0&& setFull == 0){
		param.misses++;
		printf(" M\n");
		//find lru line and replace it
		int LRU = findLRU(set, param);
		set.lines[LRU].tag = tag;
		//set use count back to 0
		cache.sets[setNum].lines[LRU].uses = 0 ;
	}
	return param;
}

int main(int argc, char **argv){
	struct Cache cache;//cache to be made
	struct Parameters param;//beginning parameters
	int opt;
	int num =0;
	char *file;//file to use
	//loop over arguments
	while(-1 != (opt = getopt(argc, argv, "m:s:e:b:i:r:"))){
		switch(opt){
			case 'm':
				param.M = atoi(optarg);
				num++;
				break;
			case 's':
				param.s = atoi(optarg);
				num++;
				break;
			case 'e': 
				param.e = atoi(optarg);
				num++;
				break;
			case 'b':
				param.b = atoi(optarg);
				num++;
				break;
			case 'i':
				file = optarg;
				num++;
				break;
			case 'r':
				//unused becasue I only did lru	
				num++;
				break;
			default:
				printf("Wrong argument\n");
				break;
		}

	}
	//if all arguments were not present
	if(num!=6){
		printf("[Error]Please enter the following syntax: ./cachelab -m 4 -s 2 -e 0 -b 1 -i address01 -r rlu\n");
		return 0;
	}

	param.S = pow(2.0, param.s);//total sets = 2^s
	param.E = pow(2.0, param.e);//total lines = 2^e
	param.B = pow(2.0, param.b);//blocksize = 2^b
	param.hits = 0;//start at 0
	param.misses = 0;//start at 0
	//build cache
	cache = build(param.S, param.E, param.B);
	FILE *openFile;
	//open file and iterate through inputs
	openFile = fopen(file, "r");
	char lineBuffer[5];
	char *binArray;
	if(openFile!= NULL){
		while(fgets(lineBuffer, sizeof(lineBuffer), openFile) != NULL){
			//print value
			for(int i =0;i<sizeof(lineBuffer);i++){
				if(isalpha(lineBuffer[i])||isdigit(lineBuffer[i])){    
					printf("%c", lineBuffer[i]);
				}
			}
			//turn hex value into dec
			long n = strtol(lineBuffer, NULL, 16);
			//turn dec value into binary char array
			binArray = dec2bin(n, param);
			//run simulation
			param=Simulation(cache, param,  binArray);
		}
	}


	int sum = param.hits+param.misses;
	//calcultate miss rate
	double missRate = (param.misses/(double)sum)*100;
	//calculate running time
	int runningTime = sum + (param.misses*100);
	//print result
	printf("[result] hits: %d",param.hits);
	printf(" misses: %d", param.misses);
	printf(" miss rate: %.2f", missRate);
	printf("%c", '%');
	printf(" total running time: %d", runningTime);
	printf(" cycle\n");
	fclose(openFile);

	return 0;
}
