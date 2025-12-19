/*
 * @Author: Keinvin
 * @Date: 2022-05-31 13:20:54
 * @LastEditors: Keinvin
 * @LastEditTime: 2022-05-31 15:22:16
 * @FilePath: \MDK-ARMc:\Users\Lltzanhd\Desktop\template\User\Inc\spwm.h
 */
#ifndef _H_SPWM_
#define _H_SPWM_

#ifdef __cplusplus
extern "C"
{
#endif

#include "stdint.h"

    typedef enum
    {
        HZ_50 = 0,
        HZ_100,
        HZ_400,
        HZ_800,
        HZ_1000,
        HZ_NULL,
    } fre_select_t;

    typedef struct
    {
        uint16_t tim_arr;         //????arr?
        uint16_t len;             // spwm???????
        const uint32_t *spwm_src; //????
    } spwm_t;

    void spwm_init(void);
    void spwm_FreqSelect(fre_select_t fre);
    void Transformer_Select_Reset(void);
    void Transformer_Select_24U(void);
    void Transformer_Select_24V(void);
    void Transformer_Select_24W(void);
    
#ifdef __cplusplus
}
#endif

#endif
