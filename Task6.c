#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdlib.h>
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
    lseek(fileDescriptor,offset,SEEK_SET);
    read(fileDescriptor, memoryStruct, readingByte);

    return fileDescriptor;
}

// extract the first digit of LDIR_Ord
int firstNumExtract(uint8_t hex){
    int firstDigit;
    // if 0x41 () ~ 0x4F
    if (65 <= hex <= 79){
        firstDigit = hex - 64;
    }
    else{
        firstDigit = hex;
    }
    return firstDigit;
}

// returns a series of characters in one entry, therefore they must be combined to be complete
uint16_t *longNameReader(LongDirectoryContent longDirectoryContent){

    static uint16_t longTempStrage[13];

    // first name cluster management
    for (int j=0; j<10; j+=2){
        uint8_t lowerBit = longDirectoryContent.LDIR_Name1[j];
        uint8_t upperBit = longDirectoryContent.LDIR_Name1[j+1];

        // conbine them
        uint16_t shiftedUpperBit = upperBit >> 8;
        uint16_t combinedBit = shiftedUpperBit | lowerBit;

        longTempStrage[j/2] = combinedBit; // only one char can be stored so this gotta be fixed
    }

    // middle name cluster management
    for (int j=0; j<12; j+=2){
        uint8_t lowerBit = longDirectoryContent.LDIR_Name2[j];
        uint8_t upperBit = longDirectoryContent.LDIR_Name2[j+1];

        // conbine them
        uint16_t shiftedUpperBit = upperBit >> 8;
        uint16_t combinedBit = shiftedUpperBit | lowerBit;

        longTempStrage[5 + (j/2)] = combinedBit;
    }

    // last name cluster management
    for (int j=0; j<4; j+=2){
        uint8_t lowerBit = longDirectoryContent.LDIR_Name3[j];
        uint8_t upperBit = longDirectoryContent.LDIR_Name3[j+1];

        // conbine them
        uint16_t shiftedUpperBit = upperBit >> 8;
        uint16_t combinedBit = shiftedUpperBit | lowerBit;

        longTempStrage[11 + (j/2)] = combinedBit;
    }

    return longTempStrage;
}

void shortNameReader(DirectoryContent directoryEntry){

    uint8_t shortNameStorage[12];
       
    if (directoryEntry.DIR_Name[0] == ' '){
        printf("This entry is not valid\n");
        return;
    }
    if (directoryEntry.DIR_Name[0] == 0xE5){
        printf("This entry is currently unused\n");
        return;
    }
    
    // read first 8 bytes
    for (int j=0; j<8; j++){
        // when ' ' and extention sector is not null -> .
        if ((directoryEntry.DIR_Name[j] == ' ' && j != 0) && (directoryEntry.DIR_Name[8] != ' ')){
            printf(".");
            break;
        }
        else if (!(directoryEntry.DIR_Name[j]<32 || directoryEntry.DIR_Name[j]>127)){
            printf("%c",directoryEntry.DIR_Name[j]);
        }
    }

    // read last 3 bytes
    for (int j = 8; j<11;j++){
        if (directoryEntry.DIR_Name[j] == ' ' && j == 8){
            break;
        }
        else if (!(directoryEntry.DIR_Name[j]<32 || directoryEntry.DIR_Name[j]>127)){
            printf("%c",directoryEntry.DIR_Name[j]);
        }
    }
    return;
}

void dataReader(DirectoryContent directoryContent){
    printf("The higher 16 bits of first cluster is %d\n", directoryContent.DIR_FstClusHI);
        printf("The lower 16 bits of first cluster is %d\n", directoryContent.DIR_FstClusLO);

        int hour;
        int minute;
        int second;

        second = (directoryContent.DIR_WrtTime & 31)*2;
        minute = (directoryContent.DIR_WrtTime >> 5)& 63;
        hour = (directoryContent.DIR_WrtTime >> 11) & 31;

        printf("The last modified time is %d:%d:%d\n", hour,minute,second);

        int year;
        int month;
        int day;

        day = directoryContent.DIR_WrtDate & 31;
        month = (directoryContent.DIR_WrtDate >> 5) & 15;
        year = (directoryContent.DIR_WrtDate >> 9 & 127) + 1980;

        printf("The last modified date is %d/%d/%d\n", year, month, day);

        printf("Size of the file is %hu\n", directoryContent.DIR_FileSize);

        printf("==============Attribute Management===========\n");

        int attribute = directoryContent.DIR_Attr;
       
        int readOnly = attribute & 1;
        int hidden = (attribute >> 1) & 1;
        int system = (attribute >> 2) & 1;
        int volumeName = (attribute >> 3) & 1;
        int directory = (attribute >> 4) & 1;
        int archive = (attribute >> 5) & 1;

        printf("a- %d\n", archive);
        printf("d- %d\n", directory);
        printf("v- %d\n", volumeName);
        printf("s- %d\n", system);
        printf("h- %d\n", hidden);
        printf("r- %d\n", readOnly);

        printf("=============================================\n");

        //If both the directory bit and the volume ID/ Name bit are both zero, the entry corresponds to a 
        //regular file, such as a text file, a PDF, etc
        //if just the volume bit is set, the entry represents the name of the 
        //‘disk’, which is normally shown alongside the drive letter in Windows;
        // however, if just the directory bit is set, the entry represents the name of a directory, or folder in Windows
        if (directory == 0 && volumeName == 0){
            // like .pdf
            printf("Type: regular file\n");
        }
        else if (directory == 0 && volumeName == 1){
            // represents name of the disk (volume label)
            printf("Type: volume label\n");
        }
        else if (directory == 1 && volumeName == 0){
            // the entry represents the name of a directory, or folder in Windows
            printf("Type: directory\n");
        }
        else {
            printf("Both directory and volume name are set to 1. There might be something wrong\n");
        }
        printf("\n");

}


int main(){
    BootSector bootSector;
    DirectoryContent directoryContent;
    int fileDescriptor = fileReader("fat16.img", &bootSector, 0, sizeof(BootSector));


    // get the size of the filesystem up to root directly (byte)
    int beginningOfRootDirectry = (bootSector.BPB_RsvdSecCnt + bootSector.BPB_NumFATs*bootSector.BPB_FATSz16)*bootSector.BPB_BytsPerSec;

    // array to store root directory
    DirectoryContent directoryArray[bootSector.BPB_RootEntCnt]; 

    fileReader("fat16.img",directoryArray,beginningOfRootDirectry,bootSector.BPB_RootEntCnt*sizeof(DirectoryContent));


    //for (int i=0; i<bootSector.BPB_RootEntCnt/sizeof(DirectoryContent); i++){
    int i=0;
    while(directoryArray[i].DIR_Name[0] != 0){

        bool isRegularFile;
        bool longName = false;
        int longNameEntry = 0;

        int attribute = directoryArray[i].DIR_Attr;


        // long name management
        if (attribute == 15){
            LongDirectoryContent longDirectoryContent;

            // move to the place where long directory content starts and read
            fileReader("fat16.img", &longDirectoryContent, beginningOfRootDirectry + i * sizeof(DirectoryContent), sizeof(LongDirectoryContent));

            // analyze how much memory the long name takes
            int longNameEntry = firstNumExtract(longDirectoryContent.LDIR_Ord);
            uint16_t longNameStorage[longNameEntry][13];

            
            // store each long name sequencially in longNameStorage
            int longNameIndex = 0;
            for (longNameIndex; longNameIndex<longNameEntry; longNameIndex++){
                fileReader("fat16.img", &longDirectoryContent, beginningOfRootDirectry + (i + longNameIndex) * sizeof(DirectoryContent), sizeof(LongDirectoryContent));
                uint16_t *ptr;
                ptr = longNameReader(longDirectoryContent);
                for (int j=0;j<13;j++){
                    longNameStorage[longNameIndex][j] = *(ptr + j);
                }
            }

            // reverse and print the long name
            printf("Name (long): ");
            for (int j=longNameEntry; 0<j; j--){ // use j-1 to access array
                for(int k=0; k<13; k++){
                    printf("%c", longNameStorage[j-1][k]);
                }                
            }
            printf("\n");

            dataReader(directoryArray[i+longNameIndex]);

            // increment the index of outer loop because the index has been incremented for long name directories
            i = i + longNameIndex;
        }
        
        // short name management
        else{
            printf("Name (short): ");
            shortNameReader(directoryArray[i]);
            printf("\n");
            dataReader(directoryArray[i]);
        }
        
        i++;
    }

    close(fileDescriptor);
    return 0;
}