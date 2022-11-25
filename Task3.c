#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>


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

int fileReader(char* file, BootSector* bootSector){
    int fileDescriptor = open(file, O_RDONLY);

    read(fileDescriptor, bootSector, sizeof(BootSector));

    return fileDescriptor;
}

int main(){
    BootSector bootSector;
    int fileDescriptor = fileReader("fat16.img",&bootSector);

    // get the size of reserved sectors
    int rsvdSec = bootSector.BPB_RsvdSecCnt;
    printf("The number of reserved sectors is %d\n", rsvdSec);
    int bytsPerSec = bootSector.BPB_BytsPerSec;
    printf("Bytes per sector is %d\n", bytsPerSec);
    int sizeOfSector = rsvdSec * bytsPerSec;
    printf("The size(byte) of reserved sector is %d\n", sizeOfSector);

    // the number of clusters in FAT

    int clusterInFAT = bootSector.BPB_FATSz16 / bootSector.BPB_SecPerClus;
    printf("The number of clusters in FAT is %d\n", clusterInFAT);

    // stoer it in an array

    // jump to the head of FAT
    lseek(fileDescriptor, sizeOfSector, SEEK_SET);

    int FATsize = bootSector.BPB_FATSz16;

    // make an array of 16 int
    int cache[FATsize]; 
    int *cachePointer = cache;

    int entry = 0;
    
    // read the cluster one by one
    int i = 0;
    while (cache[i]<0xfff8)
    {
        read(fileDescriptor, (cachePointer+i), sizeof(int));
        // go to the next cluster
        printf("entry is %d\n",entry);
        printf("scanned number is %d\n", cache[i]);
        printf("counter is %d\n\n\n",i);
        i++;
    }

    printf("The result is \n%x", cache);
    
    close(fileDescriptor);

    return 0;
}