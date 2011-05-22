#include "target.h"
#include "floppy-low.h"
#include "delay.h"
#include "uartutil.h"

// OUT masks
#define WRITE_DATA              _BV(WRITE_DATA_PIN)
#define WRITE_GATE              _BV(WRITE_GATE_PIN)
#define SIDE_SELECT             _BV(SIDE_SELECT_PIN)
#define HEAD_STEP               _BV(HEAD_STEP_PIN)
#define DIR_SELECT              _BV(DIR_SELECT_PIN)
#define MOTOR_ENABLE            _BV(MOTOR_ENABLE_PIN)
#define DRIVE_SELECT            _BV(DRIVE_SELECT_PIN)
#define DENSITY_SELECT          _BV(DENSITY_SELECT_PIN)

// IN masks
#define READ_DATA               _BV(READ_DATA_PIN)
#define TRACK_ZERO              _BV(TRACK_ZERO_PIN)
#define INDEX                   _BV(INDEX_PIN)
#define WRITE_PROTECT           _BV(WRITE_PROTECT_PIN)

#define FLOPPY_OUT_MASK     (WRITE_DATA | WRITE_GATE | SIDE_SELECT | HEAD_STEP | DIR_SELECT | MOTOR_ENABLE | DRIVE_SELECT | DENSITY_SELECT)
#define FLOPPY_IN_MASK      (READ_DATA | TRACK_ZERO | INDEX | WRITE_PROTECT_PIN)

#define SET_LO(x)       AT91F_PIO_ClearOutput( AT91C_BASE_PIOA, x )
#define SET_HI(x)       AT91F_PIO_SetOutput( AT91C_BASE_PIOA, x )
#define IS_HI(x)        AT91F_PIO_IsInputSet( AT91C_BASE_PIOA, x )

#define DELAY_MS(x)     delay_ms(x)
#define DELAY_US(x)     delay_us(x)

void floppy_low_init(void)
{
    // Enable PIO
    AT91F_PMC_EnablePeriphClock ( AT91C_BASE_PMC, _BV(AT91C_ID_PIOA) ) ;

    // Set outputs
    AT91F_PIO_CfgOutput( AT91C_BASE_PIOA, FLOPPY_OUT_MASK );
    AT91F_PIO_SetOutput( AT91C_BASE_PIOA, FLOPPY_OUT_MASK );

    // Input glitch filter for index
    AT91F_PIO_InputFilterEnable( AT91C_BASE_PIOA, FLOPPY_IN_MASK ); // glitch filter
    AT91F_PIO_OutputDisable( AT91C_BASE_PIOA, FLOPPY_IN_MASK ); // its an input
    AT91F_PIO_Enable( AT91C_BASE_PIOA, FLOPPY_IN_MASK ); // enable
}

#define INDEX_INTERRUPT_LEVEL    3   // 0=lowest  7=highest

void floppy_low_enable_index_intr(func f)
{
    // Configure -> ext IRQ1 is index
    AT91F_PIO_CfgPeriph(AT91C_BASE_PIOA, INDEX, READ_DATA);
    // open external IRQ1
    AT91F_AIC_ConfigureIt(AT91C_BASE_AIC, AT91C_ID_IRQ1, 
                          INDEX_INTERRUPT_LEVEL, 
                          AT91C_AIC_SRCTYPE_EXT_NEGATIVE_EDGE, f); // AIC_IDCR, AIC_SVR, AIC_SMR, AIC_ICCR
    AT91F_AIC_EnableIt (AT91C_BASE_AIC, AT91C_ID_IRQ1); // AIC_IECR    

    // force FIQ
    //*AT91C_AIC_FFER = AT91C_ID_IRQ1;
}

void floppy_low_disable_index_intr(void)
{
    AT91F_AIC_DisableIt (AT91C_BASE_AIC, AT91C_ID_IRQ1); // AIC_IECR        
}

void floppy_low_select_on()
{
    SET_LO(DRIVE_SELECT);
    DELAY_MS(1); // 0.5 us Max
}

void floppy_low_select_off()
{
    SET_HI(DRIVE_SELECT);
    DELAY_MS(1);
}

void floppy_low_motor_on()
{
    SET_LO(MOTOR_ENABLE);
    DELAY_MS(500);
}

void floppy_low_motor_off()
{
    SET_HI(MOTOR_ENABLE);
    DELAY_MS(500);
}

u32 floppy_low_is_track_zero()
{
    return !IS_HI(TRACK_ZERO);
}

void floppy_low_set_dir(u32 dir)
{
    if(dir) {
        SET_HI(DIR_SELECT);
    } else {
        SET_LO(DIR_SELECT);
    }
    DELAY_MS(1);
}

void floppy_low_set_density(u32 high)
{
  if(high) {
      SET_HI(DENSITY_SELECT);
  } else {
      SET_LO(DENSITY_SELECT);
  }
}

void floppy_low_set_side(u32 side)
{
    if(side) {
        SET_HI(SIDE_SELECT);
    } else {
        SET_LO(SIDE_SELECT);
    }
    DELAY_US(100);
}

void floppy_low_step_track()
{
    SET_LO(HEAD_STEP);
    DELAY_US(1);
    SET_HI(HEAD_STEP);
    
    DELAY_MS(4);
}

void floppy_low_step_n_tracks(u32 dir,u32 n)
{
    floppy_low_set_dir(dir);
    for(u32 i=0;i<n;i++) {
        floppy_low_step_track();
    }
    
    DELAY_MS(14);
}

u08 floppy_low_seek_zero()
{
    floppy_low_set_dir(DIR_OUTWARD);
    u08 cnt = 0;
    for(u32 i=0;i<84;i++) {
        if(floppy_low_is_track_zero()) {
            break;
        }
        floppy_low_step_track();
        cnt++;
    }
    
    DELAY_MS(16);
    return cnt;
}

int floppy_low_is_write_protected(void)
{
    return !IS_HI( WRITE_PROTECT );
}

