/**
 * @file lv_port_disp_templ.c
 *
 */

 /*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_disp_template.h"
#include "lvgl.h"
#include "bsp_lcd.h"
/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(void);

static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
#if LV_USE_GPU
static void gpu_blend(lv_disp_drv_t * disp_drv, lv_color_t * dest, const lv_color_t * src, uint32_t length, lv_opa_t opa);
static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
        const lv_area_t * fill_area, lv_color_t color);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_color_t ltdcShowBuf[LCD_PIXEL_WIDTH*LCD_PIXEL_HEIGHT] __attribute__((at(LCD_FRAME_BUFFER))) = {0};//LTDC显存
/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_disp_init(void)
{
    disp_init();
	
    static lv_disp_buf_t draw_buf_dsc_1;
		static lv_color_t buf1[LCD_PIXEL_WIDTH * LCD_PIXEL_HEIGHT] __attribute__((at(LCD_FRAME_BUFFER+BUFFER_OFFSET+BUFFER_OFFSET))) = {0};//跳过LTDC使用到的两个缓冲区
		static lv_color_t buf2[LCD_PIXEL_WIDTH * LCD_PIXEL_HEIGHT] __attribute__((at(LCD_FRAME_BUFFER+BUFFER_OFFSET+BUFFER_OFFSET+BUFFER_OFFSET))) = {0};//跳过LTDC使用到的两个缓冲区后面继续第二个缓冲
    lv_disp_buf_init(&draw_buf_dsc_1,buf1,buf2,LCD_PIXEL_WIDTH*LCD_PIXEL_HEIGHT);   /*Initialize the display buffer*/

    lv_disp_drv_t disp_drv;                         /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);                    /*Basic initialization*/
		
    disp_drv.hor_res = LCD_PIXEL_WIDTH;
    disp_drv.ver_res = LCD_PIXEL_HEIGHT;
    disp_drv.flush_cb = disp_flush;
    disp_drv.buffer = &draw_buf_dsc_1;

#if LV_USE_GPU
    /*Optionally add functions to access the GPU. (Only in buffered mode, LV_VDB_SIZE != 0)*/

    /*Blend two color array using opacity*/
    disp_drv.gpu_blend_cb = gpu_blend;

    /*Fill a memory array with a color*/
    disp_drv.gpu_fill_cb = gpu_fill;
#endif

    /*Finally register the driver*/
    lv_disp_drv_register(&disp_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/* Initialize your display and the required peripherals. */
static void disp_init(void)
{
    /*You code here*/
		LCD_Init();
		LCD_LayerInit();
		LTDC_Cmd(ENABLE);
		LCD_SetLayer(LCD_BACKGROUND_LAYER);  
		LCD_Clear(LCD_COLOR_BLACK);
}

/* Flush the content of the internal buffer the specific area on the display
 * You can use DMA or any hardware acceleration to do this operation in the background but
 * 'lv_disp_flush_ready()' has to be called when finished. */
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    /*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/
		#if 1//使用DMA2D
    
		DMA2D_InitTypeDef      DMA2D_InitStruct;
		DMA2D_FG_InitTypeDef 	 DMA2D_FG_InitStruct;
		
		DMA2D_DeInit();
		DMA2D_InitStruct.DMA2D_Mode = DMA2D_M2M; //复制数据到显存
		DMA2D_InitStruct.DMA2D_CMode = DMA2D_RGB565;//565格式
		DMA2D_InitStruct.DMA2D_OutputMemoryAdd = ((u32)ltdcShowBuf);//显存地址
		DMA2D_InitStruct.DMA2D_NumberOfLine = area->y2 - area->y1 + 1;//行数量
		DMA2D_InitStruct.DMA2D_PixelPerLine = area->x2 - area->x1 + 1;//每行像素数量
		DMA2D_InitStruct.DMA2D_OutputOffset = (area->y1 * LCD_PIXEL_WIDTH) + area->x1;//显存地址偏移
		DMA2D_Init(&DMA2D_InitStruct);//初始化
		
		DMA2D_FG_StructInit(&DMA2D_FG_InitStruct);//输入配置
		DMA2D_FG_InitStruct.DMA2D_FGCM = DMA2D_RGB565;//565格式
		DMA2D_FG_InitStruct.DMA2D_FGMA = (u32)color_p;//绘画数据
		DMA2D_FGConfig(&DMA2D_FG_InitStruct);//配置输入数据
		
		DMA2D_StartTransfer();//开始传输

		while(DMA2D_GetFlagStatus(DMA2D_FLAG_TC) == RESET)
		{
		}
		
		#else
		int32_t x;
    int32_t y;
		lv_color_t *showBuf = (lv_color_t *)ltdcShowBuf;
		
    for(y = area->y1; y <= area->y2; y++) {
        for(x = area->x1; x <= area->x2; x++) {
            /* Put a pixel to the display. For example: */
            /* put_px(x, y, *color_p)*/
						showBuf[x + (y * LCD_PIXEL_WIDTH)] = *color_p;
            color_p++;
        }
    }
		#endif
    /* IMPORTANT!!!
     * Inform the graphics library that you are ready with the flushing*/
    lv_disp_flush_ready(disp_drv);
}


/*OPTIONAL: GPU INTERFACE*/
#if LV_USE_GPU

/* If your MCU has hardware accelerator (GPU) then you can use it to blend to memories using opacity
 * It can be used only in buffered mode (LV_VDB_SIZE != 0 in lv_conf.h)*/
static void gpu_blend(lv_disp_drv_t * disp_drv, lv_color_t * dest, const lv_color_t * src, uint32_t length, lv_opa_t opa)
{
    /*It's an example code which should be done by your GPU*/
    uint32_t i;
    for(i = 0; i < length; i++) {
        dest[i] = lv_color_mix(dest[i], src[i], opa);
    }
}

/* If your MCU has hardware accelerator (GPU) then you can use it to fill a memory with a color
 * It can be used only in buffered mode (LV_VDB_SIZE != 0 in lv_conf.h)*/
static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
                    const lv_area_t * fill_area, lv_color_t color)
{
    /*It's an example code which should be done by your GPU*/
		
		#if 0 //使用DMA2D
		DMA2D_InitTypeDef      DMA2D_InitStruct;
	
		/* configure DMA2D */
		DMA2D_DeInit();
		DMA2D_InitStruct.DMA2D_Mode = DMA2D_R2M;
		DMA2D_InitStruct.DMA2D_CMode = DMA2D_RGB565;
		DMA2D_InitStruct.DMA2D_OutputGreen = color.ch.green;
		DMA2D_InitStruct.DMA2D_OutputBlue = color.ch.blue;
		DMA2D_InitStruct.DMA2D_OutputRed = color.ch.red;
		DMA2D_InitStruct.DMA2D_OutputAlpha = 0xFF;		//设置透明度
		DMA2D_InitStruct.DMA2D_OutputMemoryAdd = (u32)dest_buf;
		DMA2D_InitStruct.DMA2D_OutputOffset = (fill_area->y1 * dest_width) + fill_area->x1;
		DMA2D_InitStruct.DMA2D_NumberOfLine = fill_area->y2 - fill_area->y1;
		DMA2D_InitStruct.DMA2D_PixelPerLine = fill_area->x2 - fill_area->x1;
		DMA2D_Init(&DMA2D_InitStruct);
		
		/* Start Transfer */
		DMA2D_StartTransfer();
		
		/* Wait for CTC Flag activation */
		while(DMA2D_GetFlagStatus(DMA2D_FLAG_TC) == RESET)
		{
		}
		#else 
    int32_t x, y;
		dest_buf += dest_width * fill_area->y1; /*Go to the first line*/
		
    for(y = fill_area->y1; y <= fill_area->y2; y++) {
        for(x = fill_area->x1; x <= fill_area->x2; x++) {
            dest_buf[x] = color;
        }
        dest_buf+=dest_width;    /*Go to the next line*/
    }
		#endif
}

#endif  /*LV_USE_GPU*/

#else /* Enable this file at the top */

/* This dummy typedef exists purely to silence -Wpedantic. */
typedef int keep_pedantic_happy;
#endif
