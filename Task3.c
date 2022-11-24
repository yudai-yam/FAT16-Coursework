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

    close(fileDescriptor);

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

    // jump to the head of FAT
    lseek(fileDescriptor, 0, SEEK_CUR);

    return 0;
}