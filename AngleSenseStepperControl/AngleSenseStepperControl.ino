int smDirectionPin = 8; //Direction pin
int smStepPin = 9; //Stepper pin
int smMS2Pin = 4; //Stepper pin
int smMS1Pin = 5; //Stepper pin
int smEnablePin = 7; //Motor enable pin
int action = 0; // for incoming serial data
volatile int counter = 0; //This variable will increase or decrease depending on the rotation of encoder
int AngleUpdateCounter = 0;
int StepperCounter = 0;
int Dir = 0;
bool reset_mode = false;
int cmd = 0;
int ZeroCounter = 0;
bool ZeroCounterFlg = false;
int CrntDir = 0;

void setup(){
  /*Sets all pin to output; the microcontroller will send them(the pins) bits, it will not expect to receive any bits from thiese pins.*/
  pinMode(smDirectionPin, OUTPUT);
  pinMode(smStepPin, OUTPUT);
  pinMode(smMS2Pin, OUTPUT);
  pinMode(smMS1Pin, OUTPUT);

  //quarter step
  digitalWrite(smMS1Pin, LOW);
  digitalWrite(smMS2Pin, HIGH);  

  //Disable the motor
  pinMode(smEnablePin, OUTPUT);
  digitalWrite(smEnablePin, HIGH);

  //encoder
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);

  attachInterrupt(0, ai0, RISING);
  //B rising pulse from encodenren activated ai1(). AttachInterrupt 1 is DigitalPin nr 3 on  moustArduino.
  //attachInterrupt(1, ai1, RISING);

  Serial.begin(9600);
}

void SetFullStep()
{
  digitalWrite(smMS1Pin, LOW);
  digitalWrite(smMS2Pin, LOW);  
}

void SetHalfStep()
{
  digitalWrite(smMS1Pin, HIGH);
  digitalWrite(smMS2Pin, LOW);  
}

void SetQuarterStep()
{
  digitalWrite(smMS1Pin, LOW);
  digitalWrite(smMS2Pin, HIGH);  
}

void SetDirection(int d)
{
  if(d == 1)
  {
    digitalWrite(smDirectionPin, HIGH);
    Dir = 1;
  }
  else if(d == 2)
  {
    digitalWrite(smDirectionPin, LOW);
    Dir = -1;
  }
  else if(d == 0)
  {
    Dir = 0;
  }  
}

void Step()
{
  if(Dir == 0)
  {
    if(!(CrntDir == 0))
    {
      ZeroCounterFlg = true;
    }
    CrntDir = Dir;
    if(ZeroCounterFlg == true)
    {
      ZeroCounter++;
      if(ZeroCounter >= 1000)
      {
        ZeroCounter = 0;
        ZeroCounterFlg = false;
        digitalWrite(smEnablePin, HIGH);
      }
    }
    delay(1);
  }
  else
  {
    if(((StepperCounter > -178) && (StepperCounter < 178)) || ((StepperCounter >= 178) && (Dir == -1)) || ((StepperCounter <= -178) && (Dir == 1)))
    {
      digitalWrite(smEnablePin, LOW);
      ZeroCounter = 0;
      ZeroCounterFlg = false;
      StepperCounter += Dir;
      digitalWrite(smStepPin, HIGH);
      delayMicroseconds(800);
      digitalWrite(smStepPin, LOW);
      delayMicroseconds(800);
      CrntDir = Dir;
    }
    else
    {
      action = 0;
      SetDirection(action);
    }
  }
}

void loop()
{
  if(Serial.available() > 0) 
  {
    cmd = Serial.read();
    if(cmd == 3)
    {
      reset_mode = true;
      if(StepperCounter > 0)
      {
        action = 2;
      }
      else if(StepperCounter < 0)
      {
        action = 1;
      }
      else if(StepperCounter == 0)
      {
        action = 0;
      }
      SetDirection(action);
    }
    else if((cmd == 0) || (cmd == 1) || (cmd == 2))
    {
      action = cmd;
      SetDirection(action);
    }
    else if(cmd == 4)
    {
      Serial.print(counter);
      Serial.print(",");
      Serial.println(StepperCounter);
    }
  }

  if(reset_mode == true)
  {
    if((StepperCounter < 5) && (StepperCounter > -5))
    {
      action = 0;
      reset_mode = false;
      SetDirection(action);
    }
  }
  
  Step();
}

void ai0() 
{
  // ai0 is activated if DigitalPin nr 2 is going from LOW to HIGH
  // Check pin 3 to determine the direction
  if(digitalRead(3)==LOW) 
  {
    counter++;
    if(counter >= 2000)
    {
      counter = counter - 2000;
    }
  }
  else
  {
    counter--;
    if(counter <= -2000)
    {
      counter = counter + 2000;
    }
  }
}

void ai1() 
{
  // ai0 is activated if DigitalPin nr 3 is going from LOW to HIGH
  // Check with pin 2 to determine the direction
  if(digitalRead(2)==LOW) 
  {
    counter--;
    if(counter <= -2000)
    {
      counter = counter + 2000;
    }
  }
  else
  {
    counter++;
    if(counter >= 2000)
    {
      counter = counter - 2000;
    }
  }
}
