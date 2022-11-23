#include <stdio.h>
#include <fcntl.h>

void fileReader(char* file, off_t offset, int readingByte){
    char arr[100];
    int fileDescriptor = open(file, O_RDONLY);

    lseek(fileDescriptor, offset, SEEK_CUR);
    read(fileDescriptor, arr, readingByte);
    printf("The result (offset = %i, readingByte = %i) is \n%s", offset, readingByte, arr);
    close(fileDescriptor);
}

int main(){
    fileReader("Task1.txt", 5, 10);
    return 0;
}