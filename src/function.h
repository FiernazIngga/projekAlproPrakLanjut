#include "crow.h"
#include <windows.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <string>
#include <thread>
#include <asio.hpp>
#include <chrono>
#include <iomanip>
#include <ctime>
using namespace std;
using namespace crow;

const int maxLahanParkir = 30;
int jumlahKendaraanTerparkir = 0;

tm* waktuLocal(){
    time_t waktu_sekarang;
    time(&waktu_sekarang);
    tm* waktu_lokal = localtime(&waktu_sekarang);
    return waktu_lokal;
}

struct Parkir
{
    int harga, denda;
    int waktuMasuk[6];
    string waktuKeluar;

    void eksekusiWaktuMasuk(){
        time_t waktu_sekarang;
        time(&waktu_sekarang);
        tm* waktu_lokal = localtime(&waktu_sekarang);
            this->waktuMasuk[0] = waktu_lokal->tm_sec;
            this->waktuMasuk[1] = waktu_lokal->tm_min;
            this->waktuMasuk[2] = waktu_lokal->tm_hour;
            this->waktuMasuk[3] = waktu_lokal->tm_mday;
            this->waktuMasuk[4] = (waktu_lokal->tm_mon)+1;
            this->waktuMasuk[5] = (waktu_lokal->tm_year)+1900;
    }
    void lihatWaktu(){
        string bulan[12] = {
            "Januari", 
            "Februari", 
            "Maret", 
            "April", 
            "Mei", 
            "Juni", 
            "Juli", 
            "Agustus", 
            "September", 
            "Oktober", 
            "November", 
            "Desember"
        };
        cout << this->waktuMasuk[2] << ":" << this->waktuMasuk[1]
        << ":" << this->waktuMasuk[0]  << " " << this->waktuMasuk[3] 
        << "-" << bulan[(this->waktuMasuk[4]) - 1]  << "-" << this->waktuMasuk[5] << endl;
    }
    string waktu(){
        string bulan[12] = {
            "Januari", 
            "Februari", 
            "Maret", 
            "April", 
            "Mei", 
            "Juni", 
            "Juli", 
            "Agustus", 
            "September", 
            "Oktober", 
            "November", 
            "Desember"
        };
        ostringstream oss;
        oss << waktuMasuk[2] << ":" << waktuMasuk[1] << ":" << waktuMasuk[0]  << " "
            << waktuMasuk[3] << "-" << bulan[waktuMasuk[4] - 1] << "-" << waktuMasuk[5];
        return oss.str();
    }
};

struct DataKendaraan
{
    string id, type, plat;
    Parkir kendaraanSpesifik;
} kendaraan[maxLahanParkir];

const int maksPengumuman = 100;
int jmlPengumuman = 0;
int idPengumuman = 100;

struct Pengumuman
{
    int id;
    string judul, deskripsi, lokasi, kontak[2], prioritas;
    int waktuPosting[6];
    bool publish;
    string waktuPublish;
    void eksekusiWaktuPubllish(){
        time_t waktu_sekarang;
        time(&waktu_sekarang);
        tm* waktu_lokal = localtime(&waktu_sekarang);
            this->waktuPosting[0] = waktu_lokal->tm_sec;
            this->waktuPosting[1] = waktu_lokal->tm_min;
            this->waktuPosting[2] = waktu_lokal->tm_hour;
            this->waktuPosting[3] = waktu_lokal->tm_mday;
            this->waktuPosting[4] = (waktu_lokal->tm_mon)+1;
            this->waktuPosting[5] = (waktu_lokal->tm_year)+1900;
    }
    string waktu(){
        string bulan[12] = {
            "Januari", 
            "Februari", 
            "Maret", 
            "April", 
            "Mei", 
            "Juni", 
            "Juli", 
            "Agustus", 
            "September", 
            "Oktober", 
            "November", 
            "Desember"
        };
        ostringstream oss;
        oss << waktuPosting[2] << ":" << waktuPosting[1] << ":" << waktuPosting[0]  << " "
            << waktuPosting[3] << "-" << bulan[waktuPosting[4] - 1] << "-" << waktuPosting[5];
        return oss.str();
    }
} pengumuman[maksPengumuman];

// Cek File
string bacaFile(const string& filepath) {
    ifstream file(filepath);
    if (!file.is_open()) {
        return "Gagal membaca file: " + filepath;
    }
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

string getExecutablePath() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    string path(buffer);
    return path.substr(0, path.find_last_of("\\/"));
}

bool fileExists(const string& path) {
    return filesystem::exists(path);
}
// Cek File Selesai

int cariKendaraan(string platDiCari){
    int i = 0;

    kendaraan[jumlahKendaraanTerparkir + 1].plat = platDiCari;
    while (kendaraan[i].plat != platDiCari)
    {
        i++;
    }
    if (i < jumlahKendaraanTerparkir){ 
        return i;
    } else {
        return 1 + i;
    }
}

response returnCekKendaraan(const request& req){
    auto data = json::load(req.body);
    if (!data) {
        json::wvalue res;
        res["message"] = "Inputan data kendaraan kosong";
        return response{res};
    }
    
    string plat = "";
    plat = data["platKendaraan"].s();
    
    if (jumlahKendaraanTerparkir == 0)
    {
        json::wvalue res;
        res["message"] = "Kosong belum ada kendaraan terpakir";
        return response{res};
    } else {
        int i = cariKendaraan(plat);
        if (i >= jumlahKendaraanTerparkir)
        {
            json::wvalue res;
            res["message"] = "Kendaraan tidak ditemukan";
            return response{res};
        } else {
            json::wvalue item;  
            item["tipe"] = kendaraan[i].type;
            item["plat"] = kendaraan[i].plat;
            item["biaya"] = kendaraan[i].kendaraanSpesifik.harga;
            item["denda"] = kendaraan[i].kendaraanSpesifik.denda;
            item["waktuMsk"] = kendaraan[i].kendaraanSpesifik.waktu();
            item["waktuKlr"] = kendaraan[i].kendaraanSpesifik.waktuKeluar;
            
            json::wvalue res;
            res["data"] = move(item);
            return response{res};
        }
    }
}

bool cekLahanKendaraan(string tipeKendaraan) {
    int hitungKendaraan = 0;
    int kapasitas = 0;
    if (tipeKendaraan == "Kendaraan Roda Dua")
    {
        kapasitas = 20;
    } else {
        kapasitas = 10;
    }
    for (int i = 0; i < jumlahKendaraanTerparkir; i++) {
        if (kendaraan[i].type == tipeKendaraan) {
            hitungKendaraan++;
        }
    }
    return hitungKendaraan < kapasitas; 
}

void eksekusiKendaraan(string id, string tipeKendaraan, string plat, int harga){
    kendaraan[jumlahKendaraanTerparkir].id = id;
    kendaraan[jumlahKendaraanTerparkir].type = tipeKendaraan;
    kendaraan[jumlahKendaraanTerparkir].plat = plat;
    kendaraan[jumlahKendaraanTerparkir].kendaraanSpesifik.harga = harga;
    kendaraan[jumlahKendaraanTerparkir].kendaraanSpesifik.denda = 0;
    kendaraan[jumlahKendaraanTerparkir].kendaraanSpesifik.eksekusiWaktuMasuk();
    kendaraan[jumlahKendaraanTerparkir].kendaraanSpesifik.waktuKeluar = "Kendaraan Masih Didalam";
    jumlahKendaraanTerparkir++;
}

int cekStatusKendaraan(const string *plat){
    kendaraan[jumlahKendaraanTerparkir+1].plat = *plat;
    int i = 0;
    while (kendaraan[i].plat != *plat)
    {
        i++;
    }
    if (i < jumlahKendaraanTerparkir)
    {
        return i;
    }
    return i+1;
}

response kendaraanMasuk(const request& req) {
    auto data = json::load(req.body);
    if (!data) return response(400, "Invalid JSON");

    string id = data["id"].s();
    string tipe = data["kendaraan"].s();
    string plat = data["platKendaraan"].s();
    json::wvalue res;

    int tarif = 0;
    if (tipe == "Kendaraan Roda Dua") {
        tarif = 3000;
    } else if (tipe == "Kendaraan Roda Empat") {
        tarif = 6000;
    } else {
        res["message"] = "Tipe kendaraan tidak dikenali.";
        return response{res};
    }

    if (!cekLahanKendaraan(tipe)) {
        res["message"] = "Mohon maaf kendaraan tipe " + tipe + kendaraan[cekLahanKendaraan(tipe)].kendaraanSpesifik.waktuKeluar;
        return response{res};
    }

    int index = cekStatusKendaraan(&plat);
    if (index < jumlahKendaraanTerparkir) {
        res["cekBol"] = false;
        res["message"] = kendaraan[index].kendaraanSpesifik.waktuKeluar;
        res["tipe"] = tipe;
        res["plat"] = plat;
        return response{res};
    }

    eksekusiKendaraan(id, tipe, plat, tarif);
    res["cekBol"] = true;
    res["message"] = "berhasil";
    res["tipe"] = tipe;
    res["plat"] = plat;
    return response{res};
}


void kumpulkanData(int kendaraanTerpakir, json::wvalue::list *result){
    if (kendaraanTerpakir >= jumlahKendaraanTerparkir) {
        return;
    } else {
        json::wvalue item;
        item["id_kendaraan"] = kendaraan[kendaraanTerpakir].id;
        item["tipe_kendaraan"] = kendaraan[kendaraanTerpakir].type;
        item["plat_kendaraan"] = kendaraan[kendaraanTerpakir].plat;
        item["biaya_parkir"] = kendaraan[kendaraanTerpakir].kendaraanSpesifik.harga;
        item["denda_parkir"] = kendaraan[kendaraanTerpakir].kendaraanSpesifik.denda;
        item["waktu_masuk"] = kendaraan[kendaraanTerpakir].kendaraanSpesifik.waktu();
        item["waktu_keluar"] = kendaraan[kendaraanTerpakir].kendaraanSpesifik.waktuKeluar;
        result->emplace_back(move(item));
        kumpulkanData(kendaraanTerpakir + 1, result);
    }
} 

response kumpulanData(){
    json::wvalue::list result;
    if (jumlahKendaraanTerparkir == 0)
    { 
        json::wvalue res;
        res["cekBol"] = true;
        res["message"] = "Belum ada kendaraan didalam";
        return response{res};
    } else {
        kumpulkanData(0,&result);
        json::wvalue res;
        res["data"] = move(result);
        return response{res};
    }
}

response kumpulkanDataAsc(){
    if (jumlahKendaraanTerparkir == 0)
    {
        json::wvalue res;
        res["cekBol"] = true;
        res["message"] = "Belum ada kendaraan terpakir";
        return response{res};
    } else {
        for (int i = 1; i < jumlahKendaraanTerparkir; i++)
        {
            DataKendaraan key = kendaraan[i];
            int j = i - 1;
            while (j >= 0 && kendaraan[j].plat < key.plat)
            {
                kendaraan[j + 1] = kendaraan[j];
                j--;
            }
            kendaraan[j + 1] = key;
        }
        json::wvalue::list hasil;
        for (int i = 0; i < jumlahKendaraanTerparkir; i++)
        {
            json::wvalue item;
            item["id_kendaraan"] = kendaraan[i].id;
            item["tipe_kendaraan"] = kendaraan[i].type;
            item["plat_kendaraan"] = kendaraan[i].plat;
            item["biaya_parkir"] = kendaraan[i].kendaraanSpesifik.harga;
            item["denda_parkir"] = kendaraan[i].kendaraanSpesifik.denda;
            item["waktu_masuk"] = kendaraan[i].kendaraanSpesifik.waktu();
            item["waktu_keluar"] = kendaraan[i].kendaraanSpesifik.waktuKeluar;
            hasil.emplace_back(move(item));
        }
        json::wvalue res;
        res["cekBol"] = false;
        res["hasil"] = move(hasil);
        return response{res};
    }
}

response kumpulkanDataDesc(){
    if (jumlahKendaraanTerparkir == 0)
    {
        json::wvalue res;
        res["cekBol"] = true;
        res["message"] = "Belum ada kendaraan terpakir";
        return response{res};
    } else {
        for (int i = 1; i < jumlahKendaraanTerparkir; i++)
        {
            DataKendaraan key = kendaraan[i];
            int j = i - 1;
            while (j >= 0 && kendaraan[j].plat > key.plat)
            {
                kendaraan[j + 1] = kendaraan[j];
                j--;
            }
            kendaraan[j + 1] = key;
        }
        json::wvalue::list hasil;
        for (int i = 0; i < jumlahKendaraanTerparkir; i++)
        {
            json::wvalue item;
            item["id_kendaraan"] = kendaraan[i].id;
            item["tipe_kendaraan"] = kendaraan[i].type;
            item["plat_kendaraan"] = kendaraan[i].plat;
            item["biaya_parkir"] = kendaraan[i].kendaraanSpesifik.harga;
            item["denda_parkir"] = kendaraan[i].kendaraanSpesifik.denda;
            item["waktu_masuk"] = kendaraan[i].kendaraanSpesifik.waktu();
            item["waktu_keluar"] = kendaraan[i].kendaraanSpesifik.waktuKeluar;
            hasil.emplace_back(move(item));
        }
        json::wvalue res;
        res["cekBol"] = false;
        res["hasil"] = move(hasil);
        return response{res};
    }
}

void eksekusiMasukPengumuman(int id, string judul, string deskripsi, string lokasi, string kontak1, string kontak2, string prio, bool publish, bool awalSimpan){
    pengumuman[jmlPengumuman].id = id;
    pengumuman[jmlPengumuman].judul = judul;
    pengumuman[jmlPengumuman].deskripsi = deskripsi;
    pengumuman[jmlPengumuman].lokasi = lokasi;
    pengumuman[jmlPengumuman].kontak[0] = kontak1;
    pengumuman[jmlPengumuman].kontak[1] = kontak2;
    pengumuman[jmlPengumuman].prioritas = prio;
    if (awalSimpan)
    {
        pengumuman[jmlPengumuman].eksekusiWaktuPubllish();
        pengumuman[jmlPengumuman].waktuPublish = pengumuman[jmlPengumuman].waktu();
    } else {
        pengumuman[jmlPengumuman].waktuPublish = "Tidak di Publish";
    }
    pengumuman[jmlPengumuman].publish = publish;
    jmlPengumuman++;
    idPengumuman++;
}

response kirimDataPengumuman(const request &req){
    if (jmlPengumuman == maksPengumuman)
    {
        json::wvalue res;
        res["message"] = "Jumlah pengumuman telah penuh";
        return response{res};
    }
    
    auto data = json::load(req.body);
    string judul, deskripsi, lokasi, kontak1, kontak2, prioritas;
    bool publish, awalSimpan = false;
    judul = data["judul"].s();
    deskripsi = data["deskripsi"].s();
    lokasi = data["lokasi"].s();
    kontak1 = data["kontak"].s();
    kontak2 = data["kontak2"].s();
    prioritas = data["prio"].s();
    if (data.has("cekPublish"))
    {
        publish = data["cekPublish"].b();
    } else {
        publish = false;
    }
    awalSimpan = data["awalSimpan"].b();
    eksekusiMasukPengumuman(idPengumuman,judul,deskripsi,lokasi,kontak1,kontak2,prioritas, publish, awalSimpan);
    json::wvalue res;
    res["cekBol"] = true;
    res["message"] = "Data berhasil disimpan";
    return response{res};
}

response lihatPengumuman(){
    if (jmlPengumuman == 0)
    {
        json::wvalue res;
        res["keteranganBol"] = false;
        res["message"] = "Belum ada pengumuman yang dibuat";
        return {res};
    }
    
    int parameter = 0;
    for (int i = 0; i < jmlPengumuman; i++)
    {
        if (pengumuman[i].publish)
        {
            parameter++;
        }
    }
    if (parameter != 0)
    {
        json::wvalue res;
        res["cekParameter"] = false;
    }
    
    json::wvalue::list hasil;
    for (int i = 0; i < jmlPengumuman; i++)
    {
        json::wvalue item;
        item["noPengumuman"] = pengumuman[i].id;
        item["judul"] = pengumuman[i].judul;
        item["deskripsi"] = pengumuman[i].deskripsi;
        item["lokasi"] = pengumuman[i].lokasi;
        item["kontak1"] = pengumuman[i].kontak[0];
        item["kontak2"] = pengumuman[i].kontak[1];
        item["prioritas"] = pengumuman[i].prioritas;
        item["publish"] = pengumuman[i].publish;
        item["waktuPublish"] = pengumuman[i].waktuPublish;
        hasil.push_back(move(item));
    }
    json::wvalue res;
    res["dataPengumuman"] = move(hasil);
    res["keteranganBol"] = true;
    return response{res};
}

response publishPengumuman(int idPengumuman){
    for (int i = 0; i < jmlPengumuman; i++)
    {
        if (pengumuman[i].id == idPengumuman)
        {
            pengumuman[i].publish = true;
            pengumuman[i].eksekusiWaktuPubllish();
            pengumuman[i].waktuPublish = pengumuman[i].waktu();
            json::wvalue res;
            res["cekBol"] = true;
            res["message"] = "Pengumuman berhasil diPublish";
            return response{res};
        }
    }
    json::wvalue res;
    res["cekBol"] = false;
    res["message"] = "Pengumuman tidak berhasil diPublish";
    return response{res};
}

response unpublishPengumuman(int idPengumuman){
    for (int i = 0; i < jmlPengumuman; i++)
    {
        if (pengumuman[i].id == idPengumuman)
        {
            pengumuman[i].publish = false;
            pengumuman[i].waktuPublish = "Tidak di Publish";
            json::wvalue res;
            res["cekBol"] = true;
            res["message"] = "Pengumuman berhasil di UnPublish";
            return response{res};
        }
    }
    json::wvalue res;
    res["cekBol"] = false;
    res["message"] = "Pengumuman tidak berhasil di UnPublish";
    return response{res};
}

response lihatDataPengumuman(int idPengumuman){
    cout << "Id" << idPengumuman;
    for (int i = 0; i < jmlPengumuman; i++)
    {
        if (pengumuman[i].id == idPengumuman)
        {
            json::wvalue res;
            res["cekBol"] = true;
            res["judul"] = pengumuman[i].judul;
            res["deskripsi"] = pengumuman[i].deskripsi;
            res["lokasi"] = pengumuman[i].lokasi;
            res["kontak1"] = pengumuman[i].kontak[0];
            res["kontak2"] = pengumuman[i].kontak[1];
            res["prioritas"] = pengumuman[i].prioritas;
            return response{res};
        }
    }
    json::wvalue res;
    res["cekBol"] = false;
    res["message"] = "Data tidak berhasil, anda akan diarahkan ke menu daftar pengumuman";
    return response{res};
}

response editPengumuman(const request &req ,int idPengumuman){
    auto data = json::load(req.body);
    string prioritas;
    if (data["prioritas"].s() == "1") {
        prioritas = "Biasa";
    } else if (data["prioritas"].s() == "2") {
        prioritas = "Penting";
    } else {
        prioritas = "Darurat";
    }
    for (int i = 0; i < jmlPengumuman; i++)
    {
        if (pengumuman[i].id == idPengumuman)
        {
            pengumuman[i].judul = data["judul"].s();
            pengumuman[i].deskripsi = data["deskripsi"].s();
            pengumuman[i].lokasi = data["lokasi"].s();
            pengumuman[i].kontak[0] = data["kontak1"].s();
            pengumuman[i].kontak[1] = data["kontak2"].s();
            pengumuman[i].prioritas = prioritas;
            json::wvalue res;
            res["cekBol"] = true;
            res["message"] = "Data berhasil di update";
            return response{res};
        }
    }
    json::wvalue res;
    res["cekBol"] = false;
    res["message"] = "Data tidak berhasil di update";
    return response{res};
}

response hapusPengumuman(int idPengumuman){
    for (int i = 0; i < jmlPengumuman; i++)
    {
        if (pengumuman[i].id == idPengumuman)
        {
            for (int j = i; j < jmlPengumuman-1; j++)
            {
                pengumuman[j] = pengumuman[j+1];
            }
            jmlPengumuman--;
            json::wvalue res;
            res["cekBol"] = true;
            res["message"] = "Data berhasil dihapus";
            return response{res};
        }
    }
    json::wvalue res;
    res["cekBol"] = false;
    res["message"] = "Data tidak berhasil dihapus";
    return response{res};
}

response isiTiket(string id){
    kendaraan[jumlahKendaraanTerparkir+1].id = id;
    int index = 0;
    while (kendaraan[index].id != id)
    {
        index++;
    }
    if (index < jumlahKendaraanTerparkir)
    {
        json::wvalue res;
        res["cekBol"] = true;
        res["idKendaraan"] = kendaraan[index].id;
        res["tipeKendaraan"] = kendaraan[index].type;
        res["platKendaraan"] = kendaraan[index].plat;
        res["waktuMasuk"] = kendaraan[index].kendaraanSpesifik.waktu();
        return response{res};
    }
    json::wvalue res;
    res["cekBol"] = false;
    return response{res};
}

response lihatKendaraanPribadi(string idKendaraanPribadi){
    json::wvalue res;
    if (jumlahKendaraanTerparkir == 0)
    {
        res["cekBol"] = false;
        res["message"] = "Belum ada kendaraan bjir, anda pasti hecker";
        return response{res};
    }
    for (int i = 0; i < jumlahKendaraanTerparkir; i++)
    {
        if (kendaraan[i].id == idKendaraanPribadi)
        {
            res["cekBol"] = true;
            res["tipe"] = kendaraan[i].type;
            res["plat"] = kendaraan[i].plat;
            res["waktuMasuk"] = kendaraan[i].kendaraanSpesifik.waktu();
            res["harga"] = kendaraan[i].kendaraanSpesifik.harga;
            return response{res};
        }
    }
    res["cekBol"] = false;
    res["message"] = "Data tidak ditemukan";
    return response{res};
}
