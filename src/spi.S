#include "bsp_cfg.h"
#include "rlx/rlx_cpu_regs.h"

##the SSI_BASE is pass parameter

	.globl  ssi_wait_nobusy
	.ent    ssi_wait_nobusy
ssi_wait_nobusy:
        addiu	sp,sp,-8
        sw      ra, 8(sp)

wait_ssi:
        lw      t0, 40(a0)
        li      t1, 0x04 
        andi    t0, t0, 0x05	
	bne     t0, t1, wait_ssi
	nop

        lw      ra, 8(sp)
        addiu	sp,sp,8
        jr      ra                         #Return
	.end    ssi_wait_nobusy
######################################################################

        .globl  spi_get_status
        .ent    spi_get_status
spi_get_status:
        #a: SSI_base
        #v0: return value
        #t0, t1: using   

        addiu	sp,sp,-8
        sw      ra, 8(sp)

        sw      zero, 0x8(a0)              #disable SSI
        li      t0, 0x73c7                 
        sw      t0, 0x0(a0)                #set CTRLR0: EEPROM
        sw      zero, 0x4(a0)              #set read 1 of data frame
        li      t0, 0x1                    #enable SER               
        sw      t0, 0x10(a0)      
        li      t1, 0x1                       
        sw      t1, 0x8(a0)                #enable SSI

        li      t1, FLASH_RDSR_COM         #set read status command of SPI 
        sw      t1, 0x60(a0)
         
        jal     ssi_wait_nobusy       
        nop
        jal     spi_wait_busy
        nop

        or      t0, zero, a0               
        lw      v0, 0x60(a0)               #read read status
        nop

        lw      ra, 8(sp)
        addiu	sp,sp,8
        jr      ra                         #Return

        .end spi_get_status   
######################################################################
        .globl  spi_chip_erase
        .ent    spi_chip_erase
spi_chip_erase:
        #a0: SSI_BASE
        #t0: using    

        addiu	sp,sp,-8
        sw      ra, 8(sp)

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSI   
        li      t0, 0x71c7
        sw      t0, 0x0(a0)                #set CTRLR0: Transmit only 
        li      t0, 0x1                 
        sw      t0, 0x10(a0)               #enable SER      
        li      t0, 0x1
        sw      t0, 0x8(a0)                #enable SSI

        li      t0, FLASH_WREN_COM
        sw      t0, 0x60(a0)               #write enable of SPI
    
        jal     ssi_wait_nobusy
        nop

        li      t0, FLASH_CE_COM
        sw      t0, 0x60(a0)               #set chip earse command of SPI
        
        jal     ssi_wait_nobusy
        nop
        jal     spi_enable_write           #enable WEL and pass data to set_status
        nop

        lw      ra, 8(sp)
        addiu	sp,sp,8
        jr      ra                         #Return
        .end spi_chip_erase  
 ######################################################################
        .globl  spi_blk_erase
        .ent    spi_blk_erase
spi_blk_erase:
        #a0: SSI_BASE
        #a1: address
        #t0, t2, t3, t4: using

        addiu	sp,sp,-8
        sw      ra, 8(sp)

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSIENR
        li      t0, 0x71c7
        sw      t0, 0x0(a0)                #set CTRLR0: transmit only
        li      t0, 0x1                 
        sw      t0, 0x10(a0)               #enable SER      
        li      t0, 0x1
        sw      t0, 0x8(a0)                #enable SSIENR

        li      t0, FLASH_WREN_COM
        sw      t0, 0x60(a0)               #enable write of SPI
    
        jal     ssi_wait_nobusy
        nop
   
        sw      zero, 0x10(a0)             #disable SER    
        
        #issue WR
        li      t0, FLASH_BE_COM
        andi    t2, a1,  0xff              #addr (xx_xx--)   
        srl     a1, a1, 8                  
        andi    t3, a1,  0xff              #addr (xx_--xx) >>8
        srl     a1, a1, 8
        andi    t4, a1,  0xff              #addr (--_xxxx) >>16

        #cmd
        sw      t0, 0x60(a0)               #set sector_erase command of SPI
        #addr
        sw      t4, 0x60(a0)
        sw      t3, 0x60(a0)
        sw      t2, 0x60(a0)
       
        li      t0, 0x1                    #enable SER               
        sw      t0, 0x10(a0)      

        jal     ssi_wait_nobusy
        nop
        jal     spi_wait_busy               
        nop

        lw      ra, 8(sp)
        addiu	sp,sp,8
        jr      ra                          #Return
        .end spi_blk_erase   
 ######################################################################
        .globl  spi_command
        .ent    spi_command
spi_command:
        #a0: SSI_BASE
        #a1: address
        #a2: gernal command
        #t0, t2, t3, t4: using

        addiu	sp,sp,-8
        sw      ra, 8(sp)

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSIENR
        li      t0, 0x71c7
        sw      t0, 0x0(a0)                #set CTRLR0: transmit only
        li      t0, 0x1                 
        sw      t0, 0x10(a0)               #enable SER      
        li      t0, 0x1
        sw      t0, 0x8(a0)                #enable SSIENR

        li      t0, FLASH_WREN_COM
        sw      t0, 0x60(a0)               #enable write of SPI
    
        jal     ssi_wait_nobusy
        nop
   
        sw      zero, 0x10(a0)             #disable SER    
        
        #issue WR
        add     t0, zero, a2
        andi    t2, a1,  0xff              #addr (xx_xx--)   
        srl     a1, a1, 8                  
        andi    t3, a1,  0xff              #addr (xx_--xx) >>8
        srl     a1, a1, 8
        andi    t4, a1,  0xff              #addr (--_xxxx) >>16

        #cmd
        sw      t0, 0x60(a0)               #set sector_erase command of SPI
        #addr
        sw      t4, 0x60(a0)
        sw      t3, 0x60(a0)
        sw      t2, 0x60(a0)
       
        li      t0, 0x1                    #enable SER               
        sw      t0, 0x10(a0)      

        jal     ssi_wait_nobusy
        nop
        jal     spi_wait_busy               
        nop

        lw      ra, 8(sp)
        addiu	sp,sp,8
        jr      ra                          #Return
        .end spi_command   
 ######################################################################
        .globl  spi_se_unprotect
        .ent    spi_se_unprotect
spi_se_unprotect:
        #a0: SSI_BASE
        #a1: address
        #t0, t2, t3, t4: using

        addiu	sp,sp,-8
        sw      ra, 8(sp)

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSIENR
        li      t0, 0x71c7
        sw      t0, 0x0(a0)                #set CTRLR0: transmit only
        li      t0, 0x1                 
        sw      t0, 0x10(a0)               #enable SER      
        li      t0, 0x1
        sw      t0, 0x8(a0)                #enable SSIENR

        li      t0, FLASH_WREN_COM
        sw      t0, 0x60(a0)               #enable write of SPI
    
        jal     ssi_wait_nobusy
        nop
   
        sw      zero, 0x10(a0)             #disable SER    
        
        #issue WR
        li      t0, FLASH_SE_UNPROTECT
        andi    t2, a1,  0xff              #addr (xx_xx--)   
        srl     a1, a1, 8                  
        andi    t3, a1,  0xff              #addr (xx_--xx) >>8
        srl     a1, a1, 8
        andi    t4, a1,  0xff              #addr (--_xxxx) >>16

        #cmd
        sw      t0, 0x60(a0)               #set sector_erase command of SPI
        #addr
        sw      t4, 0x60(a0)
        sw      t3, 0x60(a0)
        sw      t2, 0x60(a0)
       
        li      t0, 0x1                    #enable SER               
        sw      t0, 0x10(a0)      

        jal     ssi_wait_nobusy
        nop
        jal     spi_wait_busy               
        nop

        lw      ra, 8(sp)
        addiu	sp,sp,8
        jr      ra                          #Return
        .end spi_se_unprotect   
 ######################################################################
        .globl  spi_se_protect
        .ent    spi_se_protect
spi_se_protect:
        #a0: SSI_BASE
        #a1: address
        #t0, t2, t3, t4: using

        addiu	sp,sp,-8
        sw      ra, 8(sp)

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSIENR
        li      t0, 0x71c7
        sw      t0, 0x0(a0)                #set CTRLR0: transmit only
        li      t0, 0x1                 
        sw      t0, 0x10(a0)               #enable SER      
        li      t0, 0x1
        sw      t0, 0x8(a0)                #enable SSIENR

        li      t0, FLASH_WREN_COM
        sw      t0, 0x60(a0)               #enable write of SPI
    
        jal     ssi_wait_nobusy
        nop
   
        sw      zero, 0x10(a0)             #disable SER    
        
        #issue WR
        li      t0, FLASH_SE_PROTECT
        andi    t2, a1,  0xff              #addr (xx_xx--)   
        srl     a1, a1, 8                  
        andi    t3, a1,  0xff              #addr (xx_--xx) >>8
        srl     a1, a1, 8
        andi    t4, a1,  0xff              #addr (--_xxxx) >>16

        #cmd
        sw      t0, 0x60(a0)               #set sector_erase command of SPI
        #addr
        sw      t4, 0x60(a0)
        sw      t3, 0x60(a0)
        sw      t2, 0x60(a0)
       
        li      t0, 0x1                    #enable SER               
        sw      t0, 0x10(a0)      

        jal     ssi_wait_nobusy
        nop
        jal     spi_wait_busy               
        nop

        lw      ra, 8(sp)
        addiu	sp,sp,8
        jr      ra                          #Return
        .end spi_se_protect   
 ######################################################################
        .globl  spi_se_erase
        .ent    spi_se_erase
spi_se_erase:
        #a0: SSI_BASE
        #a1: address
        #t0, t2, t3, t4: using

        addiu	sp,sp,-8
        sw      ra, 8(sp)

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSIENR
        li      t0, 0x71c7
        sw      t0, 0x0(a0)                #set CTRLR0: transmit only
        li      t0, 0x1                 
        sw      t0, 0x10(a0)               #enable SER      
        li      t0, 0x1
        sw      t0, 0x8(a0)                #enable SSIENR

        li      t0, FLASH_WREN_COM
        sw      t0, 0x60(a0)               #enable write of SPI
    
        jal     ssi_wait_nobusy
        nop
   
        sw      zero, 0x10(a0)             #disable SER    
        
        #issue WR
        li      t0, FLASH_SE_COM
        andi    t2, a1,  0xff              #addr (xx_xx--)   
        srl     a1, a1, 8                  
        andi    t3, a1,  0xff              #addr (xx_--xx) >>8
        srl     a1, a1, 8
        andi    t4, a1,  0xff              #addr (--_xxxx) >>16

        #cmd
        sw      t0, 0x60(a0)               #set sector_erase command of SPI
        #addr
        sw      t4, 0x60(a0)
        sw      t3, 0x60(a0)
        sw      t2, 0x60(a0)
       
        li      t0, 0x1                    #enable SER               
        sw      t0, 0x10(a0)      

        jal     ssi_wait_nobusy
        nop
        jal     spi_wait_busy               
        nop

        lw      ra, 8(sp)
        addiu	sp,sp,8
        jr      ra                          #Return
        .end spi_se_erase   
######################################################################
        .globl  spi_wait_busy
        .ent    spi_wait_busy

spi_wait_busy:
        #a0: SSI_BASE
        #t0: using

        addiu	sp,sp,-8
        sw      ra, 8(sp)
        
        sw      zero, 0x8(a0)              #disable SSIENR
        sw      zero, 0x4(a0)                #set read 3 of data frame
        li      t0, 0x73c7
        sw      t0, 0x0(a0)                #set CTRLR0: EEPROM mode
        li      t0, 0x1
        sw      t0, 0x10(a0)               #enable SER
        li      t0, 0x1
        sw      t0, 0x8(a0)                #enable SSIENR
    
__checkstat2:
        li      t0, FLASH_RDSR_COM                       
        sw      t0, 0x60(a0)               #set read status command of SPI

        jal     ssi_wait_nobusy
        nop

        lw      t0, 0x60(a0)               #load status of SPI
        nop

        andi    t0, t0, 0x01               #check SPI is not write progress
        bne     t0, zero, __checkstat2   
        nop

        lw      ra, 8(sp)
        addiu	sp,sp,8
        jr      ra                         #Return
        .end spi_wait_busy   
######################################################################
        .globl  spi_set_status
        .ent    spi_set_status

spi_set_status:
        #a0: SSI_BASE, 
        #a1: set number to status register
        #t0: using

        addiu	sp,sp,-8
        sw      ra, 8(sp)

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSIENR
        li      t0, 0x71c7
        sw      t0, 0x0(a0)                #set CTRLR0: transmit only
        li      t0, 0x1
        sw      t0, 0x10(a0)               #enable SER
        li      t0, 0x1
        sw      t0, 0x8(a0)                #enable SSIER

        li      t0, FLASH_WREN_COM
        sw      t0, 0x60(a0)               #enable write of SPI

        jal     ssi_wait_nobusy
        nop

        sw      zero, 0x10(a0)             #disable SER
      
        li      t0, FLASH_WRSR_COM                      
        sw      t0, 0x60(a0)               #set write status command of SPI
        sw      a1, 0x60(a0)               #set status value

        li      t0, 0x1
        sw      t0, 0x10(a0)               #enable SER

        jal     ssi_wait_nobusy
        nop
        jal     spi_wait_busy
        nop
        
        lw      ra, 8(sp)
        addiu	sp,sp,8
        jr      ra                         # Return
        .end spi_set_status 
######################################################################
        .globl  spi_dis_write
        .ent    spi_dis_write 

spi_dis_write: 
        #a0: SSI_BASE 
        #t0: using

        addiu	sp,sp,-8
        sw      ra, 8(sp)

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSIENR
        li      t0, 0x71c7
        sw      t0, 0x0(a0)                #set CTRLR0: transmit only
        li      t0, 0x1            
        sw      t0, 0x10(a0)               #enable SER 
        li      t0, 0x1
        sw      t0, 0x8(a0)                #enable SSIENR

        li      t0, FLASH_WRDI_COM
        sw      t0, 0x60(a0)               #disable write command of SPI
    
        jal     ssi_wait_nobusy
        nop
        jal     spi_wait_busy
        nop

        lw      ra, 8(sp)
        addiu	sp,sp,8
        jr      ra                         #Return
        .end spi_dis_write  
######################################################################
        .globl  spi_enable_write
        .ent    spi_enable_write

spi_enable_write:  
        #a0: SSI_BASE  
        #t0: using

        addiu	sp,sp,-8
        sw      ra, 8(sp)

        #to command issue mode
        sw      zero, 0x8(a0)              #diable SSIENR
        li      t0, 0x71c7 
        sw      t0, 0x0(a0)                #set CTRLR0: transmit only
        li      t0, 0x1          
        sw      t0, 0x10(a0)               #enable SER
        li      t0, 0x1
        sw      t0, 0x8(a0)                #enable SSIENR

        li      t0, FLASH_WREN_COM
        sw      t0, 0x60(a0)               #enable write command of SPI
    
        jal     ssi_wait_nobusy
        nop
        jal     spi_wait_busy
        nop

        lw      ra, 8(sp)
        addiu	sp,sp,8
        jr      ra                         #Return
        .end spi_enable_write  
######################################################################
        .globl  spi_set_protect_mem
        .ent    spi_set_protect_mem

spi_set_protect_mem: 
        #a0: SSI_BASE  
        #a1: set protect memory
        #t0, t1, t2, t3: using

        addiu	sp,sp,-8
        sw      ra, 8(sp)

        jal     spi_get_status             #return status value
        nop

        or      t3, zero, v0               #reserve status value

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSIENR 
        li      t0, 0x71c7            
        sw      t0, 0x0(a0)                #set CTRLR0: transmit only
        li      t0, 0x1
        sw      t0, 0x8(a0)                #enable SSIENR
        
        li      t0, FLASH_WREN_COM
        sw      t0, 0x60(a0)               #enable write command of SPI
    
        jal     ssi_wait_nobusy
        nop
        
        sw      zero, 0x10(a0)             #disable SER    

        li      t0, FLASH_WRSR_COM
        li      t2, 0xe3                   #remove BP2, BP1, BP0
        and     t3, t3, t2
        sll     t2, a1, 2
        or      t2, t3, t2
        sw      t0, 0x60(a0)               #set write status command of SPI
        sw      t2, 0x60(a0)               #new value
       
        sw      t0, 0x10(a0)               #enable SER       

        jal     ssi_wait_nobusy
        nop
        jal     spi_wait_busy
        nop

        lw      ra, 8(sp)
        addiu	sp,sp,8
        jr      ra                         #Return
        .end spi_set_protect_mem
######################################################################
        .globl  spi_read_id
        .ent    spi_read_id

spi_read_id:
        #a0: SSI_BASE  
        #t0: using
        #v0: return ID value(3-byte) 

        addiu	sp,sp,-8
        sw      ra, 8(sp)

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSIENR
        li      t0, 0x73c7            
        sw      t0, 0x0(a0)                #set CTRLR0: EEPROM mode
        li      t0, 0x1           
        sw      t0, 0x10(a0)               #enable SER   
        li      t0, 0x2            
        sw      t0, 0x4(a0)                #set read 3 of data frames 
        li      t0, 0x1
        sw      t0, 0x8(a0)                #enable SSIENR
        
        li      t0, FLASH_RDID_COM
        sw      t0, 0x60(a0)               #set read ID command of SPI
    
        jal     ssi_wait_nobusy
        nop

        lw      v0, 0x60(a0)               #read ID(--_xxxx)
        nop 

        andi    v0, v0, 0xff
        sll     v0, v0, 0x8

        lw      t0, 0x60(a0)               #read ID(--_--xx)
        nop

        andi    t0, t0, 0xff
        or      v0, v0, t0
        sll     v0, v0, 0x8
         
        lw      t0, 0x60(a0)               #read ID(--_----)
        nop

        andi    t0, t0, 0xff
        or      v0, v0, t0

        lw      ra, 8(sp)
        addiu	sp,sp,8
        jr      ra                         # Return
        .end spi_read_id  
######################################################################
        .globl  spi_read_ect_id
        .ent    spi_read_ect_id

spi_read_ect_id:
        ## a0: SSI_BASE
        ## t0, t1, t2, t3: using
        ## v0: return ID value(2-byte)

        addiu	sp,sp,-8
        sw      ra, 8(sp)

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSIENR
        li      t0, 0x73c7            
        sw      t0, 0x0(a0)                #set CTRLR0: EEPROM mode
        li      t0, 0x0                    #to read 1 bytes
        sw      t0, 0x4(a0)      
        li      t0, 0x1
        sw      t0, 0x8(a0)                #enable SSIENR
            
        jal     ssi_wait_nobusy
        nop

        li      t0, 0x0                    #disable slave1            
        sw      t0, 0x10(a0)      

        #cmd + dummy 3write
        li      t0, FLASH_RES_COM
        sw      t0, 0x60(a0)               #set read electronic ID command of SPI
        sb      t1, 0x60(a0)               #write dummy data
        sb      t2, 0x60(a0)               #write dummy data
        sb      t3, 0x60(a0)               #write dummy data

        li      t0, 1
        sw      t0, 0x10(a0)               #enable SER

        jal     ssi_wait_nobusy
        nop

        #read ID(1-btye)
        lw      v0, 0x60(a0)               #load ID value
        nop 
        andi    v0, v0, 0xff

        lw      ra, 8(sp)
        addiu	sp,sp,8
        jr      ra                         # Return
        .end spi_read_ect_id  
######################################################################
        .globl  spi_read_mf_id
        .ent    spi_read_mf_id

spi_read_mf_id:
        # a0: SSI_BASE
        # a1: formate (1: device->manufacturer, 0: manufacturer->device)
        #     2-byte
        # v0: return ID
        # t0, t1, t2: using 
        
        addiu	sp,sp,-8
        sw      ra, 8(sp)

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSIENR
        li      t0, 0x73c7            
        sw      t0, 0x0(a0)                #set CTRLR0: EEPROM mode 
        li      t0, 0x2             
        sw      t0, 0x4(a0)                #to read 3 bytes 
        li      t0, 0x1
        sw      t0, 0x8(a0)                #enable SSIENR
            
        jal     ssi_wait_nobusy
        nop

        sw      zero, 0x10(a0)             #disable SER 

        #cmd + dummy 2write +addr
        li      t0, FLASH_REMS_COM                    
        sw      t0, 0x60(a0)               #set read electronic manuf
        sb      t1, 0x60(a0)               #write dummy data
        sb      t2, 0x60(a0)               #write dummy data               
        sw      a1, 0x60(a0)               #set format                     
  
        li      t0, 0x1           
        sw      t0, 0x10(a0)               #enable SER

        jal     ssi_wait_nobusy
        nop

        #read ID(2-byte)
        lw      v0, 0x60(a0)               #read ID(0x--xx)
        nop
        andi    v0, v0, 0xff
        sll     v0, v0, 0x8
         
        lw      t0, 0x60(a0)               #read ID(0x----)
        nop
        andi    t0, t0, 0xff
        or      v0, v0, t0

        lw      ra, 8(sp)
        addiu	sp,sp,8
        jr      ra                         # Return
        .end spi_read_mf_id   
######################################################################
        .globl  spi_enter_dp
        .ent    spi_enter_dp

spi_enter_dp:
        # a0: SSI_BASE
        # t0: using 

        addiu	sp,sp,-8
        sw      ra, 8(sp)

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSIENR
        li      t0, 0x71c7
        sw      t0, 0x0(a0)                #set CTRLR0: transmit only 
        li      t0, 0x1              
        sw      t0, 0x10(a0)               #enable SER       
        li      t0, 0x1
        sw      t0, 0x8(a0)                #enable SSIENR

        li      t0, FLASH_DP_COM
        sw      t0, 0x60(a0)               #set deep power command of SPI

        jal     ssi_wait_nobusy
        nop

        lw      ra, 8(sp)
        addiu	sp,sp,8
        jr      ra                         #Return
        .end spi_enter_dp   
######################################################################
        .globl  spi_release_dp
        .ent    spi_release_dp

spi_release_dp:
        # a0: SSI_BASE
        # t0: using 

        addiu	sp,sp,-8                   
        sw      ra, 8(sp)

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSIENR
        li      t0, 0x71c7
        sw      t0, 0x0(a0)                #set CTRLR0: transmit only
        li      t0, 0x1              
        sw      t0, 0x10(a0)               #enable SER       
        li      t0, 0x1
        sw      t0, 0x8(a0)                #enable SSIENR

        li      t0, FLASH_RES_COM
        sw      t0, 0x60(a0)               #set release deep power command of SPI
    
        jal     ssi_wait_nobusy
        nop

        lw      ra, 8(sp)
        addiu	sp,sp,8
        jr      ra                         #Return
        .end spi_release_dp   
######################################################################
        .globl  spi_write_byte
        .ent    spi_write_byte

spi_write_byte:
        #a0: SSI_BASE
        #a1: addr (3-byte)
        #a2: data (1-byte)
        #t0, t2, t3, t4: using 

        addiu	sp,sp,-8
        sw      ra, 8(sp)

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSI
        li      t0, 0x71c7
        sw      t0, 0x0(a0)                #set CTRLR0: transmit only
        li      t0, 0x1                        
        sw      t0, 0x10(a0)               #enable SER 
        li      t0, 0x1
        sw      t0, 0x8(a0)                #enable SSI

        #issue WREN
        li      t0, FLASH_WREN_COM 
        sw      t0, 0x60(a0)               #set write command of SPI
    
        jal     ssi_wait_nobusy
        nop

        sw      zero, 0x10(a0)             #disable SER                

        #issue WR
        li      t0, FLASH_PP_COM                    
        andi    t2, a1, 0xff              #addr (xx_xx--)   
        srl     a1, a1, 8                  
        andi    t3, a1, 0xff              #addr (xx_--xx) >>8
        srl     a1, a1, 8
        andi    t4, a1, 0xff              #addr (--_xxxx) >>16

        andi    a2, a2, 0xff              #data (xx_xx--) 

        #cmd
        sw      t0, 0x60(a0)
        #addr
        sw      t4, 0x60(a0)
        sw      t3, 0x60(a0)
        sw      t2, 0x60(a0)
        #data
        sw      a2, 0x60(a0)

        li      t0, 0x1                    #enable SER            
        sw      t0, 0x10(a0)      
        
        jal     ssi_wait_nobusy
        nop
        jal     spi_wait_busy
        nop

        lw      ra, 8(sp)
        addiu	sp,sp,8
        jr      ra                         #Return
        .end spi_write_byte   
######################################################################
        .globl  spi_read_byte
        .ent    spi_read_byte

spi_read_byte: 
        #a0: SSI_BASE
        #a1: addr (3-byte) 
        #v0: return data (1-byte)
        #t0, t2, t3, t4: using

        addiu	sp,sp,-8
        sw      ra, 8(sp)

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSIENR
        li      t0, 0x73c7
        sw      t0, 0x0(a0)                #set CTRLR: transmit only
        li      t0, 0x0            
        sw      t0, 0x4(a0)                #set read 1 of data frames 
        li      t0, 0x1
        sw      t0, 0x8(a0)                #enable SSIENR

        jal     ssi_wait_nobusy
        nop

        #issue READ
        li      t0, FLASH_READ_COM 
    
        #issue WR
        andi    t2, a1,  0xff              #addr (xx_xx--)
        srl     a1, a1, 8
        andi    t3, a1,  0xff              #addr (xx_--xx) >>8
        srl     a1, a1, 8
        andi    t4, a1,  0xff              #addr (--_xxxx) >>16

        sw      zero, 0x10(a0)             #disable SER 

        #cmd(READ) +addr
        sw      t0, 0x60(a0)               #set read data command of SPI
        sw      t4, 0x60(a0)               #set read addr(--_xxxx) >>16
        sw      t3, 0x60(a0)               #set read addr(xx_--xx) >>8
        sw      t2, 0x60(a0)               #set read addr(xx_xx--)
      
        li      t0, 0x1                  
        sw      t0, 0x10(a0)               #enable SER 

        jal     ssi_wait_nobusy
        nop

        #read data
        lw      s0, 0x60(a0)               #read data 1-byte
 
        or      v0, zero, s0

        lw      ra, 8(sp)
        addiu	sp,sp,8
        jr      ra                        #Return
        .end spi_read_byte   
#####################################################################
        .globl  spi_fastread_byte
        .ent    spi_fastread_byte

spi_fastread_byte:
        #a0: SSI_BASE
        #a1: addr (3-byte) 
        #v0: return data (1-byte)
        #t0, t1, t2, t3, t4: using

        addiu	sp,sp,-8
        sw      ra, 8(sp)

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSIENR
        li      t0, 0x73c7
        sw      t0, 0x0(a0)                #set CTRLR0: transmit only
        li      t0, 0x0            
        sw      t0, 0x4(a0)                #set read 1 of data frames 
        li      t0, 0x1
        sw      t0, 0x8(a0)                #enable SSIENR
        
        sw      zero, 0x10(a0)             #disable SER 
    
        #issue WR
        andi    t2, a1, 0xff               #addr (xx_xx--) 
        srl     a1, a1, 8
        andi    t3, a1, 0xff               #addr (xx_--xx) >>8
        srl     a1, a1, 8
        andi    t4, a1, 0xff               #addr (--_xxxx) >>16
byte_raddr:   
        #cmd(fread) + addr + dummy sw
        li      t0, FLASH_FAST_READ_COM
        sw      t0, 0x60(a0)               #set fast read command of SPI
        sw      t4, 0x60(a0)               #set addr (--_xxxx) >>16
        sw      t3, 0x60(a0)               #set addr (xx_--xx) >>8 
        sw      t2, 0x60(a0)               #set addr (xx_xx--) 
        sw      t1, 0x60(a0)               #set dummy data

        li      t0, 0x1                  
        sw      t0, 0x10(a0)               #enable SER 

        jal     ssi_wait_nobusy
        nop
        
        #jal     spi_wait_busy
        #nop

 
        #read data
        lw      s0, 0x60(a0)               #load data (1-byte)
        jal     ssi_wait_nobusy
        nop

        or      v0, zero, s0               #return value
       
        lw      ra, 8(sp)
        addiu	sp,sp,8
        jr      ra                         #Return
        .end spi_fastread_byte   
	.global spi_write
	.ent    spi_write

spi_write:
# at most, one can write 126 bytes in a call to spi_write
# a0: SSI BASE
# a1: addr (3 byte)
# a2: buffer address (4 bytes)
# a3: length for write

        addiu  sp, sp, -8
        sw     ra, 8(sp)

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSIENR
        li      t0, 0x71c7
        sw      t0, 0x0(a0)                #set CTRLR0: transmit only
        li      t0, 0x1                 
        sw      t0, 0x10(a0)               #enable SER 
        li      t0, 0x1
        sw      t0, 0x8(a0)                #enable SSIENR

        li      t0, FLASH_WREN_COM 
        sw      t0, 0x60(a0)               #enable write of SPI
    
        jal     ssi_wait_nobusy
        nop

        sw      zero, 0x10(a0)             #disable SER 

        #issue WR
	li      t0, FLASH_PP_COM
        andi    t2, a1, 0xff               #addr (xx_xx--)
        srl     a1, a1, 8
        andi    t3, a1, 0xff               #addr (xx_--xx) >>8
        srl     a1, a1, 8
        andi    t4, a1, 0xff               #addr (--_xxxx) >>16

        #cmd: PP
        sw      t0, 0x60(a0)               
        #addr
        sw      t4, 0x60(a0)               #set  addr (--_xxxx) >>16
        sw      t3, 0x60(a0)               #set  addr (--_xxxx) >>8 
        sw      t2, 0x60(a0)               #set  addr (--_xxxx)     
        
        #by copying data 
	add     t0, a2, a3
loop:
        lb      t2, 0x0(a2) 
        addi    a2, a2, 1
        sw      t2, 0x60(a0)
        bne     t0, a2, loop 

        li      t0, 0x1                  
        sw      t0, 0x10(a0)               #enable SER 
       
        jal     ssi_wait_nobusy
        nop
	jal     spi_wait_busy
	nop

	lw     ra, 8(sp)
	addiu  sp, sp, 8 
	jr     ra

	.end    spi_write
#####################################################################      
        .globl  spi_write_word
        .ent    spi_write_word

spi_write_word:
        #a0: SSI_BASE
        #a1: addr (3-byte) 
        #a2: data (4-byte)
        #t0, t1, t2, t3, t4, t5: using

        addiu	sp,sp,-32
        sw      ra, 16(sp)

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSIENR
        li      t0, 0x71c7
        sw      t0, 0x0(a0)                #set CTRLR0: transmit only
        li      t0, 0x1                 
        sw      t0, 0x10(a0)               #enable SER 
        li      t0, 0x1
        sw      t0, 0x8(a0)                #enable SSIENR


word_waddr:   
        li      t0, FLASH_WREN_COM 
        sw      t0, 0x60(a0)               #enable write of SPI
    
        jal     ssi_wait_nobusy
        nop

        sw      zero, 0x10(a0)             #disable SER 

        andi    t2, a1, 0xff               #addr (xx_xx--)
        srl     t5, a1, 8
        andi    t3, t5, 0xff               #addr (xx_--xx) >>8
        srl     t5, a1, 8
        andi    t4, t5, 0xff               #addr (--_xxxx) >>16

        #cmd: PP
        li      t1, FLASH_PP_COM
        sw      t1, 0x60(a0)               #set write data command of SPI, WEL is disable after write completly.
        #addr
        sw      t4, 0x60(a0)               #set  addr (--_xxxx) >>16
        sw      t3, 0x60(a0)               #set  addr (--_xxxx) >>8 
        sw      t2, 0x60(a0)               #set  addr (--_xxxx)     

        #byte data:
        andi    t5, a2, 0xff               #data (xxxx_xx--) 
        sw      t5, 0x60(a0)               #wirte data(1-byte)
        
        srl     a2, a2, 8                  #data (xxxx_--) 
        andi    t5, a2, 0xff               #data (xxxx_--) 
        sw      t5, 0x60(a0)               #wirte data(2-byte)
        
        srl     a2, a2, 8                  #data (xx--) 
        andi    t5, a2, 0xff               #data (xx--) 
        sw      t5, 0x60(a0)               #wirte data(3-byte)
       
        srl     a2, a2, 8                  #data (--) 
        andi    t5, a2, 0xff               #data (--) 
        sw      t5, 0x60(a0)               #wirte data(4-byte)

        li      t0, 0x1                  
        sw      t0, 0x10(a0)               #enable SER 
       
        jal     ssi_wait_nobusy
        nop
        
        jal     spi_wait_busy
        nop

        lw      ra, 16(sp)
        addiu	sp,sp,32

        jr      ra                         #Return
        .end spi_write_word   
######################################################################
        .globl  spi_read_word
        .ent    spi_read_word

spi_read_word:
        #a0: SSI_BASE
        #a1: addr 
        #v0: return data
        #t0, t1, t2, t3, t4: using

        addiu	sp,sp,-32
        sw      ra,  8(sp)
        sw      s0,  16(sp)
        
        #to command issue mode
        sw      zero, 0x8(a0)              #disabe SSIENR
        li      t0, 0x73c7
        sw      t0, 0x0(a0)                #set CTRLR0: transmit only
        li      t0, 0x3            
        sw      t0, 0x4(a0)                #set read 4 of data frames 
        li      t0, 0x1
        sw      t0, 0x8(a0)

word_raddr:   
        sw      zero, 0x10(a0)             #disable SER 

        andi    t2, a1, 0xff               #addr (xx_xx--)
        srl     a1, a1, 8
        andi    t3, a1, 0xff               #addr (xx_--xx) >>8
        srl     a1, a1, 8
        andi    t4, a1, 0xff               #addr (--_xxxx) >>16

        #cmd(READ) +addr
        li      t1, FLASH_READ_COM 
        sw      t1, 0x60(a0)
        sw      t4, 0x60(a0)
        sw      t3, 0x60(a0)
        sw      t2, 0x60(a0)

        li      t0, 0x1                  
        sw      t0, 0x10(a0)               #enable SER 

        jal     ssi_wait_nobusy
        nop

        #read data
        lw      s0, 0x60(a0)               #load data (1-byte)
        jal     ssi_wait_nobusy
        nop

        lw      t4, 0x60(a0)               #load data (2-byte)
        sll     t4, t4, 8
        or      s0, t4, s0
        jal     ssi_wait_nobusy
        nop
      
        lw      t4, 0x60(a0)               #load data (3-byte)
        sll     t4, t4, 16
        or      s0, t4, s0
        jal     ssi_wait_nobusy
        nop
 
        lw      t4, 0x60(a0)               #load data (4-byte)
        sll     t4, t4, 24
        or      s0, t4, s0
        jal     ssi_wait_nobusy
        nop
 
        or      v0, zero, s0 
        
        lw      s0, 16(sp)
        lw      ra, 8(sp)
        addiu	sp,sp,32

        jr      ra                         #Return
        .end spi_read_word   
 #####################################################################
        .globl  spi_fastread_word
        .ent    spi_fastread_word

spi_fastread_word:
        #a0: SSI_BASE
        #a1: addr (3-byte) 
        #v0: return data (1-byte)
        #t0, t1, t2, t3, t4: using

        addiu	sp,sp,-8
        sw      ra, 8(sp)

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSIENR
        li      t0, 0x73c7
        sw      t0, 0x0(a0)                #set CTRLR0: transmit only
        li      t0, 0x3            
        sw      t0, 0x4(a0)                #set read 3 of data frames 
        li      t0, 0x1
        sw      t0, 0x8(a0)                #enable SSIENR
        
        sw      zero, 0x10(a0)             #disable SER 

    
        #issue WR
        andi    t2, a1, 0xff               #addr (xx_xx--) 
        srl     a1, a1, 8
        andi    t3, a1, 0xff               #addr (xx_--xx) >>8
        srl     a1, a1, 8
        andi    t4, a1, 0xff               #addr (--_xxxx) >>16
word_fraddr:   
        #cmd(fread) + addr + dummy sw
        li      t0, FLASH_FAST_READ_COM
        sw      t0, 0x60(a0)               #set fast read command of SPI
        sw      t4, 0x60(a0)               #set addr (--_xxxx) >>16
        sw      t3, 0x60(a0)               #set addr (xx_--xx) >>8 
        sw      t2, 0x60(a0)               #set addr (xx_xx--) 
        sw      t1, 0x60(a0)               #set dummy data

        li      t0, 0x1                  
        sw      t0, 0x10(a0)               #enable SER 

        jal     ssi_wait_nobusy
        nop
        
 
        #read data
        lw      s0, 0x60(a0)               #load data (1-byte)
        jal     ssi_wait_nobusy
        nop

        lw      t4, 0x60(a0)               #load data (2-byte)
        sll     t4, t4, 8
        or      s0, t4, s0
        jal     ssi_wait_nobusy
        nop
      
        lw      t4, 0x60(a0)               #load data (3-byte)
        sll     t4, t4, 16
        or      s0, t4, s0
        jal     ssi_wait_nobusy
        nop
 
        lw      t4, 0x60(a0)               #load data (4-byte)
        sll     t4, t4, 24
        or      s0, t4, s0
        jal     ssi_wait_nobusy
        nop
 
        or      v0, zero, s0               #return value
       
        lw      ra, 8(sp)
        addiu	sp,sp,8
        jr      ra                         #Return
        .end spi_fastread_word   
#####################################################################      

      
