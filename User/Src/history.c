/*
 * @Author: Keinvin
 * @Date: 2022-06-05 13:00:54
 * @LastEditors: Keinvin
 * @LastEditTime: 2022-06-08 20:49:47
 * @FilePath: \MDK-ARMc:\Users\Lltzanhd\Desktop\template\User\Src\history.c
 */
#include "history.h"
#include "stdlib.h"
#include "usart.h"

page_t page;

void page_Init(void)
{
    memset(&page,0,sizeof(page_t));
}

//link_t *initlink_t()
//{
//    link_t *p = (link_t *)malloc(sizeof(link_t)); //创建一个头结点
//    link_t *temp = p;                             //声明一个指针指向头结点，用于遍历链表
//    //生成链表
//    for (int i = 1; i < 5; i++)
//    {
//        link_t *a = (link_t *)malloc(sizeof(link_t));
//        a->index = i;
//        a->next = NULL;
//        temp->next = a;
//        temp = temp->next;
//    }
//    return p;
//}

//int selectElem(link_t *p, int elem)
//{
//    link_t *t = p;
//    int i = 1;
//    while (t->next)
//    {
//        t = t->next;
//        if (t->index == elem)
//        {
//            return i;
//        }
//        i++;
//    }
//    return -1;
//}

////更新函数，其中，add 表示更改结点在链表中的位置，newElem 为新的数据域的值
//link_t *amendElem(link_t *p, int add, int newElem)
//{
//    link_t *temp = p;
//    temp = temp->next; //在遍历之前，temp指向首元结点
//    //遍历到被删除结点
//    for (int i = 1; i < add; i++)
//    {
//        temp = temp->next;
//    }
//    temp->index = newElem;
//    return p;
//}

//link_t *insertElem(link_t *p, int elem, int add)
//{
//    link_t *temp = p; //创建临时结点temp
//    //首先找到要插入位置的上一个结点
//    for (int i = 1; i < add; i++)
//    {
//        if (temp == NULL)
//        {
//            //            printf("插入位置无效\n");
//            return p;
//        }
//        temp = temp->next;
//    }
//    //创建插入结点c
//    link_t *c = (link_t *)malloc(sizeof(link_t));
//    c->index = elem;
//    //向链表中插入结点
//    c->next = temp->next;
//    temp->next = c;
//    return p;
//}

//link_t *delElem(link_t *p, int add)
//{
//    link_t *temp = p;
//    // temp指向被删除结点的上一个结点
//    for (int i = 1; i < add; i++)
//    {
//        temp = temp->next;
//    }
//    link_t *del = temp->next;      //单独设置一个指针指向被删除结点，以防丢失
//    temp->next = temp->next->next; //删除某个结点的方法就是更改前一个结点的指针域
//    free(del);                     //手动释放该结点，防止内存泄漏
//    return p;
//}

clust_t get_fafts_info(void)
{
    clust_t info;
    FATFS *pfs = &USERFatFS;

    DWORD fre_clust, fre_size, tot_size;

    uint8_t result = f_getfree(USERPath, &fre_clust, &pfs);

    if (result == FR_OK)
    {
        // 总容量计算方法
        // pfs->csize 该参数代表一个簇占用几个物理扇区，每个扇区4k字节
        // pfs->n_fatent 簇的数量+2
        // 总容量 = 总簇数*一个簇占用大小
        // 剩余容量 = 剩余簇数*一个簇占用大小
        tot_size = (pfs->n_fatent - 2) * pfs->csize * 4; // 总容量    单位Kbyte
        fre_size = fre_clust * pfs->csize * 4;           // 可用容量  单位Kbyte

        DEBUG_INFO("f_getfree flash tot_sect: %lu kb\r\n", tot_size);
        DEBUG_INFO("f_getfree flash fre_sect: %lu kb\r\n", fre_size);
        DEBUG_INFO("f_getfree flash tot_sect: %.2f Mb\r\n", tot_size / 1024.0);
        DEBUG_INFO("f_getfree flash fre_sect: %.2f Mb\r\n", fre_size / 1024.0);
        DEBUG_INFO("total volume: %lu files\r\n", (uint16_t)(tot_size / 4.0));
        DEBUG_INFO("free volume: %lu files\r\n", (uint16_t)(fre_size / 4.0));
       info.free_mb =  fre_size / 1024.0;
       info.total_mb = tot_size / 1024.0;
       info.free_files = (uint16_t)(fre_size / 4.0);
       info.total_files = (uint16_t)(tot_size / 4.0);
    }
    else
        DEBUG_INFO("f_getfree err : %d \r\n", result);
    return info;
}

FRESULT scan_files(char *path)
{
	FRESULT res;
	DIR dir;
	UINT i;
	static FILINFO fno;
    
	res = f_opendir(&dir, path); /* Open the directory */
	if (res == FR_OK)
	{
		for (;;)
		{
			res = f_readdir(&dir, &fno); /* Read a directory item */
			if (res != FR_OK || fno.fname[0] == 0)
				break; /* Break on error or end of dir */
			if (fno.fattrib & AM_DIR)
			{ /* It is a directory */
				i = strlen(path);
				sprintf(&path[i], "/%s", fno.fname);
				res = scan_files(path); /* Enter the directory */
				if (res != FR_OK)
					break;
				path[i] = 0;
			}
			else
			{ /* It is a file. */
				page.itemNum++;
				DEBUG_INFO("%s/%s\r\n", path, fno.fname);
			}
		}
		f_closedir(&dir);
		DEBUG_INFO("fileNum:%d\r\n", page.itemNum);
        page.currentPage = 1;
        page.totalPages = (page.itemNum-1)/7 + 1;/*8个之后才有第二页*/
	}
	return res;
}

FRESULT RemoveAllFiles(char *path)
{
	FRESULT res;
	DIR dir;
	UINT i;
	static FILINFO fno;
    char pathname[30];
    
	res = f_opendir(&dir, path); /* Open the directory */
	if (res == FR_OK)
	{
		for (;;)
		{
			res = f_readdir(&dir, &fno); /* Read a directory item */
			if (res != FR_OK || fno.fname[0] == 0)
				break; /* Break on error or end of dir */
			if (fno.fattrib & AM_DIR)
			{ /* It is a directory */
				i = strlen(path);
				sprintf(&path[i], "/%s", fno.fname);
				res = scan_files(path); /* Enter the directory */
				if (res != FR_OK)
					break;
				path[i] = 0;
			}
			else
			{ /* It is a file. */
                sprintf(pathname,"sub0/%s",fno.fname);
                res = f_unlink(pathname);
			}
		}
		f_closedir(&dir);
       page.itemNum = 0;
       page.totalPages = (page.itemNum-1)/7 + 1;/*8个之后才有第二页*/
       page.currentPage = 1;
	}
	return res;
}
