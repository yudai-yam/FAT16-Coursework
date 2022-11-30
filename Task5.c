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


typedef struct __attribute__((__packed__)) { 
    uint8_t DIR_Name[11]; // Non zero terminated string 
    uint8_t DIR_Attr; // File attributes 
    uint8_t DIR_NTRes; // Used by Windows NT, ignore 
    uint8_t DIR_CrtTimeTenth; // Tenths of sec. 0...199 
    uint16_t DIR_CrtTime; // Creation Time in 2s intervals 
    uint16_t DIR_CrtDate; // Date file created 
    uint16_t DIR_LstAccDate; // Date of last read or write 
    uint16_t DIR_FstClusHI; // Top 16 bits file's 1st cluster 
    uint16_t DIR_WrtTime; // Time of last write 
    uint16_t DIR_WrtDate; // Date of last write 
    uint16_t DIR_FstClusLO; // Lower 16 bits file's 1st cluster 
    uint32_t DIR_FileSize; // File size in bytes
} DirectoryContent;



int fileReader(char* file, void* memoryStruct, int offset, int readingByte){
    int fileDescriptor = open(file, O_RDONLY);
    lseek(fileDescriptor,offset,SEEK_SET);
    read(fileDescriptor, memoryStruct, readingByte);

    return fileDescriptor;
}

void dataExtracter(BootSector bootSector, DirectoryContent directryContent){
    // get the first cluster number
    uint16_t firstClusterHi = directryContent.DIR_FstClusHI << 16;
    uint16_t firstClusterLo = directryContent.DIR_FstClusLO;
    uint16_t firstCluster = firstClusterHi | firstClusterLo;

    // goes to first cluster in data section
    const int clusterSize = bootSector.BPB_SecPerClus*bootSector.BPB_BytsPerSec;
    char dataBuffer[clusterSize]; // store one cluster

    int beginningOfDataArea = (bootSector.BPB_RsvdSecCnt + bootSector.BPB_NumFATs*bootSector.BPB_FATSz16)*bootSector.BPB_BytsPerSec;
    beginningOfDataArea += bootSector.BPB_RootEntCnt*sizeof(DirectoryContent);

    fileReader("fat16.img",dataBuffer,beginningOfDataArea+clusterSize*(firstCluster-2), clusterSize);

    printf("The first cluster's content is \n%s\n", buffer);

    // go to the FAT table based on the index
    int FATsize = bootSector.BPB_FATSz16;
    int rsvdSec = bootSector.BPB_RsvdSecCnt;
    int bytsPerSec = bootSector.BPB_BytsPerSec;
    int sizeOfReserbedSector = rsvdSec * bytsPerSec;

    uint16_t fatBuffer[FATsize/sizeof(uint16_t)];
    fileReader("fat16.img",fatBuffer, sizeOfReservedSector+sizeof(uint16_t)*firstCluster, sizeof(uint16_t));

}

int main(){
    BootSector bootSector;
    DirectoryContent directoryContent;
    int fileDescriptor = fileReader("fat16.img", &bootSector, 0, sizeof(BootSector));

    //const int clusterSize = bootSector.BPB_SecPerClus*bootSector.BPB_BytsPerSec;

    int beginningOfRootDirectry = (bootSector.BPB_RsvdSecCnt + bootSector.BPB_NumFATs*bootSector.BPB_FATSz16)*bootSector.BPB_BytsPerSec;

   

    printf("Totsec16 is : %d\n", bootSector.BPB_TotSec16);

    DirectoryContent directoryArray[bootSector.BPB_RootEntCnt]; 

    fileReader("fat16.img",directoryArray,beginningOfRootDirectry,bootSector.BPB_RootEntCnt*sizeof(DirectoryContent));

    

    // keep reading directories until the first entry of name array is 0
    int i=0;
    
    while(directoryArray[i].DIR_Name[0] != 0){


        int attribute = directoryArray[i].DIR_Attr;
        
        int volumeName = (attribute >> 3) & 1;
        int directory = (attribute >> 4) & 1;

        //If both the directory bit and the volume ID/ Name bit are both zero, the entry corresponds to a 
        //regular file, such as a text file, a PDF, etc
        if (directory == 0 && volumeName == 0){
            // like .pdf
            printf("Type: regular file\n");
            dataExtracter(bootSector, directoryArray[i]);
        }
        else{
            printf("This is to be ignored since this is not a regular file\n");
        }

        i++;
        //fileReader("fat16.img",buffer, beginningOfDataArea, clusterSize);
    } 
    
    return 0;
}