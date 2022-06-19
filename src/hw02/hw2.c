#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "disk.h"
#include "hw1.h"
#include "hw2.h"

/********************** 추가적인 변수&함수 **********************/
#define DIR_ENTRY_NUM (BLOCK_SIZE/sizeof(DirEntry))
#define CURRENT_DIR_ENTRY_IDX 0
#define PARENT_DIR_ENTRY_IDX 1
#define ROOT_INODE_NUM 0

const char* CURRENT_DIRCTORY_NAME = ".";
const char* PARENT_DIRECTORY_NAME = "..";
const char SLASH_CHAR = '/';
const char* SLASH_STRING = "/";
const int NUM_OF_INDIRECT_BLOCK_PTR=BLOCK_SIZE/(sizeof(int)); // 128개
const int MAX_ALLOC_BLOCKS =NUM_OF_DIRECT_BLOCK_PTR + NUM_OF_INDIRECT_BLOCK_PTR; // 132
const int DESC_TABLE_SIZE=sizeof(int)+((sizeof(int)+sizeof(int))*DESC_ENTRY_NUM); //628

int CountSLASH(char* name);
void GetFileNames(char* name, int count, char** nameList);
int GetFreeDescTableEntryIdx(FileDescTable* pFileDescTable);
int GetFreeFileTableEntryIdx(FileTable* pFileTable);
/************************** 2018203023 **************************/

FileDescTable* pFileDescTable;
FileTable* pFileTable;
FileSysInfo* pFileSysInfo;


int OpenFile(const char* name, OpenFlag flag)
{
  /** 1.free inode 검색 **/
  int freeInodeNum=GetFreeInodeNum();
  if(freeInodeNum<0)
    return -1; // free inode 없음
  
  /** 2. parent directory 탐색 **/
  BOOL isExist=0; // 이미 존재하는 파일이면 1, 없으면 0

  int count=CountSLASH(name);
  char** nameList=calloc(count, sizeof(char*));
  GetFileNames(name, count, nameList);
  char* openFileName=nameList[count-1];
  BOOL isFound;

  int parentInodeNum=ROOT_INODE_NUM;
  int parentBlockNum; // file이 생성될(또는 이미 존재하는) directory block
  int parentFreeEntryIdx;
  Inode* parentInode=malloc(sizeof(Inode));
  DirEntry* pEntry=malloc(sizeof(DirEntry));
  GetInode(parentInodeNum, parentInode);

  /** 2. new directory entry가 추가될 parent directory의 inode를 찾는다 **/
  // 경로에 있는 이름들을 탐색
  isFound=0;
  for(int i=0;i<count-1;i++){
    isFound=0;
    // direct block 탐색
    for(int blockPtrIdx=0;blockPtrIdx<NUM_OF_DIRECT_BLOCK_PTR;blockPtrIdx++){
      if(isFound==1)
        break;
      for(int entryIdx=0;entryIdx<DIR_ENTRY_NUM;entryIdx++){
        if(isFound==1)
          break;
        if(GetDirEntry(parentInode->dirBlockPtr[blockPtrIdx], entryIdx, pEntry)<0){ // invalid entry
          //printf("%d,%d:(%s, %s)", parentInode->dirBlockPtr[blockPtrIdx], entryIdx, pEntry->name, nameList[i]);
          continue;
        }
        else{
          if(strcmp(pEntry->name, nameList[i])==0){
            isFound=1;
            //printf("%d,%d:(%s, %s)", parentInode->dirBlockPtr[blockPtrIdx], entryIdx, pEntry->name, nameList[i]); // nameList[i]를 찾은 위치
            parentInodeNum=pEntry->inodeNum;
            GetInode(parentInodeNum, parentInode); // parent Inode 업데이트
            break;
          }
          else{
            //printf("strcmp:%d (%s,%s) ", strcmp(pEntry->name, nameList[i]), pEntry->name, nameList[i]);
            continue;
          }
        }
      }
    }
    // indirect block 탐색
    if(isFound==0 && parentInode->indirectBlockPtr!=0){
      char* indirectBlockPtrs=malloc(BLOCK_SIZE);
      DevReadBlock(parentInode->indirectBlockPtr, indirectBlockPtrs);
      int blockPtr; // block pointer = block number
      for(int i=0;i<NUM_OF_INDIRECT_BLOCK_PTR;i++){
        blockPtr=indirectBlockPtrs[i];
        if(isFound==1)
          break;
        for(int entryIdx=0;entryIdx<DIR_ENTRY_NUM;entryIdx++){
          if(GetDirEntry(blockPtr, entryIdx, pEntry)<0) // invalid entry
            continue;
          else{
            if(strcmp(pEntry->name, nameList[i])==0){
              isFound=1;
              parentInodeNum=pEntry->inodeNum;
              GetInode(parentInodeNum, parentInode); // parent Inode 업데이트
              break;
            }
          }
        }
      }
      free(indirectBlockPtrs);
    }

    if(isFound==0){ // 유효하지 않은 경로(중간에 존재하지 않는 directory가 있다)
      printf("%-6s", "#139");
      return -1;
    }
  }
  /** 3. parent directory에서 free entry를 찾는다 **/
  isFound=0;
  // parent directory에 더이상 공간이 없는 경우
  if(parentInode->allocBlocks==MAX_ALLOC_BLOCKS){
    printf("%-6s", "#147");
    return -1;
  }
  /** 3-1. 이미 존재하는 파일인지 확인 **/
  // direct block 에서 중복 확인
  for(int blockPtrIdx=0;blockPtrIdx<NUM_OF_DIRECT_BLOCK_PTR;blockPtrIdx++){
    if(isExist==1)
      break;
    if(parentInode->dirBlockPtr[blockPtrIdx]==0){
      continue;
    }
    for(int entryIdx=0;entryIdx<DIR_ENTRY_NUM;entryIdx++){
      if(GetDirEntry(parentInode->dirBlockPtr[blockPtrIdx], entryIdx, pEntry)<0){
        continue;
      }
      else{
        Inode* pInode = malloc(sizeof(Inode));
        GetInode(pEntry->inodeNum, pInode);
        if((strcmp(pEntry->name, openFileName)==0)&&((pInode->type)==FILE_TYPE_FILE)){
          isExist=1;
          isFound=1;
          break;
        }
        free(pInode);
      }
    }
  }
  // indirect block이 존재하는 경우, indirect block 에서 중복 확인
  if((isExist==0)&&(parentInode->indirectBlockPtr!=0)){
    char* indirectBlockPtrs=malloc(BLOCK_SIZE);
    DevReadBlock(parentInode->indirectBlockPtr, indirectBlockPtrs);
    int blockPtr; // block pointer = block number
    for(int i=0;i<NUM_OF_INDIRECT_BLOCK_PTR;i++){
      if(isExist==1)
        break;
      blockPtr=indirectBlockPtrs[i];
      for(int entryIdx=0;entryIdx<DIR_ENTRY_NUM;entryIdx++){
        if(GetDirEntry(blockPtr, entryIdx, pEntry)<0){ // invalid entry
          continue;
        }
        else{
          Inode* pInode=malloc(sizeof(Inode));
          GetInode(pEntry->inodeNum, pInode);
          if((strcmp(pEntry->name, openFileName)==0) && (pInode->type==FILE_TYPE_FILE)){
            isExist=1;
            isFound=1;
            break;
          }
          free(pInode);
        }
      }
    }
    free(indirectBlockPtrs);
  }
  // 3-2. direct block에서 free entry를 찾는다.
  if(isFound==0){
    for(int blockPtrIdx=0;blockPtrIdx<NUM_OF_DIRECT_BLOCK_PTR;blockPtrIdx++){
      if(isFound==1)
        break;
      // direct block을 추가적으로 할당해야 하는 경우.
      if(parentInode->dirBlockPtr[blockPtrIdx]==0){
        isFound=1;
        parentBlockNum=GetFreeBlockNum();
        SetBlockBytemap(parentBlockNum);
        parentInode->dirBlockPtr[blockPtrIdx]=parentBlockNum;
        // 새로 할당한 direct block의 모든 entry를 invalid로 초기화한다.
        DirEntry* newBlockEntry=malloc(BLOCK_SIZE);
        for(int i=0;i<DIR_ENTRY_NUM;i++)
          newBlockEntry[i].inodeNum=INVALID_ENTRY;
        DevWriteBlock(parentBlockNum, (char*)newBlockEntry);
        free(newBlockEntry);
        parentFreeEntryIdx=0;
        break;
      }
      for(int entryIdx=0;entryIdx<DIR_ENTRY_NUM;entryIdx++){
        if(GetDirEntry(parentInode->dirBlockPtr[blockPtrIdx], entryIdx, pEntry)<0){
          isFound=1;
          parentBlockNum=parentInode->dirBlockPtr[blockPtrIdx];
          parentFreeEntryIdx=entryIdx;
          break;
        }
        else
          continue;
      }
    }
  }
  // 3-3. indirect block에서 free entry를 찾는다.
  // 3-3-1. indirect block이 아직 할당 안 되어 있었을 경우
  if(isFound==0){
    if(parentInode->indirectBlockPtr==0){
      isFound=1;
      int indirectBlockNum = GetFreeBlockNum(); /** indirect block number, 새로운 directory block을 가리키는 포인터들이다. **/
      SetBlockBytemap(indirectBlockNum); 
      parentInode->indirectBlockPtr=indirectBlockNum; 
      // 새로 할당한 directory block의 모든 entry를 invalid로 초기화한다.
      DirEntry* newBlockEntry=malloc(BLOCK_SIZE);
      for(int i=0;i<DIR_ENTRY_NUM;i++)
        newBlockEntry[i].inodeNum=INVALID_ENTRY;
      DevWriteBlock(parentBlockNum, (char*)newBlockEntry);
      int* indirectBlockEntry=calloc(NUM_OF_INDIRECT_BLOCK_PTR, sizeof(int*));
      indirectBlockEntry[0]=parentBlockNum;
      DevWriteBlock(indirectBlockNum, (char*)indirectBlockEntry);

      free(indirectBlockEntry);
      free(newBlockEntry);
      parentFreeEntryIdx=0;
    }
  }
  // 3-3-2. indirect block에서 free entry 탐색.
  if(isFound==0){
    char* indirectBlockPtrs=malloc(BLOCK_SIZE);
    DevReadBlock(parentInode->indirectBlockPtr, indirectBlockPtrs);
    int blockPtr; // block pointer = block number
    for(int i=0;i<NUM_OF_INDIRECT_BLOCK_PTR;i++){
      blockPtr=indirectBlockPtrs[i];
      if(isFound==1)
        break;
      for(int entryIdx=0;entryIdx<DIR_ENTRY_NUM;entryIdx++){
        if(GetDirEntry(blockPtr, entryIdx, pEntry)<0){ // invalid entry
          isFound=1;
          parentInodeNum=pEntry->inodeNum;
          GetInode(parentInodeNum, parentInode); // parent Inode 업데이트
          break;
        }
        else
          continue;
      }
    }
    free(indirectBlockPtrs);
  }

  // 새 파일이 추가될 parent block number와 parent block entry 출력
  // printf("(%d,%d)\n", parentBlockNum, parentFreeEntryIdx);

  // printf("파일 생성\t");
  /** 4-1. 존재하지 않는 file 생성. parent directory에 open file을 entry로 추가 **/
  if((isExist==0)&&(flag==OPEN_FLAG_CREATE)){
    DirEntry* parentDirBlock=malloc(BLOCK_SIZE);
    DevReadBlock(parentBlockNum, (char*)parentDirBlock);
    parentDirBlock[parentFreeEntryIdx].inodeNum=freeInodeNum;
    strcpy(parentDirBlock[parentFreeEntryIdx].name, openFileName);
    DevWriteBlock(parentBlockNum, (char*)parentDirBlock);
    free(parentDirBlock);

    /** 5. inode 설정 **/
    Inode* pInode=malloc(sizeof(Inode));
    GetInode(freeInodeNum, pInode);
    pInode->type=FILE_TYPE_FILE;
    // allocblocks, dirBlockPtr, indirectBlockPtr, size는 모두 0이므로 수정할 필요 없음.
    PutInode(freeInodeNum, pInode);
    SetInodeBytemap(freeInodeNum);
    free(pInode);

    /** 6. update FileSystemInfo **/
    pFileSysInfo=malloc(BLOCK_SIZE);
    DevReadBlock(FILESYS_INFO_BLOCK, (char*)pFileSysInfo);
    pFileSysInfo->numAllocInodes++;
    DevWriteBlock(FILESYS_INFO_BLOCK, (char*)pFileSysInfo);
    free(pFileSysInfo);

    /** 7. descriptor table, file table, file object 설정 **/
    // descriptor table이 없었다면 생성.
    if(pFileDescTable==NULL)
      pFileDescTable=calloc(sizeof(FileDescTable), sizeof(char));
    // file table이 없었다면 생성. 
    if(pFileTable==NULL)
      pFileTable=calloc(sizeof(FileTable), sizeof(char));

    int freeFDTableEtnry=GetFreeDescTableEntryIdx(pFileDescTable);
    int freeFileTableEntry=GetFreeFileTableEntryIdx(pFileTable);
    // printf("%d, %d\n", freeFDTableEtnry, freeFileTableEntry);

    // descriptor table
    pFileDescTable->numUsedDescEntry++;
    pFileDescTable->pEntry[freeFDTableEtnry].bUsed=1;
    pFileDescTable->pEntry[freeFDTableEtnry].fileTableIndex=freeFileTableEntry;
    // file table
    pFileTable->numUsedFile++;
    pFileTable->pFile[freeFileTableEntry].bUsed=1;
    pFileTable->pFile[freeFileTableEntry].inodeNum=freeInodeNum;
    pFileTable->pFile[freeFileTableEntry].fileOffset=0;

    free(parentInode);
    free(pEntry);
    return freeFDTableEtnry; // file descriptor number 반환.
  }
  /** 4-2. 이미 존재하는 파일 open **/
  else if((isExist==1)&&(flag==OPEN_FLAG_CREATE)){
    //printf("이미 존재하는 파일%d 오픈\t", pEntry->inodeNum);
    /** 7. descriptor table, file table, file object 설정 **/
    // descriptor table이 없었다면 생성.
    if(pFileDescTable==NULL)
      pFileDescTable=calloc(sizeof(FileDescTable), sizeof(char));
    // file table이 없었다면 생성. 
    if(pFileTable==NULL)
      pFileTable=calloc(sizeof(FileTable), sizeof(char));

    int freeFDTableEtnry=GetFreeDescTableEntryIdx(pFileDescTable);
    int freeFileTableEntry=GetFreeFileTableEntryIdx(pFileTable);
    // printf("%d, %d\n", freeFDTableEtnry, freeFileTableEntry);

    // descriptor table
    pFileDescTable->numUsedDescEntry++;
    pFileDescTable->pEntry[freeFDTableEtnry].bUsed=1;
    pFileDescTable->pEntry[freeFDTableEtnry].fileTableIndex=freeFileTableEntry;
    // file table
    pFileTable->numUsedFile++;
    pFileTable->pFile[freeFileTableEntry].bUsed=1;
    pFileTable->pFile[freeFileTableEntry].inodeNum=pEntry->inodeNum;
    pFileTable->pFile[freeFileTableEntry].fileOffset=0;

    free(parentInode);
    free(pEntry);
    return freeFDTableEtnry; // file descriptor number 반환.
  }

  /** 4-3. OPEN_FLAG_TRUNCATE **/
  else if((isExist==1)&&(flag==OPEN_FLAG_TRUNCATE)){
    //printf("OPEN_FLAG_TRUNCATE %d\t", pEntry->inodeNum);
    /** 5. descriptor table, file table, file object 설정 **/
    // descriptor table이 없었다면 생성.
    if(pFileDescTable==NULL)
      pFileDescTable=calloc(sizeof(FileDescTable), sizeof(char));
    // file table이 없었다면 생성. 
    if(pFileTable==NULL)
      pFileTable=calloc(sizeof(FileTable), sizeof(char));

    int freeFDTableEtnry=GetFreeDescTableEntryIdx(pFileDescTable);
    int freeFileTableEntry=GetFreeFileTableEntryIdx(pFileTable);
    // printf("%d, %d\n", freeFDTableEtnry, freeFileTableEntry);

    // open file의 크기를 0으로 만들기 위해 해당 파일의 inode를 획득한다.
    Inode* pInode = malloc(sizeof(Inode));
    GetInode(pEntry->inodeNum, pInode);

    /** 6. file이 가지고 있는 data를 모두 지운다. **/
    int blockCount=0; // file에 할당된 data block개수.
    // direct block의 data를 지운다.
    for(int i=0;i<NUM_OF_DIRECT_BLOCK_PTR;i++){
      int directBlockNum=pInode->dirBlockPtr[i];
      if(directBlockNum==0)
        continue;
      else{
        blockCount++;
        ResetBlockBytemap(directBlockNum);
        // 내용을 0으로 덮어쓴다.
        char* pBuff=calloc(BLOCK_SIZE, sizeof(char));
        DevWriteBlock(directBlockNum, pBuff);
        pInode->dirBlockPtr[i]=0;
      }
    }
    // indirect block 이 가리키는 data block 을 모두 지운다
    if(pInode->indirectBlockPtr!=0){
      int indirectBlockNum=pInode->indirectBlockPtr;
      int* indirectBlock=malloc(BLOCK_SIZE);
      int dataBlockNum;
      // indirect block 획득
      DevReadBlock(indirectBlockNum, (char*)indirectBlock);
      for(int i=0;i<NUM_OF_INDIRECT_BLOCK_PTR;i++){
        dataBlockNum=indirectBlock[i];
        if(dataBlockNum!=0){
          blockCount++;
          ResetBlockBytemap(dataBlockNum);
          // indirect block entry가 가리키는 data block을 0으로 덮어쓴다.
          char* pBuff=calloc(BLOCK_SIZE, sizeof(char));
          DevWriteBlock(dataBlockNum, pBuff);
          free(pBuff);
        }
      }
      free(indirectBlock);
      pInode->indirectBlockPtr=0;
    }
    // Update inode.
    pInode->allocBlocks=0;
    pInode->size=0;
    PutInode(pEntry->inodeNum, pInode);

    // Update FilseSysInfo .
    DevReadBlock(FILESYS_INFO_BLOCK, (char*)pFileSysInfo);
    pFileSysInfo->numFreeBlocks+=blockCount;
    pFileSysInfo->numAllocBlocks-=blockCount;
    pFileSysInfo->numFreeBlocks+=blockCount;
    DevWriteBlock(FILESYS_INFO_BLOCK, (char*)pFileSysInfo);

    // descriptor table
    pFileDescTable->numUsedDescEntry++;
    pFileDescTable->pEntry[freeFDTableEtnry].bUsed=1;
    pFileDescTable->pEntry[freeFDTableEtnry].fileTableIndex=freeFileTableEntry;
    // file table
    pFileTable->numUsedFile++;
    pFileTable->pFile[freeFileTableEntry].bUsed=1;
    pFileTable->pFile[freeFileTableEntry].inodeNum=pEntry->inodeNum;
    pFileTable->pFile[freeFileTableEntry].fileOffset=pInode->size; /** offset=(file size) **/

    free(pInode);
    free(parentInode);
    free(pEntry);
    return freeFDTableEtnry;
  }
  else if((isExist==0)&&(flag==OPEN_FLAG_TRUNCATE)){
    free(parentInode);
    free(pEntry);
    return -1;
  }

  /** 4-4. OPEN_FLAG_APPEND **/
  else if((isExist==1)&&(flag==OPEN_FLAG_APPEND)){
    //printf("OPEN_FLAG_APPEND%d\t", pEntry->inodeNum);
    /** 7. descriptor table, file table, file object 설정 **/
    // descriptor table이 없었다면 생성.
    if(pFileDescTable==NULL)
      pFileDescTable=calloc(sizeof(FileDescTable), sizeof(char));
    // file table이 없었다면 생성. 
    if(pFileTable==NULL)
      pFileTable=calloc(sizeof(FileTable), sizeof(char));

    int freeFDTableEtnry=GetFreeDescTableEntryIdx(pFileDescTable);
    int freeFileTableEntry=GetFreeFileTableEntryIdx(pFileTable);
    // printf("%d, %d\n", freeFDTableEtnry, freeFileTableEntry);

    // open file의 크기를 알기 위해 해당 파일의 inode를 획득한다.
    Inode* pInode = malloc(sizeof(Inode));
    GetInode(pEntry->inodeNum, pInode);

    // descriptor table
    pFileDescTable->numUsedDescEntry++;
    pFileDescTable->pEntry[freeFDTableEtnry].bUsed=1;
    pFileDescTable->pEntry[freeFDTableEtnry].fileTableIndex=freeFileTableEntry;
    // file table
    pFileTable->numUsedFile++;
    pFileTable->pFile[freeFileTableEntry].bUsed=1;
    pFileTable->pFile[freeFileTableEntry].inodeNum=pEntry->inodeNum;
    pFileTable->pFile[freeFileTableEntry].fileOffset=pInode->size; /** offset=(file size) **/

    free(pInode);
    free(parentInode);
    free(pEntry);
    return freeFDTableEtnry;
  }
  else if((isExist==0)&&(flag==OPEN_FLAG_APPEND)){
    free(parentInode);
    free(pEntry);
    return -1;
  }

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
  /** 1. free block, free inode 검색 **/
  int newDirBlockNum=GetFreeBlockNum();
  int newDirInodeNum=GetFreeInodeNum();
  if((newDirBlockNum==-1) || (newDirInodeNum==-1)){ // inode, block 부족
    printf("%-6s", "#60"); 
    return -1;
  }

  pFileSysInfo=malloc(BLOCK_SIZE);
  DevReadBlock(FILESYS_INFO_BLOCK, (char*)pFileSysInfo);
	int parentInodeNum=ROOT_INODE_NUM; // 최종적으로 생성할 파일의 parent directory의 inode number가 저장된다.
	int parentBlockNum; // 생성할 파일을 추가할 direct block number
	int parentFreeEntryIdx; // block에서의 index

	int count=CountSLASH(name);	// count = (절대경로에 포함된 이름의 개수) = (SLASH 개수)
	char** nameList=calloc(count, sizeof(char*)); // 절대경로의 이름들이 저장될 배열
	GetFileNames(name, count, nameList);
  char* newDirName=nameList[count-1]; // new directory name

	Inode* parentInode = malloc(sizeof(Inode));
	DirEntry* pEntry = malloc(sizeof(DirEntry)); // 새로 생성할 파일이 저장될 entry
	GetInode(parentInodeNum, parentInode);

  int editionalDirectBlockPtr=0;
  int editionalIndirectBlockPtr=0;
  BOOL editionalDirectBlock=0; // direct block이 추가된 경우
  BOOL isFound;

  /** 2. new directory entry가 추가될 parent directory의 inode를 찾는다 **/
  // 경로에 있는 이름들을 탐색
  isFound=0;
  for(int i=0;i<count-1;i++){
    isFound=0;
    // direct block 탐색
    for(int blockPtrIdx=0;blockPtrIdx<NUM_OF_DIRECT_BLOCK_PTR;blockPtrIdx++){
      if(isFound==1)
        break;
      for(int entryIdx=0;entryIdx<DIR_ENTRY_NUM;entryIdx++){
        if(isFound==1)
          break;
        if(GetDirEntry(parentInode->dirBlockPtr[blockPtrIdx], entryIdx, pEntry)<0){ // invalid entry
          //printf("%d,%d:(%s, %s)", parentInode->dirBlockPtr[blockPtrIdx], entryIdx, pEntry->name, nameList[i]);
          continue;
        }
        else{
          if(strcmp(pEntry->name, nameList[i])==0){
            isFound=1;
            //printf("%d,%d:(%s, %s)", parentInode->dirBlockPtr[blockPtrIdx], entryIdx, pEntry->name, nameList[i]); // nameList[i]를 찾은 위치
            parentInodeNum=pEntry->inodeNum;
            GetInode(parentInodeNum, parentInode); // parent Inode 업데이트
            break;
          }
          else{
            //printf("strcmp:%d (%s,%s) ", strcmp(pEntry->name, nameList[i]), pEntry->name, nameList[i]);
            continue;
          }
        }
      }
    }
    // indirect block 탐색
    if(isFound==0 && parentInode->indirectBlockPtr!=0){
      char* indirectBlockPtrs=malloc(BLOCK_SIZE);
      DevReadBlock(parentInode->indirectBlockPtr, indirectBlockPtrs);
      int blockPtr; // block pointer = block number
      for(int i=0;i<NUM_OF_INDIRECT_BLOCK_PTR;i++){
        blockPtr=indirectBlockPtrs[i];
        if(isFound==1)
          break;
        for(int entryIdx=0;entryIdx<DIR_ENTRY_NUM;entryIdx++){
          if(GetDirEntry(blockPtr, entryIdx, pEntry)<0) // invalid entry
            continue;
          else{
            if(strcmp(pEntry->name, nameList[i])==0){
              isFound=1;
              parentInodeNum=pEntry->inodeNum;
              GetInode(parentInodeNum, parentInode); // parent Inode 업데이트
              break;
            }
          }
        }
      }
      free(indirectBlockPtrs);
    }

    if(isFound==0){ // 유효하지 않은 경로(중간에 존재하지 않는 directory가 있다)
      printf("%-6s", "#139");
      return -1;
    }
  }
  /** 3. parent directory에서 free entry를 찾는다 **/
  isFound=0;
  // parent directory에 더이상 공간이 없는 경우
  if(parentInode->allocBlocks==MAX_ALLOC_BLOCKS){
    printf("%-6s", "#147");
    return -1;
  }
  /** 3-1. 이미 존재하는 디렉토리인지 확인 **/
  // direct block 에서 중복 확인
  for(int blockPtrIdx=0;blockPtrIdx<NUM_OF_DIRECT_BLOCK_PTR;blockPtrIdx++){
    if(parentInode->dirBlockPtr[blockPtrIdx]==0){
      continue;
    }
    for(int entryIdx=0;entryIdx<DIR_ENTRY_NUM;entryIdx++){
      if(GetDirEntry(parentInode->dirBlockPtr[blockPtrIdx], entryIdx, pEntry)<0){
        continue;
      }
      else{
        Inode* pInode = malloc(sizeof(Inode));
        GetInode(pEntry->inodeNum, pInode);
        //printf("%s\t", pEntry->name);
        if((strcmp(pEntry->name, newDirName)==0)&&((pInode->type)==FILE_TYPE_DIR)){
          //printf("%-6s", "#165");
          return -1;
        }
        free(pInode);
      }
    }
  }
  // indirect block이 존재하는 경우, 에서 중복 확인
  if(parentInode->indirectBlockPtr!=0){
    char* indirectBlockPtrs=malloc(BLOCK_SIZE);
    DevReadBlock(parentInode->indirectBlockPtr, indirectBlockPtrs);
    int blockPtr; // block pointer = block number
    for(int i=0;i<NUM_OF_INDIRECT_BLOCK_PTR;i++){
      blockPtr=indirectBlockPtrs[i];
      for(int entryIdx=0;entryIdx<DIR_ENTRY_NUM;entryIdx++){
        if(GetDirEntry(blockPtr, entryIdx, pEntry)<0){ // invalid entry
          continue;
        }
        else{
          Inode* pInode=malloc(sizeof(Inode));
          GetInode(pEntry->inodeNum, pInode);
          if((strcmp(pEntry->name, newDirName)==0) && (pInode->type==FILE_TYPE_DIR)){
            printf("%-6s", "#187");
            return -1;
          }
          free(pInode);
        }
      }
    }
    free(indirectBlockPtrs);
  }
  // 3-2. direct block에서 free entry를 찾는다.
  for(int blockPtrIdx=0;blockPtrIdx<NUM_OF_DIRECT_BLOCK_PTR;blockPtrIdx++){
    if(isFound==1)
      break;
    // direct block을 추가적으로 할당해야 하는 경우.
    if(parentInode->dirBlockPtr[blockPtrIdx]==0){
      isFound=1;
      pFileSysInfo->numAllocBlocks++;
      pFileSysInfo->numFreeBlocks--;
      SetBlockBytemap(newDirBlockNum);
      parentBlockNum=GetFreeBlockNum();
      parentInode->dirBlockPtr[blockPtrIdx]=parentBlockNum;
      // 새로 할당한 direct block의 모든 entry를 invalid로 초기화한다.
      DirEntry* newBlockEntry=malloc(BLOCK_SIZE);
      for(int i=0;i<DIR_ENTRY_NUM;i++)
        newBlockEntry[i].inodeNum=INVALID_ENTRY;
      DevWriteBlock(parentBlockNum, (char*)newBlockEntry);
      free(newBlockEntry);
      parentFreeEntryIdx=0;
      break;
    }
    for(int entryIdx=0;entryIdx<DIR_ENTRY_NUM;entryIdx++){
      if(GetDirEntry(parentInode->dirBlockPtr[blockPtrIdx], entryIdx, pEntry)<0){
        isFound=1;
        parentBlockNum=parentInode->dirBlockPtr[blockPtrIdx];
        parentFreeEntryIdx=entryIdx;
        break;
      }
      else
        continue;
    }
  }
  // 3-3. indirect block에서 free entry를 찾는다.
  // 3-3-1. indirect block이 아직 할당 안 되어 있었을 경우
  if(isFound==0){
    if(parentInode->indirectBlockPtr==0){
      isFound=1;
      pFileSysInfo->numAllocBlocks++;
      pFileSysInfo->numFreeBlocks--;
      SetBlockBytemap(newDirBlockNum);
      int indirectBlockNum = GetFreeBlockNum(); /** indirect block number, 새로운 directory block을 가리키는 포인터들이다. **/
      SetBlockBytemap(indirectBlockNum); 
      parentBlockNum=GetFreeBlockNum(); /** 새로운 directory block 여기에 entry가 저장된다 **/
      parentInode->indirectBlockPtr=indirectBlockNum; 
      // 새로 할당한 directory block의 모든 entry를 invalid로 초기화한다.
      DirEntry* newBlockEntry=malloc(BLOCK_SIZE);
      for(int i=0;i<DIR_ENTRY_NUM;i++)
        newBlockEntry[i].inodeNum=INVALID_ENTRY;
      DevWriteBlock(parentBlockNum, (char*)newBlockEntry);
      int* indirectBlockEntry=calloc(NUM_OF_INDIRECT_BLOCK_PTR, sizeof(int*));
      indirectBlockEntry[0]=parentBlockNum;
      DevWriteBlock(indirectBlockNum, (char*)indirectBlockEntry);

      free(indirectBlockEntry);
      free(newBlockEntry);
      parentFreeEntryIdx=0;
    }
  }
  // 3-3-2. indirect block에서 free entry 탐색.
  if(isFound==0){
    char* indirectBlockPtrs=malloc(BLOCK_SIZE);
    DevReadBlock(parentInode->indirectBlockPtr, indirectBlockPtrs);
    int blockPtr; // block pointer = block number
    for(int i=0;i<NUM_OF_INDIRECT_BLOCK_PTR;i++){
      blockPtr=indirectBlockPtrs[i];
      if(isFound==1)
        break;
      for(int entryIdx=0;entryIdx<DIR_ENTRY_NUM;entryIdx++){
        if(GetDirEntry(blockPtr, entryIdx, pEntry)<0){ // invalid entry
          isFound=1;
          parentInodeNum=pEntry->inodeNum;
          GetInode(parentInodeNum, parentInode); // parent Inode 업데이트
          break;
        }
        else
          continue;
      }
    }
    free(indirectBlockPtrs);
  }

  //printf("(%d,%d)\n", parentBlockNum, parentFreeEntryIdx);

  /** 4. parent directory에 new dirctory entry를 추가한다. **/ 
  DirEntry* parentDirBlock=malloc(BLOCK_SIZE);
  DevReadBlock(parentBlockNum, (char*)parentDirBlock); // entry를 추가할 parent directory block.
  parentDirBlock[parentFreeEntryIdx].inodeNum=newDirInodeNum;
  strcpy(parentDirBlock[parentFreeEntryIdx].name, newDirName);
  DevWriteBlock(parentBlockNum, (char*)parentDirBlock);
  free(parentDirBlock);

  /** 5. new directory block 생성, 설정 **/
  DirEntry* newDirBlock = malloc(BLOCK_SIZE);
  strcpy(newDirBlock[CURRENT_DIR_ENTRY_IDX].name, CURRENT_DIRCTORY_NAME);
  newDirBlock[CURRENT_DIR_ENTRY_IDX].inodeNum=parentInodeNum;
  strcpy(newDirBlock[PARENT_DIR_ENTRY_IDX].name, PARENT_DIRECTORY_NAME);
  newDirBlock[PARENT_DIR_ENTRY_IDX].inodeNum=newDirInodeNum;
  for(int i=PARENT_DIR_ENTRY_IDX+1;i<BLOCK_SIZE/sizeof(DirEntry);i++)
    newDirBlock[i].inodeNum=INVALID_ENTRY;
  DevWriteBlock(newDirBlockNum, (char*)newDirBlock);

  /** 6. new direcotry inode 생성 **/
  Inode* newDirInode=malloc(sizeof(Inode));
  GetInode(newDirInodeNum, newDirInode);
  newDirInode->allocBlocks=1;
  newDirInode->dirBlockPtr[0]=newDirBlockNum;
  newDirInode->indirectBlockPtr=0;
  newDirInode->size=(newDirInode->size)*BLOCK_SIZE;
  newDirInode->type=FILE_TYPE_DIR;
  PutInode(newDirInodeNum, newDirInode);
  PutInode(parentInodeNum, parentInode);

  /** 7. Update metadata **/
  // bytemap
  SetBlockBytemap(newDirBlockNum);
  SetInodeBytemap(newDirInodeNum);
  // FileSysInfo
  pFileSysInfo->numAllocBlocks++;
  pFileSysInfo->numFreeBlocks--;
  pFileSysInfo->numAllocInodes++;
  DevWriteBlock(FILESYS_INFO_BLOCK, (char*)pFileSysInfo);

  free(pFileSysInfo);
  free(nameList);
  free(parentInode);
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
  strcpy(pDirBlock[CURRENT_DIR_ENTRY_IDX].name, CURRENT_DIRCTORY_NAME);
  pDirBlock[CURRENT_DIR_ENTRY_IDX].inodeNum=freeInodeNum; // inode number 저장.
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

// descirptor table의 fee entry index를 반환한다. free entry가 없으면 -1 반환.
int GetFreeDescTableEntryIdx(FileDescTable* pFileDescTable){
  if(pFileDescTable->numUsedDescEntry==DESC_ENTRY_NUM)
    return -1;
  for(int i=0;i<DESC_ENTRY_NUM;i++){
    if(pFileDescTable->pEntry[i].bUsed==0)
      return i;
  }
}

// file table의 free entry index를 반환한다. free entry가 없으면 -1 반환.
int GetFreeFileTableEntryIdx(FileTable* pFileTable){
  if(pFileTable->numUsedFile==MAX_FILE_NUM)
    return -1;
  for(int i=0;i<MAX_FILE_NUM;i++){
    if(pFileTable->pFile[i].bUsed==0)
      return i;
  }
}