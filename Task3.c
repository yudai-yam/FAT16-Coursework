#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdbool.h>

typedef struct __attribute__((__packed__)) { 
    uint8_t BS_jmpBoot[ 3 ]; // x86 jump instr. to boot code 
    uint8_t BS_OEMName[ 8 ]; // What created the filesystem 
    uint16_t BPB_BytsPerSec; // Bytes per Sector 
    uint8_t BPB_SecPerClus; // Sectors per Cluster 
    uint16_t BPB_RsvdSecCnt; // Reserved Sector Count 
    uint8_t BPB_NumFATs; // Number of copies of FAT 
    uint16_t BPB_RootEntCnt; // FAT12/FAT16: size of root DIR 
    uint16_t BPB_TotSec16; // Sectors, may be 0, see below 
    uint8_t BPB_Media; // Media type, e.g. fixed 
    uint16_t BPB_FATSz16; // Sectors in FAT (FAT12 or FAT16) 
    uint16_t BPB_SecPerTrk; // Sectors per Track 
    uint16_t BPB_NumHeads; // Number of heads in disk 
    uint32_t BPB_HiddSec; // Hidden Sector count 
    uint32_t BPB_TotSec32; // Sectors if BPB_TotSec16 == 0 
    uint8_t BS_DrvNum; // 0 = floppy, 0x80 = hard disk 
    uint8_t BS_Reserved1; //  
    uint8_t BS_BootSig; // Should = 0x29 
    uint32_t BS_VolID; // 'Unique' ID for volume 
    uint8_t BS_VolLab[ 11 ]; // Non zero terminated string 
    uint8_t BS_FilSysType[ 8 ];  // e.g. 'FAT16   ' (Not 0 term.) 
} BootSector;

int fileReader(char* file, void* bootSector, int offset, int readingByte){
    int fileDescriptor = open(file, O_RDONLY);

    lseek(fileDescriptor, offset, SEEK_SET);
    read(fileDescriptor, bootSector, readingByte);

    return fileDescriptor;
}

// 2 <= index < FATsize
void FATTableScanner(uint16_t buffer[], int index, bool first, int FATsize){
    if (first){
        printf("%d", index);
        if (!(index>=2 && index < FATsize)){
            printf("\nThis index is invalid");
            printf("FAT size: %d", FATsize);
            return;
        }
    }

    if (buffer[index] == 0x0000){
        printf("\nFree cluster detected\n");
    }
    else if (buffer[index] == 0x0001 || buffer[index] == 0x0002){
        printf("\nInvalid value detected\n");
    }
    else if (buffer[index] == 0xfff7){
        printf("\nOne or more bad sectors detected\n");
    }
    else if (buffer[index] >= 0xfff8){
        printf("\nEnd of file detected\n");
    }
    else {
        printf(" -> %d", buffer[index]);
        FATTableScanner(buffer,buffer[index], false, FATsize);
    }
}

int main(){
    BootSector bootSector;
    
    int fileDescriptor = fileReader("fat16.img",&bootSector, 0, sizeof(BootSector));

    // get the size of reserved sectors
    int rsvdSec = bootSector.BPB_RsvdSecCnt;
    int bytsPerSec = bootSector.BPB_BytsPerSec;
    int sizeOfSector = rsvdSec * bytsPerSec;
    int FATsize = bootSector.BPB_FATSz16;

    // make an array of 16 int and put data in
    uint16_t cache[FATsize]; 
    fileReader("fat16.img",cache, sizeOfSector, sizeof(uint16_t)*FATsize);

    printf("========== FAT Table ==========\n");
    
    // read the cluster one by one
    for (int i=2; i<FATsize; i++) //(cache[i]<0xfff8)
    // 1st cluster is for copy of media descriptor 
    // 2nd one is for end-of-file maker
    {
        // go to the next clusters
        printf("%d: %d\n", i, cache[i]);
    } 

    printf("===============================\n\n");

    printf("~List of clusters making up a file~\n");

    FATTableScanner(cache,6,true, FATsize);

    close(fileDescriptor);

    return 0;
}