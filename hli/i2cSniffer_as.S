// be sure the file extension is .S (uppercase) otherwise you will get compiler errors
#include <xtensa/coreasm.h>
#include <xtensa/corebits.h>
#include <xtensa/config/system.h>
#include "freertos/xtensa_context.h"
#include "esp_private/panic_reason.h"
#include "sdkconfig.h"
#include "soc/soc.h"
#include "soc/gpio_reg.h"
#include "soc/dport_reg.h"
#include "i2cdefs.h"

#define i2creg  a0

.macro wr_reg   reg,val
    movi    a14, \reg       // a14 = reg-addr
    s32i    a15, a14, 0     // a15 -> *a14
    memw    
.endm

.macro wr_regi   reg,val
    movi    a14, \reg       // a14 = reg-addr
    movi    a15, \val       // a15 = value
    s32i    a15, a14, 0     // a15 -> *a14
    memw    
.endm

.macro rd_reg   reg
    movi    a14, \reg       // a14 = reg-addr
    l32i    a15, a14, 0     // a15 = [reg]
    memw    
.endm

.macro gpio_0_setout pin_mask
    wr_regi GPIO_OUT_W1TS_REG, \pin_mask
.endm

.macro gpio_0_resout pin_mask
    wr_regi GPIO_OUT_W1TC_REG, \pin_mask
.endm

.macro set_int_type pinreg, int_type
    rd_reg \pinreg  
    movi    a14, ~GPIO_PIN_INT_TYPE_MASK
    and     a15,a15, a14
    movi    a14, GPIO_PIN_INT_TYPE_SET(\int_type)
    or      a15,a15, a14
    wr_reg \pinreg
.endm

.macro clr_int pinreg
    rd_reg \pinreg  
    movi    a14, ~GPIO_PIN_INT_TYPE_MASK
    and     a15,a15, a14
    wr_reg \pinreg
.endm


    .data
_l5_intr_stack:
    .space      16

    .section .iram1,"ax"
    .global     xt_highint5
    .type       xt_highint5,@function
    .align      4
xt_highint5:

    movi    a0, _l5_intr_stack // assuming a0 saved in EXCSAVE_5
    s32i    a13, a0, 0
    s32i    a14, a0, 4
    s32i    a15, a0, 8

    movi    i2creg, i2c_data			// pointer to data structure

    rd_reg SDA_STATUS_REG
    bbci a15, SDA_PIN16, scl_interrupt    // branch if bit is clear

    // --------------------------------------------------------------------------------
    // SDA rising or falling edge interrupt
    // --------------------------------------------------------------------------------
    wr_regi   SDA_STATUS_W1TC_REG , SDA_PIN_MASK  // Clear int status bit
    rd_reg    SCL_IN_REG            // i2c-start or stop occurs only if SCL = 1
    bbci a15, SCL_PIN16, scl_interrupt
    
    gpio_0_setout TP2_PIN_MASK	
    
    // --------------------------------------------------------------------------------
    // if (status == 0)
    // Startbit detected
    // --------------------------------------------------------------------------------
    l32i    a15, i2creg, I2C_State     
    bnez    a15, sda_rising
    
    movi    a15,1       
    s32i    a15, i2creg, I2C_State  // status = 1

    movi    a15,0		// some var init
    s32i    a15, i2creg, I2C_BitCount
    s32i    a15, i2creg, I2C_ByteCount

   // Pin-Config change to rising edge
    set_int_type  SDA_PIN_REG, IO_INTR_POSEDGE
    J xt_intend

sda_rising:    
    // --------------------------------------------------------------------------------
    // Stopbit detected
    // --------------------------------------------------------------------------------

    l8ui    a15, i2creg, I2C_Buffer     // get I2C_Addr
    movi    a14, I2C_ADDR      			// I2C_ADDR (bit0 = 0)
    bne     a15, a14 , xt_rdframe       // bit0 != 0 -> read frame

    // I2c wr-frame
    l32i    a15, i2creg, I2C_ByteCount  // get ByteCounter
    bnei    a15, 2, xt_notvalid         // if (ByteCounter != 2

    l8ui    a15, i2creg, I2C_Buffer+1   // get i2c 1.data byte
    s32i    a15, i2creg, I2C_DataAddr   // save i2c DataAddr
    movi    a15,1
    s32i    a15, i2creg, I2C_DataAvailable	// 1= DataAddr has readed
    J xt_restart	

xt_rdframe:
    l8ui    a15, i2creg, I2C_Buffer     // get I2C_Addr
    movi    a14, I2C_ADDR+1             // check if rd-frame          
    bne     a15, a14 , xt_notvalid      // if == I2C_ADDR)

    l32i    a15, i2creg, I2C_ByteCount  // get ByteCounter
    movi    a14,9						// must be == 9
    bne	    a15, a14, xt_notvalid     // if (ByteCounter != 9)

    // new data available
    l32i    a15, i2creg, I2C_DataAvailable  // if (DataAvailable != 1)
    bnei    a15, 1, xt_notvalid             // Reset no addr available

    movi    a15,2   // new data avalable
    s32i    a15, i2creg, I2C_DataAvailable  // 

    // Diasable Interrupt will be new wnabled by 
	// main programm after readin the data
    clr_int SDA_PIN_REG
    clr_int SCL_PIN_REG
    j xt_clr_state

xt_notvalid:
    movi    a15,0       
    s32i    a15, i2creg, I2C_DataAvailable  // DataAvailable = 0

xt_restart:
    set_int_type SDA_PIN_REG, IO_INTR_NEGEDGE    // INT at falling edge for startbit

xt_clr_state:
    movi    a15,0       
    s32i    a15, i2creg, I2C_State          // I2C_State = 0

    J xt_intend

scl_interrupt:    
    // --------------------------------------------------------------------------------
    // SCL rising edge interrupt
    // --------------------------------------------------------------------------------
    rd_reg  SCL_STATUS_REG
    bbci    a15, SCL_PIN16, xt_intend          // branch if bit is clear
    wr_regi SCL_STATUS_W1TC_REG, SCL_PIN_MASK  // Clear int status bit

    l32i    a15, i2creg, I2C_State      // check status    
    beqz    a15, xt_intend              // read only if startbit detected

    gpio_0_setout TP1_PIN_MASK          // set test pin

    // Shift SDA Bit into DataByte
    l32i    a15, i2creg, I2C_DataByte   // get DataByte
	slli	a13, a15, 1                 // a13 DataByte = (DataByte << 1)
    rd_reg  SDA_IN_REG                  // read SDA-Pin (A15)
    bbci    a15, SDA_PIN16, xt_nobit    // skip if zero
    addi    a13, a13, 1                 // a13 DataByte set bit0
xt_nobit:
    s32i    a13, i2creg, I2C_DataByte   // store DataByte

    // Incr BitCount
    l32i    a15, i2creg, I2C_BitCount   // load BitCount
    addi    a15, a15, 1                 // ++BitCount
    s32i    a15, i2creg, I2C_BitCount   // store BitCount
    bnei	a15, 8, xt_bitcntneq8       // if (BitCount == 8)

    // Datebyte complete
    l32i    a15, i2creg, I2C_ByteCount  // load ByteCount
    movi    a14, MAX_I2CBYTE            // if < MAX_I2CBYTE
    blt     a14, a15, xt_notvalid

    // Store new Byte in Buffer, idx = ByteCount = a15
    add     a14, i2creg, a15           // A14 Buffptr = i2c_data + idx
    l32i    a13, i2creg, I2C_DataByte  // A13 = Databyte
    s8i     a13, a14, I2C_Buffer       // store A13 -> a14+Buffer
    addi    a15, a15, 1                // ++ ByteCount
    s32i    a15, i2creg, I2C_ByteCount
    J xt_intend

xt_bitcntneq8:  // BitCount != 8   
    movi    a14,9
    bne	    a15, a14, xt_intend        // if (BitCount == 9)
    movi    a15, 0                     // BitCount = 0
    s32i    a15, i2creg, I2C_BitCount  // store BitCount

xt_intend:
    gpio_0_resout TP1_PIN_MASK
    gpio_0_resout TP2_PIN_MASK

    // Restore Register
    movi    a0, _l5_intr_stack
    l32i    a13, a0, 0
    l32i    a14, a0, 4
    l32i    a15, a0, 8

    rsr     a0, EXCSAVE_5 // restore a0
    rfi     5

    .global ld_include_xt_highint5
ld_include_xt_highint5:
