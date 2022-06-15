#include <stdio.h>
#include <stdlib.h>
#include "disk.h"
#include "hw1.h"
#include "hw2.h"

FileDescTable* pFileDescTable;
FileSysInfo* pFileSysInfo;

const wchar_t* CURRENT_DIRCTORY_NAME = L".";
const wchar_t* PARENT_DIRECTORY_NAME = L"..";

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


int MakeDirectory(char* name)
{

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
   * size_t wcstombs(char *str, const wchar_t *pwcs, size_t n)
   * wide-char string pwcs를 str에서 시작하는 char 배열로 전환한다.
   * **/
  wcstombs(pDirBlock[0].name, CURRENT_DIRCTORY_NAME, MAX_NAME_LEN);
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
