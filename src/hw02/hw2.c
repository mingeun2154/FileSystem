#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk.h"
#include "hw1.h"
#include "hw2.h"

/********************** 추가적인 변수&함수 **********************/
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
  char* token;
  token=strtok(name, "/");
  while(token){
    printf("%s\n", token);
    token=strtok(NULL, "/");
  }

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
  DevWriteBlock(freeBlockNum, (char*)pDirBlock); // 빈 block(block7)에 root directory가 저장된다.
  free(pDirBlock);

/************************************* inode 생성 *************************************/
  Inode* pInode=malloc(sizeof(Inode));
  GetInode(freeInodeNum, pInode);
  pInode->allocBlocks=1;
  pInode->size = (pInode->allocBlocks * BLOCK_SIZE);
  pInode->type=FILE_TYPE_DIR;
  pInode->dirBlockPtr[0]=freeBlockNum;
  pInode->indirectBlockPtr=INVALID_ENTRY;
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
  char* string = "/temp1/temp2/temp3";
  // Extract the first token
  char * token = strtok(string, "/");
  while(token!=NULL){
    printf("%s\n", token); //printing the token
    token=strtok(NULL, "/");
  }
}