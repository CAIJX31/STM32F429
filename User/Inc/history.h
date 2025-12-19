/*
 * @Author: Keinvin
 * @Date: 2022-05-31 13:20:54
 * @LastEditors: Keinvin
 * @LastEditTime: 2022-06-08 20:49:49
 * @FilePath: \MDK-ARMc:\Users\Lltzanhd\Desktop\template\User\Inc\history.h
 */
#ifndef _H_HISTORY_
#define _H_HISTORY_

#ifdef __cplusplus
extern "C"
{
#endif

#include "stdint.h"
#include "fatfs.h"

typedef struct
{
    float free_mb;
    float total_mb;
    uint16_t free_files;
    uint16_t total_files;
}clust_t;
    
 typedef struct
{
    int16_t currentPage;
    int16_t totalPages;
    int16_t itemNum;
}page_t;
    
//typedef struct Link
//{
//    uint16_t index;
//    struct Link *next;
//} link_t;
    
    
 extern page_t page;
    
    
    clust_t get_fafts_info(void);
    void page_Init(void);
    FRESULT scan_files(char *path);
    FRESULT RemoveAllFiles(char *path);
   
#ifdef __cplusplus
}
#endif

#endif
