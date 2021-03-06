/*
 * Copyright (c) 2020 Yuri Bobrov <ubobrov@yandex.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed as is in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "csc.h"

/*
 *
 */
void yuyv422toNV12(int width, int height, unsigned char *FrameIn, unsigned char *FrameOut) {       
    int x,y,u,v;       
    const int FrameInSize = width*height*2;
    const int YBufOutSize = height*width;
    
    for (x = 0, u = 0; x < FrameInSize; x+=2, u++)
    {
        FrameOut[u] = FrameIn[x];
    }
    u = YBufOutSize;
    v = YBufOutSize + 1;
    for (y = 0; y < height; y+=2) {
        for (x = 0; x < width*2; x+=4) {
            FrameOut[u] = (FrameIn[1+x+y*width*2] + FrameIn[1+x+(y+1)*width*2])/2;
            u += 2;
            FrameOut[v] = (FrameIn[3+x+y*width*2] + FrameIn[3+x+(y+1)*width*2])/2;
            v += 2;
        }
    }
}

/*
 *
 */
void uyvy422toNV12(int width, int height, unsigned char *FrameIn, unsigned char *FrameOut) {       
    int x,y,u,v;       
    const int FrameInSize = width*height*2;
    const int YBufOutSize = height*width;
    
    for (x = 1, u = 0; x < FrameInSize; x+=2, u++)
    {
        FrameOut[u] = FrameIn[x];
    }
    u = YBufOutSize;
    v = YBufOutSize + 1;
    for (y = 0; y < height; y+=2) {
        for (x = 0; x < width*2; x+=4) {
            FrameOut[u] = (FrameIn[0+x+y*width*2] + FrameIn[0+x+(y+1)*width*2])/2;
            u += 2;
            FrameOut[v] = (FrameIn[2+x+y*width*2] + FrameIn[2+x+(y+1)*width*2])/2;
            v += 2;
        }
    }
}

/*
 *
 */
void uyvy422to420(int width, int height, unsigned char *FrameIn, unsigned char *FrameOut) {       
    int x,y,u,v;       
    const int FrameInSize = width*height*2;
    const int YBufOutSize = height*width;
    const int UVBufOutSize = height*width/4;
    
    for (x = 1, u = 0; x < FrameInSize; x+=2, u++)
    {
        FrameOut[u] = FrameIn[x];
    }
    u = YBufOutSize;
    v = YBufOutSize + UVBufOutSize;
    for (y = 0; y < height; y+=2) {
        for (x = 0; x < width*2; x+=4)
        {
            FrameOut[u] = (FrameIn[0+x+y*width*2] + FrameIn[0+x+(y+1)*width*2])/2;
            u++;
            FrameOut[v] = (FrameIn[2+x+y*width*2] + FrameIn[2+x+(y+1)*width*2])/2;
            v++;
        }
    }
}

#if defined(CPU_HAS_NEON)  

#define IS_ALIGNED(x, a) (((x) & ((typeof(x))(a) - 1)) == 0)
/*
 *  U  Y  V  Y
 * {d0,d1,d2,d3}
 */

void ExtractY_NEON(const uint8 *src_uyvy, uint8 *dst_y, int width) {
    asm volatile (
       "1:                                      \n"
#ifndef __aarch64__
       "vld4.u8    {d0,d1,d2,d3}, [%0]!        \n" // load 16 pairs of UYVY
       "subs  %2, %2, #16                     \n" // 16 processed per loop
       "vst2.u8   {d1,d3}, [%1]!              \n" // store back Y and Y
#else 
       "ld4 {v0.8b,v1.8b,v2.8b,v3.8b},[%0],#32 \n"
       "mov v2.8b, v3.8b                        \n"
       "subs  %2, %2, #16                     \n"
       "st2 {v1.8b,v2.8b},[%1],#16             \n"
#endif       
       "bgt   1b                              \n" // Loop back if not done
       : "+r"(src_uyvy), // %0
        "+r"(dst_y), // %1
        "+r"(width)     // %3      // output registers
       :                            // input registers
       : "memory", "cc", "q0", "q1", "q2", "q3" // Clobber List
    );                            
}

void ExtractUV_NEON(const uint8 *src_uyvy, uint8 *dst_uv, int width) {
    asm volatile (
       "1:                                      \n"
#ifndef __aarch64__       
       "vld4.u8    {d0,d1,d2,d3}, [%0]!        \n" // load 16 pairs of UYVY
       "subs  %2, %2, #16                     \n" // 16 processed per loop
       "vst2.u8   {d0,d2}, [%1]!              \n" // store back U and V
#else 
       "ld4 {v0.8b,v1.8b,v2.8b,v3.8b},[%0],#32 \n"
       "mov v1.8b, v2.8b                        \n"
       "subs  %2, %2, #16                     \n"
       "st2 {v0.8b,v1.8b},[%1],#16             \n"
#endif       
       "bgt   1b                              \n" // Loop back if not done
       : "+r"(src_uyvy), // %0
         "+r"(dst_uv), // %1
         "+r"(width)     // %3      // output registers
       :                            // input registers
       : "memory", "cc", "q0", "q1", "q2", "q3" // Clobber List
    );                             
}

void ExtractU_V_NEON(const uint8 *src_uyvy, uint8 *dst_u, uint8 *dst_v, int width) {
    asm volatile (
       "1:                                      \n"
#ifndef __aarch64__        
       "vld4.u8    {d0,d1,d2,d3}, [%0]!        \n" // load 16 pairs of UYVY
       "subs  %3, %3, #16                     \n" // 16 processed per loop
       "vst1.u8   {d0}, [%1]!              \n" // store back U
       "vst1.u8   {d2}, [%2]!              \n" // store back V
#else       
       "ld4 {v0.8b,v1.8b,v2.8b,v3.8b},[%0],#32 \n"
       "subs  %3, %3, #16                     \n"
       "st1 {v0.8b},[%1],#8             \n"
       "st1 {v2.8b},[%2],#8             \n"
#endif       
       "bgt   1b                              \n" // Loop back if not done
       : "+r"(src_uyvy), // %0
         "+r"(dst_u), // %1
         "+r"(dst_v), // %2
         "+r"(width)     // %3      // output registers
       :                            // input registers
       : "memory", "cc", "q0", "q1", "q2", "q3" // Clobber List
    );                             
}

/*
 *  Y  U  Y  V
 * {d0,d1,d2,d3}
 */
void ExtractY_yuyv_NEON(const uint8 *src_uyvy, uint8 *dst_y, int width) {
    asm volatile (
       "1:                                      \n"
#ifndef __aarch64__         
       "vld4.u8    {d0,d1,d2,d3}, [%0]!        \n" // load 16 pairs of UYVY
       "subs  %2, %2, #16                     \n" // 16 processed per loop
       "vst2.u8   {d0,d2}, [%1]!              \n" // store back Y and Y
#else 
       "ld4 {v0.8b,v1.8b,v2.8b,v3.8b},[%0],#32 \n"
       "subs  %2, %2, #16                     \n"
       "mov v1.8b, v2.8b                        \n"
       "st2 {v0.8b,v1.8b},[%1],#16             \n"
#endif       
       "bgt   1b                              \n" // Loop back if not done
       : "+r"(src_uyvy), // %0
        "+r"(dst_y), // %1
        "+r"(width)     // %3      // output registers
       :                            // input registers
       : "memory", "cc", "q0", "q1", "q2", "q3" // Clobber List
    );                            
}

void ExtractUV_yuyv_NEON(const uint8 *src_uyvy, uint8 *dst_uv, int width) {
    asm volatile (
       "1:                                      \n"
#ifndef __aarch64__         
       "vld4.u8    {d0,d1,d2,d3}, [%0]!        \n" // load 16 pairs of UYVY
       "subs  %2, %2, #16                     \n" // 16 processed per loop
       "vst2.u8   {d1,d3}, [%1]!              \n" // store back U and V
#else 
       "ld4 {v0.8b,v1.8b,v2.8b,v3.8b},[%0],#32 \n"
       "mov v2.8b, v3.8b                        \n"
       "subs  %2, %2, #16                     \n"
       "st2 {v1.8b,v2.8b},[%1],#16             \n"
#endif       
       "bgt   1b                              \n" // Loop back if not done
       : "+r"(src_uyvy), // %0
         "+r"(dst_uv), // %1
         "+r"(width)     // %3      // output registers
       :                            // input registers
       : "memory", "cc", "q0", "q1", "q2", "q3" // Clobber List
    );                             
}

void ExtractU_V_yuyv_NEON(const uint8 *src_uyvy, uint8 *dst_u, uint8 *dst_v, int width) {
    asm volatile (
       "1:                                      \n"
#ifndef __aarch64__         
       "vld4.u8    {d0,d1,d2,d3}, [%0]!        \n" // load 16 pairs of UYVY
       "subs  %3, %3, #16                     \n" // 16 processed per loop
       "vst1.u8   {d1}, [%1]!              \n" // store back U
       "vst1.u8   {d3}, [%2]!              \n" // store back V
#else 
       "ld4 {v0.8b,v1.8b,v2.8b,v3.8b},[%0],#32 \n"
       "subs  %3, %3, #16                     \n"
       "st1 {v1.8b},[%1],#8             \n"
       "st1 {v3.8b},[%2],#8             \n"
#endif       
       "bgt   1b                              \n" // Loop back if not done
       : "+r"(src_uyvy), // %0
         "+r"(dst_u), // %1
         "+r"(dst_v), // %2
         "+r"(width)     // %3      // output registers
       :                            // input registers
       : "memory", "cc", "q0", "q1", "q2", "q3" // Clobber List
    );                             
}

int UYVYToNV12_neon(const uint8 *src_uyvy, int src_stride_uyvy,
               uint8 *dst_y, int dst_stride_y,
               uint8 *dst_uv, int dst_stride_uv,
               int width, int height) {
    int i;
    if (!IS_ALIGNED(width, 16)) {
        //printf("CPU: %d  ALIGNED: %d\n", TestCpuFlag(kCpuHasNEON), IS_ALIGNED(width, 16));
        return -1;
    }

    for (i = 0; i < height; i+=2) {
        ExtractUV_NEON(src_uyvy, dst_uv, width);
        dst_uv += dst_stride_uv;

        ExtractY_NEON(src_uyvy, dst_y, width);
        dst_y += dst_stride_y;
        src_uyvy += src_stride_uyvy;

        ExtractY_NEON(src_uyvy, dst_y, width);
        dst_y += dst_stride_y;
        src_uyvy += src_stride_uyvy;
    }

    return 0;

}

int UYVYTo420P_neon(const uint8 *src_uyvy, int src_stride_uyvy,
               uint8 *dst_y, int dst_stride_y,
               uint8 *dst_u, int dst_stride_u,
               uint8 *dst_v, int dst_stride_v,
               int width, int height) {
    int i;

    if (!IS_ALIGNED(width, 16)) {
        //printf("CPU: %d  ALIGNED: %d\n", TestCpuFlag(kCpuHasNEON), IS_ALIGNED(width, 16));
        return -1;
    }

    for (i = 0; i < height; i+=2) {
        ExtractU_V_NEON(src_uyvy, dst_u, dst_v, width);
        dst_u += dst_stride_u;
        dst_v += dst_stride_v;

        ExtractY_NEON(src_uyvy, dst_y, width);
        dst_y += dst_stride_y;
        src_uyvy += src_stride_uyvy;

        ExtractY_NEON(src_uyvy, dst_y, width);
        dst_y += dst_stride_y;
        src_uyvy += src_stride_uyvy;
    }

    return 0;

}

int YUYVToNV12_neon(const uint8 *src_uyvy, int src_stride_uyvy,
               uint8 *dst_y, int dst_stride_y,
               uint8 *dst_uv, int dst_stride_uv,
               int width, int height) {
    int i;
    if (!IS_ALIGNED(width, 16)) {
        //printf("CPU: %d  ALIGNED: %d\n", TestCpuFlag(kCpuHasNEON), IS_ALIGNED(width, 16));
        return -1;
    }

    for (i = 0; i < height; i+=2) {
        ExtractUV_yuyv_NEON(src_uyvy, dst_uv, width);
        dst_uv += dst_stride_uv;

        ExtractY_yuyv_NEON(src_uyvy, dst_y, width);
        dst_y += dst_stride_y;
        src_uyvy += src_stride_uyvy;

        ExtractY_yuyv_NEON(src_uyvy, dst_y, width);
        dst_y += dst_stride_y;
        src_uyvy += src_stride_uyvy;
    }

    return 0;

}

int YUYVTo420P_neon(const uint8 *src_uyvy, int src_stride_uyvy,
               uint8 *dst_y, int dst_stride_y,
               uint8 *dst_u, int dst_stride_u,
               uint8 *dst_v, int dst_stride_v,
               int width, int height) {
    int i;

    if (!IS_ALIGNED(width, 16)) {
        //printf("CPU: %d  ALIGNED: %d\n", TestCpuFlag(kCpuHasNEON), IS_ALIGNED(width, 16));
        return -1;
    }

    for (i = 0; i < height; i+=2) {
        ExtractU_V_yuyv_NEON(src_uyvy, dst_u, dst_v, width);
        dst_u += dst_stride_u;
        dst_v += dst_stride_v;

        ExtractY_yuyv_NEON(src_uyvy, dst_y, width);
        dst_y += dst_stride_y;
        src_uyvy += src_stride_uyvy;

        ExtractY_yuyv_NEON(src_uyvy, dst_y, width);
        dst_y += dst_stride_y;
        src_uyvy += src_stride_uyvy;
    }

    return 0;

}

#endif
