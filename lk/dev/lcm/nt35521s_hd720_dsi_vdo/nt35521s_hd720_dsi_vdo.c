#ifndef BUILD_LK
#include <linux/string.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
    #include <platform/disp_drv_platform.h>
	
#elif defined(BUILD_UBOOT)
    #include <asm/arch/mt_gpio.h>
#else
    #include <linux/delay.h>
    #include <mach/mt_gpio.h>
#endif
#ifdef BUILD_LK
#define LCD_DEBUG(fmt)  dprintf(CRITICAL,fmt)
#else
#define LCD_DEBUG(fmt)  printk(fmt)
#endif

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------
#define FRAME_WIDTH  										(720)
#define FRAME_HEIGHT 										(1280)


#define REGFLAG_DELAY             							0xFFFC
#define REGFLAG_END_OF_TABLE      							0xFFFD   // END OF REGISTERS MARKER


#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif

static unsigned int lcm_esd_test = FALSE;      ///only for ESD test

#define LCM_DSI_CMD_MODE									0

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))

#define LCM_RM68200_ID (0x6820)


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)										lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   			lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    

struct LCM_setting_table {
    unsigned int cmd;
    unsigned char count;
    unsigned char para_list[128];
};
//update initial param for IC rm68200 0.01
static struct LCM_setting_table rm68200_hd720_dsi_video_hf[] = {

    {0xFF,4,{0xAA,0x55,0xA5,0x80}},
    {0x6F,2,{0x11,0x00}},
    {0xF7,2,{0x20,0x00}},
    {0x6F,1,{0x1E}},
    {0xFA,1,{0x20}},
    {0x6F,1,{0x1E}},
    {0xFA,1,{0x00}},
    {0x6F,1,{0x1F}},
    {0xFA,1,{0x00}},
    {0x6F,1,{0x11}},
    {0xF3,1,{0x01}},
    {0xF0,5,{0x55,0xAA,0x52,0x08,0x00}},
    {0xBD,5,{0x01,0xA0,0x0C,0x08,0x01}},
    {0x6F,1,{0x02}},
    {0xB8,1,{0x0C}},
    {0xBB,2,{0x11,0x11}},
    {0xBC,2,{0x00,0x00}},
    {0xB6,1,{0x01}},
    {0xF0,5,{0x55,0xAA,0x52,0x08,0x01}},
    {0xB0,2,{0x09,0x09}},
    {0xB1,2,{0x09,0x09}},
    {0xBC,2,{0x78,0x01}},
    {0xBD,2,{0x78,0x01}},
    {0xCA,1,{0x00}},
    {0xC0,1,{0x04}},
    {0xB5,2,{0x03,0x03}},
    {0xBE,1,{0x60}},  //58
    {0xB3,2,{0x1B,0x1B}},
    {0xB4,2,{0x0F,0x0F}},
    {0xB9,2,{0x26,0x26}},
    {0xBA,2,{0x14,0x14}},
    {0xF0,5,{0x55,0xAA,0x52,0x08,0x02}},
    {0xEE,1,{0x01}},
    {0xB0,16,{0x00,0x00,0x00,0x77,0x00,0xA5,0x00,0xC2,0x00,0xD0,0x00,0xF2,0x01,0x16,0x01,0x44}},
    {0xB1,16,{0x01,0x67,0x01,0xA0,0x01,0xCB,0x02,0x11,0x02,0x4D,0x02,0x4F,0x02,0x83,0x02,0xBE}},
    {0xB2,16,{0x02,0xE2,0x03,0x13,0x03,0x34,0x03,0x5B,0x03,0x80,0x03,0xA0,0x03,0xB7,0x03,0xD0}},
    {0xB3,4,{0x03,0xE9,0x03,0xFA}},
    {0x6F,1,{0x02}},
    {0xF7,1,{0x47}},
    {0x6F,1,{0x0A}},
    {0xF7,1,{0x02}},
    {0x6F,1,{0x17}},
    {0xF4,1,{0x70}},
    {0x6F,1,{0x11}},
    {0xF3,1,{0x01}},
    {0xF0,5,{0x55,0xAA,0x52,0x08,0x06}},
    {0xB0,2,{0x12,0x10}},
    {0xB1,2,{0x18,0x16}},
    {0xB2,2,{0x00,0x02}},
    {0xB3,2,{0x31,0x31}},
    {0xB4,2,{0x31,0x34}},
    {0xB5,2,{0x34,0x31}},
    {0xB6,2,{0x31,0x33}},
    {0xB7,2,{0x33,0x33}},
    {0xB8,2,{0x31,0x08}},
    {0xB9,2,{0x2E,0x2D}},
    {0xBA,2,{0x2D,0x2E}},
    {0xBB,2,{0x09,0x31}},
    {0xBC,2,{0x33,0x33}},
    {0xBD,2,{0x33,0x31}},
    {0xBE,2,{0x31,0x34}},
    {0xBF,2,{0x34,0x31}},
    {0xC0,2,{0x31,0x31}},
    {0xC1,2,{0x03,0x01}},
    {0xC2,2,{0x17,0x19}},
    {0xC3,2,{0x11,0x13}},
    {0xE5,2,{0x31,0x31}},
    {0xC4,2,{0x17,0x19}},
    {0xC5,2,{0x11,0x13}},
    {0xC6,2,{0x03,0x01}},
    {0xC7,2,{0x31,0x31}},
    {0xC8,2,{0x31,0x34}},
    {0xC9,2,{0x34,0x31}},
    {0xCA,2,{0x31,0x33}},
    {0xCB,2,{0x33,0x33}},
    {0xCC,2,{0x31,0x09}},
    {0xCD,2,{0x2D,0x2E}},
    {0xCE,2,{0x2E,0x2D}},
    {0xCF,2,{0x08,0x31}},
    {0xD0,2,{0x33,0x33}},
    {0xD1,2,{0x33,0x31}},
    {0xD2,2,{0x31,0x34}},
    {0xD3,2,{0x34,0x31}},
    {0xD4,2,{0x31,0x31}},
    {0xD5,2,{0x00,0x02}},
    {0xD6,2,{0x12,0x10}},
    {0xD7,2,{0x18,0x16}},
    {0xD8,5,{0x00,0x00,0x00,0x00,0x00}},
    {0xD9,5,{0x00,0x00,0x00,0x00,0x00}},
    {0xE7,1,{0x00}},
    {0xF0,5,{0x55,0xAA,0x52,0x08,0x05}},
    {0xED,1,{0x30}},
    {0xF0,5,{0x55,0xAA,0x52,0x08,0x03}},
    {0xB1,1,{0x00,0x00}},
    {0xB0,1,{0x00,0x00}},
    {0xF0,5,{0x55,0xAA,0x52,0x08,0x05}},
    {0xE5,1,{0x00}},
    {0xF0,5,{0x55,0xAA,0x52,0x08,0x05}},
    {0xB0,2,{0x17,0x06}},
    {0xB8,1,{0x00}},
    {0xBD,5,{0x03,0x03,0x01,0x00,0x03}},
    {0xB1,2,{0x17,0x06}},
    {0xB9,2,{0x00,0x03}},
    {0xB2,2,{0x17,0x06}},
    {0xBA,2,{0x00,0x00}},
    {0xB3,2,{0x17,0x06}},
    {0xBB,2,{0x00,0x00}},
    {0xB4,2,{0x17,0x06}},
    {0xB5,2,{0x17,0x06}},
    {0xB6,2,{0x17,0x06}},
    {0xB7,2,{0x17,0x06}},
    {0xBC,2,{0x00,0x03}},
    {0xE5,1,{0x06}},
    {0xE6,1,{0x06}},
    {0xE7,1,{0x06}},
    {0xE8,1,{0x06}},
    {0xE9,1,{0x06}},
    {0xEA,1,{0x06}},
    {0xEB,1,{0x06}},
    {0xEC,1,{0x06}},
    {0xF0,5,{0x55,0xAA,0x52,0x08,0x05}},
    {0xC0,1,{0x0B}},
    {0xC1,1,{0x09}},
    {0xC2,1,{0x0B}},
    {0xC3,1,{0x09}},
    {0xF0,5,{0x55,0xAA,0x52,0x08,0x03}},
    {0xB2,5,{0x05,0x00,0x54,0x00,0x00}},
    {0xB3,5,{0x05,0x00,0x54,0x00,0x00}},
    {0xB4,5,{0x05,0x00,0x54,0x00,0x00}},
    {0xB5,5,{0x05,0x00,0x54,0x00,0x00}},
    {0xF0,5,{0x55,0xAA,0x52,0x08,0x05}},
    {0xC4,1,{0x10}},
    {0xC5,1,{0x10}},
    {0xC6,1,{0x10}},
    {0xC7,1,{0x10}},
    {0xF0,5,{0x55,0xAA,0x52,0x08,0x03}},
    {0xB6,5,{0x05,0x00,0x54,0x00,0x00}},
    {0xB7,5,{0x05,0x00,0x54,0x00,0x00}},
    {0xB8,5,{0x05,0x00,0x54,0x00,0x00}},
    {0xB9,5,{0x05,0x00,0x54,0x00,0x00}},
    {0xF0,5,{0x55,0xAA,0x52,0x08,0x05}},
    {0xC8,2,{0x07,0x20}},
    {0xC9,2,{0x03,0x20}},
    {0xCA,2,{0x07,0x00}},
    {0xCB,2,{0x03,0x00}},
    {0xF0,5,{0x55,0xAA,0x52,0x08,0x03}},
    {0xBA,5,{0x44,0x00,0x00,0x00,0x70}},
    {0xBB,5,{0x44,0x00,0x00,0x00,0x70}},
    {0xBC,5,{0x44,0x00,0x00,0x00,0x70}},
    {0xBD,5,{0x44,0x00,0x00,0x00,0x70}},
    {0xF0,5,{0x55,0xAA,0x52,0x08,0x05}},
    {0xD1,5,{0x00,0x05,0x00,0x07,0x10}},
    {0xD2,5,{0x00,0x05,0x04,0x07,0x10}},
    {0xD3,5,{0x00,0x00,0x0A,0x07,0x10}},
    {0xD4,5,{0x00,0x00,0x0A,0x07,0x10}},
    {0xF0,5,{0x55,0xAA,0x52,0x08,0x05}},
    {0xD0,7,{0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
    {0xD5,11,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
    {0xD6,11,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
    {0xD7,11,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
    {0xD8,5,{0x00,0x00,0x00,0x00,0x00}},
    {0xF0,5,{0x55,0xAA,0x52,0x08,0x03}},
    {0xC4,1,{0x60}},
    {0xC5,1,{0x40}},
    {0xC6,1,{0x60}},
    {0xC7,1,{0x40}},
    {0x6F,1,{0x01}},
    {0xF9,1,{0x46}},
    
    {0x11,1,{0x00}},
    {REGFLAG_DELAY, 120, {}},
    {0x29,1,{0x00}},
    {REGFLAG_DELAY, 10, {}},

	  // Setting ending by predefined flag
	  {REGFLAG_END_OF_TABLE, 0x00, {}}	
	
};
							
static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 0, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
	{0x29, 0, {0x00}},
    {REGFLAG_DELAY, 100, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
    // Display off sequence
    {0x28, 1, {0x00}},
    {REGFLAG_DELAY, 20, {}},

    // Sleep Mode On
    {0x10, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};

static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
    unsigned int i;

    for(i = 0; i < count; i++)
    {
        unsigned cmd;
        cmd = table[i].cmd;

        switch (cmd) {

            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;

            case REGFLAG_END_OF_TABLE :
                break;

            default:
                dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
        }
    }
}

// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
    memset(params, 0, sizeof(LCM_PARAMS));
    params->type   = LCM_TYPE_DSI;
    params->width  = FRAME_WIDTH;
    params->height = FRAME_HEIGHT;

	params->dsi.mode   = BURST_VDO_MODE;	//BURST_VDO_MODE;	//SYNC_PULSE_VDO_MODE;

    // DSI
    /* Command mode setting */
    params->dsi.LANE_NUM				= LCM_FOUR_LANE;
    //The following defined the fomat for data coming from LCD engine.
    params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
    params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
    params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
    params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

   // Highly depends on LCD driver capability.
	params->dsi.packet_size=256;
    params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
   
	params->dsi.vertical_sync_active				= 2;
	params->dsi.vertical_backporch					= 14;
	params->dsi.vertical_frontporch					= 16;
	params->dsi.vertical_active_line				= FRAME_HEIGHT; 

	params->dsi.horizontal_sync_active				= 10;
	params->dsi.horizontal_backporch				= 80;
	params->dsi.horizontal_frontporch				= 80;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	//improve clk quality
	params->dsi.PLL_CLOCK = 260; //lcm clock 
	//params->dsi.HS_TRAIL = 120;
	//params->dsi.ufoe_enable = 1;
  //params->dsi.ssc_disable = 1;

}

static void lcm_init(void)
{
	SET_RESET_PIN(1);    
	MDELAY(10); 
	SET_RESET_PIN(0);
	MDELAY(10); 
	SET_RESET_PIN(1);
	MDELAY(120); 

    push_table(rm68200_hd720_dsi_video_hf, sizeof(rm68200_hd720_dsi_video_hf) / sizeof(struct LCM_setting_table), 1);
    //LCD_DEBUG("uboot:tm_rm68200_lcm_init\n");
}


static void lcm_suspend(void)
{
    push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
    //LCD_DEBUG("uboot:tm_rm68200_lcm_suspend\n");
	SET_RESET_PIN(0);
	MDELAY(10); 
}
static void lcm_resume(void)
{
	lcm_init();
    //LCD_DEBUG("uboot:tm_rm68200_lcm_resume\n");

}

static unsigned int lcm_compare_id(void)
{
	unsigned int id = 0;
	unsigned char buffer[10];
	unsigned int array[16];

	SET_RESET_PIN(1);  //NOTE:should reset LCM firstly
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(50);
	SET_RESET_PIN(1);
	MDELAY(120);
	array[0]=0x00063902;
	array[1]=0x8198ffff;
	array[2]=0x00000104;
	dsi_set_cmdq(array, 3, 1);
	MDELAY(10);
 
	array[0] = 0x00033700;// return byte number
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0x00, &buffer[0], 1);
	read_reg_v2(0x01, &buffer[1], 1);
	read_reg_v2(0x02, &buffer[2], 1);
	
	if((0x98==buffer[0]) && (0x81==buffer[1]))
	{
		id = 0x9881;
	}

#ifdef BUILD_LK
	printf("[LK]------ili9881 read id = 0x%x, 0x%x, 0x%x---------\n", buffer[0], buffer[1], buffer[2]);
#else
	printk("[KERNEL]------ili9881 read id = 0x%x, 0x%x, 0x%x---------\n", buffer[0], buffer[1], buffer[2]);
#endif
	return (0x9881 == id) ? 1 : 0;
}

LCM_DRIVER nt35521s_hd720_dsi_vdo_lcm_drv = 
{
    .name           = "nt35521s_hd720_dsi_vdo",
    .set_util_funcs = lcm_set_util_funcs,
    .get_params     = lcm_get_params,
    .init           = lcm_init,
    .suspend        = lcm_suspend,
    .resume         = lcm_resume,
    //.compare_id     = lcm_compare_id,
   
};
