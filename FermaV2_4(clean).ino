#include <Bounce2.h>

#define readyornot 2   //ножка к которой подключён управляющий контроллер
#define button1 4      //первая кнопка для запуска человека
#define button2 5      //вторая кнопка для запуска человека
#define datch1 6       //датчик, который установлен на "бездвигательной двери" (нулевое состояние)
#define datch2 7       //датчик, который на "двери с двигателем" (нулевое состояние)
#define datch3 8       //датчик, к которому подъезжает "дверь с двигателем" (при открытии)
#define rotor 10         //ножка управления двигателем
#define magnit 11      //управление магнитом

#define noBlock 1
#define yBlock 0
#define rotorOn 0
#define rotorOff 1
#define lowstate 0

Bounce debouncer_datch1 = Bounce();
Bounce debouncer_datch2 = Bounce();
Bounce debouncer_datch3 = Bounce();
//---------Переменные в программе-------------------
byte programstate = 0;         //состояние програмы в данный момент (главная переменная)
byte count_d = 0;
bool workmode = 0;
byte innerFlag = 0;
unsigned long Press_first_button = 0;           //время нажатия первой кнопки
byte button_press_1 = 0;
byte button_press_2 = 0;
byte val_datch_1 = 0;
byte val_datch_2 = 0;
byte val_datch_3 = 0;
unsigned long autoclose = 0;                 //временной маркер автоматического возврата в исходное состояние
unsigned long alarm_close = 0;                 //временной маркер для аварийного возврата в исходное состояние
unsigned long sync_time = 0;                    //время синхранизации
//-------------------------------------------------

//------------------------------------------------
//--------------УСТАНОВКА------------------------
//----------------------------------------------
void setup() {    
  pinMode(readyornot, INPUT);                 //вход управляющей ножки
  pinMode(button1, INPUT);             //пин первой кнопки (с "внутринней подтяжкой")
  pinMode(button2, INPUT);             //пин второй кнопки (с "внутринней подтяжкой")
  pinMode(datch1, INPUT);                   //вход первого датчика 
  pinMode(datch2, INPUT);                   //вход второго датчика
  pinMode(datch3, INPUT);                   //вход третьего датчика
  pinMode(rotor, OUTPUT);                    //выход на двигатель
  pinMode(magnit, OUTPUT);                  //выход на магнит
  
  digitalWrite(readyornot, HIGH);            //изначально не готов
  digitalWrite(button1, HIGH);              //замыкаем на землю(при нажатии)
  digitalWrite(button2, HIGH);               //аналогично
  digitalWrite(datch1, HIGH);                //изначально низкий уровень6
  digitalWrite(datch2, HIGH);                // --||--
  digitalWrite(datch3, HIGH);                // --||--
  digitalWrite(rotor, rotorOff);                  //двигатель выключен
  digitalWrite(magnit, HIGH);               //магнит включён
  debouncer_datch1.attach(datch1);
  debouncer_datch1.interval(20);
  debouncer_datch2.attach(datch2);
  debouncer_datch2.interval(20);
  debouncer_datch3.attach(datch3);
  debouncer_datch3.interval(20);
  val_datch_3 = 1;
  val_datch_2 = 1;
  val_datch_1 = 1;  

 digitalWrite(rotor, rotorOn);
 delay(500);
 if(digitalRead(button1) == LOW){             //при нажатии первой
    innerFlag = 3;             
    count_d = 20;            
 }
 while(val_datch_3 == noBlock){
    digitalWrite(rotor, rotorOn);
    Check_datchs();       
  }
 while(val_datch_2 == noBlock){    //если ворота не на месте
      digitalWrite(rotor, rotorOn);
      Check_datchs();         
  }
  digitalWrite(rotor, rotorOff);  
  programstate = 0; 
  workmode = 1; 
}
//------------НАЧАЛО ОСНОВНОГО ЦИКЛА--------
//-------------------------------------------
void(* resetFunc) (void) = 0;
void loop() { 
if(innerFlag == 3){   
    while(val_datch_3 == noBlock){
        digitalWrite(rotor, rotorOn);
        Check_datchs();               
    }
    digitalWrite(rotor, rotorOff);
    delay(1000);
    while(val_datch_2 == noBlock){    //если ворота не на месте
       digitalWrite(rotor, rotorOn); 
       Check_datchs();              
    }
    digitalWrite(rotor, rotorOff);
    delay(1000);          
    count_d--;
    if(!count_d>0)      
      resetFunc();
 }
//----------------------
  if(digitalRead(readyornot) == LOW){
     workmode = 1;
  } else
  workmode = 0;
//---------------------
  if(workmode == 1){
   if(programstate == 0){          //если контроллер "сказал", что трасса готова и программа в нулевом состоянии
     Check_but();
     autoclose = 0;    
    }
  }  
    //----------------------alarm_close----------------
    if((val_datch_1 == noBlock)&&(innerFlag == 0)){
      innerFlag = 1;
      alarm_close = millis();
    } else if((alarm_close + 15000 < millis())&&(innerFlag == 1)){
             digitalWrite(magnit, HIGH);
             delay(500);
              while(val_datch_2 == noBlock){    //если ворота не на месте
                  digitalWrite(rotor, rotorOn);
                  Check_datchs(); 
               }
               digitalWrite(rotor, rotorOff);                                         
               programstate = 0;
               innerFlag = 2;               
      }
   //----------------------------------------------------
  //----------------------auto_close----------------
     if(autoclose+9000<millis()&&(programstate != 0)&&(autoclose != 0)){
      if((digitalRead(magnit) == LOW)&&(val_datch_1 == yBlock)){             
              while(val_datch_2 == noBlock){    //если ворота не на месте
                  digitalWrite(rotor, rotorOn);
                  Check_datchs();
                }
               digitalWrite(rotor, rotorOff);                          
               programstate = 5; 
               autoclose = 0;      
        }             
    }  
  //----------разброс по состояниям-------------
  switch(programstate){   
      case 0:       //нулевое
      break;
      
      case 1:         //первое(открытие вторых ворот)           
          while(val_datch_3 == noBlock){     // пока нету приграды на датчике
              digitalWrite(rotor, rotorOn);       //поднимаем ворота двигателем
              Check_datchs();
          }
          digitalWrite(rotor, rotorOff);   //при достижении, выключаем
          programstate = 2;     //идём дальше
          autoclose = millis();        
      break;
      
      case 2:
          if(sync_time + 3000 < millis()){ 
            Check_datchs();              
            if(val_datch_1 == yBlock){
                digitalWrite(magnit, LOW);       //отпускаем магнит              
            } else{            
              programstate = 3;           //идём дальше
            }
          }
      break;
      case 3:
      Check_datchs();
        if(val_datch_1 == yBlock){
           programstate = 2;
        } else {        //через 0,05 секунд
            programstate = 4;
          } 
      break;
      case 4:  
      Check_datchs();      
      if(val_datch_1 == yBlock){
        Magnit_on(); 
        delay(1000);
        while(val_datch_2 ==  noBlock){ 
          Check_datchs();   
          digitalWrite(rotor, rotorOn);         //доводим их двигателем
        }        
        programstate = 5;         //идём дальше      
      }      
      break;
      
      case 5:       
        digitalWrite(rotor, rotorOff);        //проверям, что двигатель выключен
        Check_datchs(); 
        delay(500);
        Magnit_on();        
        if((val_datch_1 ==  yBlock)&&(val_datch_2 ==  yBlock)&&(val_datch_3 ==  noBlock)){  //если у нас ворота стоят (по датчикам) на своих местах
          programstate = 0;   //переходим в нулевое состояние
        }else{
            programstate = 4;
          }
      break;
      
      default:
      break;
    }
}
//-------------------------------------------
//------------КОНЕЦ ОСНОВНОГО ЦИКЛА--------
//-------------------------------------------
void Check_but(){
    if(digitalRead(button1) == LOW){             //при нажатии первой
       button_press_1 = 1;       
     }else if(button_press_1 == 1){
          button_press_1 = 2;               
          Press_first_button = millis();        //запоминаем текущее время              
      }
     if(button_press_1 == 2){
       if(digitalRead(button2) == LOW){             //при нажатии первой
           button_press_2 = 1;                
       } else if(button_press_2 == 1){
            button_press_2 = 0;            
            innerFlag = 0;             
            button_press_1 = 0;           
            if(Press_first_button + 3000 > millis()){
              programstate = 1;
              sync_time = millis();
            } else
                programstate = 0;
        }
     }
 }  
//-----------------------------
void Magnit_on(){
    if(digitalRead(datch1) == yBlock){
          digitalWrite(magnit, HIGH);      //включаем магнит
      }  
  }
//---------------------------------------------
void Check_datchs(){
  debouncer_datch1.update();
  debouncer_datch2.update();
  debouncer_datch3.update();
  val_datch_1 = debouncer_datch1.read();
  val_datch_2 = debouncer_datch2.read();
  val_datch_3 = debouncer_datch3.read();
}
