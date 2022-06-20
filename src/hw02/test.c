#include "hw1.h"
#include "hw2.h"
#include "disk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCKS 512
#define DATA_REGION_FIRST (INODELIST_BLOCK_FIRST+INODELIST_BLOCKS)

int CreateFileSystemTest(void);
void MakeDirectoryTest(void);
void OpenFileTest();
void WriteFileTest();

int main(void){
  printf("Test start\n");
  CreateFileSystem();
  //CreateFileSystemTest();
  //MakeDirectoryTest();
  //OpenFileTest();
  WriteFileTest();

  return 0;
}

int CreateFileSystemTest(void){
  printf("-------------------------------------------------------");
  printf("CreateFilSystemTest\n");
  int rootInodeNum;
  int rootBlockNum;

/******************** FileSysInfo Check ********************/
  printf("%-20s\t", "FileSysInfo 초기화 테스트 ....");

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

  printf("%10s\n", "OK");

  printf("%-20s\t", "FileSysInfo 수정 테스트 ....");

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

  printf("%10s\n", "OK");

/******************** Directory block Check ********************/
  printf("%-20s\t","Check inode, data block ....");
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

  printf("%10s\n", "OK");
}

void MakeDirectoryTest(void){
  printf("-------------------------------------------------------");
  printf("MakeDirectoryTest\n");

  /************************* case1. 디렉토리 최초 생성 *************************/
  printf("%-30s\t", "디렉토리를 최초로 생성 ...");
  // case1-1. depth=1
  //printf("%-50s", "MakeDirectory(/temp1)");
  if(MakeDirectory("/temp1")<0){
    printf("%10s\n", "failed");
    return;
  }
  //printf("%-50s", "MakeDirectory(/temp1A)");
  if(MakeDirectory("/temp1A")<0){
    printf("%10s\n", "failed");
    return;
  }
  // case1-2. depth=2
  //printf("%-50s", "MakeDirectory(/temp1/temp2)");
  if(MakeDirectory("/temp1/temp2")<0){
    printf("%10s\n", "failed");
    return;
  }
  //printf("%-50s", "MakeDirectory(/temp1/temp2A)");
  if(MakeDirectory("/temp1/temp2A")<0){
    printf("%10s\n", "failed");
    return;
  }
  // case1-3. depth=3
  //printf("%-50s", "MakeDirectory(/temp1/temp2/temp3)");
  if(MakeDirectory("/temp1/temp2/temp3")<0){
    printf("%10s\n", "failed");
    return; 
  }
  // case1-4. depth=4
  //printf("%-50s", "MakeDirectory(/temp1/temp2/temp3/temp4A)");
  if(MakeDirectory("/temp1/temp2/temp3/temp4A")<0){
    printf("%10s\n", "failed");
    return;
  }
  // case1-5. depth=5
  //printf("%-50s", "MakeDirectory(/temp1/temp2/temp3/temp4A/temp5)");
  if(MakeDirectory("/temp1/temp2/temp3/temp4A/temp5")<0){
    printf("%10s\n", "failed");
    return;
  }

  printf("%10s\n", "OK");
  /********************** case2. 동일한 디렉토리 생성 시도 **********************/
  printf("%-30s\t", "이미 존재하는 디렉토리 생성 ...");
  // case2-1. depth=1
  if(!(MakeDirectory("/temp1")<0)){
    printf("%10s\n", "failed");
    return;
  }
  if(!(MakeDirectory("/temp1A")<0)){
    printf("%10s\n", "failed");
    return;
  }
  // case2-2. depth=2
  if(!(MakeDirectory("/temp1/temp2")<0)){
    printf("%10s\n", "failed");
    return;
  }
  if(!(MakeDirectory("/temp1/temp2A")<0)){
    printf("%10s\n", "failed");
    return;
  }
  // case2-3. depth=3
  if(!(MakeDirectory("/temp1/temp2/temp3")<0)){
    printf("%10s\n", "failed");
    return; 
  }
  // case2-4. depth=4
  if(!(MakeDirectory("/temp1/temp2/temp3/temp4A")<0)){
    printf("%10s\n", "failed");
    return;
  }
  /**
  // case2-5. depth=5
  if(!(MakeDirectory("/temp1/temp2/temp3/temp4/temp5")<0)){
    printf("%10s\n", "failed");
    return;
  }
  **/

  printf("%10s\n", "OK");
  /************************* case3. direct block을 추가하는 경우  ***********************/
  printf("%-30s\t", "direct block을 추가하는 경우 ...");

  //MakeDirectory("/temp1");
  //MakeDirectory("/temp2");
  MakeDirectory("/temp3");
  MakeDirectory("/temp4");
  MakeDirectory("/temp5");
  MakeDirectory("/temp6");
  MakeDirectory("/temp7");
  MakeDirectory("/temp8");
  MakeDirectory("/temp9");
  MakeDirectory("/temp10");
  MakeDirectory("/temp11");
  MakeDirectory("/temp12");
  MakeDirectory("/temp13");
  MakeDirectory("/temp14");

  if(MakeDirectory("/temp15")<0)
    printf("%10s\n", "failed");
  if(MakeDirectory("/temp16")<0)
    printf("%10s\n", "failed");
  if(MakeDirectory("/temp17")<0)
    printf("%10s\n", "failed");
  
  printf("%10s\n", "OK");
}

void OpenFileTest(){
  printf("-------------------------------------------------------");
  printf("OpenFileTest\n");

  MakeDirectory("/temp1");
  MakeDirectory("/temp1/temp2");

  int fd1;
  int fd2;
  int fd3;
  printf("%-50s\t", "OpenFile(/temp1/a.c, OPEN_FLAG_CREATE) ...");
  if((fd1=OpenFile("/temp1/a.c", OPEN_FLAG_CREATE))<0){
    printf("/temp1/a.c 실패\n");
    return;
  }
  printf("%10s\n", "OK");
  printf("%-50s\t", "OpenFile(/temp1/temp2/a.c, OPEN_FLAG_CREATE) ...");
  if((fd2=OpenFile("temp1/temp2/a.c", OPEN_FLAG_CREATE))<0){
    printf("/temp1/temp2/a.c 실패\n");
    return;
  }
  printf("%10s\n", "OK");
  printf("%-50s\t", "OpenFile(/temp1/temp2/a.c, OPEN_FLAG_CREATE) ...");
  if((fd3=OpenFile("temp1/temp2/a.c", OPEN_FLAG_CREATE))<0){
    printf("/temp1/temp2/a.c 실패\n");
    return;
  }
  else if(fd2==fd3){
    printf("/temp1/temp2/a.c 실패\n");
    return;
  }
  printf("%10s\n", "OK");

}


void WriteFileTest(){
  printf("-------------------------------------------------------");
  printf("WriteFileTest\n");
  MakeDirectory("/temp1");
  MakeDirectory("/temp1/temp2");

  int fd1;
  int fd2;
  int fd3;
  char* pBuff=calloc(BLOCK_SIZE, sizeof(char));
  strcpy(pBuff, "hello wrold");
  fd1=OpenFile("/temp1/a.c", OPEN_FLAG_CREATE);

  printf("%-50s\t", "WriteFile(/temp1/a.c, \"helo world\", 20) ...");
  if(WriteFile(fd1, pBuff, 20)<0){
    printf("failed\n");
    return;
  }
    
  printf("%10s\n", "OK");

}