#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>

#define MAX_SIZE 20
char file1[MAX_SIZE] = "a", file2[MAX_SIZE] = "b", outputFile[MAX_SIZE] = "c";
int r1, c1, r2, c2;
int A[MAX_SIZE][MAX_SIZE], B[MAX_SIZE][MAX_SIZE], C1[MAX_SIZE][MAX_SIZE], C2[MAX_SIZE][MAX_SIZE], C3[MAX_SIZE][MAX_SIZE];
struct thread_data{int i; int j;};

void getFilesNames(int argc, char *argv[]);
void readMatrices();
void readMatrix(char fileName[], int *row, int *col, int mat[MAX_SIZE][MAX_SIZE]);
void perMatrix();
void calculateMatrix();
void perRow();
void createRowsThreads();
void *calculateRow(void *threadArgs);
void perElement();
void createElementsThreads();
void *calculateElement(void *threadArgs);
void printData(char method[], int threads, long seconds, long useconds);
void writeMatrix(char method[], int row, int col, int mat[MAX_SIZE][MAX_SIZE]);

int main(int argc, char *argv[]) {
    getFilesNames(argc, argv);
    readMatrices();
    perMatrix();
    perRow();
    perElement();
    return 0;
}

void getFilesNames(int argc, char *argv[]) {
    if(argc > 1 && argv[1] != NULL)
        strcpy(file1, argv[1]);
    if(argc > 2 && argv[2] != NULL)
        strcpy(file2, argv[2]);
    if(argc > 3 && argv[3] != NULL)
        strcpy(outputFile, argv[3]);
    strcat(file1, ".txt");
    strcat(file2, ".txt");
}

void readMatrices() {
    readMatrix(file1, &r1, &c1, A);
    readMatrix(file2, &r2, &c2, B);
}

void readMatrix(char fileName[], int *row, int *col, int mat[MAX_SIZE][MAX_SIZE]) {
    FILE *f = fopen(fileName, "r");
    fscanf(f, "row=%d col=%d", row, col);
    for(int i = 0; i < *row; i++)
        for(int j = 0; j < *col; j++)
            fscanf(f, "%d", &mat[i][j]);
    fclose(f);
}

void perMatrix() {
    char method[MAX_SIZE] = "matrix";
    struct timeval stop, start;
    gettimeofday(&start, NULL);  //start checking time
    calculateMatrix();
    gettimeofday(&stop, NULL);  //end checking time
    //printing data
    printData(method, 1, stop.tv_sec - start.tv_sec, stop.tv_usec - start.tv_usec);
    writeMatrix(method, r1, c2, C1);
}

void calculateMatrix() {
    for(int i = 0; i < r1; i++) {
        for(int j = 0; j < c2; j++) {
            int sum = 0;
            for(int k = 0; k < c1; k++)
                sum += A[i][k] * B[k][j];
            C1[i][j] = sum;
        }
    }
}

void perRow() {
    char method[MAX_SIZE] = "row";
    struct timeval stop, start;
    gettimeofday(&start, NULL);  //start checking time
    createRowsThreads();
    gettimeofday(&stop, NULL);  //end checking time
    //printing data
    printData(method, r1, stop.tv_sec - start.tv_sec, stop.tv_usec - start.tv_usec);
    writeMatrix(method, r1, c2, C2);
}

void createRowsThreads() {
    pthread_t threads[r1];
    int i, rc;
    for(i = 0; i < r1; i++) {
        struct thread_data *threadData = malloc(sizeof(struct thread_data));
        threadData->i = i;
        rc = pthread_create(&threads[i], NULL, calculateRow, (void *)threadData);
        if(rc) {
            printf("ERROR; return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }
    for(i = 0; i < r1; i++)
        pthread_join(threads[i], NULL);
}

void *calculateRow(void *threadArgs) {
    struct thread_data *threadData = (struct thread_data *) threadArgs;
    int i = threadData->i;
    for(int j = 0; j < c2; j++) {
        int sum = 0;
        for(int k = 0; k < c1; k++)
            sum += A[i][k] * B[k][j];
        C2[i][j] = sum;
    }
    free(threadArgs);
    return NULL;
}

void perElement() {
    char method[MAX_SIZE] = "element";
    struct timeval stop, start;
    gettimeofday(&start, NULL);  //start checking time
    createElementsThreads();
    gettimeofday(&stop, NULL);  //end checking time
    //printing data
    printData(method, r1*c2, stop.tv_sec - start.tv_sec, stop.tv_usec - start.tv_usec);
    writeMatrix(method, r1, c2, C3);
}

void createElementsThreads() {
    pthread_t threads[r1][c2];
    int i, j, rc;
    for(i = 0; i < r1; i++) {
        for(j = 0; j < c2; j++) {
            struct thread_data *threadData = malloc(sizeof(struct thread_data));
            threadData->i = i;
            threadData->j = j;
            rc = pthread_create(&threads[i][j], NULL, calculateElement, (void *)threadData);
            if(rc) {
                printf("ERROR; return code from pthread_create() is %d\n", rc);
                exit(-1);
            }
        }
    }
    for(i = 0; i < r1; i++)
        for(j = 0; j < c2; j++)
            pthread_join(threads[i][j], NULL);
}

void *calculateElement(void *threadArgs) {
    struct thread_data *threadData = (struct thread_data *) threadArgs;
    int i = threadData->i, j = threadData->j;
    int sum = 0;
    for(int k = 0; k < c1; k++)
        sum += A[i][k] * B[k][j];
    C3[i][j] = sum;
    free(threadArgs);
    return NULL;
}

void printData(char method[], int threads, long seconds, long useconds) {
    printf("Thread per %s\n", method);
    printf("Number of threads created: %d\n", threads);
    printf("Seconds taken: %ld\n", seconds);
    printf("Microseconds taken: %ld\n\n", useconds);
}

void writeMatrix(char method[], int row, int col, int mat[MAX_SIZE][MAX_SIZE]) {
    char fileName[2*MAX_SIZE];
    sprintf(fileName, "%s_per_%s.txt", outputFile, method);
    FILE *f = fopen(fileName, "w");
    fprintf(f, "Method: A thread per %s\n", method);
    fprintf(f, "row=%d col=%d\n", row, col);
    for(int i = 0; i < row; i++) {
        for(int j = 0; j < col; j++) {
            fprintf(f, "%d ", mat[i][j]);
        }
        fprintf(f, "\n");
    }
    fclose(f);
}