#include <stdio.h>
#include <stdlib.h>
#include "disk.h"
#include "hw1.h"

// block 1개 당 inode 개수
int INODE_NUM_PER_BLOCK = BLOCK_SIZE/sizeof(Inode);
int entrySize=sizeof(int);

// FileSys info, inode bytemap, block bytemap, inode list 를 0으로 채워서 초기화한다.
void FileSysInit(void)
{
  char* pBuff;
  pBuff = calloc(BLOCK_SIZE, sizeof(char));
  DevCreateDisk();
  for(int i=0;i<INODELIST_BLOCK_FIRST+INODELIST_BLOCKS;i++)
    DevWriteBlock(i, pBuff);
  free(pBuff);
}

// InodeBytemap[inodeno]를 1로 설정하고 disk로 저장한다.
void SetInodeBytemap(int inodeno)
{
  // 0<=inodeno<64
  // 실제 inode 개수는 64개이다. 
  char* pBuff;
  pBuff = calloc(BLOCK_SIZE, sizeof(char));
  DevReadBlock(INODE_BYTEMAP_BLOCK_NUM, pBuff);
  pBuff[inodeno]=1;
  DevWriteBlock(INODE_BYTEMAP_BLOCK_NUM, pBuff);
  free(pBuff);
}

// InodeBytemap[inodeno]를 0으로 설정하고 disk에 저장한다.
void ResetInodeBytemap(int inodeno)
{
  char* pBuff;
  pBuff = calloc(BLOCK_SIZE, sizeof(char));
  DevReadBlock(INODE_BYTEMAP_BLOCK_NUM, pBuff);
  pBuff[inodeno]=0;
  DevWriteBlock(INODE_BYTEMAP_BLOCK_NUM, pBuff);
  free(pBuff);
}


// BlockBytemap[blkno]를 1로 설정하고 disk에 저장한다.
void SetBlockBytemap(int blkno)
{
  char* pBuff;
  pBuff = malloc(BLOCK_SIZE);
  DevReadBlock(BLOCK_BYTEMAP_BLOCK_NUM, pBuff);
  pBuff[blkno]=1;
  DevWriteBlock(BLOCK_BYTEMAP_BLOCK_NUM, pBuff);
  free(pBuff);
}


void ResetBlockBytemap(int blkno)
{
  char* pBuff;
  pBuff = malloc(BLOCK_SIZE);
  DevReadBlock(BLOCK_BYTEMAP_BLOCK_NUM, pBuff);
  pBuff[blkno]=0;
  DevWriteBlock(BLOCK_BYTEMAP_BLOCK_NUM, pBuff);
  free(pBuff);
}

// *
void PutInode(int inodeno, Inode* pInode)
{
  Inode* pBuff;
  pBuff = malloc(BLOCK_SIZE);
  /**
    inodeno - blockno 
    0 ~ 15 - 3
    16 ~ 31 - 5 
    32 ~ 47 - 6
    48 ~ 63 - 7
    blockno = INODE_BLOCK_FIRST+inodeno/(BLOCK_SIZE/sizeof(Inode))
   **/
  int blockno = INODELIST_BLOCK_FIRST+inodeno/INODE_NUM_PER_BLOCK;
  int inodeIndex=inodeno%INODE_NUM_PER_BLOCK; // argument로 전달받은 inode의 index
  DevReadBlock(blockno, (char*)pBuff);
  // copy pInode to pBuffer
  pBuff[inodeIndex]=*pInode;
  DevWriteBlock(blockno, (char*)pBuff);
  free(pBuff);
}

// *
void GetInode(int inodeno, Inode* pInode)
{
  Inode* pBuff;
  pBuff=malloc(BLOCK_SIZE);
  /**
    inodeno - blockno 
    0 ~ 15 - 3
    16 ~ 31 - 5 
    32 ~ 47 - 6
    48 ~ 63 - 7
    blockno = INODE_BLOCK_FIRST+inodeno/(BLOCK_SIZE/sizeof(Inode))
   **/
  int blockno = INODELIST_BLOCK_FIRST+inodeno/INODE_NUM_PER_BLOCK;
  int inodeIndex=inodeno%INODE_NUM_PER_BLOCK; // argument로 전달받은 inode의 index
  DevReadBlock(blockno, (char*)pBuff);
  *pInode=pBuff[inodeIndex];
  free(pBuff);
}

int GetFreeInodeNum(void)
{
  int numberOfInodes=INODE_NUM_PER_BLOCK*INODELIST_BLOCKS;
  char* pBuff;
  pBuff=malloc(BLOCK_SIZE);
  DevReadBlock(INODE_BYTEMAP_BLOCK_NUM, pBuff);
  int i;
  for(i=0;i<numberOfInodes;i++)
    if(pBuff[i]==0)
      break;
  if(i==numberOfInodes)
    return -1; // there is no free inode
  return i;

  free(pBuff);
}


int GetFreeBlockNum(void)
{
  // metadata를 위한 block들은 제외하고 조사한다 
  int firstBlockRegionBlock=INODELIST_BLOCK_FIRST+INODELIST_BLOCKS;
  char* pBuff;
  pBuff = malloc(BLOCK_SIZE);
  DevReadBlock(BLOCK_BYTEMAP_BLOCK_NUM, pBuff);
  int i;
  for(i=firstBlockRegionBlock;i<BLOCK_SIZE;i++)
    if(pBuff[i]==0)
      break;
  if(i==BLOCK_SIZE){
    free(pBuff);
    return -1; // there is no free block
  }
  free(pBuff);
  return i;
}

void PutIndirectBlockEntry(int blkno, int index, int number)
{
  int* pBuff=malloc(BLOCK_SIZE);
  DevReadBlock(blkno, (char*)pBuff);
  pBuff[index]=number;
  DevWriteBlock(blkno, (char*)pBuff);
  free(pBuff);
}

int GetIndirectBlockEntry(int blkno, int index)
{
  int number;
  int* pBuff=malloc(BLOCK_SIZE);
  DevReadBlock(blkno, (char*)pBuff);
  number=pBuff[index];
  free(pBuff);
  return number;
}

void RemoveIndirectBlockEntry(int blkno, int index)
{
  int* pBuff;
  pBuff = calloc(BLOCK_SIZE/entrySize, entrySize);
  DevReadBlock(blkno, (char*)pBuff);
  pBuff[index]=INVALID_ENTRY;
  DevWriteBlock(blkno, (char*)pBuff);
  free(pBuff);
}

void PutDirEntry(int blkno, int index, DirEntry* pEntry)
{
  DirEntry* pBuff;
  pBuff = calloc(BLOCK_SIZE/sizeof(DirEntry), sizeof(DirEntry));
  DevReadBlock(blkno, (char*)pBuff);
  pBuff[index]=*pEntry;
  DevWriteBlock(blkno, (char*)pBuff);
  free(pBuff);
}

int GetDirEntry(int blkno, int index, DirEntry* pEntry)
{
  DirEntry* pBuff;
  pBuff = calloc(BLOCK_SIZE/sizeof(DirEntry), sizeof(DirEntry));
  DevReadBlock(blkno, (char*)pBuff);
  if(pBuff[index].inodeNum!=INVALID_ENTRY){
    *pEntry=pBuff[index];
    free(pBuff);
    return 1;
  }
  *pEntry=pBuff[index];
  free(pBuff);
  return -1;
}

void RemoveDirEntry(int blkno, int index)
{
  DirEntry* pBuff;
  pBuff = calloc(BLOCK_SIZE/sizeof(DirEntry), sizeof(DirEntry));
  DevReadBlock(blkno, (char*)pBuff);
  pBuff[index].inodeNum=INVALID_ENTRY;
  DevWriteBlock(blkno, (char*)pBuff);
  free(pBuff);
}
