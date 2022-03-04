#include <stdio.h>

extern "C" {
/******************************************************************************/
/*           Cortex-M0+ Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
    printf("Non maskable interrupt called\n");
    while (1) {
    }
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
    printf("Hard fault occurred\n");
    while (1) {
    }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
    // do nothing
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
    // do nothing
}
}
