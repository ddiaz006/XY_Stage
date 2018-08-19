#include <LiquidCrystal.h>

#include <Adafruit_MotorShield.h>
#include <Wire.h>
#include <Adafruit_MotorShield.h>

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
// Or, create it with a different I2C address (say for stacking)
// Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x61); 

// Connect a stepper motor with 200 steps per revolution (1.8 degree)
// to motor port #1 (M1 and M2)
Adafruit_StepperMotor *myMotor1 = AFMS.getStepper(200, 1);

// to motor port #2 (M3 and M4)
Adafruit_StepperMotor *myMotor2 = AFMS.getStepper(200, 2);


int horizontalSwitch = 2;
int verticalSwitch = 3;
int incomingByte = 0;   // for incoming serial data
String command = "";

float globalX = -99.0;
float globalY = -99.0;
float x = 60;
float y = 60;
float xmax = 134.0;
float ymax = 95.0;
int ix = -1;
int iy = -1;
int ib = 0;
int stepx = 142;
int stepy = 142;//135;

const int asize = 7;
float diodex = 14.11;
float diodey = 13.48;
//               D1     D2     D3     D4     D5     D6
float ax[] = {0, 37.04, 37.04, 23.29, 23.29, 9.67,  9.67};
float ay[] = {0, 12, 24.6 , 12, 24.6 , 12, 24.6};

////               D1     D2     D3     D4     D5     D6
//float ax[] = {0, 37.04, 37.04, 23.29, 23.29, 9.67,  9.67};
//float ay[] = {0, 12.11, 24.6 , 12.11, 24.6 , 12.11, 24.6};

//   Boards:  1  2     3     4      5      6
float bx[] = {0, 47.62,   95.24,   95.24 ,   47.62 ,   0    };
float by[] = {0, 0   , 0   , 54.49 ,   54.49 ,   54.49};


///
/////               D1     D2     D3     D4     D5     D6
///float ax[] = {0, 37.06, 37.06, 23.31, 23.31, 9.56,  9.56};
///float ay[] = {0, 11.84, 24.4 , 11.84, 24.4 , 11.84, 24.4};
///
/////   Boards:  1  2     3     4      5      6
///float bx[] = {0, 48,   96,   96 ,   48 ,   0    };
///float by[] = {0, 0   , 0   , 57 ,   57 ,   57};

void setup() {
  Serial.begin(9600);           // set up Serial library at 9600 bps

  AFMS.begin();  // create with the default frequency 1.6KHz
  //AFMS.begin(1000);  // OR with a different frequency, say 1KHz
  
  myMotor1->setSpeed(100);  // 10 rpm@100 
  myMotor2->setSpeed(500);  // 10 rpm@500
  
  pinMode(horizontalSwitch, INPUT_PULLUP);
  pinMode(verticalSwitch, INPUT_PULLUP);
  setHome();
}

void loop() {
  //set up for commands
  String data="";
  String xval="";
  String yval="";

  if(Serial.available()>0)
  {
    char p = Serial.peek();;
    //delay(150);
    while(Serial.available()>0){
    command += char(Serial.read());
    delay(150);
    }
    if(p =='m'){
    data = getValue(command, ',', 0);
    xval = getValue(command, ',', 1);
    yval = getValue(command, ',', 2);
    }
    if(p =='d'){
    data = getValue(command, ',', 0);
    xval = getValue(command, ',', 1);
    yval = getValue(command, ',', 2);
    }
    if(p =='i'){
    data = getValue(command, ',', 0);
    xval = getValue(command, ',', 1);
    }
    if(p =='b'){
    data = getValue(command, ',', 0);
    xval = getValue(command, ',', 1);
    yval = getValue(command, ',', 2);
    }
    if(p =='a'){
    data = getValue(command, ',', 0);
    xval = getValue(command, ',', 1);
    yval = getValue(command, ',', 2);
    }
    Serial.println(command);
  }
  if(command =="home") {setHome(); Update();}
  
  if(command =="n") { if(ix<asize-1){ix = ix + 1; iy = iy + 1; XYMove(ax[ix], ay[iy]); Update();} else Serial.println("Bad Point"); }
  if(command =="p") { if(iy >-1    ){ix = ix - 1; iy = iy - 1; XYMove(ax[ix], ay[iy]); Update();} else Serial.println("Bad Point"); }
  if(command =="reset") {ix = 0; iy = 0; Update();}
  if(command =="rs"){RasterSet();}
  if(command =="r" ){Raster();}
  if(command =="x" ){Show();}
  
  if(data == "move") {
    x=xval.toFloat();
    y=yval.toFloat();
    XYMove(x,y);
    Update();
  }
  if(data == "dmove") {
    x=xval.toFloat();
    y=yval.toFloat();
    DXYMove(x,y);
    Update();
  }
  if(data == "adjust") {
    x=xval.toFloat();
    y=yval.toFloat();
    Adjust(x,y);
    //Update();
  }
  if(data =="imove"){
  int ipoint = xval.toInt();
  XYMove(ax[ipoint], ay[ipoint]);
  ix = ipoint;
  iy = ipoint;
  Update();
  }
  if(data =="bmove"){
  int bpoint = xval.toInt() - 1;
  int ipoint = yval.toInt();
  ib = bpoint -1;
  if(bx[bpoint]+ax[ipoint]>0.0 && by[bpoint]+ay[ipoint] >0.0){
  XYMove(bx[bpoint]+ax[ipoint], by[bpoint]+ay[ipoint]);
  }
  else Serial.println("bad point");
  ix = ipoint;
  iy = ipoint;
  ib = bpoint;
  Update();
  }
  command = "";
}

void Update(){
  Serial.println("  ");
  Serial.print("POS: ");
  Serial.print(globalX);
  Serial.print(",");
  Serial.print(globalY);
  Serial.print(",  Board: ");
  Serial.print(ib+1);
  Serial.print(",  Point: ");
  Serial.println(ix);
  Serial.println("  ");
}

void Show(){
  int pts[]= {5,6,4,3,1,2};
  int pts2[]= {1,2,4,3,5,6};
  //loop through boards
  for(int i = 0; i<6; i++){
    for(int j = 0; j <6; j++){
      Serial.print("Moving to Board: ");
      Serial.print(i);
      Serial.print(", Diode: ");
      Serial.println(j);
      if(i<3){
        if(bx[i]+ax[pts[j]]>0.0 && by[i]+ay[pts[j]] >0.0){
          XYMove(bx[i]+ax[pts[j]], by[i]+ay[pts[j]]);
          delay(600);
        }
      }
      else{
        if(bx[i]+ax[pts2[j]]>0.0 && by[i]+ay[pts2[j]] >0.0){
          XYMove(bx[i]+ax[pts2[j]], by[i]+ay[pts2[j]]);
          delay(600);
        }
      }
    }
  }
  Serial.println("Finished cycles");
}

String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void Adjust(float dx, float dy)
{
  Serial.println(" ");
  Serial.println("bx      by");
  for(int i=0; i<6; i++)
  {
    bx[i] = bx[i] - dx;
    by[i] = by[i] - dy;
    Serial.print(bx[i]);
    Serial.print("      ");
    Serial.println(by[i]);
  }
}

void XYMove(float x, float y)
{
  if(x<xmax && y<ymax && x>=0 && y>=0){  
    float dx = x-globalX;
    float dy = y-globalY;
    //Move to x, y
    Serial.print("Moving...");

    if(dx>=0){
      for (int i=0; i <=(dx*stepx); i++){
        myMotor1->onestep(FORWARD, DOUBLE);
    }
    }
    if(dx<0){
     for (int i=0; i <=(abs(dx)*stepx); i++){
     myMotor1->onestep(BACKWARD, DOUBLE);
    }
    }
    if(dy>=0){
    for (int j=0; j <=(dy*stepy); j++){
      myMotor2->onestep(FORWARD, DOUBLE);
    }
    }
    if(dy<0){
     for (int j=0; j <=(abs(dy)*stepy); j++){
     myMotor2->onestep(BACKWARD, DOUBLE);
    }
    }
    globalX = x;
    globalY = y;
    myMotor1->release();
    myMotor2->release();
    Serial.println("  complete!");
    Serial.print("Moved to ");
    Serial.print(globalX);
    Serial.print(",");
    Serial.println(globalY);
  }
  else{Serial.print("Can't move here, point off track ");}
}

void DXYMove(float x, float y)
{
  if((globalX + x)<xmax && (globalY + y)<ymax && (globalX + x)>=0 && (globalY + y)>=0 ){  
    //Move to x, y
    Serial.print("Moving...");
    if(x>=0){
      for (int i=0; i <=(x*stepx); i++){
        myMotor1->onestep(FORWARD, DOUBLE);
    }
    }
    if(x<0){
     for (int i=0; i <=(abs(x)*stepx); i++){
     myMotor1->onestep(BACKWARD, DOUBLE);
    }
    }
    if(y>=0){
    for (int j=0; j <=(y*stepy); j++){
      myMotor2->onestep(FORWARD, DOUBLE);
    }
    }
    if(y<0){
     for (int j=0; j <=(abs(y)*stepy); j++){
     myMotor2->onestep(BACKWARD, DOUBLE);
    }
    }
    globalX = globalX + x;
    globalY = globalY + y;
    myMotor1->release();
    myMotor2->release();
    Serial.println("  complete!");
    Serial.print("Moved to ");
    Serial.print(globalX);
    Serial.print(",");
    Serial.println(globalY);
  }
  else{Serial.print("Can't move here, point off track ");}
}


bool highHorizontal()
{
  bool a;
  int count = 0;
  for(int i = 0; i<10; i++){
  a = digitalRead(horizontalSwitch);
  if(a==HIGH) count =count + 1;
  }
  if(count>4) return true;
  else return false;
}

bool highVertical()
{
  bool a;
  int count = 0;
  for(int i = 0; i<10; i++){
  a = digitalRead(verticalSwitch);
  if(a==HIGH) count =count + 1;
  }
  if(count>4) return true;
  else return false;
}

void setHome()
{
  delay(500);
  Serial.print("Moving to home position...");
  while (highHorizontal()){
    myMotor1->onestep(BACKWARD, DOUBLE); 
  }
  while (highVertical()){
    myMotor2->onestep(BACKWARD, DOUBLE); 
  }
  myMotor1->release();
  myMotor2->release();
  Serial.println("complete!");
  globalX = 0;
  globalY = 0;
  int ix = -1;
  int iy = -1;
  int ib = 0;
}

void  horizontalHomeRoutine(){
  while (highHorizontal()) 
  {
    myMotor1->onestep(BACKWARD, DOUBLE); 
  }
  delay(100);  
  for (int i=0; i <=5000; i++)
  {
    myMotor1->onestep(FORWARD, DOUBLE); 
  }
  myMotor1->release();
}

void  verticalHomeRoutine(){
  while (highVertical() ){
    myMotor2->onestep(BACKWARD, DOUBLE);
  }
  delay(100);
  for (int i=0; i <=5000; i++)
  {
    myMotor2->onestep(FORWARD, DOUBLE); 
  }
  myMotor2->release();
}

//Caleb, I think that these should be functions with inputs
// e.g., Raster(int boardNumber, float xOffSet, float yOffSet, float stepSize)
// and   RasterSet(int boardNumber, float xOffSet, float yOffSet)
// or making definitions of the rastering home position like we have for 
// the diodes and boards up top. Since you will be using this code next and 
// not me, I will leave it like this for now 
/*void Raster(){
  //XYMove(bx[3],by[3])
  for (int H=0; H <10; ++H){
    if (H%2!=0){
      XYMove(bx[3]+14.56, by[3]+(13.3+0.6*H));
    }
    else if (H%2==0){
      XYMove(bx[3]+28.65, by[3]+(13.3+0.6*H));
    }
    else{
      Serial.println("Problem with H");
    }
  }
  Serial.print("Done");
}
void RasterSet(){
  XYMove(bx[3]+14.56, by[3]+12.7);
  Serial.println("Done Raster Set");
}
*/
void Raster(){
  //XYMove(bx[3],by[3])
  for (int H=0; H <17; ++H){
    if (H%2!=0){
      XYMove(bx[3]+20.20, by[3]+(12.3+0.6*H));
    }
    else if (H%2==0){
      XYMove(bx[3]+30.65, by[3]+(12.3+0.6*H));
      if (H==16){
        for (int K=16; K >0; --K){
          if (K%2!=0){
            XYMove(bx[3]+20.20, by[3]+(12.3+0.6*K));
          }
          else if (K%2==0){
          XYMove(bx[3]+30.65, by[3]+(12.3+0.6*K));
          }
        }
      }
    }
    else{
      Serial.println("Problem with H");
    }
  }
  Serial.print("Done");
}
void RasterSet(){
  XYMove(bx[3]+20.20, by[3]+12.3);
  Serial.println("Done Raster Set");
}
