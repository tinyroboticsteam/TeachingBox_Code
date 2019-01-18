//## http://arduino.ru/forum/proekty/antidrebezg-dlya-shilda-lcd-knopki

//#define digitdebug
//#define letterdebug
//#define syllabledebug
//#define modedebug

#include <LiquidCrystal_1602_RUS.h>
LiquidCrystal_1602_RUS lcd(9, 8, 4, 5, 6, 7 );

#define bs_none   0         // статус - "нет нажатия"
#define bs_push   1         // статус - "нажатие"
#define bs_hold_1 2         // статус - "удержание (1-2 секунды) и более", через(5-30 секунд) - переход в статус 3
#define bs_hold_2 3         // статус - "удержание (5-30 секунд) и более" 

#define bf_none   0         // значение флага нет нажатие
#define bf_push   1         // значение флага нажатие
#define bf_hold_1 2         // значение флага удержание (0,5-2 секунды) 
#define bf_hold_2 3         // значение флага удержание (5-30 секунд)и более 

#define long_1_Time 750     // удержание (0,5-2 секунды) - статус 2, через(5-30 секунд)переход в статус 3
#define long_2_Time 5000    // удержание (5-30 секунд)и более - статус 3
#define long_3_Time 250     // время авто нажатий
long timerButton_2 = 0;     // время задержка кнопки для антидребезга

#define bs_read_1 0         // статус первое чтение с АЦП
#define bs_lag    1         // статус задержка и второе чтение с АЦП 
#define bs_score  2         // статус вычисление разницы между 2 чтениями 
#define bs_change 3         // статус присвоения кнопки

#define bf_read_1 0         // значение флага первое чтение с АЦП
#define bf_lag    1         // значение флага задержка и второе чтение с АЦП 
#define bf_score  2         // значение флага вычисление разници между 2 чтениями 

byte deBounce = 0;          // переменная СТАТУС антидребезга

#define deBounceTime  20    // задержка (в мс от 5 до 30 мс) для реализации антидребезга
#define p_p           0     // устанавливаем разницу (в единицах от 0 до 10) между двумя считываниями значений с АЦП входа А0

int temp_read = 1023;
int temp_read_1;
int temp_read_2;
int temp;

int clik_1;
int clik_10 = 0;
int clik_s = 0;

byte buttonPin = A0;       // пин подключения кнопки (на АЦП получаем: кнопка не нажата кнопка 0>850, кнопка 1<90,кнопка 2<250, кнопка3<430,кнопка 4<650,кнопка 5<850)
byte buttonState = 0;      // переменная СТАТУС КНОПКИ
byte curButton = 0;        // ТЕКУЩАЯ КНОПКА
long timerButton = 0;      // Время нажатия кнопки
long timerButton_1 = 0;    // Время задержка кнопки для антидребезга
byte pressBut = 0;
byte buttonFlag = 0;

//---------------------------------------------------------------
//    Массив строк для хранения слов
//---------------------------------------------------------------

char* myStrings[] = {
  "А",
  "Арбуз",
  "Б",
  "Ботинки",
  "В",
  "Велосипед",
  "Г",
  "Гусь",
  "Д",
  "Дом",
  "Е",
  "Енот",
  "Ё",
  "Ёж",
  "Ж",
  "Жук",
  "З",
  "Зайка",
  "И",
  "Иголка",
  "Й",
  "Йогурт",
  "К",
  "Кот",
  "Л",
  "Лиса",
  "М",
  "Медведь",
  "Н",
  "Носки",
  "О",
  "Окошко",
  "П",
  "Платье",
  "Р",
  "Рыба",
  "С",
  "Слон",
  "Т",
  "Тигр",
  "У",
  "Уточка",
  "Ф",
  "Флаг",
  "Х",
  "Хлеб",
  "Ц",
  "Цветок",
  "Ч",
  "Чашка",
  "Ш",
  "Шар",
  "Щ",
  "Щука",
  "Ъ",
  "сЪел",
  "Ы",
  "Ыыыы",
  "Ь",
  "гусЬ",
  "Э",
  "Экскаватор",
  "Ю",
  "Юла",
  "Я",
  "Яблоко"
};

//---------------------------------------------------------------
//    Массив строк для хранения слогов
//---------------------------------------------------------------

char* mySyllables[] = {
  "МА",
  "МА-МА",
  "ПА",
  "ПА-ПА",
  "БА",
  "БА-БА",
  "НЯ",
  "НЯ-НЯ",
  "ВА, СЯ",
  "ВА-СЯ",
  "ФА",
  "ФА-Я",
  "РЫ, БА",
  "РЫ-БА"
};

byte type     = -1; // текущий режим: цифры - 0, буквы - 1, слоги - 2
byte prevbutt = 0;  // предыдущая нажатая кнопка: LEFT или DOWN, нужно для правильного последовательного перехода вперёд / назад

//--------------------- start ReadKey ---------------------------
//    Функция АНТИДРЕБЕЗГ с присвоением номера нажатой кнопки
//---------------------------------------------------------------
byte ReadKey()                                     // Функция устраняет дребезг контактов: считывает значение с АЦП и преобразует в номер нажатой кнопки
{
  switch (deBounce)                                // Статус антидребезга
  {
    case bs_read_1:
      temp_read_1 = analogRead(buttonPin);         // первое считывание с АЦП входа А0
      timerButton_1 = millis();                    // запоминаем время
      deBounce = bs_lag;
      break;
    case bs_lag:                                   // задержка
      if (millis() - timerButton_1 > deBounceTime) // задержка между считываниями
      {
        temp_read_2 = analogRead(buttonPin);       // второе считывание с АЦП входа А0
        deBounce = bs_score;
        break;
      }
      deBounce = bs_lag;
      break;
    case bs_score:                                 // вычисление разници между 2 чтениями
      temp = temp_read_1 - temp_read_2;            // определение разницы значений считаных с АЦП
      if (temp < 0)temp = (int)(temp * -1);        // делаем положительный знак
      if (temp > p_p)
      {
        deBounce = bs_read_1;
        break;
      }
      deBounce = bs_change;
      break;
    default:
      temp_read = ((temp_read_2 + temp_read_1) / 2); //усредняем значение
      deBounce = bs_read_1;
  }
  if (temp_read <  80) return 1;                   // RIGHT  вправо
  if (temp_read < 237) return 2;                   // UP     вверх
  if (temp_read < 412) return 3;                   // DOWN   вниз
  if (temp_read < 626) return 4;                   // LEFT   влево
  if (temp_read < 866) return 5;                   // SELECT выбрать
  return 0;
} //end ReadKey
//---------------- start checkButton ----------------
//   Присвоение статуса кнопки
//---------------------------------------------------
void checkButton() {
  switch (buttonState)  {                          // проверяем СТАТУС КНОПКИ
    //------------------------------------------
    case (bs_none):                                // если ничего не нажато(=0)
      curButton = ReadKey();                       // считываем номер нажатой кнопки
      if (curButton != 0)                          // если кнопка нажата (т.е., значение >0)
      {
        buttonState = bs_push;                     // следующий статус нажатия (=1)
        buttonFlag = bs_push;                      // флаг сработки = нажата кнопка
        timerButton = millis();                    // запоминаем время
      }
      else
      {
        buttonState = bs_none;                     //флаг сработки = статуса нет нажатия кнопки
        buttonFlag = bf_none;                      //флаг сработки = нет нажатия кнопки
      }
      break;
    //------------------------------------------
    case (bs_push):                                // если нажата кнопка 
      if ((ReadKey() == curButton))                // если кнопка все еще нажата
      {
        if (millis() - timerButton > long_1_Time)  // если кнопка удерживается (1-2 секунды) и более
        {
          buttonState = bs_hold_1;                 // статус удержание (1-2 секунды) и более (=2)
          buttonFlag = bs_hold_1;                  // флаг сработки = нажатие кнопки
        }
      }
      else
      {
        buttonState = bs_none;                     // флаг сработки = статуса нет нажатия кнопки
        buttonFlag = bf_none;                      // флаг сработки = нет нажатия кнопки
      }
      break;
    //------------------------------------------
    case (bs_hold_1):                              // удержание (1-2 секунды)и более (=2)
      if ((ReadKey() == curButton))                // если кнопка все еще нажата
      {
        if (millis() - timerButton > long_2_Time)  // если кнопка удерживается(5-30 секунд))и более
        {
          buttonState = bs_hold_2;                 // изменение статуса на "удержание (5-30 секунд) и более"
          buttonFlag = bf_hold_2;                  // флаг сработки = 1-2 секунды и более
        }
      }
      else
      {
        buttonState = bs_none;                     // флаг сработки = статуса нет нажатия кнопки
        buttonFlag = bf_none;                      // флаг сработки = нет нажатия кнопки
      }
      break;
    //------------------------------------------
    case (bs_hold_2):                              // удержание (5-30 секунд)и более (=3)
      if ((ReadKey() == curButton))                // если кнопка все еще нажата
      {
        if (millis() - timerButton > long_1_Time)  // если кнопка удерживается более времени для автоповтора
        {
          timerButton = millis();                  // запоминаем время
          buttonFlag = bf_hold_2;                  // флаг сработки = 5-30 секунд и более
        }
      }
      else
      {
        buttonState = bs_none;                     // флаг сработки = нет нажатия кнопки
        buttonFlag = bf_none;                      // флаг сработки = нет нажатия кнопки
      }
      break;
      //------------------------------------------
  }  //end switch
}    //end checkButton
//------------------------
int lik = 0;
int clik = 0;
int s = 0;

void setup()
{
  lcd.begin(16, 2);                                 // Инициализация LCD экрана
  lcd.setCursor(0, 0);
  lcd.print(L"Максим, привет!");                    // Печать приветствия
  delay(500);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
}

//---------------------------------------------------------------
//    Функция для вывода на экран цифр от 0 до 9 и числа 10
//---------------------------------------------------------------

void mydigit(byte ind)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  if (ind == 0)
  {
    lcd.print(L"0 - это ноль");
    lcd.setCursor(0, 1);
    lcd.print(L"");
  }
  if (ind == 1)
  {
    lcd.print(L"1 - один");
    lcd.setCursor(0, 1);
    lcd.print(L"|");
  }
  if (ind == 2)
  {
    lcd.print(L"2 - два");
    lcd.setCursor(0, 1);
    lcd.print(L"||");
  }

  if (ind == 3)
  {
    lcd.print(L"3 - три");
    lcd.setCursor(0, 1);
    lcd.print(L"|||");
  }

  if (ind == 4)
  {
    lcd.print(L"4 - четыре");
    lcd.setCursor(0, 1);
    lcd.print(L"||||");
  }

  if (ind == 5)
  {
    lcd.print(L"5 - пять");
    lcd.setCursor(0, 1);
    lcd.print(L"|||||");
  }

  if (ind == 6)
  {
    lcd.print(L"6 - шесть");
    lcd.setCursor(0, 1);
    lcd.print(L"||||||");
  }

  if (ind == 7)
  {
    lcd.print(L"7 - семь");
    lcd.setCursor(0, 1);
    lcd.print(L"|||||||");
  }

  if (ind == 8)
  {
    lcd.print(L"8 - восемь");
    lcd.setCursor(0, 1);
    lcd.print(L"||||||||");
  }

  if (ind == 9)
  {
    lcd.print(L"9 - девять");
    lcd.setCursor(0, 1);
    lcd.print(L"|||||||||");
  }

  if (ind == 10)
  {
    lcd.print(L"10 - десять");
    lcd.setCursor(0, 1);
    lcd.print(L"||||||||||");
  }
}

int digitcnt     = 0;   // счётчик, показывающий отображаемую цифру / число
int lettercnt    = 0;   // счётчик, показывающий отображаемую букву
int syllablecnt  = 0;   // счётчик, показывающий отображаемый слог

int prevrandnum  = -1;  // предыдущее сгенерированное случайное число для цифр
int prevrandnum2 = -1;  // предыдущее сгенерированное случайное число для букв
int prevrandnum3 = -1;  // предыдущее сгенерированное случайное число для слогов

void loop()
{
  checkButton();

  if (buttonFlag != bf_none)                          // Если кнопка нажата и статус кнопки не равен нулю
  {
//---------------------------------------------------------------
//    Кнопка UP отвечает за случайный режим вывода информации на экран
//    Алгоритм обработки нажатия кнопки одинаков для любого режима:
//    цифр, букв, слогов
//---------------------------------------------------------------
    
    if (curButton == 2 && buttonFlag == 1) {          // Однократное нажатие кнопки UP

      buttonFlag = bf_none;
      
      if (type == 0)                                  // текущий режим: цифры - 0, буквы - 1, слоги - 2
      {
        int randNumber = random(0, 11);               // генерация случайного числа из диапазона 0-10
        while (prevrandnum == randNumber)             // цикл используется, чтобы следующее случайное число не было равно предыдущему
        {
          randNumber = random(0, 11);
        }
        mydigit(randNumber);                          // вывод на экран цифры или числа 10 в соответствии со случайно сгенерированным значением
        prevrandnum =  randNumber;                    // сохраняется последнее сгенерированное случайное число
      }

      if (type == 1)                                
      {
        int randNumber = random(0, 32);
        while (prevrandnum2 == randNumber)
        {
          randNumber = random(0, 32);
        }

        int newlettercnt = randNumber * 2;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(myStrings[newlettercnt]);
        lcd.setCursor(0, 1);
        lcd.print(myStrings[newlettercnt + 1]);

        prevrandnum2 = randNumber;
      }

      if (type == 2)
      {
        int randNumber = random(0, 6);
        while (prevrandnum3 == randNumber)
        {
          randNumber = random(0, 6);
        }

        int newlsyllablecnt = randNumber * 2;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(mySyllables[newlsyllablecnt]);
        lcd.setCursor(0, 1);
        lcd.print(mySyllables[newlsyllablecnt + 1]);

        prevrandnum3 = randNumber;
      }

    }
    if (curButton == 2 && buttonFlag == 2) {         // Удержание кнопки UP: не используется
      if (millis() - timerButton_2 > long_3_Time) {
        timerButton_2 = millis();
      }
    }

//---------------------------------------------------------------
//    Кнопка DOWN отвечает за режим перехода к следующему элементу
//    для отображения: цифре, букве или слогу
//    Алгоритм обработки нажатия кнопки одинаков для любого режима: 
//    цифр, букв, слогов
//---------------------------------------------------------------
    if (curButton == 3 && buttonFlag == 1) {           // Однократное нажатие кнопки DOWN
      buttonFlag = bf_none;

      if (type == 0)
      {
//---------------------------------------------------------------
//    Следующий оператор IF обрабатывает случай, когда после нажатия кнопки LEFT (назад)
//    сразу нажимается кнопка DOWN (вперёд): 
//    например, если отображается цифра 4, и до этого была нажата кнопка LEFT (назад), 
//    то при нажатии на DOWN (вперёд) должна отображаться цифра 5
//    особенность кода такая, что если эту ситуацию не обработать,
//    то сначала будет отображена цифра 3, и только потом, после ещё
//    двух нажатий кнопки DOWN будет отображена цифра 5
//    При обработке учитывается предыдущая нажатая кнопка
//---------------------------------------------------------------
        #ifdef digitdebug
        Serial.print("DOWN: in digitcnt = ");
        Serial.println(digitcnt);
        #endif
        if (prevbutt == 1) // 1 - LEFT: если предыдущая нажатая кнопка - LEFT
        {
          if (digitcnt == 10) // нумерация начинается с 0, поэтому значение 10
          {                   // элементы в массиве нумеруются от 0 до 10
                              // этот IF работает, когда с помощью LEFT дошли до 0
                              // а потом нажали DOWN и перешли к 1. 
                              // Это пограничная ситуация
            digitcnt = 1;
            #ifdef digitdebug            
            Serial.println("DOWN: set digitcnt = 1");
            #endif
          }
          else
          {
            digitcnt = digitcnt + 2;
            if (digitcnt > 10)
              digitcnt = 0;
            #ifdef digitdebug
            Serial.print("DOWN: set digitcnt = ");
            Serial.println(digitcnt);              
            #endif  
          }
        }
        
        #ifdef digitcnt
        Serial.print("DOWN: print digitcnt = ");
        Serial.println(digitcnt);        
        #endif
        
        mydigit(digitcnt);       // вывод цифры или числа на экран
        digitcnt = digitcnt + 1; // переход к следующему числу
        if (digitcnt == 11)      // этот if обрабатывает условие, когда после 
          digitcnt = 0;          // нажатия на кнопку DOWN от 10 надо перейти к 0 
        
        #ifdef digitdebug
        Serial.print("DOWN: out digitcnt = ");
        Serial.println(digitcnt);
        #endif
          
      }

      if (type == 1)  
      {
//---------------------------------------------------------------
//    Следующий оператор IF обрабатывает случай, когда после нажатия кнопки LEFT (назад)
//    сразу нажимается кнопка DOWN (вперёд): 
//    например, если отображается буква "В", и до этого была нажата кнопка LEFT (назад), 
//    то при нажатии на DOWN (вперёд) должна отображаться цифра буква "Г"
//    особенность кода такая, что если эту ситуацию не обработать,
//    то сначала будет отображена буква "Б", и только потом, после ещё
//    двух нажатий кнопки DOWN будет отображена буквва "Г"
//---------------------------------------------------------------
        if (prevbutt == 1) // если предыдущая кнопка: 1 - LEFT
        {
        #ifdef letterdebug
        Serial.print("DOWN: in lettercnt = ");
        Serial.println(lettercnt);
        #endif
                  
          if (lettercnt == 62) // элементы в массиве нумеруются от 0 до 65
          {
            lettercnt = 0;
            #ifdef letterdebug
            Serial.println("DOWN: set lettercnt = 0");            
            #endif
          }
          else
          {
            lettercnt = lettercnt + 4;
            if (lettercnt > 66)
              lettercnt = 0;
            #ifdef letterdebug
            Serial.print("DOWN: set lettercnt = ");
            Serial.println(lettercnt);                    
            #endif
          }
        }
        #ifdef letterdebug
        Serial.print("DOWN: print lettercnt = ");
        Serial.println(lettercnt);   
        #endif
                
        lcd.clear();                           // вывод буквы и соответствующего ей слова на экран
        lcd.setCursor(0, 0);
        lcd.print(myStrings[lettercnt]);
        lcd.setCursor(0, 1);
        lcd.print(myStrings[lettercnt + 1]);
        
        lettercnt = lettercnt + 2;            // переход к следующей букве 
        
        if (lettercnt == 66)                  // этот if обрабатывает условие, когда после нажатия 
          lettercnt = 0;                      // на кнопку DOWN от буквы "Я" нужно перейти к "А"
        
        #ifdef letterdebug
        Serial.print("DOWN: out lettercnt = ");
        Serial.println(lettercnt);          
        #endif
      }
      if (type == 2)
      {
        if (prevbutt == 1 ) // 1 - LEFT
        {
//---------------------------------------------------------------
//    Следующий оператор IF обрабатывает случай, когда после нажатия кнопки LEFT (назад)
//    сразу нажимается кнопка DOWN (вперёд): 
//    например, если отображается слог "БА", и до этого была нажата кнопка LEFT (назад), 
//    то при нажатии на DOWN (вперёд) должен отображаться слог "НЯ"
//    особенность кода такая, что если эту ситуацию не обработать,
//    то сначала будет отображён слог "ПА", и только потом, после ещё
//    двух нажатий кнопки DOWN будет отображён слог "НЯ"
//---------------------------------------------------------------
        #ifdef syllabledebug
        Serial.print("DOWN: in syllable = ");
        Serial.println(syllablecnt);
        #endif
        
          if (syllablecnt == 10)
          {
            syllablecnt = 0;
            #ifdef syllabledebug
            Serial.println("DOWN: set syllablecnt = 0");            
            #endif
          }
          else
          {
            syllablecnt = syllablecnt + 4;
            if (syllablecnt > 14)
              syllablecnt = 0;
            #ifdef syllabledebug
            Serial.print("DOWN: set syllablecnt = ");
            Serial.println(syllablecnt);                    
            #endif
          }
        }
        #ifdef syllabledebug
        Serial.print("DOWN: print lettercnt = ");
        Serial.println(syllablecnt);   
        #endif

        lcd.clear();                           // вывод слога на экран
        lcd.setCursor(0, 0);
        lcd.print(mySyllables[syllablecnt]);
        lcd.setCursor(0, 1);
        lcd.print(mySyllables[syllablecnt + 1]);

        syllablecnt = syllablecnt + 2;    // этот if обрабатывает условие, когда после нажатия
        if (syllablecnt == 14)            // на кнопку DOWN от слога "МА" нужно перейти к "РЫ, БА"
          syllablecnt = 0;

        #ifdef syllabledebug
        Serial.print("DOWN: out syllablecnt = ");
        Serial.println(syllablecnt);          
        #endif
      }

      prevbutt = 2; // DOWN

    }
    if (curButton == 3 && buttonFlag == 2) {         // Удержание кнопки DOWN: не используется
      if (millis() - timerButton_2 > long_3_Time) {
      }
    }
    
    if (curButton == 1 && buttonFlag == 1) {            // Однократное нажатие кнопки RIGHT: не используется
      buttonFlag = bf_none;
    }

//---------------------------------------------------------------
//    Кнопка LEFT отвечает за режим перехода к предыдущему 
//    элементу для отображения: цифре, букве или слогу
//    Алгоритм обработки нажатия кнопки одинаков для любого режима: 
//    цифр, букв, слогов
//---------------------------------------------------------------
    
    if (curButton == 4 && buttonFlag == 1) {            // Однократное нажатие кнопки LEFT
      buttonFlag = bf_none;
      if (type == 0)
      {
        #ifdef digitdebug
        Serial.print("LEFT: in digitcnt = ");
        Serial.println(digitcnt);
        #endif

        if (prevbutt == 2 ) // 2 - DOWN
        {
//---------------------------------------------------------------
//    Следующий оператор IF обрабатывает случай, когда после нажатия кнопки DOWN (вперёд)
//    сразу нажимается кнопка LEFT (назад): 
//    например, если отображается цифра 4, и до этого была нажата кнопка DOWN (вперёд), 
//    то при нажатии на LEFT (назад) должна отображаться цифра 3
//    особенность кода такая, что если эту ситуацию не обработать,
//    то сначала будет отображена цифра 5, и только потом, после ещё
//    двух нажатий кнопки LEFT будет отображена цифра 3
//    При обработке учитывается предыдущая нажатая кнопка
//---------------------------------------------------------------
          if (digitcnt == 0)
          {
            digitcnt = 9;
            #ifdef digitdebug
            Serial.print("LEFT: set digitcnt = 9");
            #endif
            
          }
          else
          {
            digitcnt = digitcnt - 2;
            if (digitcnt < 0)
              digitcnt = 10;
            #ifdef digitdebug
            Serial.print("LEFT: set digitcnt = ");
            Serial.println(digitcnt);              
            #endif
              
          }
        }
        #ifdef digitdebug
        Serial.print("LEFT: print digitcnt = ");
        Serial.println(digitcnt);        
        #endif
        
        mydigit(digitcnt);

        digitcnt = digitcnt - 1;
        if (digitcnt == -1)
          digitcnt = 10;
          
        #ifdef digitdebug
        Serial.print("LEFT: out digitcnt = ");
        Serial.println(digitcnt);
        #endif
      }

      if (type == 1)
      {
//---------------------------------------------------------------
//    Следующий оператор IF обрабатывает случай, когда после нажатия кнопки DOWN (вперёд)
//    сразу нажимается кнопка LEFT (назад): 
//    например, если отображается буква "В", и до этого была нажата кнопка DOWN (вперёд), 
//    то при нажатии на LEFT (назад) должна отображаться цифра буква "Б"
//    особенность кода такая, что если эту ситуацию не обработать,
//    то сначала будет отображена буква "Г", и только потом, после ещё
//    двух нажатий кнопки LEFT будет отображена буквва "Б"
//---------------------------------------------------------------
        
        if (prevbutt == 2 ) // 2 - DOWN
        {
        
        #ifdef letterdebug
        Serial.print("LEFT: in lettercnt = ");
        Serial.println(lettercnt);
        #endif
        
          if (lettercnt == 0)
          {
            lettercnt = 62;
            #ifdef letterdebug
            Serial.println("LEFT: set lettercnt = 62");            
            #endif
          }
          else
          {
            lettercnt = lettercnt - 4;
            if (lettercnt < 0)
              lettercnt = 64;
            #ifdef letterdebug
            Serial.print("LEFT: set lettercnt = ");
            Serial.println(lettercnt);                    
            #endif
          }
        }   

        #ifdef letterdebug
        Serial.print("LEFT: print lettercnt = ");
        Serial.println(lettercnt);   
        #endif

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(myStrings[lettercnt]);
        lcd.setCursor(0, 1);
        lcd.print(myStrings[lettercnt + 1]);

        lettercnt = lettercnt - 2;
        if (lettercnt == -2)
          lettercnt = 64;
          
        #ifdef letterdebug
        Serial.print("LEFT: out lettercnt = ");
        Serial.println(lettercnt);          
        #endif
      }
      if (type == 2)
      {
//---------------------------------------------------------------
//    Следующий оператор IF обрабатывает случай, когда после нажатия кнопки DOWN (вперёд)
//    сразу нажимается кнопка LEFT (назад): 
//    например, если отображается слог "БА", и до этого была нажата кнопка DOWN (вперёд), 
//    то при нажатии на LEFT (назад) должен отображаться слог "ПА"
//    особенность кода такая, что если эту ситуацию не обработать,
//    то сначала будет отображён слог "НЯ", и только потом, после ещё
//    двух нажатий кнопки LEFT будет отображён слог "ПА"
//---------------------------------------------------------------
        
        if (prevbutt == 2 ) // 2 - DOWN
        {

        #ifdef syllabledebug
        Serial.print("LEFT: in syllablecnt = ");
        Serial.println(syllablecnt);
        #endif

          if (syllablecnt == 0)
          {
            syllablecnt = 10;

            #ifdef syllabledebug
            Serial.println("LEFT: set syllablecnt = 10");            
            #endif
          }
          else
          {
            syllablecnt = syllablecnt - 4;
            if (syllablecnt < 0)
              syllablecnt = 12;

            #ifdef syllabledebug
            Serial.print("LEFT: set syllablecnt = ");
            Serial.println(syllablecnt);                    
            #endif
              
          }
        } 

        #ifdef syllabledebug
        Serial.print("LEFT: print syllablecnt = ");
        Serial.println(syllablecnt);   
        #endif
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(mySyllables[syllablecnt]);
        lcd.setCursor(0, 1);
        lcd.print(mySyllables[syllablecnt + 1]);

        syllablecnt = syllablecnt - 2;
        if (syllablecnt == -2)
          syllablecnt = 12;

        #ifdef syllabledebug
        Serial.print("LEFT: out syllablecnt = ");
        Serial.println(syllablecnt);   
        #endif

      }
      prevbutt = 1; // LEFT
    }
//---------------------------------------------------------------
//    Кнопка SELECT отвечает за выбор текущего режима: 
//    цифры - 0, буквы - 1, слоги - 2
//---------------------------------------------------------------
    if (curButton == 5 && buttonFlag == 1) {         // Однократное нажатие кнопки SELECT
      buttonFlag = bf_none;

      type = type + 1;
      if (type == 3) // пока только 0
        type = 0;

      if (type == 0)             // очистка дисплея и вывод информации о режиме: "Цифры"
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(L"     Цифры");
        digitcnt = 0;            // обнуление индекса текущего элемента для цифр, букв, слогов
        lettercnt = 0;
        syllablecnt = 0;
        #ifdef modedebug
        Serial.println("MODE: Цифры");
        #endif
        
      }
      if (type == 1)              // очистка дисплея и вывод информации о режиме: "Буквы"
      {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(L"     Буквы");
        digitcnt = 0;            // обнуление индекса текущего элемента для цифр, букв, слогов
        lettercnt = 0;
        syllablecnt = 0;
        #ifdef modedebug
        Serial.println("MODE: Буквы");
        #endif
      }
      
      if (type == 2)
      {
        lcd.clear();                // очистка дисплея и вывод информации о режиме: "Слоги"
        lcd.setCursor(0, 0);
        lcd.print(L"     Слоги");
        digitcnt = 0;               // обнуление индекса текущего элемента для цифр, букв, слогов
        lettercnt = 0;
        syllablecnt = 0;
        #ifdef modedebug
        Serial.println("MODE: Слоги");
        #endif
      }
    }
    if (curButton == 5 && buttonFlag == 2) {           // Удержание кнопки SELECT: не используется
      if (millis() - timerButton_2 > long_3_Time) {
        timerButton_2 = millis();
      }
    }
    if (curButton == 5 && buttonFlag == 3) {           // Длительное нажатие кнопки SELECT (нажатие более 5 секунд): не используется
      if (millis() - timerButton_2 > long_3_Time) {
        timerButton_2 = millis();
      }
    }
  }   // end if
}     // end loop
