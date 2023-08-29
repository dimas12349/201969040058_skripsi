#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <Firebase_ESP_Client.h> 
#include "addons/TokenHelper.h" 
#include "addons/RTDBHelper.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Servo.h>

#define ssid "Dimas1!"
#define password "57581234"
#define BOTtoken "6120189084:AAHxpcNzBTnSGX8Nr1JwPFR70BBjNqCGOnY"
#define CHAT_ID "5356698405"
#define akunfirebase "AIzaSyBu26gd5yVig3TC7cuxENKE2g95GcsMx6U" 
#define urlalamatfirebase "https://pakanhaviidz-default-rtdb.asia-southeast1.firebasedatabase.app/" 

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
FirebaseData fbdata; 
FirebaseAuth fbauth; 
FirebaseConfig fbconfig;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
Servo servo;

unsigned long lastTimeBotRan;
String dabes, db_oky, status, pesan, pesan_tombol, isi_pesan, nama_pengirim, id_bot, db_waktu, get_waktu_ntp, cek;
int i, numNewMessages, id_pesan, buka_pagar, tutup_pagar, start, end;

void setup() {
  wifi_firebase();
}

void loop() {
  notifikasi_pesan();
  waktu_ntp();
}

void wifi_firebase() {
  servo.attach(13);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }
  
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  timeClient.begin();
  timeClient.setTimeOffset(25200);
  
  fbconfig.api_key = akunfirebase; 
  fbconfig.database_url = urlalamatfirebase;

  while (!Firebase.ready()) { 
    if (Firebase.signUp(&fbconfig, &fbauth, "", "")) {
      fbconfig.token_status_callback = tokenStatusCallback; 
      Firebase.begin(&fbconfig, &fbauth); 
      Firebase.reconnectWiFi(true); 
      savetime_dt();
      pesan = "Firebase siap digunakan. \xF0\x9F\x86\x97"; 
      bot.sendMessage(CHAT_ID, pesan);
      pesan = "Silakan ketik atau tekan perintah berikut /start  guna mengontrol makan kucing Anda."; 
      bot.sendMessage(CHAT_ID, pesan);
    } else { 
      pesan = "Firebase belum siap \xE2\x9D\x8C"; 
      bot.sendMessage(CHAT_ID, pesan); 
      delay(500); 
    } 
  }
}

void savetime_dt() {
  Firebase.RTDB.getString(&fbdata, "waktu_jadwal/havid", "hallo");
  dabes = fbdata.stringData();
}

void nol() {
  buka_pagar = 0;
  tutup_pagar = 0;
  start = 0;
  end = 0;
  start = dabes.indexOf("|");
  end = dabes.indexOf("|", start + 1);
}

void notifikasi_pesan() {
  if (millis() - lastTimeBotRan >= 100)  {
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      handleNewMessages();
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    lastTimeBotRan = millis();
  }
}

void handleNewMessages() {
  for (i=0; i<numNewMessages; i++) {
    nol();
    isi_pesan = bot.messages[i].text; 
    nama_pengirim = bot.messages[i].from_name; 
    id_bot = String(bot.messages[i].chat_id); 
    id_pesan = bot.messages[i].message_id; //Perbarui Keyboard Sebaris
    isi_pesan.trim();
    isi_pesan.toUpperCase();
    db_waktu = "|" + isi_pesan + "|";

    if (isi_pesan == "MENU" || isi_pesan == "/START") {
      menu();
    } else if (isi_pesan == "OTOMATIS") {
      status = "OTOMATIS";
      pesan = "masukkan atau atur waktu\nseperti 12:00";
      bot.sendMessage(id_bot, pesan);
    } else if (status == "OTOMATIS") {
      otomatis();
    } else if (isi_pesan == "MANUAL") {
      manual();
    } else if (isi_pesan == "HAPUS") {
      hapus();
    } else if (isi_pesan.indexOf("HAPUS_WAKTU") != -1){
      hapus_waktu();
    } else if (isi_pesan == "LIST") {
      list();
    }
  }
}

void menu() {
  pesan = "Menu tersedia :";
  pesan_tombol = "[[{ \"text\":\"Beri pakan sekarang\",\"callback_data\":\"MANUAL\" }],";
  pesan_tombol += "[{ \"text\":\"Set Jadwal pemberian pakan\",\"callback_data\":\"OTOMATIS\" }],";
  pesan_tombol += "[{ \"text\":\"Hapus waktu pemberian pakan\",\"callback_data\":\"HAPUS\" }],";
  pesan_tombol += "[{ \"text\":\"Lihat Jadwal\",\"callback_data\":\"LIST\" }]]";

  if (isi_pesan == "MENU") {
    bot.sendMessageWithInlineKeyboard(id_bot, pesan, "", pesan_tombol, id_pesan);
  } else {
    bot.sendMessageWithInlineKeyboard(id_bot, pesan, "", pesan_tombol);
  }
}

void otomatis() {
  if (dabes.indexOf(db_waktu) != -1) {
    pesan = "waktu sudah terdaftar\n\n" + isi_pesan;
    pesan_tombol = "[[{ \"text\":\"Lihat Jadwal\",\"callback_data\":\"LIST\" }]]";
  } else {
    pesan = "sukses" + isi_pesan;
    pesan_tombol = "[[{ \"text\":\"kembali ke menu\",\"callback_data\":\"MENU\" }]]";
    Firebase.RTDB.setString(&fbdata, "waktu_jadwal/havid", dabes + db_waktu);
    savetime_dt();
  }
  
  bot.sendMessageWithInlineKeyboard(id_bot, pesan, "", pesan_tombol);
  status = "";
}

void manual() {
  servo.write(70);
  delay(500);
  servo.write(20);
  pesan = "Makan kucing siap di hidangkan ";
  pesan_tombol = "[[{ \"text\":\"kembali ke menu\",\"callback_data\":\"MENU\" }]]";
  bot.sendMessageWithInlineKeyboard(id_bot, pesan, "", pesan_tombol, id_pesan);
}

void hapus() {
  pesan = "pilih waktu yang dihapus";
  pesan_tombol = "[";

  while (start != -1 && end != -1) {
    db_oky = dabes.substring(start, end + 1);
    buka_pagar = db_oky.indexOf("|");
    tutup_pagar = db_oky.indexOf("|", buka_pagar + 1);

    pesan_tombol += "[{ \"text\":\"" + db_oky.substring(buka_pagar + 1, tutup_pagar) + "\",\"callback_data\":\"HAPUS_WAKTU" + db_oky.substring(buka_pagar + 1, tutup_pagar) + "\" }],";

    start = dabes.indexOf("|", end + 1);
    end = dabes.indexOf("|", end + 2);
  }

  pesan_tombol += "[{ \"text\":\"Lihat Jadwal\",\"callback_data\":\"MENU\" }]]";
  bot.sendMessageWithInlineKeyboard(id_bot, pesan, "", pesan_tombol, id_pesan);
}

void hapus_waktu() {
  isi_pesan.replace("HAPUS_WAKTU", "");
  
  while (start != -1 && end != -1) {
    db_oky = dabes.substring(start, end + 1);
    
    if (db_oky.indexOf(isi_pesan) != -1) {
      pesan = "terhapus " + isi_pesan;
      pesan_tombol = "[[{ \"text\":\"kembali ke menu\",\"callback_data\":\"MENU\" }]]";
      bot.sendMessageWithInlineKeyboard(id_bot, pesan, "", pesan_tombol, id_pesan);
      dabes.replace(db_oky, "");

      if (dabes.indexOf("|") == -1){
        Firebase.RTDB.deleteNode(&fbdata, "waktu_jadwal/havid");
      } else {
        Firebase.RTDB.setString(&fbdata, "waktu_jadwal/havid", dabes);
      }

      savetime_dt();
    }
	
    start = dabes.indexOf("|", end + 1);
    end = dabes.indexOf("|", end + 2);
  }
}

void list() {
  while (start != -1 && end != -1) {
    db_oky = dabes.substring(start, end + 1);
    buka_pagar = db_oky.indexOf("|");
    tutup_pagar = db_oky.indexOf("|", buka_pagar + 1);

    status += db_oky.substring(buka_pagar + 1, tutup_pagar) + "\n";
    start = dabes.indexOf("|", end + 1);
    end = dabes.indexOf("|", end + 2);
  }

  pesan = "berikut daftar waktu jadwal yang tersimpan:\n\n" + status;
  pesan_tombol = "[[{ \"text\":\"kembali ke menu\",\"callback_data\":\"MENU\" }]]";
  bot.sendMessageWithInlineKeyboard(id_bot, pesan, "", pesan_tombol, id_pesan);
  status = "";
}

void waktu_ntp() {
  nol();
  timeClient.forceUpdate();
  get_waktu_ntp = timeClient.getFormattedDate();
  buka_pagar = get_waktu_ntp.indexOf("T");
  tutup_pagar = get_waktu_ntp.indexOf(":", buka_pagar + 5);
  cek = get_waktu_ntp.substring(buka_pagar + 1, tutup_pagar);
  
  if (dabes.indexOf("|" + cek + "|") != -1) {
    servo.write(70);
    delay(500);
    servo.write(20);
    delay(59500);
    pesan = "Makan kucing terjadwal siap di hidangkan ";
    bot.sendMessage(CHAT_ID, pesan);
  }
}