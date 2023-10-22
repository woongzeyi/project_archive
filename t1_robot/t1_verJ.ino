#include "DCMotor.h"
#include "PS2X_lib_temi.h"

#define PS2X_VIBRATE 0
#define _DCMotorPWM_Phase 1

byte Mode;
PS2X ps2_controller;
DCMotor motor = DCMotor(_DCMotorPWM_Phase);

void setup() {
  Serial.begin(9600);
  /* Setup controller */
  ps2_controller.config_gamepad(2, 19, 18, 13, 0, 0);
  /* Setup motor */
  motor.setMotorL(5, 6);
  motor.setMotorR(10, 9);
  /* Setup IR Sensor */
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A6, INPUT);
  digitalWrite(A0,HIGH);
  digitalWrite(A1,HIGH);
  digitalWrite(A2,HIGH);
  digitalWrite(A3,HIGH);
  digitalWrite(A6,HIGH);
}

void loop() {
  /* Establish and refresh connection, i.e. "vibrate" signal */
  read_sensor_values();
  ps2_controller.read_gamepad(0, PS2X_VIBRATE);
  Change_Mode();
  if (Mode == 0) {
    manual_control();
  } else {
    line_tracking();
  }
  delay(16);
}

void Change_Mode() {
  if (ps2_controller.ButtonPressed(PSB_SELECT)) {
    if (Mode == 0) {
      Mode = (byte)(1);
    } else {
      Mode = (byte)(0);
    }
  }
}

void line_tracking() {
  calc_pid();
  motor_control();
}

byte IrStatus;
int   Kp = 96, Ki = 4, Kd = 0;     //                  //pid參數 
float error = 0, P = 0, I = 0, D = 0, PID_value = 0;  //pid參數 
float previous_error = 0;                             //誤差值 
int initial_motor_speed = 255;                        //初始速度 
int left_motor_speed;
int right_motor_speed;

void read_sensor_values()
{
    IrStatus=0;
    if(analogRead(A0)>300) IrStatus +=1;  // R2
    if(analogRead(A1)>300) IrStatus +=2;  // R1
    if(analogRead(A2)>300) IrStatus +=4;  // M
    if(analogRead(A3)>300) IrStatus +=8;  // L1
    if(analogRead(A6)>300) IrStatus +=16; // L2
    /*
        e     01  03   02  06  04   0C  08   18  10     0E  1C  07   1E   0F  1F   0
    L2  4     ○   ○   ○   ○   ○   ○   ○   ●   ●     ○   ●   ○   ●   ○   ●   ○
    L1  2     ○   ○   ○   ○   ○   ●   ●   ●   ○     ●   ●   ○   ●   ●   ●   ○
    M   0     ○   ○   ○   ●   ●   ●   ○   ○   ○     ●   ●   ●   ●   ●   ●   ○
    R1  -2    ○   ●   ●   ●   ○   ○   ○   ○   ○     ●   ○   ●   ●   ●   ●   ○
    R2  -4    ●   ●   ○   ○   ○   ○   ○   ○   ○     ○   ○   ●   ○   ●   ●   ○
    Error    -4   -3  -2   -1   0    1   2   3    4     0    2  -2    0   0  Stop   脫軌
    */    
    if (IrStatus == 0x1C)      { // 11100
      error = 2;  
    }else if(IrStatus == 0x07) { // 00111
      error = -2;        
    }else if(IrStatus == 0x0F) { // 01111
      error = 0;  
    }else if(IrStatus == 0x1E) { // 11110
      error = 0;  
    }else if(IrStatus == 0x0E) { // 01110
      error = 0;  
    }else if(IrStatus == 0x04)  {// 00100
      error = 0;  
    }else if(IrStatus == 0x06)  {// 00110
      error = -1; 
    }else if(IrStatus == 0x02)  {// 00010
      error = -2; 
    }else if(IrStatus == 0x03)  {// 00011
      error = -3; 
    }else if(IrStatus == 0x01)  {// 00001
      error = -4; 
    }else if(IrStatus == 0x0C)  {// 01100
      error = 1; 
    }else if(IrStatus == 0x08)  {// 01000
      error = 2; 
    }else if(IrStatus == 0x18)  {// 11000
      error = 3; 
    }else if(IrStatus == 0x10)  {// 10000
      error = 4; 
    }else if(IrStatus == 0x00)  {// 00000  出軌，停車?
      if(error == -4)     error = -5;
      else if(error == 4) error = 5;      
    } else {  // 沒在判斷式的狀態      
    }
}
//****計算PID
void calc_pid()
{
  P = error;
  I = I*0.8 + error ;  // *0.8,不然會越積越大，沒收斂
  D = error - previous_error;
  previous_error = error;
  PID_value = (Kp * P) + (Ki * I) + (Kd * D); 
 // if(PID_value >255)  PID_value =255;
 // if(PID_value <-255) PID_value =-255;
}
//** 換算PWM，控制馬達
void motor_control()
{
  if (IrStatus == 0x1F || IrStatus == 0x0F) { //停車狀態
      motor.Stop( 1 );
  } else {//控制馬達速度
      left_motor_speed = initial_motor_speed - PID_value;
      right_motor_speed = initial_motor_speed + PID_value;
      
      if( left_motor_speed > 255) left_motor_speed = 255;
      else if( left_motor_speed < 0) left_motor_speed = 0;
      
      if( right_motor_speed > 255) right_motor_speed = 255;
      else if( right_motor_speed < 0) right_motor_speed = 0;
      
      motor.Forward(left_motor_speed,right_motor_speed);
  }
}

void manual_control() {
  int y_axis = ps2_controller.Analog(PSS_LY);
  int x_axis = ps2_controller.Analog(PSS_RX);
  Serial.print("X: ");
  Serial.print(x_axis);
  Serial.print("\t-> ");
  x_axis = map(x_axis, 0, 255, -100, 100);
  Serial.print(x_axis);
  Serial.print("\tY: ");
  Serial.print(y_axis);
  Serial.print("\t-> ");
  y_axis = map(y_axis, 0, 255, -255, 255);
  Serial.println(y_axis);

  if (y_axis == 1) {
    if (x_axis < 0) {
      motor.Left((float) -x_axis * 255.0f / 100.0f);
    } else {
      motor.Right((float) x_axis * 255.0f / 100.0f);
    }
  } else if (y_axis < 0) {
    /* Magic LMAO */
    motor.Forward(x_axis < 0 ? (float) abs(y_axis) * (1.0f + (float) x_axis / 100.0f) : (float) abs(y_axis), x_axis > 0 ? (float) abs(y_axis) * (1.0f - (float) x_axis / 100.0f) : (float) abs(y_axis));
  } else {
    /* This is magic too, HAHA */
    motor.Reverse(x_axis < 0 ? (float) abs(y_axis) * (1.0f + (float) x_axis / 100.0f) : (float) abs(y_axis), x_axis > 0 ? (float) abs(y_axis) * (1.0f - (float) x_axis / 100.0f) : (float) abs(y_axis));
  }
}
