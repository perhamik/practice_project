#define _DEBUG 0

short readyornot = 2;   //ножка к которой подключён управляющий контроллер
short button1 = 4;      //первая кнопка для запуска человека
short button2 = 5;      //вторая кнопка для запуска человека
short datch1 = 6;       //датчик, который установлен на "бездвигательной двери" (нулевое состояние)
short datch2 = 7;       //датчик, который на "двери с двигателем" (нулевое состояние)
short datch3 = 8;       //датчик, к которому подъезжает "дверь с двигателем" (при открытии)
short dvig = 10;         //ножка управления двигателем
short magnit = 11;      //управление магнитом
bool dvig_on = LOW;
bool dvig_off = HIGH;
bool lowstate = LOW;

//---------Переменные в программе-------------------
short programstate = 0;         //состояние програмы в данный момент (главная переменная)
bool workmode = 0;
short innerFlag = 0;
volatile short state = 0;       //управляющая переменная контроллером
unsigned long Press_first_button = 0;           //время нажатия первой кнопки
short button_press_1 = 0;
short button_press_2 = 0;
unsigned long timemarker = 0;                   //временной маркер для отладки
unsigned long closetimer = 0;                 //временной маркер задержки включения двигателя
unsigned long autoclose = 0;                 //временной маркер автоматического возврата в исходное состояние
unsigned long alarm_close = 0;                 //временной маркер для аварийного возврата в исходное состояние
unsigned long deleey = 0;                    //временной маркер для отладки_2
//-------------------------------------------------

//------------------------------------------------
//--------------УСТАНОВКА------------------------
//----------------------------------------------
void setup() { 
   
  pinMode(readyornot, INPUT_PULLUP);                 //вход управляющей ножки
  //attachInterrupt(0, rdy_or_not, CHANGE);     //привязываем прерывание
  pinMode(button1, INPUT_PULLUP);             //пин первой кнопки (с "внутринней подтяжкой")
  pinMode(button2, INPUT_PULLUP);             //пин второй кнопки (с "внутринней подтяжкой")
  pinMode(datch1, INPUT_PULLUP);                   //вход первого датчика 
  pinMode(datch2, INPUT_PULLUP);                   //вход второго датчика
  pinMode(datch3, INPUT_PULLUP);                   //вход третьего датчика
  pinMode(dvig, OUTPUT);                    //выход на двигатель
  pinMode(magnit, OUTPUT);                  //выход на магнит

  digitalWrite(readyornot, HIGH);            //изначально не готов
  digitalWrite(button1, HIGH);              //замыкаем на землю(при нажатии)
  digitalWrite(button2, HIGH);               //аналогично
  digitalWrite(datch1, HIGH);                //изначально низкий уровень6
  digitalWrite(datch2, HIGH);                // --||--
  digitalWrite(datch3, HIGH);                // --||--
  digitalWrite(dvig, dvig_off);                  //двигатель выключен
  digitalWrite(magnit, HIGH);               //магнит включён
  if(_DEBUG)
    Serial.begin(9600);
 Serial.println("start");
 while(digitalRead(datch3) == HIGH){
    digitalWrite(dvig, dvig_on);
    if(deleey + 500 <millis()){
        Serial.println("datch3 == HIGH");   
        deleey = millis();
    }     
  }
 while(digitalRead(datch2) == HIGH){    //если ворота не на месте
      digitalWrite(dvig, dvig_on);
       if(deleey + 500 <millis()){
        Serial.println("datch2 == HIGH");   
        deleey = millis();
    }        
  }
  digitalWrite(dvig, dvig_off);  
  programstate = 0; 
  workmode = 1;
 
}
//-------------------------------------------

//-------------------------------------------
//------------НАЧАЛО ОСНОВНОГО ЦИКЛА--------
//-------------------------------------------
//void(* resetFunc) (void) = 0;
void loop() {
  start:
/*if(state == 1){
   workmode = 1;
} else
workmode = 0;*/
if(workmode == 1){
 if(programstate == 0){          //если контроллер "сказал", что трасса готова и программа в нулевом состоянии
   Check_but();
   autoclose = 0;
   goto start;
  }
}  
    //----------------------alarm_close----------------
    if((digitalRead(datch1) == HIGH)&&(innerFlag == 0)){
      innerFlag = 1;
      alarm_close = millis();
    } else if((alarm_close + 15000 < millis())&&(innerFlag == 1)){
             digitalWrite(magnit, HIGH);
             delay(500);
              while(digitalRead(datch2) == HIGH){    //если ворота не на месте
                  digitalWrite(dvig, dvig_on);
                  if(deleey + 500 <millis()){
                    Serial.println("CLOOOSING_ALARM"); 
                    deleey = millis();
                  }
                           
                }
               digitalWrite(dvig, dvig_off);  
               Serial.println("Closed_alarm");                           
               programstate = 0;
               innerFlag = 2;               
      }
   //----------------------------------------------------
  //----------------------auto_close----------------
     if(autoclose+9000<millis()&&(programstate != 0)&&(autoclose != 0)){
      if((digitalRead(magnit) == LOW)&&(digitalRead(datch1) == LOW)){
             digitalWrite(magnit, HIGH);
             delay(500);
              while(digitalRead(datch2) == HIGH){    //если ворота не на месте
                  digitalWrite(dvig, dvig_on);
                  if(deleey + 500 <millis()){
                    Serial.println("CLOOOSING_auto"); 
                    deleey = millis();
                  }             
                }
               digitalWrite(dvig, dvig_off);  
               Serial.println("Closed_auto");           
               programstate = 0; 
               autoclose = 0;      
        }
             
    }
  //------------------------------------------------
  
  //----------разброс по состояниям-------------
  switch(programstate){   
      case 0:       //нулевое
      break;
      
      case 1:         //первое(открытие вторых ворот)           
        if(digitalRead(datch3) == HIGH){     // пока нету приграды на датчике
            digitalWrite(dvig, dvig_on);       //поднимаем ворота двигателем
        }else{
          programstate = 2;     //идём дальше
          autoclose = millis();
        }
      break;
      
      case 2:
          digitalWrite(dvig, dvig_off);   //при достижении, выключаем      
          if(digitalRead(datch1) == LOW){
              digitalWrite(magnit, LOW);       //отпускаем магнит              
          } else{
            closetimer = millis();     //запоминаем время
            programstate = 3;           //идём дальше
          }
      break;
      case 3:
        if(digitalRead(datch1) == LOW){
           programstate = 2;
        } else if(closetimer+50 < millis()){        //через 0,5 секунд
            programstate = 4;
          } 
      break;
      case 4:
      if(digitalRead(datch1) == LOW){
          digitalWrite(magnit, HIGH);      //проверяем, что магнит включён
      }      
      if((closetimer+1500 < millis())&&(digitalRead(datch1) == LOW)){
        delay(500);
        digitalWrite(dvig, dvig_on);         //доводим их двигателем      
        if(digitalRead(datch2) == LOW){    //если ворота на месте
          programstate = 5;         //идём дальше  
        }
      }
      
      break;
      
      case 5:        
        digitalWrite(dvig, dvig_off);        //проверям, что двигатель выключен        
        if((digitalRead(datch1) ==  LOW)&&(digitalRead(datch2) ==  LOW)&&(digitalRead(datch3) ==  HIGH)){  //если у нас ворота стоят (по датчикам) на своих местах
          programstate = 0;   //переходим в нулевое состояние
        }
      break;
      
      default:
      break;
    }
//--------------------------------------------------------------    
//-------------------------Отладка-----------------------------   
   if(_DEBUG){
        Debug_run();
      }
//--------------------------------------------------------------
}
//-------------------------------------------
//------------КОНЕЦ ОСНОВНОГО ЦИКЛА--------
//-------------------------------------------
void Check_but(){
    if(digitalRead(button1) == LOW){             //при нажатии первой
       button_press_1 = 1;
       Serial.println("button_press_1 = 1;");
     }else if(button_press_1 == 1){
          button_press_1 = 2;               
          Press_first_button = millis();        //запоминаем текущее время 
          Serial.println("button_press_1 = 2;");       
      }
     if(button_press_1 == 2){
       if(digitalRead(button2) == LOW){             //при нажатии первой
           button_press_2 = 1; 
           Serial.println("button_press_2 = 1;");         
       } else if(button_press_2 == 1){
            button_press_2 = 0;            
            innerFlag = 0;             
            button_press_1 = 0;
            Serial.println("button_press_2 = 0;");
            Serial.println("button_press_1 = 0;");
            Serial.println(Press_first_button);
            if(Press_first_button + 10000 > millis())
              programstate = 1;
            
        }
     }
    if(deleey + 500 <millis()){
      Serial.print("Wait for buttons"); 
      Serial.print("     ");
      Serial.println(millis());
      deleey = millis();
    }
 }
//----------------------------------------------
//----------------УПРАВЛЯЮЩЕЕ ПРЕРЫВАНИЕ--------
//----------------------------------------------
void rdy_or_not(){
  Serial.println("Interrupt");
    if(digitalRead(readyornot) == HIGH){      //если на управляющей ножке высокий уровень
        state = 0;      // есть готовность
     } else state = 1;      // иначе -> нету
  }
//-----------------------------------------------
void Debug_run(){
  if(timemarker + 500 < millis()){
    Serial.print("Datch 1 = ");
    Serial.print(digitalRead(datch1));
    Serial.println();
    Serial.print("Datch 2 = ");
    Serial.print(digitalRead(datch2));
    Serial.println();
    Serial.print("Datch 3 = ");
    Serial.print(digitalRead(datch3));
    Serial.println();
    Serial.print("Dvig = ");
    Serial.print(digitalRead(dvig));
    Serial.println();
    Serial.print("Magnit = ");
    Serial.print(digitalRead(magnit));
    Serial.println();
    Serial.print("Button 1 = ");
    Serial.print(digitalRead(button1));
    Serial.println();
    Serial.print("Button 2 = ");
    Serial.print(digitalRead(button2));
    Serial.println();
    Serial.print("programstate = ");
    Serial.println(programstate);
    Serial.println();
    Serial.println();
    timemarker = millis();
   }
}
