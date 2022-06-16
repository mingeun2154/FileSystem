#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk.h"
#include "hw1.h"
#include "hw2.h"

/********************** 추가적인 변수&함수 **********************/
#define DIRECT_BLOCK_NUM 4

const char* CURRENT_DIRCTORY_NAME = ".";
const char* PARENT_DIRECTORY_NAME = "..";
const char SLASH_CHAR = '/';
const char* SLASH_STRING = "/";

int CountSLASH(char* name);
void GetFileNames(char* name, int count, char** nameList);
/************************** 2018203023 **************************/


FileDescTable* pFileDescTable;
FileSysInfo* pFileSysInfo;

int OpenFile(const char* name, OpenFlag flag)
{

}


int WriteFile(int fileDesc, char* pBuffer, int length)
{

}


int ReadFile(int fileDesc, char* pBuffer, int length)
{

}

int CloseFile(int fileDesc)
{

}

int RemoveFile(char* name)
{

}


// return 0 on success, -1 on failure.
int MakeDirectory(char* name)
{
  FileSysInfo* fileSysInfo=malloc(BLOCK_SIZE);
  DevReadBlock(FILESYS_INFO_BLOCK, (char*)fileSysInfo);

  if(fileSysInfo->numFreeBlocks==0)
    return -1;

  /** 1. free block, free inode 검색 **/
  int freeBlock=GetFreeBlockNum();
  int freeInode=GetFreeInodeNum();

  /** 2. 만드려는 파일의 상위 디렉토리의 inode number 얻는다. 
   *     그리고 parent directory의 빈 directory entry 위치를 얻는다.**/
  int count=CountSLASH(name);// count = (절대경로에 포함된 이름의 개수) = (SLASH 개수)
  char** nameList=calloc(count, sizeof(char*)); // 절대경로의 이름들이 저장될 배열
  GetFileNames(name, count, nameList);

  int rootInodeNum=fileSysInfo->rootInodeNum;
  int parentInodeNum=rootInodeNum; // 최종적으로 생성할 파일의 parent directory의 inode number가 저장된다.
  Inode* pInode = malloc(sizeof(Inode));
  DirEntry* pEntry = malloc(sizeof(DirEntry)); // 새로 생성할 파일이 저장될 entry
  GetInode(parentInodeNum, pInode);
  int dirEntryNum=BLOCK_SIZE/sizeof(DirEntry); // 블럭 한개 당 directory entry 개수

  int freeBlockPtr; // 생성할 파일을 추가할 direct block의 index
  int freeEntryIdx; // block에서의 index
  if(count==1){ /** root directory에 만드는 경우 **/
    BOOL isFound=0;
    Inode* checkDup=malloc(sizeof(Inode)); // 이미 동일한 이름의 디렉토리가 있는지 확인하기 위한 inode.
    // 빈 directory entry를 찾는다.
    /** indirect block에서 빈자리를 찾는 경우 **/
    if((pInode->indirectBlockPtr)!=0){ 
      for(int j=0;j<dirEntryNum;j++){
        if(GetDirEntry(pInode->indirectBlockPtr, j, pEntry)<0){
          freeBlockPtr=pInode->indirectBlockPtr; 
          freeEntryIdx=j;
          isFound=1;
          break;
        }
        GetInode(pEntry->inodeNum, checkDup);
        if((checkDup->type==FILE_TYPE_DIR) && (strcmp(pEntry->name, nameList[1])==0)){
          printf("이미 존재하는 디렉토리입니다\n");
          return -1;
        }
      }
      if(isFound==0)
      printf("%s 용량 초과\n", pEntry->name);
      return -1;
    }
    /** direct block에서 빈자리를 찾는 경우  **/
    else{
      GetDirEntry(pInode->dirBlockPtr[0], 0, pEntry);
      for(int i=0;i<pInode->allocBlocks;i++){
        if(isFound!=0)
          break;
        for(int j=0;j<dirEntryNum;j++){
          if(GetDirEntry(pInode->dirBlockPtr[i], j, pEntry)<0){
            freeBlockPtr=i; 
            freeEntryIdx=j;
            isFound=1;
            break;
          }
        }
        GetInode(pEntry->inodeNum, checkDup);
        if((checkDup->type==FILE_TYPE_DIR) && (strcmp(pEntry->name, nameList[1])==0)){
          printf("이미 존재하는 디렉토리입니다\n");
          return -1;
        }
      }
      // printf("%d %d\n", freeBlockPtr, freeEntryIdx);
    }
    free(checkDup);
  }// count=1

  /** root directory가 아닌 directory에 새 파일을 생성하는 경우 **/
  else{
    /** 절대경로의 모든 이름 개수만큼 반복
    ex. "/temp1/temp2/temp3" : root->temp1->temp2 -> temp2의 inode획득 **/
    for(int i=0;i<=count-1;i++){
      BOOL flag = 1; // 원하는 파일을 찾으면 false(0)
      /** direct block, indirect block에서 이름이 nameList[i]인 파일을 찾는다 **/ 
      while(flag){
        GetInode(parentInodeNum, pInode);
        /** direct Block pointer(최대 4개)가 가리키는 블록을 탐색. **/
        for(int j=0;j<pInode->allocBlocks;j++){
          if(!flag)
            break;
          int dirEntryIndex=0;
          while(GetDirEntry(pInode->dirBlockPtr[j], dirEntryIndex, pEntry)){
            // block 하나에는 directory entry가 16개 존재한다.
            if(dirEntryIndex >= dirEntryNum)
              break;
            /** 경로상의 파일을 찾은 경우 **/
            if(strcmp(pEntry->name, nameList[i])==0){
              flag=0; 
              parentInodeNum=pEntry->inodeNum; 
              break;
            }
            dirEntryIndex++;
          }
        }
        /** inderct block 탐색 **/
        if(flag==1){
          if(pInode->indirectBlockPtr==0){ // direct block에 없고 indirect block이 없다 -> 존재하지 않는 경로
            return -1;
          }
          // indirect block
          int dirEntryIndex=0;
          while(GetDirEntry(pInode->indirectBlockPtr, dirEntryIndex, pEntry)){
            if(dirEntryIndex>=dirEntryNum){
              return -1; // 특정 이름이 존재하지 않는다. -> 존재하지 않는 경로
            }
            /** 경로상의 파일을 찾음 **/ 
            if(strcmp(pEntry->name, nameList[i])==0){
              flag=0;
              parentInodeNum=pEntry->inodeNum;
              break;
            }
          }
        }
      }
    }
  }// count>1
  
  printf("%s", pEntry->name);
  printf("%d\n", pEntry->inodeNum);

  free(fileSysInfo);
  free(nameList);
  free(pInode);
  free(pEntry);

  return 0;
}

int RemoveDirectory(char* name)
{

}

// disk를 포맷하고 root directory를 생성.
void CreateFileSystem(void)
{
  int freeBlockNum;
  int freeInodeNum;
  
  FileSysInit(); // 디스크 초기화
  freeBlockNum=GetFreeBlockNum(); // root directory block이 저장될 data region block number.
  freeInodeNum=GetFreeInodeNum(); // root directory의 inode number.

/************************************* data region 수정 *************************************/
  DirEntry* pDirBlock = malloc(BLOCK_SIZE); // directory block는 directory entry의 배열이다
  /** 
   * Directory block을 disk에 저장.
   * **/
  strcpy(pDirBlock[0].name, CURRENT_DIRCTORY_NAME);
  pDirBlock[0].inodeNum=freeInodeNum; // inode number 저장.
  for(int i=1;i<BLOCK_SIZE/sizeof(DirEntry);i++)
    pDirBlock[i].inodeNum=INVALID_ENTRY;
  DevWriteBlock(freeBlockNum, (char*)pDirBlock); // 빈 block(block7)에 root directory가 저장된다.
  free(pDirBlock);

/************************************* inode 생성 *************************************/
  Inode* pInode=malloc(sizeof(Inode));
  GetInode(freeInodeNum, pInode);
  pInode->allocBlocks=1;
  pInode->size = (pInode->allocBlocks * BLOCK_SIZE);
  pInode->type=FILE_TYPE_DIR;
  pInode->dirBlockPtr[0]=freeBlockNum;
  //pInode->indirectBlockPtr=INVALID_ENTRY;
  PutInode(freeInodeNum, pInode);
  free(pInode);

/*************************************** FileSysInfo 초기화 ***************************************/
  FileSysInfo* pFSIBlock = malloc(BLOCK_SIZE);
  int numberOfBlocks=FS_DISK_CAPACITY/BLOCK_SIZE; // 디스크를 구성하는 block 총 개수 (512)
  pFSIBlock->blocks=numberOfBlocks; 
  pFSIBlock->rootInodeNum=freeInodeNum;
  pFSIBlock->rootInodeNum=freeInodeNum;
  pFSIBlock->diskCapacity=FS_DISK_CAPACITY;
  pFSIBlock->numAllocBlocks=0;
  pFSIBlock->numFreeBlocks=numberOfBlocks-(INODELIST_BLOCK_FIRST+INODELIST_BLOCKS); // 512-7
  pFSIBlock->numAllocInodes=0;
  pFSIBlock->blockBytemapBlock=BLOCK_BYTEMAP_BLOCK_NUM;
  pFSIBlock->inodeBytemapBlock=INODE_BYTEMAP_BLOCK_NUM;
  pFSIBlock->inodeListBlock=INODELIST_BLOCK_FIRST; // 3
  pFSIBlock->dataRegionBlock=INODELIST_BLOCK_FIRST+INODELIST_BLOCKS; // 3+4

/****************************************** metadata 수정 ******************************************/
  /** FileSysInfo 수정 (root directory 추가) **/
  pFSIBlock->numAllocBlocks++;
  pFSIBlock->numFreeBlocks--;
  pFSIBlock->numAllocInodes++;
  DevWriteBlock(FILESYS_INFO_BLOCK, (char*)pFSIBlock);
  free(pFSIBlock);

  /** bytemap 수정**/
  SetBlockBytemap(freeBlockNum);
  SetInodeBytemap(freeInodeNum);
}

void OpenFileSystem(void)
{

}

void CloseFileSystem(void)
{

}

Directory* OpenDirectory(char* name)
{

}


FileInfo* ReadDirectory(Directory* pDir)
{

}

int CloseDirectory(Directory* pDir)
{

}

/****************** 추가적으로 구현한 함수입니다 ******************/
// 절대경로에서 SLASH(/)의 개수를 반환한다.
int CountSLASH(char *name){
  int count =0;
  int i=0;
  while(name[i]!='\0'){
    if(name[i]==SLASH_CHAR)
      count++;
    i++;
  }

  return count;
}

// 절대경로에 포함된 모든 디렉토리(파일) 이름을 전달받은 nameList에 담는다.
void GetFileNames(char* name, int count, char** nameList){
  // count = (절대경로에 포함된 이름의 개수) = (SLASH 개수)
  char* path = calloc(strlen(name), sizeof(char)); // name을 쓰기가 가능한 메모리 공간에 담는다
  strcpy(path, name);
  char * token = strtok(path, SLASH_STRING);
  int i=0;
  while(token!=NULL){
    nameList[i++]=token;
    token=strtok(NULL, "/");
  }
}