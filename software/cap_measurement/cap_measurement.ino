
#include <Wire.h>
#include <FDC1004.h>
#include <LiquidCrystal.h>
#include <Button.h>

#define D_PRINTS

const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
Button button1(12);
Button button2(10);
Button button3(11);
Button button4(9);

FDC1004 fdc(FDC1004_400HZ);
volatile int capdac = 0;
volatile int32_t calibration_cap_offset[4] = {0, 0, 0, 0};
int val;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void welcome_screen()
{
  lcd.clear();
  lcd.print("Cap-measurement:");
  lcd.setCursor(3, 1);
  lcd.print("Press Key1");
#ifdef D_PRINTS
  Serial.print("Capacitance measurement unit\nPress <Key1> to continue\n");
#endif
}

void setup()
{
  Wire.begin();
#ifdef D_PRINTS
  Serial.begin(115200);
#endif
  lcd.begin(16, 2);
  button1.begin();
  button2.begin();
  button3.begin();

  welcome_screen();
  while(!button1.pressed());
}

int menu_1()
{
  int val = 0;
  int dispFlg = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("1:Single ch mode  3:Calibration");
  lcd.setCursor(0, 1);
  lcd.print("2:Dual ch mode");
  delay(400);
#ifdef D_PRINTS
  Serial.print("Key1: Single ch mode\nKey2: Dual ch mode\nKey3: Calibrate cap measuring unit\n");
#endif
  val = 0;
  dispFlg = 0;
  do{
    if(button1.pressed())
    {
      val = 1;
    }
    else if(button2.pressed())
    {
      val = 2;
    }
    else if(button3.pressed())
    {
      val = 3;
    }
    
    if(0 == dispFlg)
      lcd.scrollDisplayLeft();
      
    dispFlg++;
    
    if(5 == dispFlg)
      dispFlg = 0;
      
    delay(100);
  }while(!val);
  return val;
}

int sing_ch_menu()
{
  int val = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("1: ch1");
  lcd.setCursor(8, 0);
  lcd.print("2: ch2");
  lcd.setCursor(0, 1);
  lcd.print("3: ch3");
  lcd.setCursor(8, 1);
  lcd.print("4: ch4");

#ifdef D_PRINTS
  Serial.print("Key1: ch1   Key2: ch2\nKey3: ch3   Key4: ch4\n");
#endif

  do{

    if(button1.pressed())
      val = 1;
    else if(button2.pressed())
      val = 2;
    else if(button3.pressed())
      val = 3;
    else if(button4.pressed())
      val = 4;

  }while(!val);
  return val;
}

int dual_ch_menu()
{
  int val = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("1: ch1 & ch2");
  lcd.setCursor(0, 1);
  lcd.print("2: ch3 & ch4");

#ifdef D_PRINTS
  Serial.print("Key1: ch1 & ch2\nKey2: ch3 & ch4\n");
#endif

  do{

    if(button1.pressed())
      val = 1;
    else if(button2.pressed())
      val = 2;

  }while(!val);
  return val;
}

void read_sing_cap(int ch)
{
  int32_t sample_cap_val = 0;
  int j = 0;
  int32_t cap_val = 0;
  int count = 0;
  do{
    if(0 == count)
    {
      lcd.clear();
      sample_cap_val = 0;
      for(j = 0; j < 10; j++)
      {
        sample_cap_val += measure_cap(ch - 1);
        delay(5);
      }
      cap_val =  sample_cap_val / 10;
      lcd.print("Ch");
      lcd.print(ch);
      lcd.print(":");
      lcd.setCursor(0, 1);
      lcd.print(cap_val);
      lcd.print("fF");
    }
    count++;
    
    if(10 == count)
      count = 0;
      
    delay(100);
  }while(!button1.pressed());
}

void read_dual_cap(int ch)
{
  int32_t cap_val = 0;
  int32_t sample_cap_val = 0;
  int j = 0;
  int count = 0;
  do{
    if(0 == count)
    {
      lcd.clear();
      switch(ch)
      {
        case 1:
          sample_cap_val = 0;
          for(j = 0; j < 10; j++)
          {
            sample_cap_val += measure_cap(0);
            delay(5);
          }
          cap_val =  sample_cap_val / 10;
          lcd.print("C1:");
          lcd.print(cap_val);
          lcd.print("fF");
          sample_cap_val = 0;
          for(j = 0; j < 10; j++)
          {
            sample_cap_val += measure_cap(1);
            delay(5);
          }
          cap_val =  sample_cap_val / 10;
          lcd.setCursor(0, 1);
          lcd.print("C2:");
          lcd.print(cap_val);
          lcd.print("fF");
          break;
        case 2:
          sample_cap_val = 0;
          for(j = 0; j < 10; j++)
          {
            sample_cap_val += measure_cap(2);
            delay(5);
          }
          cap_val =  sample_cap_val / 10;
          lcd.print("C3:");
          lcd.print(cap_val);
          lcd.print("fF");
          sample_cap_val = 0;
          for(j = 0; j < 10; j++)
          {
            sample_cap_val += measure_cap(3);
            delay(5);
          }
          cap_val =  sample_cap_val / 10;
          lcd.setCursor(0, 1);
          lcd.print("C4:");
          lcd.print(cap_val);
          lcd.print("fF");
          break;
      }
    }
    count++;
    
    if(10 == count)
      count = 0;
      
    delay(100);
  }while(!button1.pressed());
}

void calibrate_fdc(void)
{
  int32_t sample_cap_val = 0;
  int i = 0;
  int j = 0;
  calibration_cap_offset[0] = 0;
  calibration_cap_offset[1] = 0;
  calibration_cap_offset[2] = 0;
  calibration_cap_offset[3] = 0;
  
  for(i = 0; i < 4; i++)
  {
    sample_cap_val = 0;
    lcd.clear();
    lcd.print("calibrating...");
    lcd.setCursor(0, 1);
    lcd.print("Ch");
    lcd.print(i);
#ifdef D_PRINTS
    Serial.print("calibrating... Channel:");
    Serial.println(i);
#endif
    for(j = 0; j < 10; j++)
    {
      sample_cap_val += measure_cap(i);
      delay(100);
    }
    calibration_cap_offset[i] = sample_cap_val / 10;
  }
}


int32_t measure_cap(int channel)
{
  int32_t capacitance = 0;
  uint8_t measurement = 0;
#ifdef D_PRINTS
  char result[100];
#endif

  fdc.configureMeasurementSingle(measurement, channel, capdac);
  fdc.triggerSingleMeasurement(measurement, FDC1004_100HZ);
  //wait for completion
  delay(15);
  uint16_t value[2];
  if (! fdc.readMeasurement(measurement, value)) {
    
    // calculate capacitance;
    // The absolute capacitance is a function of the capdac and the measurement
    // We only use the msb because the FDC1004 only has 16bits effective resolution;
    // the last 8 bits are more or less random noise. 
    int16_t msb = (int16_t) value[0];
    capacitance = ((int32_t)457) * ((int32_t)msb); //in attofarads
    capacitance /= 1000; //in femtofarads
    capacitance += ((int32_t)3028) * ((int32_t)capdac);
    capacitance -= calibration_cap_offset[channel];
#ifdef D_PRINTS
    sprintf(result, "Ch:%1d > Raw:%04X %04X, Offset:%02X > Cap:", channel+1, msb, value[1], capdac);
    Serial.print(result);
    Serial.print(capacitance);
    Serial.print(" fF\n");
#endif    
    //adjust capdac
    int16_t upper_bound = 0x4000;
    int16_t lower_bound = -1 * upper_bound;
    if (msb > upper_bound) {
      if (capdac < FDC1004_CAPDAC_MAX) capdac++;
    } else if (msb < lower_bound) {
      if (capdac > 0) capdac--;
    }
  }
  return capacitance;
}

void loop()
{
  int choice = 0;

  choice = menu_1();
  switch(choice)
  {
    case 1: // Single ch mode
      choice = sing_ch_menu();
      read_sing_cap(choice);
      break;
    case 2: // Dual ch mode
      choice = dual_ch_menu();
      read_dual_cap(choice);
      break;
    case 3: // FDC1004 calibration mode
      calibrate_fdc();
      break;
  }
  lcd.clear();
  delay(200);
}
