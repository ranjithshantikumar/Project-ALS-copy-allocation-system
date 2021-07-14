/*

This programm is created as an alternative to "FERAG ALS" System,it does copy allocation work ,
we have taken meticulous care as to make the inhouse designed system 1:1,and achived it after 5 major forks and 
32 version controls of the program ,continously working for it for over 60 days start date 14/9/20 project complete date
The original ALS consists of Beckhoff Cx1000 module processor running TWinCAt OS,it has 4 run time and and each runtime is capable of 
running 5 threads each,we  by careful programming and using Instructions per second and scan cycle manipulation and
we have tested all the logics of this circuit and found its satisfactory after practical testing in production
the program uses following signal from the existing syatem,final program eddit1 22/1/2021,EDIT2 :22/1/22021 INPUT OUPUT MIX UP IN STACKSELECT 
last good program1.6.1_stackselect_ok_ALS1
*/
unsigned long Asa = 0b0;
unsigned long Asa2 = 0b0;
volatile unsigned long Release;
volatile unsigned long Release2;
const int Error_from_MTS2 = 9;
const int Error_from_MTS1 = 7;
const int solenoid = 12;
const int solenoid2 = 11;
const byte gripperpulse = 2;
const byte interruptPin1 = 3;
const byte interruptPin2 = 18;
volatile byte state ;
volatile byte FallingEdgeFlag = 0;
volatile bool copysenseflag;
int Batch_counter_MTS1 = 0;
int Batch_counter_MTS2 = 0;
int stackSize;
const int stackSizeSelector = 10;
bool GripperUnlacthFlag;
bool GripperUnlacthFlag1;
int BatchSperationCount;
const int ProgOK = 6;
const int TwentyCopiesLed = 4;
const int FiftyCopiedLed = 5;


void setup()
{
  pinMode(Error_from_MTS2, INPUT);
  pinMode(Error_from_MTS1, INPUT);
  pinMode(stackSizeSelector, INPUT);
  pinMode(solenoid, OUTPUT);
  pinMode(solenoid2, OUTPUT);
  pinMode(ProgOK,OUTPUT);
  pinMode(TwentyCopiesLed,OUTPUT);
  pinMode(FiftyCopiedLed, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(gripperpulse), gripper, RISING);
  attachInterrupt(digitalPinToInterrupt(interruptPin1), blink2, FALLING);
  attachInterrupt(digitalPinToInterrupt(interruptPin2), blink3, RISING);        //denex rising edge detect
  bool SwitchDetect = digitalRead(stackSizeSelector);
  if (SwitchDetect == true)
  {
    stackSize = 10;
    BatchSperationCount = stackSize * 2;
    digitalWrite(TwentyCopiesLed, HIGH);
    digitalWrite(FiftyCopiedLed, LOW);
  }
  else if (SwitchDetect == false)
  {
    stackSize = 25;
    BatchSperationCount = stackSize * 2;
    digitalWrite(FiftyCopiedLed, HIGH);
    digitalWrite(TwentyCopiesLed, LOW);
  }
  digitalWrite(ProgOK, HIGH);
}


void Asa_Update()
{
  bitWrite(Asa, 0, 1);
}

void Falling_Edge_Update()
{
  bool CopycountFlg = bitRead(Asa, 22);
  if (CopycountFlg == 1)
  {
    Batch_counter_MTS1++;
  }
  if (Batch_counter_MTS1 <= stackSize)
  {
    bitClear(Asa, 22);
  }
  int ErorGate = digitalRead( Error_from_MTS2);
  if ((Batch_counter_MTS1 == stackSize) && (ErorGate == LOW ))
  {
    GripperUnlacthFlag = 0 ;
    Batch_counter_MTS1 = 0;
  }
  int ErorGate1 = digitalRead( Error_from_MTS1);
  if ((Batch_counter_MTS1 == BatchSperationCount) && (ErorGate1 == LOW ))
  {
    GripperUnlacthFlag = 0 ;
    Batch_counter_MTS1 = stackSize;  //edited from 11 to 10
  }
  if (Batch_counter_MTS1 == BatchSperationCount)
  {
    GripperUnlacthFlag = 0 ;
    Batch_counter_MTS1 = 0;
  }
  bool CopycountFlg2 = bitRead(Asa2, 28);
  if (CopycountFlg2 == 1)
  {
    Batch_counter_MTS2++;
  }
  if (Batch_counter_MTS2 == stackSize) //use this portion for error sinal in mts1
  {
    GripperUnlacthFlag1 = 0 ;
    Batch_counter_MTS2 = 0;
  }
  Release = Asa <<  1;
  Release2 = Asa2 <<  1;
  Asa = Release;
  Asa2 = Release2;
  FallingEdgeFlag = 0;
}

void loop() {

  if ((copysenseflag == true) && (state == HIGH))                           //add else further
  {
    Asa_Update();
    copysenseflag = false;

  }

  if (state == HIGH)
  {
    bool shiftedValue = bitRead(Asa, 31);                            // shift the overflow high bit value from register 1
    bitWrite(Asa2, 0, shiftedValue);
  }                                                                  // reading the shifted value

  bool opengripper = bitRead(Asa, 22);

  if ((Batch_counter_MTS1 <= stackSize) && (state == true) && (opengripper == HIGH)) //
  {

    digitalWrite(solenoid, opengripper);

  }
  if ((Batch_counter_MTS1 <= stackSize) && (state == true) && (opengripper == 0)) //this line only activates if asa 22 is 0.
  {

    digitalWrite(solenoid, opengripper);

  }
  if ((Batch_counter_MTS1 == stackSize) && (state == true ) ) // try greater than or equal to
  {
    GripperUnlacthFlag = 1;
  }
  if ( GripperUnlacthFlag == 1)
  {
    digitalWrite(solenoid, LOW);
  }

  // for mts 2
  bool opengripper2 = bitRead(Asa2, 28);

  if ((Batch_counter_MTS2 <= stackSize) && (state == true) && (opengripper2 == HIGH)) //
  {

    digitalWrite(solenoid2, opengripper2);

  }
  if ((Batch_counter_MTS2 <= stackSize) && (state == true) && (opengripper2 == 0)) //this line only activates if asa 22 is 0.
  {

    digitalWrite(solenoid2, opengripper2);

  }
  if ((Batch_counter_MTS2 == stackSize) && (state == true ) )
  {
    GripperUnlacthFlag1 = 1;
  }
  if ( GripperUnlacthFlag1 == 1)
  { digitalWrite(solenoid2, LOW);
  }
  if (FallingEdgeFlag == 1 )                                     //falling edge gripperflag,denotes the gripper movement
  {
    Falling_Edge_Update();
  }

}

void gripper()
{
  state = 1;
}

void blink2()
{
  state = 0;
  FallingEdgeFlag = 1;
}
void blink3()
{
  copysenseflag = true;

}



