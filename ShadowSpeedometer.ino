
#include <math.h>

#include <U8glib.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

U8GLIB_SSD1309_128X64 u8g(7, 6, 4, 5);

SoftwareSerial gpsSerial(10, 11);
TinyGPSPlus gps;

// These use a lot of memory, might need to trim it down
char speed_buffer[16] = "NaN mph";
char temp_speed_buffer[16] = "0";
char sat_count_buffer[16] = "sats: X";
char time_buffer[16] = "00:00";
char top_speed_buffer[16] = "-Not Yet-";

int mph = 0;
int top_speed = 0;
bool dst = true;

#define weewoo_bmp_width 20
#define weewoo_bmp_height 21
const unsigned char weewoo_bmp [] PROGMEM=
{
  0x18, 0x86, 0x01, 0xfc, 0xff, 0x03, 0xfe, 0xff, 0x07, 0xfc,
  0xff, 0x07, 0xfc, 0xff, 0x03, 0xfc, 0xff, 0x03, 0xfc, 0xf9,
  0x03, 0xfe, 0xf9, 0x07, 0xfe, 0xf1, 0x07, 0x1e, 0x80, 0x07,
  0x3e, 0xc0, 0x0f, 0x7f, 0xe0, 0x0f, 0x7f, 0xe0, 0x0f, 0x7f,
  0xe0, 0x0f, 0x7e, 0xef, 0x07, 0xfe, 0xff, 0x07, 0xf8, 0xff,
  0x01, 0xe0, 0x7f, 0x00, 0x80, 0x1f, 0x00, 0x00, 0x06, 0x00,
  0x00, 0x00, 0x00
};

#define honda_width 48
#define honda_height 38
const unsigned char honda_bits[] PROGMEM= {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3f,
  0x00, 0x00, 0x00, 0x00, 0xe0, 0x1f, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x0f,
  0x00, 0x00, 0x00, 0x00, 0xff, 0x07, 0x00, 0x00, 0x00, 0xc0, 0xff, 0x01,
  0x00, 0x00, 0x00, 0xf8, 0x3f, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x07, 0x07,
  0x00, 0x00, 0xc0, 0xff, 0xe0, 0x03, 0x00, 0x00, 0xf0, 0x1f, 0xfc, 0x03,
  0x00, 0x00, 0xfe, 0x83, 0xff, 0x01, 0x00, 0x80, 0xff, 0xf0, 0xff, 0x00,
  0x00, 0xf0, 0x1f, 0xfe, 0x1f, 0x00, 0x00, 0xfc, 0xc3, 0xff, 0x81, 0x00,
  0x80, 0x7f, 0xf8, 0x1f, 0x78, 0x00, 0xe0, 0x8f, 0xff, 0x83, 0x3f, 0x00,
  0xfc, 0xf1, 0x3f, 0xf8, 0x1f, 0x00, 0x3e, 0xfe, 0x83, 0xff, 0x0f, 0x00,
  0xcf, 0x3f, 0xf8, 0xff, 0x01, 0x00, 0xf3, 0x9f, 0xff, 0x07, 0x00, 0x00,
  0xfb, 0xf1, 0x3f, 0xe0, 0x03, 0x00, 0x17, 0xff, 0x01, 0xff, 0x01, 0x00,
  0xee, 0xff, 0xfc, 0xff, 0x00, 0x00, 0xdc, 0xe3, 0xff, 0x1f, 0x00, 0x00,
  0x38, 0xff, 0x7f, 0x00, 0x00, 0x00, 0x70, 0xff, 0x0f, 0x00, 0x00, 0x00,
  0xe0, 0x0e, 0xfe, 0x01, 0x00, 0x00, 0xc0, 0xf9, 0xff, 0x03, 0x00, 0x00,
  0x80, 0xf3, 0xff, 0x0f, 0x00, 0x00, 0x00, 0xef, 0xff, 0x3f, 0x00, 0x00,
  0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x8e, 0xf9, 0xf3, 0xcc, 0x1f, 0x1e,
  0x8e, 0x1d, 0xf6, 0xc9, 0x31, 0x1b, 0xfe, 0x1d, 0xb6, 0xcb, 0x31, 0x3f,
  0x8e, 0x19, 0x37, 0xcb, 0xb9, 0x71, 0xdf, 0xf3, 0x7b, 0xef, 0xdf, 0xf9
};

const u8g_fntpgm_uint8_t creep2[2451] U8G_FONT_SECTION("creep2") = {
  0,5,11,0,254,7,1,135,3,29,32,255,254,9,254,7,
  254,0,0,0,5,4,1,1,7,7,5,2,0,128,128,128,
  128,128,0,128,3,3,3,5,1,4,160,160,160,5,5,5,
  5,0,0,80,248,80,248,80,5,7,7,5,0,0,32,120,
  160,112,40,240,32,4,4,4,5,0,1,144,32,64,144,5,
  7,7,5,0,0,64,160,160,64,168,144,120,1,2,2,5,
  2,4,128,128,3,9,9,5,0,254,32,64,128,128,128,128,
  128,64,32,3,9,9,5,0,254,128,64,32,32,32,32,32,
  64,128,3,3,3,5,0,1,160,64,160,5,5,5,5,0,
  0,32,32,248,32,32,2,2,2,5,1,255,64,128,4,1,
  1,5,0,2,240,1,1,1,5,2,0,128,4,8,8,5,
  0,255,16,16,32,32,64,64,128,128,4,7,7,5,0,0,
  96,144,144,176,208,144,96,2,7,7,5,1,0,64,192,64,
  64,64,64,64,4,7,7,5,0,0,96,144,16,32,64,128,
  240,4,7,7,5,0,0,240,16,32,96,16,144,96,4,7,
  7,5,0,0,96,96,160,160,240,32,32,4,7,7,5,0,
  0,240,128,224,144,16,144,96,4,7,7,5,0,0,96,144,
  128,224,144,144,96,4,7,7,5,0,0,240,16,16,32,32,
  64,64,4,7,7,5,0,0,96,144,144,96,144,144,96,4,
  7,7,5,0,0,96,144,144,112,16,144,96,1,3,3,5,
  1,1,128,0,128,2,4,4,5,0,0,64,0,64,128,3,
  5,5,5,0,0,32,64,128,64,32,4,3,3,5,0,1,
  240,0,240,3,5,5,5,0,0,128,64,32,64,128,4,7,
  7,5,0,0,224,16,48,64,64,0,64,4,6,6,5,0,
  0,240,144,176,176,128,240,4,7,7,5,0,0,96,144,144,
  240,144,144,144,4,7,7,5,0,0,192,160,160,224,144,144,
  240,4,7,7,5,0,0,96,144,128,128,128,144,96,4,7,
  7,5,0,0,192,160,144,144,144,160,192,4,7,7,5,0,
  0,240,128,128,192,128,128,240,4,7,7,5,0,0,240,128,
  128,192,128,128,128,4,7,7,5,0,0,96,144,128,128,176,
  144,96,4,7,7,5,0,0,144,144,240,144,144,144,144,3,
  7,7,5,0,0,224,64,64,64,64,64,224,4,7,7,5,
  0,0,112,16,16,16,16,144,112,4,7,7,5,0,0,144,
  160,192,160,160,144,144,4,7,7,5,0,0,128,128,128,128,
  128,128,240,4,7,7,5,0,0,144,240,240,144,144,144,144,
  4,7,7,5,0,0,144,208,208,176,176,144,144,4,7,7,
  5,0,0,96,144,144,144,144,144,96,4,7,7,5,0,0,
  224,144,144,224,128,128,128,4,8,8,5,0,255,96,144,144,
  144,144,176,112,16,4,7,7,5,0,0,224,144,144,224,192,
  160,144,4,7,7,5,0,0,96,144,128,96,16,144,96,3,
  7,7,5,0,0,224,64,64,64,64,64,64,4,7,7,5,
  0,0,144,144,144,144,144,144,112,4,7,7,5,0,0,144,
  144,144,144,144,144,96,4,7,7,5,0,0,144,144,144,144,
  240,240,144,4,7,7,5,0,0,144,144,96,144,144,144,144,
  4,7,7,5,0,0,144,144,144,112,16,144,112,4,7,7,
  5,0,0,240,16,16,32,64,128,240,2,9,9,5,1,254,
  192,128,128,128,128,128,128,128,192,4,8,8,5,0,255,128,
  128,64,64,32,32,16,16,2,9,9,5,1,254,192,64,64,
  64,64,64,64,64,192,3,2,2,5,1,5,64,160,4,1,
  1,5,0,0,240,2,2,2,5,1,5,128,64,4,5,5,
  5,0,0,112,144,144,144,112,4,7,7,5,0,0,128,128,
  224,144,144,144,224,4,5,5,5,0,0,96,144,128,144,96,
  4,7,7,5,0,0,16,16,112,144,144,144,112,4,5,5,
  5,0,0,96,144,240,128,112,4,9,9,5,0,254,32,80,
  64,224,64,64,64,64,128,4,7,7,5,0,254,112,144,144,
  144,112,16,96,4,7,7,5,0,0,128,128,224,144,144,144,
  144,3,7,7,5,0,0,64,0,192,64,64,64,96,2,9,
  9,5,1,254,64,0,192,64,64,64,64,64,128,4,7,7,
  5,0,0,128,128,144,160,192,160,144,2,7,7,5,1,0,
  128,128,128,128,128,128,64,4,5,5,5,0,0,144,240,144,
  144,144,4,5,5,5,0,0,224,144,144,144,144,4,5,5,
  5,0,0,96,144,144,144,96,4,7,7,5,0,254,224,144,
  144,144,224,128,128,4,7,7,5,0,254,112,144,144,144,112,
  16,16,4,5,5,5,0,0,224,144,128,128,128,4,5,5,
  5,0,0,112,128,96,16,224,3,7,7,5,0,0,64,64,
  224,64,64,64,32,4,5,5,5,0,0,144,144,144,144,112,
  4,5,5,5,0,0,144,144,144,144,96,4,5,5,5,0,
  0,144,144,144,240,144,4,5,5,5,0,0,144,144,96,144,
  144,4,7,7,5,0,254,144,144,144,144,112,144,96,4,5,
  5,5,0,0,240,32,64,128,240,4,9,9,5,0,254,48,
  64,64,64,128,64,64,64,48,1,7,7,5,2,0,128,128,
  128,128,128,128,128,4,9,9,5,0,254,192,32,32,32,16,
  32,32,32,192,4,2,2,5,0,1,80,160,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,0,0,0,
  5,0,1,1,7,7,5,2,0,128,0,128,128,128,128,128,
  4,7,7,5,0,255,64,96,208,192,208,96,64,4,7,7,
  5,0,0,96,144,128,192,128,208,176,5,5,5,5,0,0,
  136,112,80,112,136,5,7,7,5,0,0,80,80,248,80,248,
  32,32,1,7,7,5,2,0,128,128,128,0,128,128,128,4,
  7,7,5,0,0,96,144,224,144,112,144,96,3,1,1,5,
  0,6,160,5,7,7,5,0,0,48,72,176,160,176,72,48,
  4,7,7,5,0,2,112,144,144,144,112,0,240,4,4,4,
  5,0,1,80,160,160,80,4,2,2,5,0,3,240,16,4,
  1,1,5,0,2,240,5,7,7,5,0,0,48,72,176,160,
  160,72,48,4,1,1,5,0,6,240,3,3,3,5,1,3,
  64,160,64,5,5,5,5,0,0,32,32,248,32,248,3,4,
  4,5,1,3,96,160,64,224,3,4,4,5,1,3,224,64,
  32,192,2,2,2,5,1,4,64,128,4,7,7,5,0,254,
  144,144,144,144,224,128,128,4,8,8,5,0,254,112,208,208,
  208,80,80,80,96,1,1,1,5,2,2,128,2,2,2,5,
  1,254,64,192,3,4,4,5,1,3,192,64,64,224,3,5,
  5,5,0,1,64,160,64,0,224,4,4,4,5,0,1,160,
  80,80,160,4,8,8,5,0,255,32,32,0,240,0,80,112,
  16,4,9,9,5,0,254,32,32,0,240,0,32,80,32,112,
  4,11,11,5,0,254,112,16,48,16,112,0,240,0,80,112,
  16,4,7,7,5,0,0,32,0,32,32,192,128,112,4,9,
  9,5,0,0,64,32,96,144,144,240,144,144,144,4,9,9,
  5,0,0,32,64,96,144,144,240,144,144,144,4,9,9,5,
  0,0,96,144,96,144,144,240,144,144,144,4,9,9,5,0,
  0,80,160,96,144,144,240,144,144,144,4,9,9,5,0,0,
  80,0,96,144,144,240,144,144,144,4,9,9,5,0,0,64,
  0,96,144,144,240,144,144,144,4,7,7,5,0,0,112,160,
  160,240,160,160,176,4,9,9,5,0,254,96,144,128,128,128,
  144,96,32,96,4,9,9,5,0,0,64,32,240,128,128,192,
  128,128,240,4,9,9,5,0,0,32,64,240,128,128,192,128,
  128,240,4,9,9,5,0,0,96,144,240,128,128,192,128,128,
  240,4,9,9,5,0,0,80,0,240,128,128,192,128,128,240,
  3,9,9,5,0,0,128,64,224,64,64,64,64,64,224,3,
  9,9,5,0,0,32,64,224,64,64,64,64,64,224,3,9,
  9,5,0,0,64,160,224,64,64,64,64,64,224,3,9,9,
  5,0,0,160,0,224,64,64,64,64,64,224,5,7,7,5,
  0,0,96,80,72,232,72,80,96,4,9,9,5,0,0,80,
  160,144,144,208,208,176,176,144,4,9,9,5,0,0,64,32,
  96,144,144,144,144,144,96,4,9,9,5,0,0,32,64,96,
  144,144,144,144,144,96,4,9,9,5,0,0,96,144,96,144,
  144,144,144,144,96,4,9,9,5,0,0,80,160,96,144,144,
  144,144,144,96,4,9,9,5,0,0,80,0,96,144,144,144,
  144,144,96,4,4,4,5,0,0,144,96,96,144,5,7,7,
  5,0,0,104,152,176,208,144,144,96,4,9,9,5,0,0,
  64,32,144,144,144,144,144,144,112,4,9,9,5,0,0,32,
  64,144,144,144,144,144,144,112,4,9,9,5,0,0,32,80,
  0,144,144,144,144,144,112,4,9,9,5,0,0,144,0,144,
  144,144,144,144,144,112,4,9,9,5,0,0,32,64,144,144,
  144,112,16,144,96,4,7,7,5,0,0,128,224,144,144,144,
  224,128,4,7,7,5,0,255,192,160,160,208,144,224,128,4,
  8,8,5,0,0,64,32,0,112,144,144,144,112,4,8,8,
  5,0,0,32,64,0,112,144,144,144,112,4,8,8,5,0,
  0,32,80,0,112,144,144,144,112,5,8,8,5,0,0,40,
  80,0,112,144,144,144,112,4,7,7,5,0,0,80,0,112,
  144,144,144,112,4,7,7,5,0,0,32,0,112,144,144,144,
  112,4,5,5,5,0,0,112,160,240,160,176,4,7,7,5,
  0,254,96,144,128,144,96,32,96,4,8,8,5,0,0,64,
  32,0,96,144,240,128,112,4,8,8,5,0,0,32,64,0,
  96,144,240,128,112,4,8,8,5,0,0,32,80,0,96,144,
  240,128,112,4,7,7,5,0,0,80,0,96,144,240,128,112,
  3,8,8,5,0,0,128,64,0,192,64,64,64,96,3,8,
  8,5,0,0,32,64,0,192,64,64,64,96,3,8,8,5,
  0,0,64,160,0,192,64,64,64,96,3,7,7,5,0,0,
  160,0,192,64,64,64,96,4,7,7,5,0,0,96,16,112,
  144,144,144,96,4,8,8,5,0,0,80,160,0,224,144,144,
  144,144,4,8,8,5,0,0,64,32,0,96,144,144,144,96,
  4,8,8,5,0,0,32,64,0,96,144,144,144,96,4,8,
  8,5,0,0,96,144,0,96,144,144,144,96,4,8,8,5,
  0,0,80,160,0,96,144,144,144,96,4,7,7,5,0,0,
  80,0,96,144,144,144,96,4,5,5,5,0,0,96,0,240,
  0,96,5,6,6,5,0,0,8,112,176,208,144,96,4,8,
  8,5,0,0,64,32,0,144,144,144,144,112,4,8,8,5,
  0,0,32,64,0,144,144,144,144,112,4,8,8,5,0,0,
  96,144,0,144,144,144,144,112,4,8,8,5,0,0,80,0,
  0,144,144,144,144,112,4,10,10,5,0,254,32,64,0,144,
  144,144,144,112,144,96,4,8,8,5,0,254,128,128,224,144,
  144,224,128,128,4,9,9,5,0,254,80,0,144,144,144,144,
  112,144,96
};

int timezone_correct(int hour)
{
  if (hour > 4)
    return hour-4;
  return (23+hour)-4;
}

void draw(void)
{
  // u8g.setFont(u8g_font_unifont);
  u8g.setFont(creep2);

  // Draw border
  u8g.drawLine(0, 0, 128, 0); // top
  u8g.drawLine(0, 63, 128, 63); // bottom
  u8g.drawLine(0, 0, 0, 64); // left
  u8g.drawLine(127, 0, 127, 64); // right

  // Show speed
  u8g.drawStr(4, 16, speed_buffer);

  // Show top speed
  u8g.drawStr(4, 26, top_speed_buffer);

  // Show number of connected satellites
  u8g.drawStr(4, 40, sat_count_buffer);

  // Show time
  u8g.drawStr(4, 60, time_buffer);

  // Going too fast, display the weewoo
  if (mph > 85)
  {
    u8g.drawStr(70, 15, "!warning!");
    u8g.drawStr(63, 50, "!high speed!");
    u8g.drawXBMP(80, 20, weewoo_bmp_width, weewoo_bmp_height, weewoo_bmp);
  }
  // Otherwise fill the dead space with a nice pretty honda logo
  else
  {
    u8g.drawXBMP(66, 12, honda_width, honda_height, honda_bits);
  }
}

void update_info()
{
  // sprintf is slow as fuck
  // only update strings if the gps has pulled new data
  if (gps.speed.isUpdated())
  {
    mph = gps.speed.mph();
    dtostrf(mph, 3, 0, temp_speed_buffer);
    sprintf(speed_buffer, "%s mph", temp_speed_buffer);

    // Update top speed over 90 mph
    if (mph > top_speed && mph > 90)
    {
      top_speed = gps.speed.mph();
      sprintf(top_speed_buffer, "%s top", temp_speed_buffer);
    }
  }

  if (gps.time.isUpdated())
  {
    // All of this is hacky as fuck
    int h = timezone_correct(gps.time.hour());
    if (dst && h > 12) h++;
    sprintf(time_buffer, "%02d:%02d %s", h > 12 ? h-12 : h, gps.time.minute(), h > 12 ? "pm": "am");
  }

  if (gps.satellites.isUpdated())
  {
    Serial.print("New sat count: ");
    Serial.println(gps.satellites.value());
    sprintf(sat_count_buffer, "sats: %2d", gps.satellites.value());
  }
}

void setup(void)
{
  // Basic setup stuff
  Serial.begin(9600);
  gpsSerial.begin(9600);
  Serial.println("GPS Start");
  u8g.setColorIndex(1);

  // Reset pin for oled
  pinMode(8, OUTPUT);

}

void loop(void)
{
  // Render to oled
  u8g.firstPage();
  do {
    // Pull gps data
    while (gpsSerial.available()) {
      gps.encode(gpsSerial.read());
    }
    update_info();
    //
    draw();
  } while( u8g.nextPage() );
}

