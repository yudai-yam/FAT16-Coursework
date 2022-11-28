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



typedef struct __attribute__((__packed__)) {  // 32 bytes
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

typedef struct __attribute__((__packed__)) { // 32 bytes
    uint8_t LDIR_Ord; // Order/ position in sequence/ set 
    uint8_t LDIR_Name1[ 10 ]; // First 5 UNICODE characters 
    uint8_t LDIR_Attr; // = ATTR_LONG_NAME (xx001111) 
    uint8_t LDIR_Type; // Should = 0 
    uint8_t LDIR_Chksum; // Checksum of short name 
    uint8_t LDIR_Name2[ 12 ]; // Middle 6 UNICODE characters 
    uint16_t LDIR_FstClusLO; // MUST be zero 
    uint8_t LDIR_Name3[ 4 ]; // Last 2 UNICODE characters
} LongDirectoryContent;



int fileReader(char* file, void* memoryStruct, int offset, int readingByte){
    int fileDescriptor = open(file, O_RDONLY);
    lseek(fileDescriptor,offset,SEEK_CUR);
    read(fileDescriptor, memoryStruct, readingByte);

    return fileDescriptor;
}

// returns a series of characters in one entry, therefore they must be combined to be complete
uint16_t longNameReader(DirectoryContent directoryEntry){

    LongDirectoryContent longDirectoryContent;

    // keep reading LongDirectoryContent until it finds LDIR_Ord = 1 or 0x41
    uint16_t longTempStrage[13];

    // read it into longDirectoryContent (first round)
    // read the same cluster again as long directory 
    fileReader("fat16.img", &longDirectoryContent, 0, sizeof(LongDirectoryContent));


    // first name cluster management
    for (int j=0; j<10; j+=2){
        uint8_t lowerBit = longDirectoryContent.LDIR_Name1[j];
        uint8_t upperBit = longDirectoryContent.LDIR_Name1[j+1];

        // conbine them
        uint16_t shiftedUpperBit = upperBit >> 8;
        uint16_t combinedBit = shiftedUpperBit | lowerBit;

        strcat(longTempStrage, combinedBit);
    }

    // middle name cluster management
    for (int j=0; j<12; j+=2){
        uint8_t lowerBit = longDirectoryContent.LDIR_Name2[j];
        uint8_t upperBit = longDirectoryContent.LDIR_Name2[j+1];

        // conbine them
        uint16_t shiftedUpperBit = upperBit >> 8;
        uint16_t combinedBit = shiftedUpperBit | lowerBit;

        strcat(longTempStrage, combinedBit);
    }

    // last name cluster management
    for (int j=0; j<4; j+=2){
        uint8_t lowerBit = longDirectoryContent.LDIR_Name3[j];
        uint8_t upperBit = longDirectoryContent.LDIR_Name3[j+1];

        // conbine them
        uint16_t shiftedUpperBit = upperBit >> 8;
        uint16_t combinedBit = shiftedUpperBit | lowerBit;

        strcat(longTempStrage, combinedBit);
    }

    // keep reading LongDirectoryContent until it finds LDIR_Ord = 1 or 0x41
    //if (longDirectoryContent.LDIR_Ord = 0x01){
        return longTempStrage;
   
 
}



int main(){
    BootSector bootSector;
    DirectoryContent directoryContent;
    int fileDescriptor = fileReader("fat16.img", &bootSector, 0, sizeof(BootSector));


    // get the size of the filesystem up to root directly (byte)
    int beginningOfRootDirectry = (bootSector.BPB_RsvdSecCnt + bootSector.BPB_NumFATs*bootSector.BPB_FATSz16)*bootSector.BPB_BytsPerSec;

    // array to store root directory
    DirectoryContent directoryArray[bootSector.BPB_RootEntCnt/sizeof(DirectoryContent)]; 

    fileReader("fat16.img",directoryArray,beginningOfRootDirectry,bootSector.BPB_RootEntCnt);


    for (int i=0; i<bootSector.BPB_RootEntCnt/sizeof(DirectoryContent); i++){

        bool isRegularFile;
        bool longName = false;


        // long name management
        uint16_t longNameStorage[100];

        while (directoryArray[i].DIR_Attr == 15){
            strcat(longNameStorage,longNameReader(directoryArray[i]));
            i++;
            // You need to reverse the string arrays
        }
        printf("Name (long): %s", longNameStorage);

            
    }
}