#include "Arduino.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"
#include "AudioFileSourceSD.h"
#include "SD.h"
#include "SPI.h"


// Defina os pinos do SD card no ESP32
#define SD_CS 5
#define SPI_MOSI 23
#define SPI_MISO 19
#define SPI_SCK 18

// Defina os pinos I2S no ESP32
#define I2S_DOUT 25  // Pino de dados (DATA)
#define I2S_BCLK 27  // Pino de clock (BCK)
#define I2S_LRC 26   // Pino de seleção de canal (WS/LRC)

AudioGeneratorMP3 *mp3;
AudioFileSourceSD *file;
AudioOutputI2S *out;

void setup() {
  Serial.begin(115200);

  //Configura e inicia o SPI para conexão com o cartão SD
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  SPI.setFrequency(1000000);
  SD.begin(SD_CS);

  if (!SD.begin(SD_CS)) {
    Serial.println("Falha ao montar o cartão SD");
    return;
  }

  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("Nenhum cartão SD detectado");
    return;
  }

  Serial.print("Tipo de cartão SD: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("Desconhecido");
  }

  Serial.println("Listando arquivos no cartão SD:");
  File root = SD.open("/");
  listarArquivos(root, 0);


  // Cria os objetos de áudio
  file = new AudioFileSourceSD("/musica.mp3");  // Caminho do arquivo MP3 no SD
  out = new AudioOutputI2S();
  out->SetPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);  // Configura os pinos BCK, WS, DATA
  out->SetGain(1.0); // Volume máximo
  mp3 = new AudioGeneratorMP3();

  // Inicia a reprodução
  mp3->begin(file, out);
}

void loop() {
  if (mp3->isRunning()) {
    if (!mp3->loop()) {
      mp3->stop();
      Serial.println("Reprodução concluída.");
    }
  } else {
    Serial.println("Reprodução não iniciada.");
    delay(1000);
  }
}


void listarArquivos(File dir, int nivel) {
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) {
      break;  // Sai do loop se não houver mais arquivos
    }
    for (int i = 0; i < nivel; i++) {
      Serial.print("  ");
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      listarArquivos(entry, nivel + 1);  // Lista recursivamente diretórios
    } else {
      Serial.print("  -  ");
      Serial.print(entry.size());
      Serial.println(" bytes");
    }
    entry.close();
  }
}