#include "onewire.h"

extern uint8_t command_buffer[8]; // Буффер команд
extern uint8_t read_buffer[8]; // Буффер для чтения
extern uint8_t temperature_buffer[2]; //Сюда записываем данные с датчика
extern uint8_t id_buffer[24]; //Буффер для уникального ID датчиков
extern uint8_t number_of_devices; //Количество устройств
extern uint8_t massage_buffer[12]; //Буффер для общения с 1-Wire
extern float result_buffer; //Буффер для ответа
extern uint8_t sleep_flag; //Флаг сна КЭП

extern UART_HandleTypeDef huart1;

//-----------------------------------------------------------------------------
// sendReset - посылать RESET в начале общения.
// 		OW_SEND_RESET или OW_NO_RESET
// процедура общения с шиной 1-wire
// command - массив байт, отсылаемых в шину.
// cLen - длина буфера команд, столько байт отошлется в шину
// data - если требуется чтение, то ссылка на буфер для чтения
// dLen - длина буфера для чтения. Прочитается не более этой длины
// readStart - с какого символа передачи начинать чтение (нумеруются с 0)
//		можно указать OW_NO_READ, тогда можно не задавать data и dLen
//-----------------------------------------------------------------------------
uint8_t OW_Send(uint8_t sendReset, uint8_t *command, uint16_t cLen, uint8_t *data, uint16_t dLen, uint8_t readStart) 
{
  if (sendReset == OW_SEND_RESET) {
    if (OW_Reset() == OW_NO_DEVICE) {
      return OW_NO_DEVICE;
    }
  }
  while (cLen > 0) {
    OW_toBits(*command, command_buffer);
    command++;
    cLen--;
    
    /*Посылаем команду, после чего начинаем читать данные*/
    OW_SendBits(8);
    
    /*Если прочитанные данные кому-то нужны - выкинем их в буфер*/
    if (readStart == 0) {
      *data = OW_toByte(read_buffer);
      data++;
      dLen--;
    } else {
      if (readStart != OW_NO_READ) {
        readStart--;
      }
    }
  }
  return OW_OK;
}
/*Осуществляет сброс и проверку на наличие устройств на шине*/
uint8_t OW_Reset() 
{
  uint8_t presence = 1;
  
  HAL_UART_DeInit(&huart1);
  
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_HalfDuplex_Init(&huart1);

  __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_TC);
  USART1->DR = 0xf0;
  while (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TC) == RESET);  
  presence = USART1->DR;
  
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_HalfDuplex_Init(&huart1);
  
  if (presence != 0xf0) {
    return OW_OK;
  }
  return OW_NO_DEVICE;
}
//------------------------------------------------------------------------------
//Данная функция осуществляет сканировнаие сети 1-wite и записывает найденные 
//ID устройств в массив buf, по 8 байт на каждое устройство.
//Переменная num ограничивает количество находимых устройств, чтобы не 
//переполнить буфер.
//------------------------------------------------------------------------------
uint8_t OW_Scan(uint8_t *buf, uint8_t num) 
{
  uint8_t found = 0;
  uint8_t *lastDevice;
  uint8_t *curDevice = buf;
  uint8_t numBit, lastCollision, currentCollision, currentSelection;
  
  lastCollision = 0;
  while (found < num) {
    numBit = 1;
    currentCollision = 0;
    
    /*Посылаем команду на поиск устройств*/
    OW_Send(OW_SEND_RESET, (uint8_t *)"\xf0", 1, 0, 0, OW_NO_READ);
    
    for (numBit = 1; numBit <= 64; numBit++) {
      OW_toBits(OW_READ_SLOT, command_buffer);
      OW_SendBits(2);
      
      if (read_buffer[0] == OW_R_1) {
        if (read_buffer[1] == OW_R_1) {
          //Две единицы, заканчиваем поиск
          return found;
        } else {
          //10 - на данном этапе только 1
          currentSelection = 1;
        }
      } else {
        if (read_buffer[1] == OW_R_1) {
          //01 - на данном этапе только 0
          currentSelection = 0;
        } else {
          //00 - коллизия
          if (numBit < lastCollision) {
            //идём по дереву
            if (lastDevice[(numBit -1) >> 3] & 1 << ((numBit - 1) & 0x07)) {
              //(numBit -1) >> 3 - номер байта
              //(numBit - 1) & 0x07 - номер бита в байте
              currentSelection = 1;
              
              //Если пошли по правой ветке, запоминаем номер бита
              if (currentCollision < numBit) {
                currentCollision = numBit;
              }
            } else {
              currentCollision = 0;
            }
          } else {
            if (numBit == lastCollision) {
              currentSelection = 0;
            } else {
              //Идём по правой ветке
              currentSelection = 1;
              
              //Если пошла по правой ветке, запоминаем номер бита
              if (currentCollision < numBit) {
                currentCollision = numBit;
              }
            }
          }
        }
      }
      if (currentSelection == 1) {
        curDevice[(numBit - 1) >> 3] |= 1 << ((numBit - 1) & 0x07);
        OW_toBits(0x01, command_buffer);
      } else {
        curDevice[(numBit - 1) >> 3] &= ~(1 << ((numBit - 1) & 0x07));
        OW_toBits(0x00, command_buffer);
      }
      OW_SendBits(1);
    }
    found++;
    lastDevice = curDevice;
    curDevice += 8;
    if (currentCollision == 0)
      return found;
    
    lastCollision = currentCollision;
  }
  return found;
}

/*Внутренняя процедура. Записивает указанное число бит*/
void OW_SendBits(uint8_t num_bits) 
{
  uint8_t i;
  for (i = 0; i < num_bits; i++) {
    __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_TXE);
    __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_RXNE);
    USART1->DR = command_buffer[i];    
    while (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_TXE) == RESET);
    while (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE) == RESET);
    if (OW_toByte(command_buffer) == 0xff) {
      read_buffer[i] = USART1->DR;
    }
  }
}
/*Создание сообщения*/
void OW_Massage(uint8_t flag)
{
  uint8_t i;
  massage_buffer[0] = OW_MATCH_ROM;
  massage_buffer[9] = OW_READ_SCRATCHPAD;
  massage_buffer[10] = massage_buffer[11] = OW_READ_SLOT;
  for (i = 1; i < 9; i++){
    massage_buffer[i] = id_buffer[(i-1)+(8*flag)];
  }
}
/*Вывод результата на экран*/
void OW_Output(void)
{
  if (massage_buffer[1] == 0x10) {
    printf("Sensor 1820: ");
    result_buffer = (float)temperature_buffer[0];
    printf("%.1lf\n", result_buffer/2);
    
  } 
  if (massage_buffer[1] == 0x28) {
    printf("Sensor 18B20: ");
    result_buffer = (float)(temperature_buffer[0] | (temperature_buffer[1] << 8));
    printf("%.1lf", result_buffer/16);
  }
}

/*byte to bits*/
void OW_toBits(uint8_t byte, uint8_t *bits) 
{
  uint8_t i;
  for (i = 0; i < 8; i++) {
    if (byte & 0x01) {
      *bits = OW_1;
    } else {
      *bits = OW_0;
    }
    bits++;
    byte = byte >> 1;
  }
}
/*bits to byte*/
uint8_t OW_toByte(uint8_t *bits)
{
  uint8_t byte, i;
  byte = 0;
  for (i = 0; i < 8; i++) {
    byte = byte >> 1;
    if (*bits == OW_R_1) {
      byte |= 0x80;
    }
    bits++;
  }
  return byte;
}
