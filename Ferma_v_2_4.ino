#include <Bounce2.h>
//======================================
//Убран дребезг контактов на датчиках
//======================================
#define _DEBUG 0

#define readyornot 2   //ножка к которой подключён управляющий контроллер
#define servis_button 3
#define button1 4      //первая кнопка для запуска человека
#define button2 5      //вторая кнопка для запуска человека
#define datch1 6       //датчик, который установлен на "бездвигательной двери" (нулевое состояние)
#define datch2 7       //датчик, который на "двери с двигателем" (нулевое состояние)
#define datch3 8       //датчик, к которому подъезжает "дверь с двигателем" (при открытии)
#define dvig 10         //ножка управления двигателем
#define magnit 11      //управление магнитом
#define datch_no_pregrad 1
#define datch_yest_pregrada 0
#define dvig_on 0
#define dvig_off 1
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
unsigned long timemarker = 0;                   //временной маркер для отладки
unsigned long autoclose = 0;                 //временной маркер автоматического возврата в исходное состояние
unsigned long alarm_close = 0;                 //временной маркер для аварийного возврата в исходное состояние
unsigned long deleey = 0;                    //временной маркер для отладки_2
unsigned long sync_time = 0;                    //временной маркер для отладки_2
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
  debouncer_datch1.attach(datch1);
  debouncer_datch1.interval(20);
  debouncer_datch2.attach(datch2);
  debouncer_datch2.interval(20);
  debouncer_datch3.attach(datch3);
  debouncer_datch3.interval(20);
  val_datch_3 = 1;
  val_datch_2 = 1;
  val_datch_1 = 1;
  
  if(_DEBUG)
    Serial.begin(9600);
 Serial.println("start");
 digitalWrite(dvig, dvig_on);
 delay(500);
 if(digitalRead(button1) == LOW){             //при нажатии первой
           innerFlag = 3;             
           count_d = 20;            
 }
 while(val_datch_3 == datch_no_pregrad){
    digitalWrite(dvig, dvig_on);
    Check_datchs();
    if(deleey + 500 <millis()){
        Serial.println("datch3");   
        deleey = millis();
    }     
  }
 while(val_datch_2 == datch_no_pregrad){    //если ворота не на месте
      digitalWrite(dvig, dvig_on);
      Check_datchs();
       if(deleey + 500 <millis()){
        Serial.println("datch2");   
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
void(* resetFunc) (void) = 0;
void loop() {
  start:  
 //----------------------
  if((millis()>4294950296)&&(programstate == 0)){   //переполнение переменной    4294950296
      resetFunc();
    }
//----------------------
if(innerFlag == 3){
  progref:   
    while(val_datch_3 == datch_no_pregrad){
        digitalWrite(dvig, dvig_on);
        Check_datchs();               
    }
    digitalWrite(dvig, dvig_off);
    delay(1000);
    while(val_datch_2 == datch_no_pregrad){    //если ворота не на месте
       digitalWrite(dvig, dvig_on); 
       Check_datchs();              
    }
    digitalWrite(dvig, dvig_off);
    delay(1000);          
    count_d--;
    if(count_d>0)
      goto progref;
    else
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
     goto start;
    }
  }  
    //----------------------alarm_close----------------
    if((val_datch_1 == datch_no_pregrad)&&(innerFlag == 0)){
      innerFlag = 1;
      alarm_close = millis();
    } else if((alarm_close + 15000 < millis())&&(innerFlag == 1)){
             digitalWrite(magnit, HIGH);
             delay(500);
              while(val_datch_2 == datch_no_pregrad){    //если ворота не на месте
                  digitalWrite(dvig, dvig_on);
                  Check_datchs();
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
      if((digitalRead(magnit) == LOW)&&(val_datch_1 == datch_yest_pregrada)){             
              while(val_datch_2 == datch_no_pregrad){    //если ворота не на месте
                  digitalWrite(dvig, dvig_on);
                  Check_datchs();
                  if(deleey + 500 <millis()){
                    Serial.println("CLOOOSING_auto"); 
                    deleey = millis();
                  }             
                }
               digitalWrite(dvig, dvig_off);  
               Serial.println("Closed_auto");           
               programstate = 5; 
               autoclose = 0;      
        }
             
    }
  //------------------------------------------------
  
  //----------разброс по состояниям-------------
  switch(programstate){   
      case 0:       //нулевое
      break;
      
      case 1:         //первое(открытие вторых ворот)           
          while(val_datch_3 == datch_no_pregrad){     // пока нету приграды на датчике
              digitalWrite(dvig, dvig_on);       //поднимаем ворота двигателем
              Check_datchs();
          }
          digitalWrite(dvig, dvig_off);   //при достижении, выключаем
          programstate = 2;     //идём дальше
          autoclose = millis();        
      break;
      
      case 2:
          if(sync_time + 3000 < millis()){ 
            Check_datchs();              
            if(val_datch_1 == datch_yest_pregrada){
                digitalWrite(magnit, LOW);       //отпускаем магнит              
            } else{            
              programstate = 3;           //идём дальше
            }
          }
      break;
      case 3:
      Check_datchs();
        if(val_datch_1 == datch_yest_pregrada){
           programstate = 2;
        } else {        //через 0,05 секунд
            programstate = 4;
          } 
      break;
      case 4:  
      Check_datchs();      
      if(val_datch_1 == datch_yest_pregrada){
        Magnit_on(); 
        delay(1000);
        while(val_datch_2 ==  datch_no_pregrad){ 
          Check_datchs();   
          digitalWrite(dvig, dvig_on);         //доводим их двигателем
        }        
        programstate = 5;         //идём дальше      
      }      
      break;
      
      case 5:       
        digitalWrite(dvig, dvig_off);        //проверям, что двигатель выключен
        Check_datchs(); 
        delay(500);
        Magnit_on();        
        if((val_datch_1 ==  datch_yest_pregrada)&&(val_datch_2 ==  datch_yest_pregrada)&&(val_datch_3 ==  datch_no_pregrad)){  //если у нас ворота стоят (по датчикам) на своих местах
          programstate = 0;   //переходим в нулевое состояние
        }else{
            programstate = 4;
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
            if(Press_first_button + 3000 > millis()){
              programstate = 1;
              sync_time = millis();
            } else
                programstate = 0;
              
            
        }
     }  
     
    if(deleey + 500 <millis()){
      Serial.print("Wait for buttons"); 
      Serial.print("     ");
      Serial.println(millis());
      deleey = millis();
    }
 }  
  //-----------------------------
void Magnit_on(){
    if(digitalRead(datch1) == datch_yest_pregrada){
          digitalWrite(magnit, HIGH);      //включаем магнит
      }  
  }
//---------------------------------------------
void Debug_run(){
  if(timemarker + 500 < millis()){
    Serial.print("Datch 1 = ");
    Serial.print(val_datch_1);
    Serial.println();
    Serial.print("Datch 2 = ");
    Serial.print(val_datch_2);
    Serial.println();
    Serial.print("Datch 3 = ");
    Serial.print(val_datch_3);
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
void Check_datchs(){
  debouncer_datch1.update();
  debouncer_datch2.update();
  debouncer_datch3.update();
  val_datch_1 = debouncer_datch1.read();
  val_datch_2 = debouncer_datch2.read();
  val_datch_3 = debouncer_datch3.read();
 /* Serial.print("val_datch1 = ");
  Serial.print(val_datch_1);
  Serial.print("   val_datch2 = ");
  Serial.print(val_datch_2);
  Serial.print("   val_datch3 = ");
  Serial.println(val_datch_3);*/
}
