#include "Arduino.h"
#include "LittleFS.h"
#include "GyverOLED.h"
#include "WiFi.h"
#include "WebServer.h"
#include "vector"
#include "icons_8x8.h"
#include <ESPAsyncWebServer.h>
#include "input.hpp"
#include "dino.hpp"
#include "flappyBird.hpp"
#include "main.hpp"
#include "display.hpp"

// <-----------Настройки----------->

#define SSID "esp32 tochka dostypa" // Название точки доступа
#define PASSWORD "av2mkbhh" // Пароль точки доступа
#define SCREEN_WIDTH 128 // Ширина экрана
#define SCREEN_HEIGHT 64 // Высота экрана
#define SDA_PIN 7 // Пин для SDA экрана
#define SCL_PIN 6 // Пин для SCL экрана

// <-----------Настройки отображения заряда батареи-------->
#define BAT_100_VOLTAGE 3.8 // Мин. напряжение для показа 100%
#define BAT_75_VOLTAGE 3.65 // Мин. напряжение для показа 75%
#define BAT_50_VOLTAGE 3.5 // Мин. напряжение для показа 50%
#define BAT_25_VOLTAGE 3.3 // Мин. напряжение для показа 25%


// <----------Веб морда---------->
const char* index_html PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">

<title>File Manager</title>

<style>
body {
  background: #0f172a;
  color: #e2e8f0;
  font-family: Arial;
  margin: 0;
  padding: 12px;
}

h1 {
  font-size: 20px;
  color: #38bdf8;
}

.card {
  background: #1e293b;
  padding: 10px;
  margin: 10px 0;
  border-radius: 10px;
}

input, textarea {
  width: 100%;
  margin: 5px 0;
  padding: 8px;
  border-radius: 6px;
  border: none;
}

button {
  width: 100%;
  margin-top: 5px;
  padding: 10px;
  border: none;
  border-radius: 8px;
  background: #38bdf8;
  color: black;
  font-weight: bold;
}

.file {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 8px;
  background: #0f172a;
  margin-top: 5px;
  border-radius: 8px;
}

.smallbtn {
  padding: 6px;
  font-size: 12px;
  margin-left: 4px;
  border-radius: 6px;
}

.danger { background: #ef4444; color: white; }
.rename { background: #f59e0b; }
.download { background: #22c55e; }
</style>
</head>

<body>

<h1>📁 Mobile File Manager</h1>

<div class="card">
  <h3>📤 Upload file</h3>
  <input type="file" id="fileInput">
  <input type="text" id="fileName" placeholder="File name (optional)">
  <button onclick="uploadFile()">Upload</button>
</div>

<div class="card">
  <h3>📝 Create text file</h3>
  <input type="text" id="textName" placeholder="File name">
  <textarea id="textContent" rows="6" placeholder="Text..."></textarea>

  <button onclick="uploadText()">Upload</button>
</div>

<div class="card">
  <h3>📂 Files</h3>
  <div id="files"></div>
  <button onclick="Format()">Full formatting</button>
</div>

<script>

async function loadFiles() {
  const res = await fetch('/list');
  const files = await res.json();

  const container = document.getElementById('files');
  container.innerHTML = "";

  files.forEach(f => {

    const div = document.createElement('div');
    div.className = "file";

    const isIndex = f.name === "/index.html";

    div.innerHTML = `
      <span>${f.isDir ? "📂" : "📄"} ${f.name}</span>
      <div>

        <button class="smallbtn download"
          onclick="downloadFile('${f.name}')">⬇</button>

        <button class="smallbtn rename"
          ${isIndex ? "disabled" : ""}
          onclick="renameFile('${f.name}')">✏</button>

        <button class="smallbtn danger"
          ${isIndex ? "disabled" : ""}
          onclick="deleteFile('${f.name}', ${f.isDir}')">✖️</button>

      </div>
    `;

    container.appendChild(div);
  });
}

function deleteFile(name, isDir) {
  if (isDir) {
    fetch(`/deleteDir?name=${encodeURIComponent(name)}`)
    .then(loadFiles);
  }
  else 
  {
    fetch(`/delete?name=${encodeURIComponent(name)}`)
      .then(loadFiles);
  }
}

function renameFile(name) {
  const newName = prompt("New name:");
  if (!newName) return;

  fetch(`/rename?old=${encodeURIComponent(name)}&new=${encodeURIComponent(newName)}`)
    .then(loadFiles);
}

function downloadFile(name) {
  window.location = `/download?name=${encodeURIComponent(name)}`;
}

function uploadFile() {
  const file = document.getElementById("fileInput").files[0];
  if (!file) return;

  const customName = document.getElementById("fileName").value;
  const form = new FormData();

  form.append("file", file);
  if (customName) form.append("file", customName);

  fetch("/upload", {
    method: "POST",
    body: form
  }).then(loadFiles);
}

function uploadText() {
  const name = document.getElementById("textName").value;
  const text = document.getElementById("textContent").value;

  if (!name || !text) return;

  fetch("/uploadText", {
    method: "POST",
    headers: {"Content-Type":"application/x-www-form-urlencoded"},
    body: `name=${encodeURIComponent(name)}&text=${encodeURIComponent(text)}`
  }).then(loadFiles);
}

async function Format() {
  fetch("/formatting", {
    method: "POST",
    headers: {"Content-Type":"application/x-www-form-urlencoded"},
    body: ``
  }).then(loadFiles);
}

function wrap(tagOpen, tagClose) {
  const ta = document.getElementById("textContent");
  const start = ta.selectionStart;
  const end = ta.selectionEnd;
  const text = ta.value;

  ta.value =
    text.substring(0, start) +
    tagOpen +
    text.substring(start, end) +
    tagClose +
    text.substring(end);
}

function formatBold(){ wrap("<b>", "</b>"); }
function formatItalic(){ wrap("<i>", "</i>"); }
function formatCode(){ wrap("<code>", "</code>"); }

loadFiles();

</script>

</body>
</html>
)rawliteral";

Location UserLocation = Location::FileSelecter;

GyverOLED<SSD1306_128x64> display(0x3C);

std::vector<String> files;

const char* rootPath = "/";

void refreshFS()
{
  files.clear();

  File root = LittleFS.open(rootPath);
  File cur = root.openNextFile();

  while (cur)
  {
    files.push_back(String(cur.name()));
    cur = root.openNextFile();
  }
}

void init_fs(void)
{
  if (!LittleFS.begin(true))
  {
    display.println("File system error!");
    display.update();
    pinMode(LED_BUILTIN, OUTPUT);
    for (int i = 0; i < 2; i++) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(1000);
      digitalWrite(LED_BUILTIN, LOW);
      delay(1000);
    }
    for (;;);
  }
  refreshFS();
}

AsyncWebServer server(80);

bool needDisplayReload = false;

void ensureDir(String path) {
  int idx = path.lastIndexOf('/');
  if (idx <= 0) return;

  String dir = path.substring(0, idx);
  LittleFS.mkdir(dir);
}

void setupWeb()
{
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", index_html);
  });

  server.on("/list", HTTP_GET, [](AsyncWebServerRequest *request) {

  String json = "[";
  File root = LittleFS.open("/");
  File f = root.openNextFile();

  bool first = true;

  while (f) {
    if (strcmp(f.name(), "dino_game_score.sys") != 0)
    {
      if (!first) json += ",";
      else first = false;

      json += "{\"name\":\"";
      json += f.name();
      json += "\"";
      json += ",\"isDir\":";
      json += f.isDirectory() ? "true" : "false";
      json += "}";
    }

    f = root.openNextFile();
  }

  json += "]";
  request->send(200, "application/json", json);
  });


    server.on("/delete", HTTP_GET, [](AsyncWebServerRequest *request) {

    String name = "/" + request->getParam("name")->value();

    if (name == "/index.html") {
      request->send(403, "text/plain", "protected");
      return;
    }

    LittleFS.remove(name);
    needDisplayReload = true;
    request->send(200);
  });

  server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->hasParam("name")) {
      request->send(400);
      return;
    }

    String name = "/" + request->getParam("name")->value();

    request->send(LittleFS, name, "application/octet-stream");
  });

  server.on("/formatting", HTTP_POST, [](AsyncWebServerRequest *request) {
    LittleFS.format();
    needDisplayReload = true;
    request->send(200, "text/plain", "ok");
  });

  server.on("/rename", HTTP_GET, [](AsyncWebServerRequest *request) {

  String oldN = request->getParam("old")->value();
  String newN = request->getParam("new")->value();

  if (!oldN.startsWith("/")) oldN = "/" + oldN;
  if (!newN.startsWith("/")) newN = "/" + newN;

  ensureDir(newN);

  if (oldN == "/index.html") {
    request->send(403, "protected");
    return;
  }

  File f = LittleFS.open(oldN, "r");
  File nf = LittleFS.open(newN, "w");

  while (f.available()) nf.write(f.read());

  f.close();
  nf.close();

  LittleFS.remove(oldN);
  needDisplayReload = true;

  request->send(200);
});

  server.on(
    "/upload",
    HTTP_POST,
    [](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", "uploaded");
    },
    [](AsyncWebServerRequest *request, String filename, size_t index,
       uint8_t *data, size_t len, bool final) {

      String name = filename;

      if (request->hasParam("name", true)) name = request->getParam("name",true)->value();

      if (name == F("/dino_game_score.sys"))
      {
        request->send(403, "protected");
        return;
      }

      if (!name.startsWith("/")) name = "/" + name;

      if (!index) {
        request->_tempFile = LittleFS.open("/" + filename, "w");
      }

      if (len) {
        request->_tempFile.write(data, len);
      }

      if (final) {
        request->_tempFile.close();
        needDisplayReload = true;
      }
    }
  );

  server.on("/uploadText", HTTP_POST, [](AsyncWebServerRequest *request) {

  String name = request->getParam("name", true)->value();
  String text = request->getParam("text", true)->value();

  if (!name.startsWith("/")) name = "/" + name;
  ensureDir(name);

  if (name == F("/index.html") || name == F("/dino_game_score.sys")) {
    request->send(403, "protected");
    return;
  }

  File f = LittleFS.open(name, "w");
  f.print(text);
  f.close();

  request->send(200);
  needDisplayReload = true;
});

  server.begin();
}

void setup() {
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetPinAttenuation(BAT_PIN, ADC_11db);
  display.init();
  display.clear();
  display.home();
  display.update();
  init_fs();
  WiFi.mode(WIFI_AP);
  WiFi.softAP(SSID, PASSWORD);
  setupWeb();
  server.begin();

  dino_init(&display);
}

void openMenu(void)
{
  UserLocation = Location::Server;
  display.clear();
  display.home();
  display.println(F("Точка доступа:"));
  display.println(F("Name: "));
  display.println(SSID);
  display.println(F("Pass: "));
  display.println(PASSWORD);
  display.println(F("IP: "));
  display.println(WiFi.softAPIP());
  display.update();
}

Input lastInput(ButtonState::None, ButtonState::None, ButtonState::None);

bool isInputChanged(Input input)
{
  if (
      input.Left != lastInput.Left ||
      input.Right != lastInput.Right ||
      input.Center != lastInput.Center
     )
      return true;
    return false;
}

String FileText;
bool nowOpen = false;

struct FileEntry {
  String name;
  bool isDir;
};

std::vector<FileEntry> entries;

String currentPath = "/";
String currentFileName = "";
int _index = 0;
int scrollOffset = 0;

#define MAX_VISIBLE 7   

void loadDirectory(String path) {
  entries.clear();

  File root = LittleFS.open(path);
  File file = root.openNextFile();

  if (path != F("/")) {
    entries.push_back({"..", true});
  }

  while (file) {
    if (strcmp(file.name(), "dino_game_score.sys") != 0)
    {
      FileEntry e;
      e.name = String(file.name());
      e.isDir = file.isDirectory();
      entries.push_back(e);
    }
    file = root.openNextFile();
  }

  root.close();
}


void renderSelector() {
  display.clear();

  processBat();

  
  display.setCursor(0, 0);
  display.print(currentPath);

  for (int i = 0; i < MAX_VISIBLE; i++) {
    int entryIndex = scrollOffset + i;
    if (entryIndex >= entries.size()) break;

    int y = i + 1;

    
    display.drawBitmap(
      7, y * 8,
      entries[entryIndex].isDir ? icons_8x8[70] : icons_8x8[40],
      8, 8
    );

    
    display.setCursor(17, y);
    display.print(entries[entryIndex].name);
  }

  
  display.setCursor(0, (_index - scrollOffset) + 1);
  display.print('>');

  display.update();
}


void processSelector(Input input)
{
  static bool first = true;

  if (!first && !needDisplayReload && !isInputChanged(input)) return;

  if (first || needDisplayReload) {
    loadDirectory(currentPath);
    needDisplayReload = false;
    first = false;
  }

  
  if (input.Right == ButtonState::Down || input.Right == ButtonState::Hold) _index++;
  if (input.Left == ButtonState::Down || input.Right == ButtonState::Hold) _index--;

  int size = (int)entries.size();
  _index = constrain(_index, 0, size == 0? 0 : size - 1);

  
  if (_index >= scrollOffset + MAX_VISIBLE) {
    scrollOffset = _index - MAX_VISIBLE + 1;
  }
  if (_index < scrollOffset) {
    scrollOffset = _index;
  }

  
  if (input.Center == ButtonState::Down) {
    if (entries.empty()) return;
    FileEntry e = entries[_index];

    if (e.isDir) {
      if (e.name == "..") {
        
        int lastSlash = currentPath.lastIndexOf('/', currentPath.length() - 2);
        currentPath = currentPath.substring(0, lastSlash + 1);
        if (currentPath == "") currentPath = "/";
      } else currentPath += e.name + "/";

      loadDirectory(currentPath);
      return;
    } else {
      
      currentFileName = currentPath + e.name;
      if (currentFileName == F("/dino_game_score.sys")) return;
      File f = LittleFS.open(currentFileName);
      FileText = f.readString();
      f.close();

      UserLocation = Location::File;
      nowOpen = true;
      return;
    }
  }

  renderSelector();
}

// Flappy bird в разработке!

static const unsigned char GamesCount = 1;

static const char* Games[GamesCount] PROGMEM = {
  "Dinosaur",
  //"Flappy bird",
};

static void (*GamesFn[GamesCount])(void) {
  dinosaurGame,
  //flappyBirdGame,
};

bool gameSelectorJustOpened = false;

void processGameSelector(Input input) {
  static unsigned char gameIndex = 0;
  
  if (input.Left == ButtonState::Down || input.Left == ButtonState::Hold) gameIndex--;
  else if (input.Right == ButtonState::Down || input.Right == ButtonState::Hold) gameIndex++;

  if (!gameSelectorJustOpened)
  {
    if (!isInputChanged(input)) return;
  }
  else gameSelectorJustOpened = false;

  gameIndex = constrain(gameIndex, 0, GamesCount - 1);

  display.clear();

  if (input.Center == ButtonState::Down) {
    delay(100); 
    if (GamesFn[gameIndex] == NULL)
    {
      display.setCursor(6, gameIndex + 1);
      display.print("Invalid game function!");
      display.update(6, (gameIndex + 1) * 8, 127, (gameIndex + 2) * 8);
      delay(2000); 
    }
    else GamesFn[gameIndex]();
    return;
  }
  else {
    for (int i = 0; i < GamesCount; i++)
    {
      display.setCursor(6, i + 1);
      display.print(Games[i]);
    }

    display.setCursor(0, gameIndex + 1);
    display.print('>');


    display.update();
  }
}

void processServer(Input input)
{
  static uint16_t contrast = 255;
  static const unsigned char max_Step = 20;
  static constexpr unsigned char step = 255 / max_Step;
  static constexpr unsigned char min = 255 - step * (max_Step - 1);

  if (input.Left == ButtonState::Down || input.Left == ButtonState::Hold)
  {
    contrast -= step;
    if (contrast < min) contrast = min;
    display.setContrast(contrast);
  }

  if (input.Right == ButtonState::Down || input.Right == ButtonState::Hold)
  {
    contrast += step;
    if (contrast > 255) contrast = 255;
    display.setContrast(contrast);
  }

  if (input.Center == ButtonState::Hold)
  {
    UserLocation = Location::Game;
    gameSelectorJustOpened = true;
    return;
  }

  if (input.Center == ButtonState::Down)
  {
    UserLocation = Location::FileSelecter;
    return;
  }
}

#define CHAR_WIDTH 6
#define CHAR_HEIGHT 8

#define MAX_LINES (SCREEN_HEIGHT / CHAR_HEIGHT)

#define SCROLL_STEP 7

std::vector<String> lines;
int fileScrollOffset = 0;
bool parsed = false;

void splitText(const String& text) {
    lines.clear();

    int maxCharsPerLine = SCREEN_WIDTH / 6 - 1; 
    String word = "";
    String line = "";

    for (int i = 0; i < text.length(); i++) {
        char c = text[i];

        if (c == ' ' || c == '\n') {

            if (line.length() + word.length() > maxCharsPerLine) {
                lines.push_back(line);
                line = "";
            }

            if (line.length() > 0) line += " ";
            line += word;
            word = "";

            if (c == '\n') {
                lines.push_back(line);
                line = "";
            }

        } else {
            word += c;
        }
    }

    
    if (word.length()) {
        if (line.length() + word.length() > maxCharsPerLine) {
            lines.push_back(line);
            line = word;
        } else {
            if (line.length() > 0) line += " ";
            line += word;
        }
    }

    if (line.length()) lines.push_back(line);

    parsed = true;
}

void renderText() {
    display.clear();

    display.textMode(BUF_SUBTRACT);
    display.rect(0, 0, 128, 6);
    display.setCursor(0, 0);
    display.print(currentFileName);
    display.textMode(BUF_REPLACE);

    for (int i = 0; i < MAX_LINES; i++) {
        int lineIndex = fileScrollOffset + i;
        if (lineIndex >= lines.size()) break;

        display.setCursor(0, i + 1);
        display.print(lines[lineIndex]);
    }

    display.update();
}

void processFile(Input input)
{
    if (nowOpen) {
        if (!isInputChanged(input)) return;
    } else nowOpen = false;

    if (!parsed) {
        splitText(FileText);
    }

    if (input.Center == ButtonState::Hold) {
        UserLocation = Location::FileSelecter;
        parsed = false;
        fileScrollOffset = 0;
        return;
    }

    if (input.Right == ButtonState::Down) {
        if (fileScrollOffset < (int)lines.size() - MAX_LINES) {
            fileScrollOffset += SCROLL_STEP;
        }
    }

    if (input.Left == ButtonState::Down) {
        if (fileScrollOffset > 0) {
            fileScrollOffset -= SCROLL_STEP;
        }
    }

    renderText();
}

void processBat()
{
  int raw = analogRead(BAT_PIN);
  float voltage = raw / 4095.0 * 3.3 * 2;
  const uint8_t* icon;
  if (voltage >= BAT_100_VOLTAGE) icon = icons_8x8[9];
  else if (voltage >= BAT_75_VOLTAGE) icon = icons_8x8[10];
  else if (voltage >= BAT_50_VOLTAGE) icon = icons_8x8[11];
  else if (voltage >= BAT_25_VOLTAGE) icon = icons_8x8[12];
  else icon = icons_8x8[13];

  display.drawBitmap(115, 0, icon, 8, 8);
}

void loop() {
  Input input = GetInput();

  if (input.Left == ButtonState::Hold && input.Right == ButtonState::Hold && UserLocation == Location::FileSelecter)
  {
    openMenu();
    return;
  }

  if (UserLocation == Location::Server) processServer(input);
  else if (UserLocation == Location::FileSelecter) processSelector(input);
  else if (UserLocation == Location::File) processFile(input);
  else if (UserLocation == Location::Game) processGameSelector(input);


  lastInput = input;
  delay(16); 
}