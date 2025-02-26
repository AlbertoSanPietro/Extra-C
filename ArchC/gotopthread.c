#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define MAXS 100

typedef struct {
		size_t size;
		double input[MAXS];
		double total_somma;
		double total_media;
		} thread_args;
		
		
void *array_sum(void *arg) {
	thread_args *args = (thread_args *)arg;
	args->total_somma = 0;
	for (int i=0; i<args->size; i++) {
		args->total_somma += args->input[i];
	}
	printf("la somma è: %lf\n", args->total_somma);
	return NULL;
}

void *avg(void *arg) {
	thread_args *args = (thread_args *)arg;
	args->total_media=0;
	for (size_t i =0; i < args->size; i++) {
		args->total_media +=args->input[i];
	}
	if (args->size==0) 
		goto here;
	args->total_media /=args->size;
	printf("la media è: %lf", args->total_media);
here: return NULL;	
}


	
int main(int argc, char *argv[]) {
	int Media = 0;
	int Somma = 0;
	
	/*******************************************/
	if (argc< 1) {goto error;}
		(argc >1 && strcmp(argv[1], "s") == 0) ? ((argc>2 && strcmp(argv[2], "m") == 0) ? (Somma=Media=1) : (Somma=1)) : ((argc>1 && strcmp(argv[1], "m") == 0) ? ((argc>2 && strcmp(argv[2], "s") == 0) ? (Somma=Media=1) : (Media=1)) : 0); 
	
	 if (Somma && Media) {
        goto both;
    }
    if (Somma) {
        goto addizione;
    }
    if (Media) {
        goto average;
    }
	
	error:
	{
		puts("mancano gli argomenti!");
		return -1;
	}
		
		both: {
			thread_args args;
			args.size=0;
			double num=0;
	
			puts("inserisci un numero\n");
			while(args.size < 100) {
					scanf("%lf", &num);
					if (num == -1) {break;}
					args.input[args.size++]=num;

				}	
			
			pthread_t id, id1;
			pthread_create(&id, NULL, array_sum, &args);
			pthread_create(&id1, NULL, avg, &args);
			pthread_join(id, NULL);
			pthread_join(id1, NULL);
		}
		
		
	
	addizione:
	{
		thread_args args;
		args.size=0;
		double num=0;
		
		while(args.size < 100) {
			scanf("%lf", &num);
			if (num == -1) {break;}
			args.input[args.size++]=num;

		}	

			pthread_t id;
			pthread_create(&id, NULL, array_sum, &args);
			pthread_join(id, NULL);
					
	}
	
	average: 
	{
		thread_args args;
		args.size=0;
		double num=0;
		
		while(args.size < 100) {
			scanf("%lf", &num);
			if (num == -1) {break;}
			args.input[args.size++]=num;

		}	
			pthread_t id;
			pthread_create(&id, NULL, avg, &args);
			pthread_join(id, NULL);
	}

	return 0;
}




