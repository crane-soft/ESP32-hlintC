#define SCL_PIN 44      // SLC = D7 = 44
#define SDA_PIN 7       // SDA = D8 = 7
#define TP1_PIN 8       // TP1 = D9 = 8
#define TP2_PIN 9       // TP2 = D10 = 9

#define SCL_PIN16   (SCL_PIN-32)
#define SDA_PIN16   SDA_PIN

#define SCL_PIN_MASK (1 << SCL_PIN16)
#define SDA_PIN_MASK (1 << SDA_PIN16)        
#define TP1_PIN_MASK (1 << TP1_PIN)     
#define TP2_PIN_MASK (1 << TP2_PIN)     

#define SDA_STATUS_REG GPIO_STATUS_REG
#define SCL_STATUS_REG GPIO_STATUS1_REG

#define SDA_STATUS_W1TC_REG GPIO_STATUS_W1TC_REG
#define SCL_STATUS_W1TC_REG GPIO_STATUS1_W1TC_REG

#define SDA_PIN_REG     GPIO_PIN0_REG+(SDA_PIN*4) 
#define SCL_PIN_REG     GPIO_PIN0_REG+(SCL_PIN*4) 

#define SDA_IN_REG      GPIO_IN_REG
#define SCL_IN_REG      GPIO_IN1_REG

#define IO_INTR_POSEDGE  1
#define IO_INTR_NEGEDGE  2

#define I2C_ADDR	0x5A
#define MAX_I2CBYTE 14

// Data struct definitions for asm version
#define SIZEOF_I2C_DATA (24+MAX_I2CBYTE)
#define I2C_State           0
#define	I2C_BitCount        4
#define	I2C_ByteCount       8
#define	I2C_DataByte        12
#define	I2C_DataAddr        16
#define	I2C_DataAvailable   20
#define	I2C_Buffer          24

