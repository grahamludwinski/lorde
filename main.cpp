#include <iostream>
#include <omp.h>
#include <stdio.h>
#include "stringBuilder.cpp"
#include <string.h>
#include <sys/time.h>

using namespace std;

int getNextState(int currentState, char character) {
    if (currentState < 0 || currentState > 4) {
        printf("DONT PUT CRAP DATA INTO GETNEXTSTATE\n");
        return -1;
    }
    if(character != 'a' && character != 'b' && character != 'c' && character != 'd') {
        printf("DONT PUT CRAP DATA INTO GETNEXTSTATE\n");
        return -1;
    }
    if (currentState == 0) {
        if (character == 'a') {
            return 1;
        } else if (character == 'b') {
            return 4;
        } else if (character == 'c') {
            return 4;
        } else if (character == 'd') {
            return 4;
        }
    } else if (currentState == 1) {
        if (character == 'a') {
            return 1;
        } else if (character == 'b') {
            return 2;
        } else if (character == 'c') {
            return 4;
        } else if (character == 'd') {
            return 4;
        }
    } else if (currentState == 2) {
        if (character == 'a') {
            return 4;
        } else if (character == 'b') {
            return 2;
        } else if (character == 'c') {
            return 3;
        } else if (character == 'd') {
            return 3;
        }
    } else if (currentState == 3) {
        if (character == 'a') {
            return 1;
        } else if (character == 'b') {
            return 4;
        } else if (character == 'c') {
            return 3;
        } else if (character == 'd') {
            return 3;
        }
    } else if (currentState == 4) {
        if (character == 'a') {
            return 4;
        } else if (character == 'b') {
            return 4;
        } else if (character == 'c') {
            return 4;
        } else if (character == 'd') {
            return 4;
        }
    }
}

int getFinalState(int currentState, char *string, long int start, long int end) {
    int state = currentState;
    long int index = start;

    while (index < end) {
        state = getNextState(state, string[index]);
        if(state == 4) {
            return 4;
        }
        index++;
    }

    return state;
}

int main() {
    int i;
    int numThreads = 32; // optimistic threads
    numThreads++; // normal thread
    char *s = buildString();
    int stringLength = (int)strlen(s);
    int stringLengthPerThread = stringLength / (numThreads);
    int *mappings[numThreads];

    omp_set_num_threads(numThreads);

    struct timeval start, end, serialStart, serialEnd;

    gettimeofday(&start, NULL);

    int firstLastState = 0;
    int threadNum;
    long int startIndex;
    long int endIndex;
    #pragma omp parallel private(threadNum, startIndex, endIndex)
    {
        threadNum = omp_get_thread_num();
        startIndex = stringLengthPerThread*(threadNum);
        endIndex = stringLengthPerThread*(threadNum+1);
        if(threadNum == numThreads - 1) {
            endIndex = stringLength;
        }


        if (threadNum == 0) {
            firstLastState = getFinalState(0, s, startIndex, endIndex);
        } else {
            int something[5] = {
                    4,
                    getFinalState(1, s, startIndex, endIndex),
                    getFinalState(2, s, startIndex, endIndex),
                    getFinalState(3, s, startIndex, endIndex),
                    4
            };
            mappings[threadNum] = something;
        }
    }

    int nextState;
    for (i=1; i<numThreads; i++) {
        nextState = mappings[i][firstLastState];
        if (nextState == 0) {
            printf("next state is 0 but it can never be that\n");
        }
        firstLastState = mappings[i][firstLastState];
    }

    gettimeofday(&end, NULL);

    gettimeofday(&serialStart, NULL);
    int finalState = getFinalState(0, s, 0, stringLength);
    gettimeofday(&serialEnd, NULL);

    if (finalState == 3 && firstLastState == 3) {
        long int serialTime = ((serialEnd.tv_sec * 1000000 + serialEnd.tv_usec)
                - (serialStart.tv_sec * 1000000 + serialStart.tv_usec));
        long int parallelTime = ((end.tv_sec * 1000000 + end.tv_usec)
                - (start.tv_sec * 1000000 + start.tv_usec));
        printf("Serial:   %ld ms\n", serialTime / 1000);
        printf("Parallel: %ld ms\n", parallelTime / 1000);
        printf("SUCCESS!!!!\n");
        return 0;
    } else {
        printf("Expected serial and parallel to both be 3\n");
        printf("Serial result: %d\n", finalState);
        printf("Parallel result: %d\n", firstLastState);
        return -1;
    }
}