/**
 * @cond
 ***********************************************************************************************************************
 *
 * Copyright (c) 2015, Infineon Technologies AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,are permitted provided that the
 * following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice, this list of conditions and the  following
 *   disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
 *   following disclaimer in the documentation and/or other materials provided with the distribution.
 *
 *   Neither the name of the copyright holders nor the names of its contributors may be used to endorse or promote
 *   products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE  FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY,OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT  OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 **********************************************************************************************************************/

/*******************************************************************************
**                      Author(s) Identity                                    **
********************************************************************************
**                                                                            **
** Initials     Name                                                          **
** ---------------------------------------------------------------------------**
** DM           Daniel Mysliwitz                                              **
** TA           Thomas Albersinger                                            **
**                                                                            **
**                                                                            **
*******************************************************************************/

/*******************************************************************************
**                      Revision Control History                              **
*******************************************************************************/
/* 
 * V0.3.1: 2017-05-26, DM:   global variable SystemFrequency added
 * V0.3.0: 2016-08-04, DM:   Passwd open check removed for EnterSleepMode,
 *                           EnterStopMode, EnterSlowMode
 * V0.2.9: 2015-11-26, DM:   init of DMASRCSELx added
 * V0.2.8: 2015-11-18, DM:   in function SCU_EnterStopMode() one __WFE removed
 * V0.2.7: 2015-02-10, DM:   individual header file added
 *                           OSC_CON register defined by IFXConfigWizard
 * V0.2.6: 2014-10-20, DM:   OSC_CON, OSC_PLL async. set before StopMode entry
 * V0.2.5: 2014-09-09, DM:   OSC_CON, XTAL power down handled
 * V0.2.4: 2014-06-23, TA:   conditional include of lin.h
 * V0.2.3: 2014-06-18, TA:   include bootrom.h for NVM protection control
 *                           include lin.h because of sleep functions
 * V0.2.2: 2014-06-17, DM:   fix EnterSleepMode, switch LIN Trx. into
 *                           sleep mode as well
 * V0.2.1: 2014-05-18, DM:   fix SCU_ChangeNVMProtection function
 * V0.2.0: 2014-04-26, TA:   in SCU_init(): use #defines from the header file
 *                           generated by the config wizard
 * V0.1.5: 2013-11-06, DM:   change global interrupt disable/enable in 
 *                           power saving functions
 * V0.1.4: 2013-04-05, DM:   APCLK1/2 (MI_CLK, FILT_CLK) register access
 *                           corrected
 * V0.1.3: 2013-03-26, DM:   EXINT3 removed
 * V0.1.2: 2013-03-06, DM:   NVM Protection added
 * V0.1.1: 2013-02-21, DM:   VCOSEL added to function SCU_SetSysClk()
 * V0.1.0: 2013-02-10, DM:   Initial version
 */

/*******************************************************************************
**                      Includes                                              **
*******************************************************************************/
#include <scu.h>
#include <bootrom.h>
#include <wdt1.h>

/*******************************************************************************
**                      External CallBacks                                    **
*******************************************************************************/

/*******************************************************************************
**                      Global Variable Definitions                           **
*******************************************************************************/
uint32 SystemFrequency = (uint32)SCU_FSYS;

/*******************************************************************************
**                      Global Function Definitions                           **
*******************************************************************************/
/** \brief Initializes the system clocks.
 *
 * \param None
 * \return None
 *
 * \ingroup drv_api
 */
void SCU_ClkInit(void)
{
  /* disable all interrupts                */
  __disable_irq();
 /***************************************************************************
  ** NVM Protection Control                                                **
  **************************************************************************/
/*lint -e534 Supressing MISRA 2012 MISRA 2012 Directive 4.7 */
#if (SCU_CFLASH_WPROT == 1)
  USER_CFLASH_WR_PROT_EN((uint16) SCU_CFLASH_WPROT_PW);
#endif
#if (SCU_DFLASH_WPROT == 1)
  USER_DFLASH_WR_PROT_EN((uint16) SCU_DFLASH_WPROT_PW);
#endif
/*lint +e534 */

 /***************************************************************************
  ** PLL/SYSCLK Control                                                    **
  **************************************************************************/
  SCU->NMICON.bit.NMIPLL = 0u;
  SCU->NMICON.bit.NMIWDT = 0u;

  SCU->PASSWD.reg = PASSWD_Open;
  /* select LP_CLK */
  SCU->SYSCON0.bit.SYSCLKSEL = 3u;
  SCU->PASSWD.reg = PASSWD_Close;

  /* Oscillator Select */
  SCU->PASSWD.reg = PASSWD_Open;
  SCU->OSC_CON.reg = (uint8) SCU_OSC_CON;
  SCU->PASSWD.reg = PASSWD_Close;

  SCU->PASSWD.reg = PASSWD_Open;
  SCU->PLL_CON.reg = (uint8) SCU_PLL_CON;
  SCU->PASSWD.reg = PASSWD_Close;

  SCU->PASSWD.reg = PASSWD_Open;
  SCU->CMCON1.reg = (uint8) SCU_CMCON1;
  SCU->PASSWD.reg = PASSWD_Close;

  SCU->PLL_CON.bit.RESLD = 1u;
  SCU->PLL_CON.bit.VCOBYP = 0u;
  SCU->PLL_CON.bit.OSCDISC = 0u;

  while (SCU->PLL_CON.bit.LOCK == 0u)
  {
  }

  SCU->PASSWD.reg = PASSWD_Open;
  /* 0u << 6u */
  SCU->SYSCON0.reg = 0u;
  SCU->PASSWD.reg = PASSWD_Close;

  SCU->NMICLR.bit.NMIPLLC = 1u;
 /***************************************************************************
  ** Analog Clock Control                                                  **
  **************************************************************************/

  /* set factor for MI_CLK */
  SCU->APCLK1.reg = (uint8) SCU_APCLK1;

  /* apply setting by toggling APCLK_SET */
  SCU->PASSWD.reg = 0x98;
  SCU->APCLK_CTRL1.bit.APCLK_SET = 1u;
  SCU->PASSWD.reg = 0xA8;
  __NOP();
  SCU->PASSWD.reg = 0x98;
  SCU->APCLK_CTRL1.bit.APCLK_SET = 0u;
  SCU->PASSWD.reg = 0xA8;

  /* set factor for Filt_CLK */
  SCU->APCLK2.reg = (uint8) SCU_APCLK2;

  /* apply setting by toggling APCLK_SET */
  SCU->PASSWD.reg = 0x98;
  SCU->APCLK_CTRL1.bit.APCLK_SET = 1u;
  SCU->PASSWD.reg = 0xA8;
  __NOP();
  SCU->PASSWD.reg = 0x98;
  SCU->APCLK_CTRL1.bit.APCLK_SET = 0u;
  SCU->PASSWD.reg = 0xA8;

#ifdef SCU_APCLK_CTRL1
  SCU->PASSWD.reg = 0x98;
  SCU->APCLK_CTRL1.reg = (uint8) SCU_APCLK_CTRL1;
  SCU->PASSWD.reg = 0xA8;
#endif
#ifdef SCU_APCLK_CTRL2
  SCU->PASSWD.reg = 0x98;
  SCU->APCLK_CTRL2.reg = (uint8) SCU_APCLK_CTRL2;
  SCU->PASSWD.reg = 0xA8;
#endif

  /* apply setting by toggling APCLK_SET */
  SCU->PASSWD.reg = 0x98;
  SCU->APCLK_CTRL1.bit.APCLK_SET = 1u;
  SCU->PASSWD.reg = 0xA8;
  __NOP();
  SCU->PASSWD.reg = 0x98;
  SCU->APCLK_CTRL1.bit.APCLK_SET = 0u;
  SCU->PASSWD.reg = 0xA8;

  /* enable interrupts                     */
  __enable_irq();
}

/** \brief Initializes the SCU module.
 *
 * \param None
 * \return None
 *
 * \ingroup drv_api
 */
void SCU_Init(void)
{
 /***************************************************************************
  ** System Clock Output Control                                           **
  **************************************************************************/
  SCU->COCON.reg = (uint8) (SCU_COCON);

 /***************************************************************************
  ** Watchdog Control (internal)                                           **
  **************************************************************************/
  SCU->WDTREL.reg = (uint8) SCU_WDTRL;
  SCU->WDTWINB.reg = (uint8) SCU_WDTWINB;
  SCU->WDTCON.reg = (uint8) SCU_WDTCON;

 /***************************************************************************
  ** Module Pin Select                                                     **
  **************************************************************************/
#ifdef SCU_MODPISEL
  SCU->MODPISEL.reg = (uint8) SCU_MODPISEL;
#endif
#ifdef SCU_MODPISEL1
  SCU->MODPISEL1.reg = (uint8) SCU_MODPISEL1;
#endif
#ifdef SCU_MODPISEL2
  SCU->MODPISEL2.reg = (uint8) SCU_MODPISEL2;
#endif
#ifdef SCU_MODPISEL3
  SCU->MODPISEL3.reg = (uint8) SCU_MODPISEL3;
#endif
  SCU->GPT12PISEL.reg = (uint8) SCU_GPT12PISEL;

 /***************************************************************************
  ** DMA Source Select                                                     **
  **************************************************************************/
#if (SCU_XML_VERSION >= 10109u)
#ifdef SCU_DMASRCSEL
  SCU->DMASRCSEL.reg = SCU_DMASRCSEL;
#endif
#ifdef SCU_DMASRCSEL2
  SCU->DMASRCSEL2.reg = SCU_DMASRCSEL2;
#endif
#endif
}

/** \brief Sets the device into Sleep Mode
 *
 * \param[in] none
 * \param[in] 
 * \return nothing
 *
 * \ingroup scu_api
 */
#if (PMU_SLEEP_MODE == 1)
void SCU_EnterSleepMode(void)
{
  /* This function assumes the desired     *
   * wake up sources are enabled already   */
  uint8 dummy;
  /* to remove compiler warning of unused  *
   * dummy variable                        */
  dummy = dummy;
  /* set LIN Trx into Sleep Mode           */
  LIN_Set_Mode(LIN_MODE_SLEEP);
  /* wait until LIN Trx switched           *
   * into Sleep Mode                       */
  while (LIN_Get_Mode() != LIN_GET_MODE_SLEEP)
  {
  };
  /* wait approx. 100�s in order to pass   *
   * the filter time + margin inside the   *
   * LIN Trx. to avoid a false wake up     */
  Delay_us(100);
  /* dummy read to clr wake up status      *
   * to prevent the device from wakeing up *
   * immediately because of still set      *
   * wake up flags                         */
  dummy = PMU->WAKE_STATUS.reg;
  /* dummy read to clr MON wake up status  */
  dummy = PMU->WAKE_STS_MON.reg;
  /* stop WDT1 (SysTick) to prevent any    *
   * SysTick interrupt to disturb the      *
   * Sleep Mode Entry sequence             */
  WDT1_Stop();
  /* Trigger a ShortOpenWindow on WDT1     *
   * to prevent an unserviced WDT1 by      *
   * accident                              */
  WDT1_SOW_Service(1);
  /* disable all interrupts                */
  __disable_irq();
  /* open passwd to access PMCON0 register */
  SCU->PASSWD.reg = PASSWD_Open;
  /* set SleepMode flag in PMCON0        */
  SCU->PMCON0.reg = 0x02;
/*lint -e718 Supressing MISRA 2012 Rule 17.3 */
/*lint -e746 Supressing MISRA 2012 Rule 10.4 */
  __WFE();
  __WFE();
/*lint +e746 */
/*lint +e718 */
  /* dont do anything anymore            *
   * device is in SleepMode              */
  while (1)
  {
  };
  /* device is in Sleep Mode now          *
   * wake up performes RESET              */
}
#endif

/** \brief Sets the device into Stop Mode
 *
 * \param[in] nothing
 * \param[in]
 * \return nothing
 *
 * \ingroup scu_api
 */
#if (PMU_STOP_MODE == 1)
void SCU_EnterStopMode(void)
{
  uint8 dummy;
  /* to remove compiler warning of unused  *
   * dummy variable                        */
  dummy = dummy;
  /* dummy read to clr wake up status      *
   * to prevent the device from wakeing up *
   * immediately because of still set      *
   * wake up flags                         */
  dummy = PMU->WAKE_STATUS.reg;
  /* dummy read to clr MON wake up status  */
  dummy = PMU->WAKE_STS_MON.reg;
  /* stop WDT1 (SysTick) to prevent any    *
   * SysTick interrupt to disturb the      *
   * Sleep Mode Entry sequence             */
  WDT1_Stop();
  /* Trigger a ShortOpenWindow on WDT1     *
   * to prevent an unserviced WDT1 by      *
   * accident                              */
  WDT1_SOW_Service(1);
  /* disable all interrupts                */
  __disable_irq();
  /* open access to SYSCLKSEL register     */
  SCU->PASSWD.reg = PASSWD_Open;
  /* select LP_CLK as sys clock            */
  SCU->SYSCON0.bit.SYSCLKSEL = 3U;
  SCU->PASSWD.reg = PASSWD_Close;
  /* open access to OSC_CON register       */
  SCU->PASSWD.reg = PASSWD_Open;
  /* select OSC_PLL async.                 */
  SCU->OSC_CON.bit.OSCSS = 3U;
  SCU->PASSWD.reg = PASSWD_Close;
  /* open passwd to access PMCON0 register */
  SCU->PASSWD.reg = PASSWD_Open;
  /* set PowerDown flag in PMCON0        */
  SCU->PMCON0.reg = 0x04;
/*lint -e718 Supressing MISRA 2012 Rule 17.3 */
/*lint -e746 Supressing MISRA 2012 Rule 10.4 */
  __WFE();
  __WFE();
  __NOP();
  __NOP();
  __NOP();
  __NOP();
/*lint +e746 */
/*lint +e718 */
  /* dont do anything anymore            *
   * device is in PowerDown Mode         */
  /***************************************/
  /********* DEVICE IN STOP MODE *********/
  /***************************************/
  /* Device has been woken up            */
  /* reset PLL locking                   */
  SCU->PLL_CON.bit.RESLD = 1;
  SCU->PLL_CON.bit.VCOBYP = 0;
  SCU->PLL_CON.bit.OSCDISC = 0;
  /* wait for PLL being locked           */
  while (SCU->PLL_CON.bit.LOCK == 0)
  {
  }
  /* switch fSYS back to                 *
   * user configuration                  */
  SCU->PASSWD.reg = PASSWD_Open;
  SCU->SYSCON0.reg = (uint8) (0 << 6);
  SCU->PASSWD.reg = PASSWD_Close;
  /* enable interrupts                     */
  __enable_irq();
  /* Init and service WDT1                 */
  WDT1_Init();
}
#endif

/** \brief Sets the device into Slow Mode
 *
 * \param[in] nothing
 * \param[in]
 * \return nothing
 *
 * \ingroup scu_api
 */
void SCU_EnterSlowMode(void)
{
  /* stop WDT1 (SysTick) to prevent any    *
   * SysTick interrupt to disturb the      *
   * Sleep Mode Entry sequence             */
  WDT1_Stop();
  /* Trigger a ShortOpenWindow on WDT1     *
   * to prevent an unserviced WDT1 by      *
   * accident                              */
  WDT1_SOW_Service((uint8) 1);
  /* disable all interrupts                */
  __disable_irq();
  /* open passwd to access PMCON0 register */
  SCU->PASSWD.reg = PASSWD_Open;
  /* set PowerDown flag in PMCON0        */
  SCU->PMCON0.reg = 0x08;
/*lint -e718 Supressing MISRA 2012 Rule 17.3 */
/*lint -e746 Supressing MISRA 2012 Rule 10.4 */
  __WFE();
  __WFE();
/*lint +e746 */
/*lint +e718 */
  /* dont do anything anymore            *
   * device is in PowerDown Mode         */
  /***************************************/
  /********* DEVICE IN SLOW MODE *********/
  /***************************************/
  /* enable interrupts                     */
  __enable_irq();
  /* Init and service WDT1                 */
  WDT1_Init();
}

/** \brief Gets the device out of Slow Mode
 *
 * \param[in] nothing
 * \param[in]
 * \return nothing
 *
 * \ingroup scu_api
 */
void SCU_ExitSlowMode(void)
{
  /* stop WDT1 (SysTick) to prevent any    *
   * SysTick interrupt to disturb the      *
   * Sleep Mode Entry sequence             */
  WDT1_Stop();
  /* Trigger a ShortOpenWindow on WDT1     *
   * to prevent an unserviced WDT1 by      *
   * accident                              */
  WDT1_SOW_Service((uint8) 1);
  /* disable all interrupts                */
  __disable_irq();
  /* open access to SYSCLKSEL register     */
  SCU->PASSWD.reg = PASSWD_Open;
  /* select LP_CLK as sys clock            */
  SCU->SYSCON0.bit.SYSCLKSEL = 3U;
  SCU->PASSWD.reg = PASSWD_Close;
  /* open passwd to access PMCON0 register */
  SCU->PASSWD.reg = PASSWD_Open;
  /* set PowerDown flag in PMCON0        */
  SCU->PMCON0.reg = ~(uint8) 0x08;
  __WFE();
  /* dont do anything anymore            *
   * device is in PowerDown Mode         */
  /***************************************/
  /***** DEVICE back in NORMAL MODE ******/
  /***************************************/
  /* enable interrupts                     */
  __enable_irq();
  /* Init and service WDT1                 */
  WDT1_Init();
}
