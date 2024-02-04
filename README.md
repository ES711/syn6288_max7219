# SYN6288 & MAX7219

## 專案說明
這份專案中包含以下模組

1. SYN6288語音合成模組
2. MAX7219點矩陣驅動模組(16*16)

**採用freeRTOS的方式進行開發**

目前預設點矩陣1秒會更新一次

語音合成每8秒合成一次

## IO設定
**電路板已經layout，請不要修改腳位設定**

### SPI1
* PA5 -> SPI1_SCK

* PA6 -> SPI1_MISO

* PA7 -> SPI_MOSI

* Data Size -> 8 Bits

* Prescaler -> 2 (12.5MBits/s)

### USART6
* PC6 -> USART6_TX

* PC7 -> USART6_RX

### GPIO
* PF13 -> GPIO_Output
    
    PF13的Label有修改為Max7219_8x8(非必要)

### RCC
* High Speed Clock(HSE) -> Crystal/Ceramic Resonator

### SYS
* Debug -> Serial Wire

* TimeBase Source -> TIM1(Sys_Tick被freeRTOS接管)

### Middleware
* FREERTOS -> CMSIS_V2

    其餘都不用修改

## Project Manager
* Code Generator ->  Generator peripheral...
    
    要勾選這樣才會把每個io的.h/.c生成出來

## 程式說明

### 主程式

#### taskTTS -> 負責控制syn6288

以下是向syn6288發送語音資料的方法
```c
SYN_FrameInfo(0, syn6288_data);
```

文字的格式如下

* [v6] -> 語音的音量大小，0~15

* [m0] -> 背景音樂的音量大小，0~15

* [t5] -> 說話語速 -> 0~5

```c
"[v6][m0][t5]xxxxxx"
```

這邊要注意語音資料必須是以big5編碼，且需要將資料轉換為hex格式

原本的字串如下
```
"[v6][m0][t5]雲林科技大學"
```
以big5編碼，並轉換為hex格式
```c
uint8_t syn6288_data[] = {0x5b,0x76,0x36,0x5d,0x5b,0x6d,0x30,0x5d,0x5b,0x74,0x35,0x5d,0xb6,0xb3,0xaa,0x4c,0xac,0xec,0xa7,0xde,0xa4,0x6a,0xbe,0xc7};
```
##### tips
由於c本身並沒有支援編碼轉換的功能，

因此語音資料的hex格式可以使用像是python或是線上編碼轉換器之類的工具來生成

文末有python編碼轉換並且生成hex資料的程式碼可以參考

---

#### taskYuntech -> 負責控制點矩陣

這邊我們先看用來顯示點矩陣的資料

下面的範例是用來表示⌈林⌋這個字的點矩陣
```c
//林
{
    /*
    0x00, 0x00 > top left, top right
    0x36, 0x70 > bottom left, bottom right
    */
    //top
    0x00,0x00,0x18,0x30,0x10,0x20,0x14,0x24,
    0xFE,0xFE,0x10,0x20,0x30,0x60,0x38,0x70,
    //bottom
    0x36,0x70,0x52,0xA8,0x50,0xAC,0x91,0x26,
    0x12,0x24,0x10,0x20,0x10,0x20,0x10,0x20
}
```
總共有4塊8*8的點矩陣，而一個點矩陣又有8列

因此一個字需要32個hex來表示

前16個元素為上面兩個點矩陣 -> 兩個一組，因此(0x00, 0x00)分別為(左上, 右上)

後16個元素為下面兩個點矩陣 -> 兩個一組，因此(0x36, 0x70)分別為(左下, 右下)

以下為控制點矩陣的方式
```c
for(int l = 0; l < 6; l++)//total 6 words
{
    for(int i = 0; i < 8; i++)//one step one line > so run 8 times
    {
        Max7219_data[0] = YunTech_Logo[l][i*2];//top left 
        Max7219_data[1] = YunTech_Logo[l][i*2 + 1];//top right
        Max7219_data[2] = YunTech_Logo[l][i*2 + 16];//bottom left 
        Max7219_data[3] = YunTech_Logo[l][i*2 + 17];//bottom right
        Write_Matrix(i + 1, Max7219_data);
    }
    vTaskDelay(1000);
}
```
看有幾個字，外迴圈就執行幾次

內迴圈則是因為一塊點矩陣有8行，因此執行8次 -> 每次只更新一行

Write_Matrix()

一次更新一塊
```c
HAL_GPIO_WritePin(GPIOF, Max7219_8x8_Pin, GPIO_PIN_RESET); // NSS1 low
for(int i = 0; i < MAXInUse; i++)//
{
    HAL_SPI_Transmit(&hspi1, &address, 1 ,100);   //write address            
    HAL_SPI_Transmit(&hspi1, &dat[i], 1 ,100);    //write data 	
}
HAL_GPIO_WritePin(GPIOF, Max7219_8x8_Pin, GPIO_PIN_SET); // NSS1 HIGH
```

### Hardware 資料夾
這邊會有4個檔案
* syn6288.h/.c -> 語音合成
* max7219.h/.c -> 點矩陣

.h是標頭檔，用來宣告.c裡面有甚麼function

#### syn6288.c
```c
/*
0x00 -> GB2312
0x01 -> GBK
0x02 -> BIG5
*/
frame[4] = 0x02 | music << 4;
```
要注意的只有這邊，

syn6288支援三種編碼格式，

其他兩種都是簡體中文，

因此專案中語音合成的部分都會以big5的編碼進行

#### max7219.c
驅動16*16點矩陣的max7219總共有4塊

這個function是用來寫入單顆max7219用的
```c
Write_Max7219(uint8_t address, uint8_t dat)
```

這個function是用來寫入矩陣用的
```c
Write_Matrix(uint8_t address, uint8_t* dat)
```

### python big5編碼並轉換成hex
```python
big5_str = "[v6][m0][t5]雲林科技大學".encode("big5")
hex_str = ""
for i in big5_str:
    hex_str += f"{hex(i)},"
print(hex_str.strip(","))
```

### Tips

這邊的情況通常只會發生在使用.ioc重新生成專案才會發生

非必要請避免此操作

#### 找不到syn6288.c、max7219.c

Manage Project Items

新增Hardware資料夾，並加入syn6288.c、max7219.c兩個檔案

#### 找不到syn6288.h、max7219.h

Options for Target 'syn6288_max7219' >> c/c++ >> Include Paths 

新增Hardware資料夾路徑