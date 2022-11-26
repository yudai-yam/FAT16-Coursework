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

void namePrinter(DirectoryContent directoryEntry, bool isRegularFile, bool isIgnored){
    if (isIgnored == true){
        printf("This directory is to be ignored\n\n\n\n");
        return;
    }

    if (isRegularFile){
        printf("Name: ");
        // read first 8 bytes
        bool spaceFound = false;
        for (int j=0; j<8; j++){
            if (directoryEntry.DIR_Name[j] == ' ' && j != 0){
                printf(".");
                break;
            }
            else if (!(directoryEntry.DIR_Name[j]<32 || directoryEntry.DIR_Name[j]>127)){
                printf("%c",directoryEntry.DIR_Name[j]);
            }
            if (j==7){
                printf(".");
            }
        }

        // read last 3 bytes
        for (int j = 8; j<11;j++){
            if (directoryEntry.DIR_Name[j] == ' '){
                break;
            }
            else if (!(directoryEntry.DIR_Name[j]<32 || directoryEntry.DIR_Name[j]>127)){
                printf("%c",directoryEntry.DIR_Name[j]);
            }
        }
    }
    else{
        printf("Name: %.8s", directoryEntry.DIR_Name);
    }

    printf("\n\n\n\n");
        
}


int main(){
    BootSector bootSector;
    DirectoryContent directoryContent;
    int fileDescriptor = fileReader("fat16.img",&bootSector, 0, sizeof(BootSector));
    

    // get the size of the filesystem up to root directly
    int beginningOfRootDirectry = (bootSector.BPB_RsvdSecCnt + bootSector.BPB_NumFATs*bootSector.BPB_FATSz16)*bootSector.BPB_BytsPerSec;

    // array to store root directory
    DirectoryContent directoryArray[bootSector.BPB_RootEntCnt/sizeof(DirectoryContent)];

    fileReader("fat16.img",directoryArray,beginningOfRootDirectry,bootSector.BPB_RootEntCnt);



    for (int i=0; i<bootSector.BPB_RootEntCnt/sizeof(DirectoryContent); i++){
        //printf("The size of the file is %hu\n", directoryArray[i].DIR_FileSize);
        //printf("The name is %.11s\n", directoryArray[i].DIR_Name);

        bool isRegularFile;
        bool isIgnored = false;
        

        printf("The higher 16 bits of first cluster is %d\n", directoryArray[i].DIR_FstClusHI);
        printf("The lower 16 bits of first cluster is %d\n", directoryArray[i].DIR_FstClusLO);

        int hour;
        int minute;
        int second;

        second = (directoryArray[i].DIR_WrtTime & 31)*2;
        minute = (directoryArray[i].DIR_WrtTime >> 5)& 63;
        hour = (directoryArray[i].DIR_WrtTime >> 11) & 31;

        printf("The last modified time is %d:%d:%d\n", hour,minute,second);


        int year;
        int month;
        int day;

        day = directoryArray[i].DIR_WrtDate & 31;
        month = directoryArray[i].DIR_WrtDate >> 5 & 63;
        year = (directoryArray[i].DIR_WrtDate >> 11 & 31) + 1980;

        printf("The last modified date is %d/%d/%d\n", year, month, day);

        printf("Size of the file is %d\n", directoryArray[i].DIR_FileSize);

        printf("==============Attribute Management============\n");

        int attribute = directoryArray[i].DIR_Attr;
       
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
            isRegularFile = true; // like .pdf
            printf("Type: regular file\n");
        }
        else if (directory == 0 && volumeName == 1){
            // represents name of the disk (volume label)
            isRegularFile = false;
            printf("Type: volume label\n");
        }
        else if (directory == 1 && volumeName == 0){
            // the entry represents the name of a directory, or folder in Windows
            isRegularFile = false;
            printf("Type: directory\n");
        }
        else {
            printf("Both directory and volume name are set to 1. There might be something wrong\n");
        }

        //If all lower four bits, 0...3, are set, and bits .4 and 5 are both zero, the directory entry should be 
        // ignored for now.
        if (attribute == 15){
            isIgnored = true;
        }

        namePrinter(directoryArray[i], isRegularFile, isIgnored);

    }
    
    close(fileDescriptor);

    return 0;
    }