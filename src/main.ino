#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>


#define TFT_CS     D4
#define TFT_RST    -1  // you can also connect this to the Arduino reset
// in which case, set this #define pin to -1!
#define TFT_DC     D3

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

SoftwareSerial dbgSerial(D1, D2); // RX, TX

#define WIDTH 128
#define ROWHEIGHT 25

void setup(void) {
  pinMode ( D0, INPUT_PULLDOWN_16 );
  pinMode ( D8, INPUT_PULLDOWN_16 );

  WiFi.mode( WIFI_OFF );
  WiFi.forceSleepBegin();

  Serial.begin(115200);
  //Serial.setTimeout(2000);
  dbgSerial.begin(115200);
  dbgSerial.println("GRBL LCD DRO starting");

  tft.initR(INITR_144GREENTAB);
  tft.setTextWrap(false); // Allow text to run off right edge
  tft.fillScreen(ST7735_BLACK);
  tft.setRotation(0);

  delay(5000);
}


void loop(void) {

  Serial.write("?"); // Status Report Query
  Serial.readStringUntil('<');
  String status = Serial.readStringUntil('|');
  dbgSerial.println(status);

  handleButtons(status);

  if (status == "test") {
    tft.setTextColor(ST7735_RED, ST7735_BLACK);
    tft.setTextSize(1);
    tft.setCursor(3, 2);
    tft.print("TEST");
    return;
  }

  String mPos = Serial.readStringUntil('|');
  dbgSerial.println(mPos);
  int i1 = mPos.indexOf(':');
  int i2 = mPos.indexOf(',', i1+1);
  int i3 = mPos.indexOf(',', i2+1);
  int i4 = mPos.indexOf(',', i3+1);
  float xPos = mPos.substring(i1+1, i2).toFloat();
  float yPos = mPos.substring(i2+1, i3).toFloat();
  float zPos = mPos.substring(i3+1, i4).toFloat();
  float aPos = mPos.substring(i4+1).toFloat();

  handleButtons(status);

  String line = Serial.readString();
  dbgSerial.println("------------------------------");
  dbgSerial.println(line);

  handleButtons(status);

  String FS = "";
  String Pn = "";
  String SD = "";

  i1 = line.indexOf("FS:");
  if (i1 >= 0) {
    i2 = line.indexOf('|', i1+1);
    if (i2 == -1) 
      i2 = line.indexOf('>', i1+1);
    FS = line.substring(i1+3, i2);
    dbgSerial.print("FS "); dbgSerial.println(FS);
  }

  i1 = line.indexOf("Pn:");
  if (i1 >= 0) {
    i2 = line.indexOf('|', i1+1);
    if (i2 == -1) 
      i2 = line.indexOf('>', i1+1);
    Pn = line.substring(i1+3, i2);
    dbgSerial.print("Pn "); dbgSerial.println(Pn);
  }

  i1 = line.indexOf("SD:");
  if (i1 >= 0) {
    i2 = line.indexOf('|', i1+1);
    if (i2 == -1) 
      i2 = line.indexOf('>', i1+1);
    SD = line.substring(i1+3, i2);
    dbgSerial.print("SD "); dbgSerial.println(SD);
  }

  int tColor = ST7735_WHITE;
  if (status == "Alarm" ) {
    tColor = ST7735_RED;
  } else if (status.indexOf("Hold")>=0 || status.indexOf("Door")>=0 ) {
    tColor = ST7735_ORANGE;
  } else if (status == "Run" || status== "Jog" || status== "Home") {
    tColor = ST7735_YELLOW;
  }

  int y = 2;

  tft.fillRect(0, y, WIDTH, 8, ST7735_BLACK);
  tft.setTextColor(tColor, ST7735_BLACK);
  tft.setTextSize(1);
  tft.setCursor(3, y);
  tft.print("STATUS: ");
  if (status != "")
    tft.print(status);
  else
    tft.print("NO DATA");

  handleButtons(status);

  y+= 8;
  tft.drawFastHLine(0, y+1, WIDTH, tColor);

  y+= 4;
  int col = tColor;
  drawPos("X", xPos, 6, y, ST7735_WHITE, (Pn.indexOf("X")>=0 ? ST7735_RED : tColor));
  y+=ROWHEIGHT;
  drawPos("Y", yPos, 6, y, ST7735_WHITE, (Pn.indexOf("Y")>=0 ? ST7735_RED : tColor));
  y+=ROWHEIGHT;
  drawPosSmall("Z", zPos, 6, y, ST7735_WHITE, (Pn.indexOf("Z")>=0 ? ST7735_RED : tColor));
  y+=18;
  drawPosSmall("A", aPos, 6, y, ST7735_WHITE, tColor);
  y+=18;
  tft.drawFastHLine(0, y, WIDTH, tColor);
  y+= 4;
  tft.fillRect(0, y, WIDTH, 20, ST7735_BLACK);
  tft.setCursor(3, y);
  tft.setTextSize(1);
  tft.setTextWrap(false);
  tft.print("Feed:");tft.print(FS);tft.print(" Pins:");tft.print(Pn);

  y=114;
  tft.drawFastHLine(0, y, WIDTH, tColor);

  y+= 4;
  tft.fillRect(0, y, WIDTH, 20, ST7735_BLACK);
  tft.setTextSize(1);
  tft.setCursor(3, y);
  tft.setTextColor(tColor, ST7735_BLACK);
  if (status != "Idle" && SD != "") {
    tft.print("FILE:");tft.print(SD);
  } else {
    if (status == "Idle" || status == "Alarm" ) {
      tft.print(" HOME   ");
    } else if (status == "Run" || status == "Jog") {
      tft.print(" HOLD   ");
    } else  {
      tft.print("RESET   ");
    }

    if (status == "Idle") {
      tft.print("   RUN: 1.nc");
    } else  {
      tft.print("       START");
    }

    
  }

  //delay(5000);
}

void handleButtons(String status) {
  if(digitalRead(D0)) {
    dbgSerial.println("Button: RED");
    if (status == "Idle" || status == "Alarm" ) {
      Serial.println("$H");  // HOME MACHINE
    } else if (status == "Run" || status == "Jog") {
      Serial.print("!"); // Feed Hold
    } else  {
      Serial.write('0x18'); // Soft RESET
    }
    tft.fillRect(0, 118, WIDTH/2, 20, ST7735_RED);
    delay(2000);
  }
  if(digitalRead(D8)) {
    dbgSerial.println("Button: WHITE");
    if (status == "Idle") {
      Serial.println("[ESP220]/1.nc\r"); // Run File 1.nc
    } else  {
      Serial.print("~"); // Cycle Start / Resume
    }
    tft.fillRect(WIDTH/2, 118, WIDTH, 20, ST7735_WHITE);
    delay(2000);
  }
}

// TFT Routines ------------------------------------------------------------------------------------------------------

void drawPos(String axis, double n, unsigned int xLoc, unsigned int yLoc, int color, int aColor) {
  tft.setTextSize(3);
  tft.setCursor(xLoc, yLoc);
  tft.setTextColor(aColor, ST7735_BLACK);
  tft.print(axis);
  tft.print(":");
  xLoc+=35;

  draw7Number(int(n), xLoc, yLoc, 1,color, ST7735_BLACK, 4);
  xLoc+=55;
  int partion = (int(n*100.0))%100;
  tft.fillRect(xLoc, yLoc+19, 3, 3, color);
  xLoc+=5;
  draw7Number(partion, xLoc, yLoc+8, 0.8,color, ST7735_BLACK, 2);

  //tft.setTextSize(2);
  //tft.setCursor(xLoc+20, yLoc+8);
  //tft.print("mm");
}

void drawPosSmall(String axis, double n, unsigned int xLoc, unsigned int yLoc, int color, int aColor) {
  tft.setTextSize(2);;
  tft.setCursor(xLoc, yLoc);
  tft.setTextColor(aColor, ST7735_BLACK);
  tft.print(axis);
  tft.print(":");
  xLoc+=54;

  draw7Number(int(n), xLoc, yLoc, 0.8,color, ST7735_BLACK, 4);
  xLoc+=36;
  int partion = (int(n*100.0))%100;
  tft.fillRect(xLoc, yLoc+10, 3, 3, color);
  xLoc+=5;
  draw7Number(partion, xLoc, yLoc, 0.8,color, ST7735_BLACK, 2);

  //tft.setTextSize(2);
  //tft.setCursor(xLoc+20, yLoc+8);
  //tft.print("mm");
}

/**********************************************************************************
 Routine to Draw Large 7-Segment formated number with Arduino TFT Library
    by William Zaggle  (Uses TFT Library DrawLine functions).

   int n - The number to be displayed
   int xLoc = The x location of the upper left corner of the number
   int yLoc = The y location of the upper left corner of the number
   int cSe = The size of the number. Range 1 to 10 uses Large Shaped Segments.
   fC is the foreground color of the number
   bC is the background color of the number (prevents having to clear previous space)
   nD is the number of digit spaces to occupy (must include space for minus sign for numbers < 0)
   nD < 0 Suppresses leading zero

   Sample Use: Fill the screen with a 2-digit number suppressing leading zero

          draw7Number(38,20,40,10,WHITE, BLACK, -2);

**********************************************************************************/
void draw7Number(int n, unsigned int xLoc, unsigned int yLoc, float cS, unsigned int fC, unsigned int bC, char nD) {
  unsigned int num=abs(n),i,s,t,w,col,h,a,b,si=0,j=1,d=0,S1=cS,S2=5*cS,S3=2*cS,S4=7*cS,x1=(S3/2)+1,x2=(2*S1)+S2+1,y1=yLoc+x1,y3=yLoc+(2*S1)+S4+1;
  unsigned int seg[7][3]={{(S3/2)+1,yLoc,1},{x2,y1,0},{x2,y3+x1,0},{x1,(2*y3)-yLoc,1},{0,y3+x1,0},{0,y1,0},{x1,y3,1}};
  unsigned char nums[12]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x67,0x00,0x40},c=(c=abs(cS))>10?10:(c<1)?1:c,cnt=(cnt=abs(nD))>10?10:(cnt<1)?1:cnt;
  for (xLoc+=cnt*(d=(2*S1)+S2+(2*S3)+2);cnt>0;cnt--){
    for (i=(num>9)?num%10:((!cnt)&&(n<0))?11:((nD<0)&&(!num))?10:num,xLoc-=d,num/=10,j=0;j<7;++j){
      col=(nums[i]&(1<<j))?fC:bC;s=(2*S1)/S3;
      if (seg[j][2])for(w=S2,t=seg[j][1]+S3,h=seg[j][1]+(S3/2),a=xLoc+seg[j][0]+S1,b=seg[j][1];b<h;b++,a-=s,w+=(2*s))tft.drawFastHLine(a,b,w,col);
      else for(w=S4,t=xLoc+seg[j][0]+S3,h=xLoc+seg[j][0]+S3/2,b=xLoc+seg[j][0],a=seg[j][1]+S1;b<h;b++,a-=s,w+=(2*s))tft.drawFastVLine(b,a,w,col);
      for (;b<t;b++,a+=s,w-=(2*s))seg[j][2]?tft.drawFastHLine(a,b,w,col):tft.drawFastVLine(b,a,w,col);
    }
  }
}
