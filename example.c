/**
  ******************************************************************************
  * file           : example.c
  * brief          : example program body
  ******************************************************************************
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "example.h"
#include "m24m01e.h"
#include <stdio.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define SIZE1024      1024U
#define LOCK_ID_PAGE_EXECUTE 0U /* enable to permanently LOCK ID Page */
#define LOCK_CDA_REG_EXECUTE 0U /* enable to permanently LOCK CDA Register */
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
m24m01e_object_t *pM24m01e0; /* pointer referencing the M24M01E object instance */
uint8_t Recv_Buff[1024] = {0};
uint8_t Transmit_Buff[1024] = {0};

/* Private function prototypes -----------------------------------------------*/
#if DEBUG_APPLI
#define PRINTF_APPLI(...) printf(__VA_ARGS__)
#else
#define PRINTF_APPLI(...)
#endif /* DEBUG_APPLI */

/**
  * ########## Step 1 ##########
  * The init of M24M01E is triggered by the applicativon code
  */
app_status_t app_init(void)
{
  app_status_t return_status = EXEC_STATUS_ERROR;
  /* Retrieve and store the M24M01E object pointer */
  pM24m01e0 = MX_M24M01E_getobject();
  /* Initialize the M24M01E device 0 */
  if (m24m01e_drv_init(pM24m01e0, MX_M24M01E) != 0)
  {
    PRINTF("[ERROR] Step 1: M24M01E EEPROM init error\r\n");
    goto _app_init_exit;
  }
  PRINTF("[INFO] Step 1: M24M01E EEPROM init completed\r\n");

  return_status = EXEC_STATUS_INIT_OK;

_app_init_exit:
  return return_status;
}

/**
  * ########## Step 2 ##########
  * Perform Read and Write operations.
  * The values are displayed on the terminal.
  * output: EXEC_STATUS_OK if OK, EXEC_STATUS_ERROR in case of error
  */
app_status_t app_process(void)
{

  app_status_t return_status = EXEC_STATUS_OK;

  /* At the very first initialization, M24256E-F and M24M01E-F EEPROM will have */
  /* same Device Address as Configuration Device Address (CDA) Register will be 0x00 by default.  */
  /* This will cause conflicts on I2C bus. To avoid this conflict, CDA register of M24M01E-F will be updated. */

  /* C1 (Bit 2) will be set to 1 in CDA. M24M01E-F, Bit 1 is Don't Care*/
  /* Set 0xB3 as Device Select Code will select M24M01E-F only */
  return_status = SetupCDARegister();

  /* Read CDA Register */
  return_status = ReadCDARegister();

  /* Lock CDA Register*/
  /* Enable LOCK_CDA_REG_EXECUTE to lock CDA register permanently. Irreversible action !! */
  return_status = LockCDARegister();

  /* Read DTI Register for M24M01E only*/
  return_status = ReadDTIRegister();


  /* Read/Write ID page */
  return_status = TestM24M01EIDPage();

  /* Lock ID page*/
  /* Enable LOCK_ID_PAGE_EXECUTE to lock CDA register permanently. Irreversible action !! */
  return_status = TestM24M01ELockIDPage();

  /* Read SWP Register for M24M01E only */
  return_status = ReadSWPRegister();
  /* Write SWP Register for M24M01E only*/
  return_status = WriteSWPRegister();


  /* Memory Read/Write M24M01E */
  return_status = TestM24M01EMemory();

  return return_status;
}

/** ########## Step 3 ##########
  * In this example, app_deinit is never called and is provided as a reference only.
  */
app_status_t app_deinit(void)
{
  if (m24m01e_drv_deinit(pM24m01e0) != 0)
  {
    PRINTF("[ERROR] Step 3: EEPROM deinit error\r\n");
    return EXEC_STATUS_ERROR;
  }

  return EXEC_STATUS_OK;
}

/**
  * @brief  Setup CDA register.
  * @param  None
  * @retval app_status_t
  */
app_status_t SetupCDARegister(void)
{

  uint8_t writeCDA;

  printf("***************************************************************\r\n");
  printf("                SETUP CDA REGISTER M24M01E\r\n");
  printf("***************************************************************\r\n");

  /* With Bit 1 set, only M24M01E will respond initially */

  if (m24m01e_drv_check_device_address(pM24m01e0, 0xB2) != 0)
  {
    /* Read CDA register not successful with C2 and C1 both as 0*/
    /* Attempt read with C1 bit set, assuming only C1 bit is set */

    if (m24m01e_drv_check_device_address(pM24m01e0, 0xB4) != 0)
    {
      printf("Selected Device address is incorrect. Abort!\r\n");
      return EXEC_STATUS_ERROR;
    }
    else
    {
      printf("M24M01 CDA: 0x04\r\n");
    }
  }

  else
  {
    /* Read CDA register successful with C2 and C1 both as 0*/

    PRINTF_APPLI("C1 and C2 bits of CDA in M24M01E are 0. Setting C1 bit of CDA register.\r\n");
    printf("__Writing CDA Val__M24M01E <C1 Bit Set>\r\n");
    writeCDA = 0x04;
    if (m24m01e_drv_write_cda_register(pM24m01e0, writeCDA) != 0)
    {
      printf("M24M01E CDA update failed!\r\n");
      return EXEC_STATUS_ERROR;

    }
    else
    {
      printf("Updated M24M01E CDA: 0x%2.2X\r\n", writeCDA);

    }

  }

  /* Uncomment this code section to set CDA of M24M01E back to default value of 0x00*/
  /* Note: application tests must run with different CDA values of M24M01E and M24256E */

  /*
  if (m24m01e_drv_read_cda_register(pM24m01e0, &readCDA) != 0)
  {
    printf("Selected Device address is incorrect. Abort!\r\n");
    return EXEC_STATUS_ERROR;
  }
  else
  {
    printf("M24M01E CDA: 0x%2.2X\r\n", readCDA);
    writeCDA = 0x00;
    if (m24m01e_drv_write_cda_register(pM24m01e0, &writeCDA) != 0)
    {
      printf("Write to M24M01E CDA failed, Device Addr is 0x%2.2X\r\n", M24M01E_CDA_DevSelCode);
      return EXEC_STATUS_ERROR;
    }
    else
    {
      printf("Updated M24M01E CDA: 0x%2.2X\r\n", writeCDA);
    }
  }
  */
  return EXEC_STATUS_OK;
}

/**
  * @brief  Read CDA register.
  * @param  None
  * @retval app_status_t
  */
app_status_t ReadCDARegister(void)
{

  uint8_t readCDA = 0xFF;
  int32_t return_status;
  app_status_t ret_val;

  printf("\n\n***************************************************************\r\n");
  printf("                     READ CDA REGISTER M24M01E\r\n");
  printf("***************************************************************\r\n");

  return_status = m24m01e_drv_read_cda_register(pM24m01e0, &readCDA);

  if (return_status != 0)
  {
    printf("[ERROR] Test 1: M24M01E CDA read error\r\n");
  }
  else
  {
    printf("[INFO] Test 1: M24M01E CDA Register Value: 0x%x\r\n", readCDA);
  }

  if (return_status != 0)
  {
    ret_val = EXEC_STATUS_ERROR;
  }
  else
  {
    ret_val = EXEC_STATUS_OK;
  }

  return ret_val;
}

/**
  * @brief  Read DTI register M24M01E.
  * @param  None
  * @retval app_status_t
  */
app_status_t ReadDTIRegister(void)
{

  uint8_t readDTI = 0xFF;
  app_status_t ret_val;
  printf("\n\n***************************************************************\r\n");
  printf("            READ DTI REGISTER only for M24M01E\r\n");
  printf("***************************************************************\r\n");

  if (m24m01e_drv_read_dti_reg(pM24m01e0, &readDTI) != 0)
  {
    printf("M24M01E DTI Register read Error\r\n");
    ret_val = EXEC_STATUS_ERROR;

  }
  else
  {
    printf("M24M01E DTI Register: 0x%2.2X\r\n", readDTI);
    ret_val = EXEC_STATUS_OK;
  }
  return ret_val;

}

/**
  * @brief  Test Identification page of M24M01E.
  * @param  None
  * @retval None
  */
app_status_t TestM24M01EIDPage(void)
{
  uint32_t addr = 0x00;
  uint16_t count = 0;
  int32_t return_status;
  uint8_t sample_data = 0xAA;


  printf("\n\n***************************************************************\r\n");
  printf("                     TEST M24M01E ID PAGE\r\n");
  printf("***************************************************************\r\n");

  /* M24M01E */
  printf("Read ID Page M24M01E:\r\n");

  /* Read ID Page */

  return_status = m24m01e_drv_read_id_page(pM24m01e0, Recv_Buff, addr, M24M01E_PAGESIZE);

  if (return_status != 0)
  {
    return EXEC_STATUS_ERROR;
  }
  else
  {
    for (uint16_t idx = 0; idx < M24M01E_PAGESIZE; idx++)
    {
      printf("0x%2.2X ", Recv_Buff[idx]);
    }
  }

  /* Write ID Page */
  memset(Transmit_Buff, sample_data, M24M01E_PAGESIZE);
  memset(Recv_Buff, 0x00, M24M01E_PAGESIZE);

  printf("\n\nWriting data 0x%x to complete ID page of M24M01E \r\n ", sample_data);
  return_status = m24m01e_drv_write_id_page(pM24m01e0, Transmit_Buff, addr, M24M01E_PAGESIZE);

  if (return_status != 0)
  {
    return EXEC_STATUS_ERROR;
  }
  else
  {
    HAL_Delay(5);
    if (m24m01e_drv_read_id_page(pM24m01e0, Recv_Buff, addr, M24M01E_PAGESIZE) != 0)
    {
      printf("\n\nRead ID Page Error M24M01E \r\n ");
      return EXEC_STATUS_ERROR;
    }
    else
    {
      for (count = 0; count < M24M01E_PAGESIZE; count++)
      {
        if (Recv_Buff[count] == sample_data)
        {
          printf("0x%2.2X ", Recv_Buff[count]);
        }
        else
        {
          break;
        }
      }

      if (count == M24M01E_PAGESIZE)
      {
        printf("\nAll data to M24M01E ID Page written successfully!\r\n");
      }
      else
      {
        printf("Error in ID Page write M24M01E.\r\n");
        return EXEC_STATUS_ERROR;
      }
    }
  }

  /* Clear ID Page to 0xFF */
  memset(Transmit_Buff, 0xFF, M24M01E_PAGESIZE);
  if (m24m01e_drv_write_id_page(pM24m01e0, Transmit_Buff, addr, M24M01E_PAGESIZE) != 0)
  {
    return EXEC_STATUS_ERROR;
  }
  else
  {
    printf("Cleared ID page of M24M01E to 0xFF\r\n ");
  }
  return EXEC_STATUS_OK;
}

/**
  * @brief  Test memory Read/Write operations on M24M01E.
  * @param  None
  * @retval None
  */
app_status_t TestM24M01EMemory(void)
{
  unsigned int addr = 0x00;
  uint16_t idx = 0;
  uint16_t page_count_M24M01E = M24M01E_MEMORYSIZE / M24M01E_PAGESIZE;
  uint16_t n_page;
  uint8_t test_data = 0x85;

  printf("\n\n***************************************************************\r\n");
  printf("                     TEST M24M01E MEMORY\r\n");
  printf("***************************************************************\r\n");

  /* M24M01E */
  /* 1. Read n_page (10) in memory */
  /* 2. Write 1024 bytes to memory starting from address addr(0x65) */
  /* 3. Read 1024 bytes from memory starting from address addr(0x65) */
  /* 4. Check memory contents are same as test_data(0x85) */
  /* 5. Clear memory contents to 0xFF */

  n_page = 10;
  memset(Recv_Buff, 0x00, sizeof(Recv_Buff));
  PRINTF_APPLI("Read M24M01E %d Memory Pages:\r\n", n_page);

  /* 1. Read n_page (10) in memory*/
  if (n_page <= page_count_M24M01E)
  {
    for (uint16_t page_idx = 0; page_idx < n_page; page_idx++)
    {
      if (m24m01e_drv_read_data(pM24m01e0, Recv_Buff, addr, M24M01E_PAGESIZE) != 0)
      {
        return EXEC_STATUS_ERROR;

      }
      else
      {
        PRINTF_APPLI("\n\nPage %d\n Address 0x%2.2X to 0x%2.2X: \r\n", page_idx + 1, addr, addr + M24M01E_PAGESIZE - 1);
        for (idx = 0; idx < M24M01E_PAGESIZE; idx++)
        {
          PRINTF_APPLI("0x%2.2X ", Recv_Buff[idx]);
        }
        addr += M24M01E_PAGESIZE;
        memset(Recv_Buff, 0x00, sizeof(Recv_Buff));

      }
    }
  }
  else
  {
    printf("Number of pages exceeds total page in memory of M24M01E. Abort. \r\n");
    return EXEC_STATUS_ERROR;
  }

  /* 2. Write 1024 bytes to memory starting from address addr(0x65)*/

  memset(Transmit_Buff, test_data, sizeof(Transmit_Buff));
  addr = 0x65;
  printf("\nWrite 1024 bytes to memory from Address 0x%2.2X \r\n", addr);
  if (m24m01e_drv_write_data(pM24m01e0, Transmit_Buff, addr, SIZE1024) != 0)
  {
    return EXEC_STATUS_ERROR;
  }
  else
  {
    HAL_Delay(5);

    /* 3. Read 1024 bytes from memory starting from address addr(0x65)*/

    memset(Recv_Buff, 0x00, sizeof(Recv_Buff));
    if (m24m01e_drv_read_data(pM24m01e0, Recv_Buff, addr, SIZE1024) != 0)
    {
      return EXEC_STATUS_ERROR;

    }
    else
    {
      /* 4. Check memory contents are same as test_data(0x85) */

      for (idx = 0; idx < SIZE1024; idx++)
      {
        if (Recv_Buff[idx] == test_data)
        {
          PRINTF_APPLI("0x%2.2X ", Recv_Buff[idx]);
        }
        else
        {
          break;
        }
      }
      if (idx == SIZE1024)
      {
        printf("\nWrite Success\r\n");
      }
      else
      {
        printf("\nWrite Failed\r\n");
        return EXEC_STATUS_ERROR;
      }

      /* Read Page */

      memset(Recv_Buff, 0x00, sizeof(Recv_Buff));
      printf("Read M24M01E %d Memory Pages:\r\n", n_page);

      addr = 0;
      if (n_page <= page_count_M24M01E)
      {
        for (uint16_t page_idx = 0; page_idx < n_page; page_idx++)
        {
          if (m24m01e_drv_read_data(pM24m01e0, Recv_Buff, addr, M24M01E_PAGESIZE) != 0)
          {
            return EXEC_STATUS_ERROR;

          }
          else
          {
            printf("\n\nPage %d\n Address 0x%2.2X to 0x%2.2X: \r\n", page_idx + 1, addr, addr + M24M01E_PAGESIZE - 1);
            for (idx = 0; idx < M24M01E_PAGESIZE; idx++)
            {
              printf("0x%2.2X ", Recv_Buff[idx]);
            }
            addr += M24M01E_PAGESIZE;
            memset(Recv_Buff, 0x00, sizeof(Recv_Buff));

          }
        }
      }
      else
      {
        printf("Number of pages exceeds total page in memory of M24M01E. Abort. \r\n");
        return EXEC_STATUS_ERROR;
      }
    }

  }

  /* 5. Clear memory contents to 0xFF */
  addr = 0;
  printf("\nReset memory to 0xFF from Address:0x%2.2X \r\n", addr);
  memset(Transmit_Buff, 0xFF, sizeof(Transmit_Buff));

  /* replace n_page with page_count_M24M01E to erase complete memory */
  for (uint16_t page_idx = 0; page_idx < n_page; page_idx++)
  {
    if (m24m01e_drv_write_data(pM24m01e0, Transmit_Buff, addr, M24M01E_PAGESIZE) != 0)
    {
      return EXEC_STATUS_ERROR;
    }
    else
    {
      addr += M24M01E_PAGESIZE;
    }

  }
  printf("\nMemory contents of M24M01E cleared to 0xFF \r\n");
  printf("\nM24M01E ALL Test Cases PASSED \r\n");
  return EXEC_STATUS_OK;
}

/**
  * @brief  Lock CDA register.
  * @param  None
  * @retval None
  */
app_status_t LockCDARegister(void)
{
  printf("\n\n***************************************************************\r\n");
  printf("           LOCK CDA REGISTER (NOTE: IRREVERSIBLE)\r\n");
  printf("***************************************************************\r\n");

#if (!LOCK_CDA_REG_EXECUTE)
  printf("[WARN] Enable LOCK_CDA_REG_EXECUTE in Application to execute! \r\n");
  return EXEC_STATUS_OK;
#else

  uint8_t readCDA = 0xFF;
  uint8_t writeCDA = 0xFF;

  /* Read CDA Register of M24M01E */
  if (m24m01e_drv_read_cda_register(pM24m01e0, &readCDA) != 0)
  {
    printf("M24M01E CDA Register Read Error\r\n");
    return EXEC_STATUS_ERROR;

  }
  else
  {
    printf("M24M01E CDA Register: 0x%2.2X\r\n", readCDA);
  }

  /* Write CDA Register of M24M01E, Set C2 (Bit 3) */
  printf("Setting C2 (Bit 3) and DAL (Bit 0) of CDA register in M24M01E . . .\r\n");
  writeCDA = 0x09;

  if (m24m01e_drv_write_cda_register(pM24m01e0, writeCDA) != 0)
  {
    printf("M24M01E CDA Register Write Error\r\n");
    return EXEC_STATUS_ERROR;
  }
  else
  {
    HAL_Delay(5); /* Tw max is 5ms */

    if (m24m01e_drv_read_cda_register(pM24m01e0, &readCDA) != 0)
    {
      printf("M24M01E CDA Register Read Error\r\n");
      return EXEC_STATUS_ERROR;
    }
    else
    {
      printf("Updated M24M01E CDA Register: 0x%2.2X\r\n", readCDA);
    }

  }

  /* Write CDA Register of M24M01E, Clear C2 (Bit 3) */
  printf("Attempting clearing C2 (Bit 3) of CDA register in M24M01E . . .\r\n");
  writeCDA = 0x00;

  if (m24m01e_drv_write_cda_register(pM24m01e0, writeCDA) != 0)
  {
    printf("Clearing C2 (Bit 3) of CDA register in M24M01E failed as expected . . .\r\n");
    HAL_Delay(5); /* Tw max is 5ms */
    if (m24m01e_drv_read_cda_register(pM24m01e0, &readCDA) != 0)
    {
      printf("M24M01E CDA Register Read Error\r\n");
      return EXEC_STATUS_ERROR;
    }
    else
    {
      printf("M24M01E CDA Register: 0x%2.2X\r\n", readCDA);
    }

  }
  else
  {
    printf("Clearing C2 bit of CDA register in M24M01E done successfully with DAL bit set. Undefined behaviour!\r\n");
    printf("M24M01E CDA Register Error\r\n");
    return EXEC_STATUS_UNKNOWN;
  }
  return EXEC_STATUS_OK;
#endif /* LOCK_CDA_REG_EXECUTE */
}

/**
  * @brief  Lock Identification page of M24M01E.
  * @param  None
  * @retval None
  */
app_status_t TestM24M01ELockIDPage(void)
{
  printf("\n\n***************************************************************\r\n");
  printf("           LOCK ID PAGE (NOTE: IRREVERSIBLE)\r\n");
  printf("***************************************************************\r\n");

#if (!LOCK_ID_PAGE_EXECUTE)
  printf("[WARN] Enable LOCK_ID_PAGE_EXECUTE in Application to execute! \r\n");
  return EXEC_STATUS_OK;
#else
  app_status_t ret;
  if (m24m01e_drv_lock_id_page(pM24m01e0) != 0)
  {
    ret =  EXEC_STATUS_ERROR;
  }
  else
  {
    printf("Permanently Locked ID page of M24M01E. \r\n ");
    ret = EXEC_STATUS_OK;
  }
  return ret;
#endif /* LOCK_ID_PAGE_EXECUTE */
}

/**
  * @brief  Read SWP register of M24M01E.
  * @param  None
  * @retval None
  */
app_status_t ReadSWPRegister(void)
{
  uint8_t readSWP = 0xFF;
  app_status_t ret;

  printf("\n\n***************************************************************\r\n");
  printf("            TEST SWP REGISTER only for M24M01E\r\n");
  printf("***************************************************************\r\n");

  if (m24m01e_drv_read_swp_register(pM24m01e0, &readSWP) != 0)
  {
    printf("M24M01E SWP Register Read Error\r\n");
    ret =  EXEC_STATUS_ERROR;
  }
  else
  {
    printf("M24M01E SWP Register: 0x%2.2X\r\n", readSWP);
    ret = EXEC_STATUS_OK;
  }
  return ret;
}

/**
  * @brief  Write SWP register and test write to memory of M24M01E.
  * @param  None
  * @retval None
  */
app_status_t WriteSWPRegister(void)
{
  uint8_t writeSWP = 0xFF;
  uint8_t readSWP = 0xFF;
  unsigned int addr  = 0x00;
  uint16_t nbytes = 10;
  uint8_t testdata = 0xBB;
  uint16_t idx;
  writeSWP = 0x0E; /* whole memory is write protected */
  app_status_t ret;

  if (m24m01e_drv_write_swp_register(pM24m01e0, writeSWP) != 0)
  {
    ret =  EXEC_STATUS_ERROR;
  }

  else
  {
    HAL_Delay(5);
    if (m24m01e_drv_read_swp_register(pM24m01e0, &readSWP) != 0)
    {
      ret =  EXEC_STATUS_ERROR;
    }
    else
    {
      printf("Modified M24M01E SWP Register to: 0x%2.2X. Complete memory locked.\r\n", readSWP);
      printf("Attempt to write to memory from Address 0x%2.2X with %d bytes of data. . .\r\n", addr, nbytes);
      memset(Transmit_Buff, testdata, sizeof(Transmit_Buff));
      if (m24m01e_drv_write_data(pM24m01e0, Transmit_Buff, addr, nbytes) != 0)
      {
        printf("Write failed as expected\r\n");
      }
      else
      {
        ret =  EXEC_STATUS_ERROR;
      }
      PRINTF_APPLI("Read memory from Address 0x%2.2X with %d bytes of data to check data is not written ...\r\n", addr,
                   nbytes);

      /* Read Data Written*/
      memset(Recv_Buff, 0x00, sizeof(Recv_Buff));
      if (m24m01e_drv_read_data(pM24m01e0, Recv_Buff, addr, nbytes) != 0)
      {
        ret =  EXEC_STATUS_ERROR;

      }
      else
      {
        for (idx = 0; idx < nbytes; idx++)
        {
          if (Recv_Buff[idx] == testdata)
          {
            break;
          }
          else
          {
            PRINTF_APPLI("0x%2.2X \r", Recv_Buff[idx]);
          }
        }
        if (idx == nbytes)
        {
          printf("Checked, data not written to memory as expected\r\n");

        }
      }

    }
  }

  writeSWP = 0x00; /* whole memory can be written */
  if (m24m01e_drv_write_swp_register(pM24m01e0, writeSWP) != 0)
  {
    ret =  EXEC_STATUS_ERROR;
  }

  else
  {
    HAL_Delay(5);
    if (m24m01e_drv_read_swp_register(pM24m01e0, &readSWP) != 0)
    {
      ret =  EXEC_STATUS_ERROR;
    }
    else
    {
      printf("Updated M24M01E SWP Register: 0x%2.2X\r\n", readSWP);
    }
  }
  return ret;

}



