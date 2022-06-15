#include "hw1.h"
#include "hw2.h"
#include "disk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCKS 512
#define DATA_REGION_FIRST (INODELIST_BLOCK_FIRST+INODELIST_BLOCKS)

int CreateFileSystemTest(void);

int main(void){
  printf("Test start\n");
  CreateFileSystemTest();
  return 0;
}

int CreateFileSystemTest(void){
  printf("CreateFilSystemTest\n");
  int rootInodeNum;
  int rootBlockNum;

  CreateFileSystem();

/******************** FileSysInfo Check ********************/
  printf("FileSysInfo 초기화 테스트....\t");

  FileSysInfo* pBuff=malloc(BLOCK_SIZE);
  DevReadBlock(FILESYS_INFO_BLOCK, (char*)pBuff);
  rootInodeNum = pBuff->rootInodeNum;

  if(rootInodeNum!=0){
    printf("Fail : rootInodeNum=%d\n", rootInodeNum);
    return -1;
  }
  if(pBuff->blocks!=BLOCKS){
    printf("Fail : blocks=%d\n", pBuff->blocks);
    return -1;
  }

  printf("OK\n");

  printf("FileSysInfo 수정 테스트....\t");

  if(pBuff->numAllocBlocks!=1){
    printf("Fail : numAllocBlocks=%d\n", pBuff->numAllocBlocks);
    return -1;
  }
  if(pBuff->numFreeBlocks!=(BLOCKS-DATA_REGION_FIRST-1)){
    printf("Fail : numFreeBlocks=%d\n", pBuff->numFreeBlocks);
    return -1;
  }
  if(pBuff->numAllocInodes!=1){
    printf("Fail : numAllocInodes=%d\n", pBuff->numAllocInodes);
    return -1;
  }

  printf("OK\n");

/******************** Directory block Check ********************/
  printf("inode, data block 검사....\t");
  Inode* pInode=malloc(sizeof(Inode));
  GetInode(rootInodeNum, pInode);

  if(pInode->type!=FILE_TYPE_DIR){
    printf("Fail : type=%d\n", pInode->type);
    return -1;
  }

  rootBlockNum=pInode->dirBlockPtr[0];
  DirEntry* pDirBlock = malloc(BLOCK_SIZE);
  DevReadBlock(rootBlockNum, (char*)pDirBlock);

  if(strcmp(pDirBlock[0].name, ".")){
    printf("Fail : name=%s\n", pDirBlock[0].name);
    return -1;
  }

  free(pBuff);
  free(pInode);
  free(pDirBlock);

  printf("OK\n");

  printf("Test successed\n");
}