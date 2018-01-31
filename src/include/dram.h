#ifndef  _DRAM_H_
#define  _DRAM_H_

//#include <rlx_types.h>
#include <bsp_cfg.h>
//#include <rlx_cpu_regs.h>

#define DDR_PHY_IOBASE        0xbbb00000 
#define PERIPHERAL_CLK        31250000

enum dram_type {
  NON_DDR = 0,
  DDR_1 = 1,
  DDR_2 = 2,
  DDR_3 = 3,
  DDR_4 = 4, 
  SDR   = 8
};

enum dram_page_size {
  PAGE_256K = 0,
  PAGE_512K = 1,
  PAGE_1K   = 2,
  PAGE_2K   = 3,
  PAGE_4K   = 4,
  PAGE_8K   = 5,
  PAGE_16K  = 6,
  PAGE_32K  = 7,
  PAGE_64K  = 8 
};

enum dram_bank_size {
  BANK_2   = 0,
  BANK_4   = 1,
  BANK_8   = 2 
};

enum dram_dq_width {
  DQ_16     = 0,
  DQ_32     = 1, 
  HALF_DQ32 = 2 
};

enum mode0_bst_len {
  BST_LEN_4   = 0,
  BST_LEN_FLY = 1,
  BST_LEN_8   = 2 
};

enum mode0_bst_type {
  SENQUENTIAL = 0,
  INTERLEAVE  = 1 
};   

enum dfi_ratio_type {
  DFI_RATIO_1 = 0,  // DFI= 1:1, or SDR
  DFI_RATIO_2 = 1, 
  DFI_RATIO_4 = 2  
};   

struct ddr1_pmu_portmap {

  volatile unsigned int ddr1_pmu_dpi_pi;              /* DDR Control Register (0x00) */
  volatile unsigned int ddr1_pmu_srstn;               /* DDR Control Register (0x00) */
  volatile unsigned int ddr1_pmu_dpi_pll_status;      /* DDR Control Register (0x04) */
  volatile unsigned int ddr1_pmu_dpi_cco;             /* DDR Control Register (0x08) */
  volatile unsigned int ddr1_pmu_dpi_clk;             /* DDR Control Register (0x0c) */
  volatile unsigned int ddr1_pmu_dpi_en;              /* DDR Control Register (0x10) */
  volatile unsigned int ddr1_pmu_dpi_lpf;             /* DDR Control Register (0x14) */
  volatile unsigned int ddr1_pmu_dpi_pll;             /* DDR Control Register (0x18) */
  volatile unsigned int ddr1_pmu_dpi_post_pi_sel;     /* DDR Control Register (0x20) */
  volatile unsigned int ddr1_pmu_dpi_time;            /* DDR Control Register (0x24) */
  volatile unsigned int ddr1_pmu_dpi_sel;             /* DDR Control Register (0x28) */
  volatile unsigned int ddr1_pmu_dpi_code;            /* DDR Control Register (0x2c) */
  volatile unsigned int ddr1_pmu_dpi_misc0;           /* DDR Control Register (0x30) */
  volatile unsigned int ddr1_pmu_dpi_misc1;           /* DDR Control Register (0x34) */
  volatile unsigned int ddr1_pmu_dpi_dummy;           /* DDR Control Register (0x38) */
};

struct dram_info {
  enum dram_type         device_type;   
  enum dram_page_size    page;   
  enum dram_bank_size    bank;   
  enum dram_dq_width     dq_width;

} ;

struct dram_mode_reg_info {
  enum mode0_bst_len     bst_len;
  enum mode0_bst_type    bst_type;
  //enum mode0_cas         rd_cas;
  INT32U mode0_cas;
  INT32U mode0_wr;
  INT32U mode1_dll_en_n;
  INT32U mode1_all_lat;
  INT32U mode2_cwl;
  INT32U dram_par_lat;      // for DDR4: paraity_lat
  INT32U dram_wr_lat; 
  INT32U dram_rd_lat;
  INT32U dram_add_lat;
};

struct dram_timing_info {
  INT32U  trfc_ps;
  INT32U  trefi_ps;
  INT32U  wr_max_tck;
  INT32U  trcd_ps; 
  INT32U  trp_ps;
  INT32U  tras_ps;
  INT32U  trrd_tck;
  INT32U  twr_ps;
  INT32U  twtr_tck; 
  //INT32U  trtp_ps;
  INT32U  tmrd_tck;
  INT32U  trtp_tck;
  INT32U  tccd_tck;
  INT32U  trc_ps;
};

struct dram_device_info {
  struct   dram_info          *dev;
  struct   dram_mode_reg_info *mode_reg;
  struct   dram_timing_info   *timing;
  INT32U ddr_period_ps;
  enum     dfi_ratio_type     dfi_rate; 
};


struct dpi_dll_portmap { // BASE_ADDRESS :0xBB103000
  volatile unsigned int ADR_DLY_0;                 /* 000 */
  volatile unsigned int ADR_DLY_1;                 /* 004 */
  volatile unsigned int ADR_DLY_2;                 /* 008 */
  volatile unsigned int CMD_DLY_0;                 /* 00C */
  volatile unsigned int PAD_CMD;                   /* 010 */
  volatile unsigned int PAD_BK_ADR;                /* 014 */
  volatile unsigned int PAD_DQ_0;                  /* 018 */
  volatile unsigned int PAD_DQ_1;                  /* 01C */
  volatile unsigned int reserved_0[1];           
  volatile unsigned int CMD_CTRL;                  /* 024 */
  volatile unsigned int WRITE_CTRL;                /* 028 */
  volatile unsigned int TEST_CTRL0;                /* 02C */
  volatile unsigned int DQ_DLY_0_0;                /* 030 */
  volatile unsigned int DQ_DLY_0_1;                /* 034 */
  volatile unsigned int reserved_1[2];           
  volatile unsigned int DQ_DLY_1;                  /* 040 */
  volatile unsigned int READ_CTRL_0_0;             /* 044 */
  volatile unsigned int READ_CTRL_0_1;             /* 048 */
  volatile unsigned int reserved_2[2];           
  volatile unsigned int READ_CTRL_1;               /* 054 */
  volatile unsigned int READ_CTRL_2;               /* 058 */
  volatile unsigned int PAD_DQS_0;                 /* 05C */
  volatile unsigned int PAD_DQS_1;                 /* 060 */
  volatile unsigned int PAD_DCK;                   /* 064 */
  volatile unsigned int CAL_SHIFT_CTRL;            /* 068 */
  volatile unsigned int CAL_LS_SEL_0;              /* 06C */
  volatile unsigned int CAL_LS_SEL_1;              /* 070 */
  volatile unsigned int reserved_3[2];           
  volatile unsigned int CAL_RS_SEL_0;              /* 07C */
  volatile unsigned int CAL_RS_SEL_1;              /* 080 */
  volatile unsigned int reserved_4[2];           
  volatile unsigned int DQS_IN_DLY_0_0;            /* 08C */
  volatile unsigned int DQS_IN_DLY_0_1;            /* 090 */
  volatile unsigned int reserved_5[2];           
  volatile unsigned int DQS_IN_DLY_1_0;            /* 09C */
  volatile unsigned int DQS_IN_DLY_1_1;            /* 0A0 */
  volatile unsigned int reserved_6[2];           
  volatile unsigned int DQS_IN_DLY_2_0;            /* 0AC */
  volatile unsigned int DQS_IN_DLY_2_1;            /* 0B0 */
  volatile unsigned int reserved_7[2];           
  volatile unsigned int DQS_IN_DLY_3_0;            /* 0BC */
  volatile unsigned int DQS_IN_DLY_3_1;            /* 0C0 */
  volatile unsigned int reserved_8[3];           
  volatile unsigned int PAD_REF;                   /* 0D0 */
  volatile unsigned int reserved_9[17];           
  volatile unsigned int DPI_CTRL_0;                /* 118 */
  volatile unsigned int DPI_CTRL_1;                /* 11C */
  volatile unsigned int CAL_OUT_SEL;               /* 120 */
  volatile unsigned int CAL_OUT_0_0;               /* 124 */
  volatile unsigned int CAL_OUT_0_1;               /* 128 */
  volatile unsigned int reserved_10[2];           
  volatile unsigned int WR_FIFO_0_0;               /* 134 */
  volatile unsigned int WR_FIFO_0_1;               /* 138 */
  volatile unsigned int reserved_11[2];           
  volatile unsigned int WR_FIFO_1_0;               /* 144 */
  volatile unsigned int WR_FIFO_1_1;               /* 148 */
  volatile unsigned int reserved_12[2];           
  volatile unsigned int RD_FIFO_0_0;               /* 154 */
  volatile unsigned int RD_FIFO_0_1;               /* 158 */
  volatile unsigned int reserved_13[2];           
  volatile unsigned int RD_FIFO_1_0;               /* 164 */
  volatile unsigned int RD_FIFO_1_1;               /* 168 */
  volatile unsigned int reserved_14[4];           
  volatile unsigned int TEST_CTRL1;                /* 17C */
  volatile unsigned int DQ_DLY_2;                  /* 180 */
  volatile unsigned int reserved_15[4];           
  volatile unsigned int CRT_ALL;                   /* 194 */
  volatile unsigned int BIST_DEBUG;                /* 198 */
  volatile unsigned int reserved_16[1];           
  volatile unsigned int BIST_2TO1_0;               /* 1A0 */
  volatile unsigned int BIST_2TO1_1;               /* 1A4 */
  volatile unsigned int BIST_PT;                   /* 1A8 */
  volatile unsigned int INT_CTRL;                  /* 1AC */
  volatile unsigned int INT_STATUS_0;              /* 1B0 */
  volatile unsigned int INT_STATUS_1_0;            /* 1B4 */
  volatile unsigned int INT_STATUS_1_1;            /* 1B8 */
  volatile unsigned int reserved_17[4];           
  volatile unsigned int DUMMY_0;                   /* 1CC */
  volatile unsigned int DUMMY_1;                   /* 1D0 */
  volatile unsigned int DUMMY_2;                   /* 1D4 */
  volatile unsigned int DUMMY_3;                   /* 1D8 */
}; // dpi_dll_portmap*/



#define PCTL_CCR_INIT_BFO          0
#define PCTL_CCR_INIT_BFW          1
#define PCTL_CCR_DTT_BFO           1
#define PCTL_CCR_DTT_BFW           1
#define PCTL_CCR_BTT_BFO           2
#define PCTL_CCR_BTT_BFW           1
#define PCTL_CCR_DPIT_BFO          3 
#define PCTL_CCR_DPIT_BFW          1
#define PCTL_CCR_FLUSH_FIFO_BFO    8 
#define PCTL_CCR_FLUSH_FIFO_BFW    1

#define PCTL_DCR_DDR3_BFO          0
#define PCTL_DCR_DDR3_BFW          1
#define PCTL_DCR_SDR_BFO           1
#define PCTL_DCR_SDR_BFW           1
//#define PCTL_DCR_DQ32_BFO          2
//#define PCTL_DCR_DQ32_BFW          1
//#define PCTL_DCR_DFI_RATE_BFO      4
//#define PCTL_DCR_DFI_RATE_BFW      3




#define PCTL_DCR_DDR_BFO         0
#define PCTL_DCR_DDR_BFW           4
#define PCTL_DCR_DQ32_BFO          4
#define PCTL_DCR_DQ32_BFW          2
#define PCTL_DCR_DFI_RATE_BFO      8
#define PCTL_DCR_DFI_RATE_BFW      3

#define PCTL_IOCR_RD_PIPE_BFO      8
#define PCTL_IOCR_RD_PIPE_BFW      4
#define PCTL_IOCR_TPHY_WD_BFO      12
#define PCTL_IOCR_TPHY_WD_BFW      5
#define PCTL_IOCR_TPHY_WL_BFO      17
#define PCTL_IOCR_TPHY_WL_BFW      3
#define PCTL_IOCR_TPHY_RD_EN_BFO   20
#define PCTL_IOCR_TPHY_RD_EN_BFW   5

#define PCTL_CSR_MEM_IDLE_BFO      8
#define PCTL_CSR_MEM_IDLE_BFW      1
#define PCTL_CSR_DT_IDLE_BFO       9 
#define PCTL_CSR_DT_IDLE_BFW       1
#define PCTL_CSR_BIST_IDLE_BFO     10
#define PCTL_CSR_BIST_IDLE_BFW     1
#define PCTL_CSR_DT_FAIL_BFO       11
#define PCTL_CSR_DT_FAIL_BFW       1
#define PCTL_CSR_BT_FAIL_BFO       12
#define PCTL_CSR_BT_FAIL_BFW       1

#define PCTL_DRR_TRFC_BFO          0
#define PCTL_DRR_TRFC_BFW          7
#define PCTL_DRR_TREF_BFO          8 
#define PCTL_DRR_TREF_BFW          24
#define PCTL_DRR_REF_NUM_BFO       24
#define PCTL_DRR_REF_NUM_BFW       4
#define PCTL_DRR_REF_DIS_BFO       28
#define PCTL_DRR_REF_DIS_BFW       1

#define PCTL_TPR0_TRP_BFO          0
#define PCTL_TPR0_TRP_BFW          4
#define PCTL_TPR0_TRAS_BFO         4 
#define PCTL_TPR0_TRAS_BFW         5 
#define PCTL_TPR0_TWR_BFO          9 
#define PCTL_TPR0_TWR_BFW          4
#define PCTL_TPR0_TRTP_BFO         13
#define PCTL_TPR0_TRTP_BFW         3

#define PCTL_TPR1_TRRD_BFO         0
#define PCTL_TPR1_TRRD_BFW         4
#define PCTL_TPR1_TRC_BFO          4 
#define PCTL_TPR1_TRC_BFW          6 
#define PCTL_TPR1_TRCD_BFO         10
#define PCTL_TPR1_TRCD_BFW         4
#define PCTL_TPR1_TCCD_BFO         14
#define PCTL_TPR1_TCCD_BFW         3
#define PCTL_TPR1_TWTR_BFO         17
#define PCTL_TPR1_TWTR_BFW         3
#define PCTL_TPR1_TRTW_BFO         20
#define PCTL_TPR1_TRTW_BFW         4

#define PCTL_TPR2_INIT_REF_NUM_BFO 0
#define PCTL_TPR2_INIT_REF_NUM_BFW 4
#define PCTL_TPR2_INIT_NS_EN_BFO   4 
#define PCTL_TPR2_INIT_NS_EN_BFW   1 
#define PCTL_TPR2_TMRD_BFO         5 
#define PCTL_TPR2_TMRD_BFW         2

#define PCTL_MRINFO_WR_LAT_BFO     0 
#define PCTL_MRINFO_WR_LAT_BFW     5
#define PCTL_MRINFO_RD_LAT_BFO     5 
#define PCTL_MRINFO_RD_LAT_BFW     5
#define PCTL_MRINFO_ADD_LAT_BFO    10
#define PCTL_MRINFO_ADD_LAT_BFW    5

#define PCTL_MR_BL_BFO             0 
#define PCTL_MR_BL_BFW             3
#define PCTL_MR_BT_BFO             3 
#define PCTL_MR_BT_BFW             1
#define PCTL_MR_CAS_BFO            4 
#define PCTL_MR_CAS_BFW            3
#define PCTL_MR_OP_BFO             8 
#define PCTL_MR_OP_BFW             12

#define PCTL_EMR_ADDLAT_BFO        3 
#define PCTL_EMR_ADDLAT_BFW        3

#define PCTL_CMD_DPIN_RSTN_BFO     0 
#define PCTL_CMD_DPIN_RSTN_BFW     1
#define PCTL_CMD_DPIN_CKE_BFO      1 
#define PCTL_CMD_DPIN_CKE_BFW      1
#define PCTL_CMD_DPIN_ODT_BFO      2 
#define PCTL_CMD_DPIN_ODT_BFW      1 

#define PCTL_BCR_STOP_BFO          0 
#define PCTL_BCR_STOP_BFW          1
#define PCTL_BCR_CMP_BFO           1 
#define PCTL_BCR_CMP_BFW           1
#define PCTL_BCR_LOOP_BFO          2 
#define PCTL_BCR_LOOP_BFW          1 
#define PCTL_BCR_DIS_MASK_BFO      3 
#define PCTL_BCR_DIS_MASK_BFW      1
#define PCTL_BCR_AT_STOP_BFO       4 
#define PCTL_BCR_AT_STOP_BFW       1
#define PCTL_BCR_FLUSH_CMD_BFO     8 
#define PCTL_BCR_FLUSH_CMD_BFW     1
#define PCTL_BCR_FLUSH_WD_BFO      9 
#define PCTL_BCR_FLUSH_WD_BFW      1
#define PCTL_BCR_FLUSH_RGD_BFO     10
#define PCTL_BCR_FLUSH_RGD_BFW     1
#define PCTL_BCR_FLUSH_RD_BFO      11
#define PCTL_BCR_FLUSH_RD_BFW      1
#define PCTL_BCR_FLUSH_RD_EXPC_BFO 16
#define PCTL_BCR_FLUSH_RD_EXPC_BFW 14

#define PCTL_BST_ERR_FST_TH_BFO    0 
#define PCTL_BST_ERR_FST_TH_BFW    12
#define PCTL_BST_ERR_CNT_BFO       16
#define PCTL_BST_ERR_CNT_BFW       14

#define PCTL_BSRAM0_CMD_LEVEL_BFO  0 
#define PCTL_BSRAM0_CMD_LEVEL_BFW  12
#define PCTL_BSRAM0_WD_LEVEL_BFO   16
#define PCTL_BSRAM0_WD_LEVEL_BFW   14

#define PCTL_BSRAM1_RG_LEVEL_BFO   0 
#define PCTL_BSRAM1_RG_LEVEL_BFW   14
#define PCTL_BSRAM1_RD_LEVEL_BFO   16
#define PCTL_BSRAM1_RD_LEVEL_BFW   14

#define WRAP_MISC_PAGE_SIZE_BFO    0 
#define WRAP_MISC_PAGE_SIZE_BFW    4
#define WRAP_MISC_BANK_SIZE_BFO    4 
#define WRAP_MISC_BANK_SIZE_BFW    2
#define WRAP_MISC_BST_SIZE_BFO     6 
#define WRAP_MISC_BST_SIZE_BFW     2 
#define WRAP_MISC_DDR_PARAL_BFO    8 
#define WRAP_MISC_DDR_PARAL_BFW    1

struct ms_rxi310_portmap {
  volatile unsigned int ccr;           /* 0x000 */
  volatile unsigned int dcr;           /* 0x004 */
  volatile unsigned int iocr;          /* 0x008 */
  volatile unsigned int csr;           /* 0x00c */
  volatile unsigned int drr;           /* 0x010 */
  volatile unsigned int tpr0;          /* 0x014 */
  volatile unsigned int tpr1;          /* 0x018 */
  volatile unsigned int tpr2;          /* 0x01c */
  volatile unsigned int tpr3;          /* 0x020 */
  volatile unsigned int cdpin;         /* 0x024 */
  volatile unsigned int tdpin;         /* 0x028 */
  volatile unsigned int reserved0[1];
  volatile unsigned int mrinfo;        /* 0x030 */
  volatile unsigned int mr0;           /* 0x034 */
  volatile unsigned int mr1;           /* 0x038 */
  volatile unsigned int mr2;           /* 0x03c */
  volatile unsigned int mr3;           /* 0x040 */
  volatile unsigned int mr4;           /* 0x044 */
  volatile unsigned int mr5;           /* 0x048 */
  volatile unsigned int mr6;           /* 0x04c */
  volatile unsigned int mr7;           /* 0x050 */
  volatile unsigned int dllcr4;        /* 0x054 */
  volatile unsigned int dllcr5;        /* 0x058 */
  volatile unsigned int dllcr6;        /* 0x05c */
  volatile unsigned int dllcr7;        /* 0x060 */
  volatile unsigned int dllcr8;        /* 0x064 */
  volatile unsigned int dqtr0;         /* 0x068 */
  volatile unsigned int dqtr1;         /* 0x06c */
  volatile unsigned int dqtr2;         /* 0x070 */
  volatile unsigned int dqtr3;         /* 0x074 */
  volatile unsigned int dqtr4;         /* 0x078 */
  volatile unsigned int dqtr5;         /* 0x07c */
  volatile unsigned int dqtr6;         /* 0x080 */
  volatile unsigned int dqtr7;         /* 0x084 */
  volatile unsigned int dqstr;         /* 0x088 */
  volatile unsigned int dqsbtr;        /* 0x08c */
  volatile unsigned int odtcr;         /* 0x090 */
  volatile unsigned int dtr0;          /* 0x094 */
  volatile unsigned int dtr1;          /* 0x098 */
  volatile unsigned int dtar;          /* 0x09c */
  volatile unsigned int zqcr0;         /* 0x0a0 */
  volatile unsigned int zqcr1;         /* 0x0a4 */
  volatile unsigned int zqsr;          /* 0x0a8 */
  volatile unsigned int rslr0;         /* 0x0ac */
  volatile unsigned int rslr1;         /* 0x0b0 */
  volatile unsigned int rslr2;         /* 0x0b4 */
  volatile unsigned int rslr3;         /* 0x0b8 */
  volatile unsigned int rdgr0;         /* 0x0bc */
  volatile unsigned int rdgr1;         /* 0x0c0 */
  volatile unsigned int rdgr2;         /* 0x0c4 */
  volatile unsigned int rdgr3;         /* 0x0c8 */
  volatile unsigned int mxsl;          /* 0x0cc */
  volatile unsigned int bcr;           /* 0x0d0 */
  volatile unsigned int bct;           /* 0x0d4 */
  volatile unsigned int bcm;           /* 0x0d8 */
  volatile unsigned int bst;           /* 0x0dc */
  volatile unsigned int bsram0;        /* 0x0e0 */
  volatile unsigned int bsram1;        /* 0x0e4 */
  volatile unsigned int ber;           /* 0x0e8 */
  volatile unsigned int byr;           /* 0x0ec */
  volatile unsigned int pctl_svn;      /* 0x0f0 */
  volatile unsigned int pctl_idr;      /* 0x0f8 */
  volatile unsigned int err;           /* 0x0fc */

  // SDR_PHY CONTROL REGISTER
  volatile unsigned int phy_dly0;      /* 0x100 */
  volatile unsigned int phy_dly1_rst;  /* 0x104 */
  volatile unsigned int phy_dly_clk;   /* 0x108 */
  volatile unsigned int phy_dly_st;    /* 0x10c */
  volatile unsigned int phy_dly_num;   /* 0x110 */
  volatile unsigned int reserved1[69];

  // WRAP CONTROL REGISTER
  volatile unsigned int misc;          /* 0x224 */
  volatile unsigned int cq_ver;        /* 0x228 */
  volatile unsigned int cq_mon;        /* 0x22c */
  volatile unsigned int wq_ver;        /* 0x230 */
  volatile unsigned int wq_mon;        /* 0x234 */
  volatile unsigned int rq_ver;        /* 0x240 */
  volatile unsigned int rq_mon;        /* 0x244 */
  volatile unsigned int reserved2[22];          
  volatile unsigned int wwrap_idr;     /* 0x2a0 */
  volatile unsigned int wrap_svn;      /* 0x2a4 */
}; //ms_rxi310_portmap


#define WRITE_MEM32(addr,data) 	(*(INT32U *)(addr) = (INT32U) (data) )
#define READ_MEM32(addr) 		(*(INT32U *)(addr))


#define TEST_PASS				0
#define SCAN01_TEST_0_ERR		1
#define SCAN01_TEST_1_ERR		2
#define CHECK_BOARD_55_ERR		3
#define CHECK_BOARD_AA_ERR		4
#define DATABUS_CHECK_ERR		5
#define WALKING_BACKGROUND_0_ERR 12
#define WALKING_BACKGROUND_1_ERR 13
#define GALPAT_BACKGROUND_0_ERR	6
#define GALPAT_BACKGROUND_1_ERR	7
#define MATCH_FORWARD_ERR		8
#define MATCH_BACK_ERR			9
#define MATCH_0_ERR				10
#define ADDRESS_TEST_ERR		11
#define BUS_TOGGLE_ERR			12
#define DM_TOGGLE_ERR			13


enum item_num
{
	ITEM_DATABUS_CHECK = 0,
	ITEM_SCAN01,
	ITEM_CHECKBOARD,
	ITEM_GALPAT,
	ITEM_WALKING10,
	ITEM_MARCH,
	ITEM_ADDRESS,
	ITEM_END
};

void write_mem_mask(INT32U addr, INT32U mask, INT8U value);
INT32U read_mem_mask(INT32U addr, INT32U mask);
void update_process_info(INT8U item, INT8U ratio);
INT32U mem_test_databus(INT32U empty_addr);
INT32U mem_test_scan01(INT32U start_addr, INT32U end_addr);
INT32U mem_test_checkboard(INT32U start_addr, INT32U end_addr);
INT32U mem_test_GALPAT(INT32U start_addr, INT32U end_addr);
INT32U mem_test_walking10(INT32U start_addr, INT32U end_addr);
INT32U mem_test_march(INT32U start_addr, INT32U end_addr);
INT32U mem_test_address(INT32U start_addr, INT32U end_addr);

void dpi_dll_ddr1_enable (struct ddr1_pmu_portmap *dev_map);
void dpi_dll_ddr1_init (struct dpi_dll_portmap *dll_map, struct dram_device_info *dram_info);
void dram_init (struct ms_rxi310_portmap *dev_map,struct dram_device_info *dram_info);
INT32U dram_init_all(void);
INT32U dram_test(INT32U start_addr, INT32U end_addr);
INT32U dram_test_short(INT32U start_addr, INT32U end_addr,INT32U val);
INT32U dram_test_stabel(void);

void DDR_Test_Task(void);


#define DDR 0
void ddrcode();
void dram_dumy();

#endif

