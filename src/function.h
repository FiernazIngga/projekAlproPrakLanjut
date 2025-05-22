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
#include <fstream>
#include <ctime>
using namespace std;
using namespace crow;

const int maxLahanParkir = 30;
int jumlahKendaraanTerparkir = 0;
const int jmlLaporPribadi = 100;
int indexLapor = 0;

tm* waktuLocal(){
    time_t waktu_sekarang;
    time(&waktu_sekarang);
    tm* waktu_lokal = localtime(&waktu_sekarang);
    return waktu_lokal;
}

string dapatkanWaktuSaatIni(){
    int waktuSekarang[6];
    time_t waktu_sekarang;
    time(&waktu_sekarang);
    tm* waktu_lokal = localtime(&waktu_sekarang);
        waktuSekarang[0] = waktu_lokal->tm_sec;
        waktuSekarang[1] = waktu_lokal->tm_min;
        waktuSekarang[2] = waktu_lokal->tm_hour;
        waktuSekarang[3] = waktu_lokal->tm_mday;
        waktuSekarang[4] = (waktu_lokal->tm_mon)+1;
        waktuSekarang[5] = (waktu_lokal->tm_year)+1900;
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
    oss << waktuSekarang[2] << ":" << waktuSekarang[1] << ":" << waktuSekarang[0]  << " "
        << waktuSekarang[3] << "-" << bulan[waktuSekarang[4] - 1] << "-" << waktuSekarang[5];
    return oss.str();
}

struct Parkir
{
    string harga, denda, waktuMasuk;
    string waktuKeluar;
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
    string judul, deskripsi, lokasi, kontak[2], prioritas, waktuPosting;
    bool publish;
    string waktuPublish;
} pengumuman[maksPengumuman];

struct LaporPribadi {
    string idLapor, judulLapor, deskripsiLapor, lokasiLapor, kontakLapor, prioritasLapor;
    bool keteranganLapor, keteranganLihatLaporan;
    string waktuLapor;
} laporanPribadi[jmlLaporPribadi];


void pindahKeStructKendaraan();

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

// Pengubah mulai
bool stringToBool(const std::string& str) {
    return str == "true" || str == "1";
}

string pengubahString(const string parameter){
    string hasilAkhir;
    for (int i = 0; i < parameter.size(); i++)
    {
        if (parameter[i] != ' ')
        {
            hasilAkhir += parameter[i];
        }
    }
    for (int i = 0; i < hasilAkhir.size(); i++)
    {
        hasilAkhir[i] = tolower(hasilAkhir[i]);
    }
    return hasilAkhir;
}
// Pengubah mulai

response returnCekKendaraan(const request& req) {
    json::wvalue res;
    auto data = json::load(req.body);
    if (!data) {
        res["message"] = "Inputan data kendaraan kosong";
        return response{res};
    }

    string plat = pengubahString(data["platKendaraan"].s());
    string kunciPlatCari = pengubahString(plat);

    if (jumlahKendaraanTerparkir == 0) {
        res["message"] = "Kosong belum ada kendaraan terparkir";
        return response{res};
    }

    pindahKeStructKendaraan();
    kendaraan[jumlahKendaraanTerparkir+1].plat = kunciPlatCari;

    int i = 0;
    while (pengubahString(kendaraan[i].plat) != kunciPlatCari) {
        i++;
    }

    if (i > jumlahKendaraanTerparkir) {
        res["message"] = "Kendaraan tidak ditemukan";
    } else {
        res["message"] = "Kendaraan ditemukan";
        res["data"] = json::wvalue::list({
            {
                {"tipe", kendaraan[i].type},
                {"plat", kendaraan[i].plat},
                {"biaya", kendaraan[i].kendaraanSpesifik.harga},
                {"denda", kendaraan[i].kendaraanSpesifik.denda},
                {"waktuMsk", kendaraan[i].kendaraanSpesifik.waktuMasuk},
                {"waktuKlr", kendaraan[i].kendaraanSpesifik.waktuKeluar}
            }
        });
    }

    return response{res};
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

void pindahKeStructKendaraan(){
    jumlahKendaraanTerparkir = 0;
    ifstream fileKendaraan("src/database/fileKendaraan.txt");
    if (fileKendaraan.is_open())
    {
        string line;
        while (getline(fileKendaraan, line))
        {
            stringstream ss(line);
            string id, tipe, plat, harga, waktuM, waktuK;
            if (!getline(ss, id, '|') || !getline(ss, tipe, '|') || !getline(ss, plat, '|') || !getline(ss, harga, '|') || !getline(ss, waktuM, '|') || !getline(ss, waktuK, '|'))
            {
                continue;
            }
            kendaraan[jumlahKendaraanTerparkir].id = id;
            kendaraan[jumlahKendaraanTerparkir].type = tipe;
            kendaraan[jumlahKendaraanTerparkir].plat = plat;
            kendaraan[jumlahKendaraanTerparkir].kendaraanSpesifik.harga = harga;
            kendaraan[jumlahKendaraanTerparkir].kendaraanSpesifik.waktuMasuk = waktuM;
            kendaraan[jumlahKendaraanTerparkir].kendaraanSpesifik.waktuKeluar = waktuK;
            jumlahKendaraanTerparkir++;
            cout << id << tipe << plat << harga << waktuM << waktuK << endl;
        }
    } else {
        cout << "Terjadi kesalahan" << endl;
    }
    fileKendaraan.close();
}

void eksekusiKendaraan(string *id, string *tipeKendaraan, string *plat, int *harga, string waktuMasuk, string waktuKeluar){
    ofstream kendaraanMasuk("src/database/fileKendaraan.txt", ios::app);
    if (kendaraanMasuk.is_open()) {
        kendaraanMasuk << *id << "|" << *tipeKendaraan << "|" << *plat << "|" << *harga << "|" << waktuMasuk << "|" << waktuKeluar << endl;
        kendaraanMasuk.close();
        cout << "Akun berhasil dibuat dan disimpan!\n";
    } else {
        cout << "Gagal membuka file. Data tidak tersimpan.\n";
    }
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
    json::wvalue res;
    if (jumlahKendaraanTerparkir == maxLahanParkir) {
        res["cekBol"] = false;
        res["message"] = "Lahan kendaraan telah penuh";
        return response{res};
    }
    auto data = json::load(req.body);
    if (!data) return response(400, "Invalid JSON");

    string id = data["id"].s();
    string tipe = data["kendaraan"].s();
    string plat = data["platKendaraan"].s();

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
        res["message"] = "Mohon maaf lahan parkir untuk kendaraan roda empat telah penuh maks 10";
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


    eksekusiKendaraan(&id, &tipe, &plat, &tarif, dapatkanWaktuSaatIni(), "Kendaraan Masih Didalam");
    pindahKeStructKendaraan();
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
        item["waktu_masuk"] = kendaraan[kendaraanTerpakir].kendaraanSpesifik.waktuMasuk;
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
            item["waktu_masuk"] = kendaraan[i].kendaraanSpesifik.waktuMasuk;
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
            item["waktu_masuk"] = kendaraan[i].kendaraanSpesifik.waktuMasuk;
            item["waktu_keluar"] = kendaraan[i].kendaraanSpesifik.waktuKeluar;
            hasil.emplace_back(move(item));
        }
        json::wvalue res;
        res["cekBol"] = false;
        res["hasil"] = move(hasil);
        return response{res};
    }
}

void loadPengumuman(){
    jmlPengumuman = 0;
    idPengumuman = 0;
    ifstream loadPengumuman("src/database/filePengumuman.txt");
    if (loadPengumuman.is_open())
    {
        string line;
        while (getline(loadPengumuman, line))
        {
            stringstream ss(line);
            string id, judul, deskripsi, lokasi, kontak1, kontak2, prio, publish, ketWaktu;
            if (!getline(ss, id, '|') || !getline(ss, judul, '|') || !getline(ss, deskripsi, '|') || !getline(ss, lokasi, '|') || !getline(ss, kontak1, '|') || !getline(ss, kontak2, '|') || !getline(ss, prio, '|') || !getline(ss, publish, '|') || !getline(ss, ketWaktu, '|'))
            {
                continue;
            }
            pengumuman[jmlPengumuman].id = stoi(id);
            pengumuman[jmlPengumuman].judul = judul;
            pengumuman[jmlPengumuman].deskripsi = deskripsi;
            pengumuman[jmlPengumuman].lokasi = lokasi;
            pengumuman[jmlPengumuman].kontak[0] = kontak1;
            pengumuman[jmlPengumuman].kontak[1] = kontak2;
            pengumuman[jmlPengumuman].prioritas = prio;
            pengumuman[jmlPengumuman].publish = stringToBool(publish);
            pengumuman[jmlPengumuman].waktuPublish = ketWaktu;
            jmlPengumuman++;
            idPengumuman++;
        }
    }
    loadPengumuman.close();
    
}

void eksekusiMasukPengumuman(int *id, string *judul, string *deskripsi, string *lokasi, string *kontak1, string *kontak2, string *prio, bool *publish, bool *awalSimpan){
    ofstream filePengumuman("src/database/filePengumuman.txt", ios::app);
    if (filePengumuman.is_open())
    {
        string waktu;
        if (*awalSimpan)
        {
            waktu = dapatkanWaktuSaatIni();
        } else {
            waktu = "Tidak di Publish";
        }
        filePengumuman << *id << "|" << *judul << "|" << *deskripsi << "|" << *lokasi << "|" << *kontak1 << "|" << *kontak2  << "|" << *prio  << "|" << *publish  << "|" << waktu << endl;
        cout << "Akun berhasil dibuat dan disimpan!\n";
    } else {
        cout << "Gagal membuka file. Data tidak tersimpan.\n";
    }
    filePengumuman.close();
    loadPengumuman();
}

int cekId(){
    int hitungID = 0;
    ifstream hitung("src/database/filePengumuman.txt");
    if (hitung.is_open())
    {
        string line;
        while(getline(hitung, line)){
            hitungID+=1;
        }
        hitung.close();
    }
    return hitungID;
}

response kirimDataPengumuman(const request &req){
    if (jmlPengumuman == maksPengumuman)
    {
        json::wvalue res;
        res["message"] = "Jumlah pengumuman telah penuh";
        return response{res};
    }
    
    auto data = json::load(req.body);
    int id;
    string judul, deskripsi, lokasi, kontak1, kontak2, prioritas;
    bool publish, awalSimpan = false;
    id = cekId();
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
    eksekusiMasukPengumuman(&id, &judul, &deskripsi, &lokasi, &kontak1, &kontak2, &prioritas, &publish, &awalSimpan);
    json::wvalue res;
    res["cekBol"] = true;
    res["message"] = "Data berhasil disimpan";
    return response{res};
}

response lihatPengumuman(){
    if (jmlPengumuman == 0 && indexLapor == 0)
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
    for (int i = 0; i < indexLapor; i++)
    {
        if (laporanPribadi[i].idLapor != "" && indexLapor > 0)
        {
            json::wvalue item;
            item["noPengumuman"] = laporanPribadi[i].idLapor;
            item["judul"] = laporanPribadi[i].judulLapor;
            item["deskripsi"] = laporanPribadi[i].deskripsiLapor;
            item["lokasi"] = laporanPribadi[i].lokasiLapor;
            item["kontak1"] = laporanPribadi[i].kontakLapor;
            item["kontak2"] = "";
            item["publish"] = laporanPribadi[i].keteranganLapor;
            item["prioritas"] = laporanPribadi[i].prioritasLapor;
            item["waktuPublish"] = laporanPribadi[i].waktuLapor;
            hasil.push_back(move(item));
        }
    }
    json::wvalue res;
    res["dataPengumuman"] = move(hasil);
    res["keteranganBol"] = true;
    return response{res};
}

response publishPengumuman(int *idPengumuman){

    ifstream mainPeng("src/database/filePengumuman.txt");
    ofstream tempPeng("src/database/filePengumumanTemp.txt");
    if (!mainPeng.is_open() || !tempPeng.is_open()) {
        json::wvalue res;
        res["cekBol"] = false;
        res["message"] = "Pengumuman tidak berhasil diPublish";
        return response{res};
    } else {
        string line;
        while (getline(mainPeng, line))
        {
            string id, judul, deskripsi, lokasi, kontak1, kontak2, prio, publish, awalSimpan;
            stringstream ss(line);
            if (!getline(ss, id, '|') || !getline(ss, judul, '|') || !getline(ss, deskripsi, '|') || !getline(ss, lokasi, '|') || !getline(ss, kontak1, '|') || !getline(ss, kontak2, '|') || !getline(ss, prio, '|') || !getline(ss, publish, '|') || !getline(ss, awalSimpan, '|'))
            {
                continue;
            }
            if (stoi(id) == *idPengumuman)
            {
                string waktu = dapatkanWaktuSaatIni();
                bool infoPub = true;
                tempPeng << id << "|" << judul << "|" << deskripsi << "|" << lokasi << "|" << kontak1 << "|" << kontak2  << "|" << prio  << "|" << infoPub  << "|" << waktu << endl;
            } else {
                tempPeng << line << endl;
            }
        }
    }
    mainPeng.close();
    tempPeng.close();

    if (remove("src/database/filePengumuman.txt") != 0) {
        perror("Gagal menghapus file lama");
        system("pause");
        json::wvalue res;
        res["cekBol"] = false;
        res["message"] = "Pengumuman tidak berhasil diPublish";
        return response{res};
    }

    if (rename("src/database/filePengumumanTemp.txt", "src/database/filePengumuman.txt") != 0) {
        perror("Gagal mengganti file baru");
        system("pause");
        json::wvalue res;
        res["cekBol"] = false;
        res["message"] = "Pengumuman tidak berhasil diPublish";
        return response{res};
    }
    json::wvalue res;
    res["cekBol"] = true;
    res["message"] = "Pengumuman berhasil diPublish";
    loadPengumuman();
    return response{res};
}

response unpublishPengumuman(int *idPengumuman) {
    json::wvalue res;

    ifstream mainPeng("src/database/filePengumuman.txt");
    ofstream tempPeng("src/database/filePengumumanTemp.txt");

    if (!mainPeng.is_open() || !tempPeng.is_open()) {
        res["cekBol"] = false;
        res["message"] = "Pengumuman tidak berhasil di-unpublish (gagal membuka file)";
        return response{res};
    }

    string line;
    bool found = false;
    while (getline(mainPeng, line)) {
        string id, judul, deskripsi, lokasi, kontak1, kontak2, prio, publish, awalSimpan;
        stringstream ss(line);
        if (!getline(ss, id, '|') || !getline(ss, judul, '|') || !getline(ss, deskripsi, '|') ||
            !getline(ss, lokasi, '|') || !getline(ss, kontak1, '|') || !getline(ss, kontak2, '|') ||
            !getline(ss, prio, '|') || !getline(ss, publish, '|') || !getline(ss, awalSimpan, '|')) {
            continue;
        }

        if (stoi(id) == *idPengumuman) {
            found = true;
            string waktu = "Tidak di Publish";
            bool infoPub = false;
            tempPeng << id << "|" << judul << "|" << deskripsi << "|" << lokasi << "|" << kontak1 << "|"
                     << kontak2 << "|" << prio << "|" << infoPub << "|" << waktu << endl;
        } else {
            tempPeng << line << endl;
        }
    }

    mainPeng.close();
    tempPeng.close();

    if (remove("src/database/filePengumuman.txt") != 0) {
        perror("Gagal menghapus file lama");
        res["cekBol"] = false;
        res["message"] = "Pengumuman tidak berhasil di-unpublish (gagal hapus file lama)";
        return response{res};
    }

    if (rename("src/database/filePengumumanTemp.txt", "src/database/filePengumuman.txt") != 0) {
        perror("Gagal mengganti file baru");
        res["cekBol"] = false;
        res["message"] = "Pengumuman tidak berhasil di-unpublish (gagal rename file)";
        return response{res};
    }

    if (!found) {
        res["cekBol"] = false;
        res["message"] = "ID Pengumuman tidak ditemukan";
    } else {
        res["cekBol"] = true;
        res["message"] = "Pengumuman berhasil di-unpublish";
    }

    loadPengumuman();
    return response{res};
}


response lihatDataPengumuman(int *idPengumuman){
    for (int i = 0; i < jmlPengumuman; i++)
    {
        if (pengumuman[i].id == *idPengumuman)
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
    loadPengumuman();
    json::wvalue res;
    res["cekBol"] = false;
    res["message"] = "Data tidak berhasil, anda akan diarahkan ke menu daftar pengumuman";
    return response{res};
}

response editPengumuman(const request &req ,int *idPengumuman){
    auto data = json::load(req.body);
    string prioritas;
    if (data["prioritas"].s() == "1") {
        prioritas = "Biasa";
    } else if (data["prioritas"].s() == "2") {
        prioritas = "Penting";
    } else {
        prioritas = "Darurat";
    }
    ifstream mainPeng("src/database/filePengumuman.txt");
    ofstream tempPeng("src/database/filePengumumanTemp.txt");
    if (!mainPeng.is_open() || !tempPeng.is_open()) {
        json::wvalue res;
        res["cekBol"] = false;
        res["message"] = "Data tidak berhasil di update";
        return response{res};
    } else {
        string line;
        while (getline(mainPeng, line))
        {
            string id, judul, deskripsi, lokasi, kontak1, kontak2, prio, publish, awalSimpan;
            stringstream ss(line);
            if (!getline(ss, id, '|') || !getline(ss, judul, '|') || !getline(ss, deskripsi, '|') || !getline(ss, lokasi, '|') || !getline(ss, kontak1, '|') || !getline(ss, kontak2, '|') || !getline(ss, prio, '|') || !getline(ss, publish, '|') || !getline(ss, awalSimpan, '|'))
            {
                continue;
            }
            if (stoi(id) == *idPengumuman)
            {
                tempPeng << id << "|" << data["judul"].s() << "|" << data["deskripsi"].s() << "|" << data["lokasi"].s() << "|" << data["kontak1"].s() << "|" << data["kontak2"].s()  << "|" << prioritas  << "|" << publish  << "|" << awalSimpan << endl;
            } else {
                tempPeng << line << endl;
            }
        }
    }
    mainPeng.close();
    tempPeng.close();

    if (remove("src/database/filePengumuman.txt") != 0) {
        perror("Gagal menghapus file lama");
        system("pause");
        json::wvalue res;
        res["cekBol"] = false;
        res["message"] = "Data tidak berhasil di update";
        return response{res};
    }

    if (rename("src/database/filePengumumanTemp.txt", "src/database/filePengumuman.txt") != 0) {
        perror("Gagal mengganti file baru");
        system("pause");
        json::wvalue res;
        res["cekBol"] = false;
        res["message"] = "Data tidak berhasil di update";
        return response{res};
    }
    json::wvalue res;
    res["cekBol"] = true;
    res["message"] = "Data berhasil di update";
    loadPengumuman();
    return response{res};
}

response hapusPengumuman(int *idPengumuman){
    ifstream mainPeng("src/database/filePengumuman.txt");
    ofstream tempPeng("src/database/filePengumumanTemp.txt");
    if (!mainPeng.is_open() || !tempPeng.is_open()) {
        json::wvalue res;
        res["cekBol"] = false;
        res["message"] = "Pengumuman tidak berhasil dihapus";
        return response{res};
    } else {
        string line;
        while (getline(mainPeng, line))
        {
            string id, judul, deskripsi, lokasi, kontak1, kontak2, prio, publish, awalSimpan;
            stringstream ss(line);
            if (!getline(ss, id, '|') || !getline(ss, judul, '|') || !getline(ss, deskripsi, '|') || !getline(ss, lokasi, '|') || !getline(ss, kontak1, '|') || !getline(ss, kontak2, '|') || !getline(ss, prio, '|') || !getline(ss, publish, '|') || !getline(ss, awalSimpan, '|'))
            {
                continue;
            }
            if (stoi(id) != *idPengumuman)
            {
                tempPeng << line << endl;
            }
        }
    }
    mainPeng.close();
    tempPeng.close();

    if (remove("src/database/filePengumuman.txt") != 0) {
        perror("Gagal menghapus file lama");
        system("pause");
        json::wvalue res;
        res["cekBol"] = false;
        res["message"] = "Pengumuman tidak berhasil dihapus";
        return response{res};
    }

    if (rename("src/database/filePengumumanTemp.txt", "src/database/filePengumuman.txt") != 0) {
        perror("Gagal mengganti file baru");
        system("pause");
        json::wvalue res;
        res["cekBol"] = false;
        res["message"] = "Pengumuman tidak berhasil dihapus";
        return response{res};
    }
    json::wvalue res;
    res["cekBol"] = true;
    res["message"] = "Pengumuman berhasil dihapus";
    loadPengumuman();
    return response{res};
}

response isiTiket(string *id){
    kendaraan[jumlahKendaraanTerparkir+1].id = *id;
    int index = 0;
    while (kendaraan[index].id != *id)
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
        res["waktuMasuk"] = kendaraan[index].kendaraanSpesifik.waktuMasuk;
        return response{res};
    }
    json::wvalue res;
    res["cekBol"] = false;
    return response{res};
}

response lihatKendaraanPribadi(string *idKendaraanPribadi){
    json::wvalue res;
    if (jumlahKendaraanTerparkir == 0)
    {
        res["cekBol"] = false;
        res["message"] = "Belum ada kendaraan bjir, anda pasti hecker";
        return response{res};
    }
    for (int i = 0; i < jumlahKendaraanTerparkir; i++)
    {
        if (kendaraan[i].id == *idKendaraanPribadi)
        {
            res["cekBol"] = true;
            res["tipe"] = kendaraan[i].type;
            res["plat"] = kendaraan[i].plat;
            res["waktuMasuk"] = kendaraan[i].kendaraanSpesifik.waktuMasuk;
            res["harga"] = kendaraan[i].kendaraanSpesifik.harga;
            return response{res};
        }
    }
    res["cekBol"] = false;
    res["message"] = "Data tidak ditemukan";
    return response{res};
}


int lihatIsian(){
    int index = 0;
    for (int i = 0; i < jmlLaporPribadi; i++)
    {
        if (laporanPribadi[i].idLapor != "")
        {
            index+=1;
        }
    }
    return index;
}

void loadLaporanPribadi(){
    indexLapor = 0;
    ifstream filePribadi("src/database/fileLaporPribadi.txt");
    string line;
    while (getline(filePribadi, line))
    {
        stringstream ss(line);
        string id, judul, deskripsi, lokasi, kontak, prio, waktuLapor, ketLapor, ketLihat;
        if (!getline(ss, id, '|') || !getline(ss, judul, '|') || !getline(ss, deskripsi, '|') || !getline(ss, lokasi, '|') || !getline(ss, kontak, '|') || !getline(ss, prio, '|') || !getline(ss, ketLapor, '|') || !getline(ss, ketLihat, '|') || !getline(ss, waktuLapor, '|'))
        {
            continue;
        }
        laporanPribadi[indexLapor].idLapor = id;
        laporanPribadi[indexLapor].judulLapor = judul;
        laporanPribadi[indexLapor].deskripsiLapor = deskripsi;
        laporanPribadi[indexLapor].lokasiLapor = lokasi;
        laporanPribadi[indexLapor].kontakLapor = kontak;
        laporanPribadi[indexLapor].prioritasLapor = prio;
        laporanPribadi[indexLapor].keteranganLapor = stringToBool(ketLapor);
        laporanPribadi[indexLapor].keteranganLihatLaporan = stringToBool(ketLihat);
        laporanPribadi[indexLapor].waktuLapor = waktuLapor;
        indexLapor+=1;
    }
    filePribadi.close();
}

bool eksekusiLaporanPribadi(string id, string judul, string deskripsi, string lokasi, string kontak, string prio, bool ketLapor, bool ketLihat, string waktuLapor){
    ofstream fileLaporPribadi("src/database/fileLaporPribadi.txt");
    if (fileLaporPribadi.is_open())
    {
        fileLaporPribadi << id << "|" << judul << "|" << deskripsi << "|" << lokasi << "|" << kontak << "|" << prio << "|" << ketLapor << "|" << ketLihat << "|" << waktuLapor << endl; 
        fileLaporPribadi.close();
        return 1;
    } else {
        fileLaporPribadi.close();
        return 0;
    }
}

response masukinLaporanPribadi(const request &req){
    int index = lihatIsian();
    json::wvalue res;
    if (index == 100)
    {
        res["bool"] = false;
        res["message"] = "Mohon maaf database lapor sedang penuh!!!";
        return response{res};
    }
    auto data = json::load(req.body);
    if (eksekusiLaporanPribadi(data["idLapor"].s(), data["judulLapor"].s(), data["deskripsiLapor"].s(), data["lokasiLapor"].s(), data["kontakLapor"].s(), data["prioritasLapor"].s(), false, false, dapatkanWaktuSaatIni()))
    {
        res["bool"] = true;
        res["message"] = "Laporan berhasil dikirim";
        return response{res};
    } else {
        res["bool"] = false;
        res["message"] = "Laporan tidak berhasil dikirim";
        return response{res};
    }
}

response lihatLaporanPribadi(){
    loadLaporanPribadi();
    json::wvalue res;
    if (indexLapor == 0)
    {
        res["cekBol"] = false;
        res["message"] = "Belum ada laporan dari pengguna";
        return response{res};
    }
    json::wvalue::list hasil;
    for (int i = 0; i < indexLapor; i++)
    {
        json::wvalue item;
        item["idLapor"] = laporanPribadi[i].idLapor;
        item["judulLapor"] = laporanPribadi[i].judulLapor;
        item["lokasiLapor"] = laporanPribadi[i].lokasiLapor;
        item["prioritasLapor"] = laporanPribadi[i].prioritasLapor;
        item["keteranganPublish"] = laporanPribadi[i].keteranganLapor;
        item["keteranganLihat"] = laporanPribadi[i].keteranganLihatLaporan;
        hasil.push_back(move(item));
    }
    res["cekBol"] = true;
    res["semuaData"] = move(hasil);
    return response{res};
}

response lihatLaporanUser(const string *idLaporanUser) {
    json::wvalue res;
    ifstream fileLihat("src/database/fileLaporPribadi.txt");
    ofstream fileEdit("src/database/tempFileLaporPribadi.txt");

    if (!fileLihat.is_open() || !fileEdit.is_open()) {
        res["cekBol"] = false;
        res["message"] = "Laporan tidak ditemukan (gagal membuka file)";
        return response{res};
    }

    string line;
    while (getline(fileLihat, line)) {
        stringstream ss(line);
        string id, judul, deskripsi, lokasi, kontak, prio, waktuLapor, ketLapor, ketLihat;
        if (!getline(ss, id, '|') || !getline(ss, judul, '|') || !getline(ss, deskripsi, '|') ||
            !getline(ss, lokasi, '|') || !getline(ss, kontak, '|') || !getline(ss, prio, '|') ||
            !getline(ss, ketLapor, '|') || !getline(ss, ketLihat, '|') || !getline(ss, waktuLapor, '|')) {
            continue;
        }

        if (id == *idLaporanUser) {
            bool ketLihatLaporanBaru = true;
            fileEdit << id << "|" << judul << "|" << deskripsi << "|" << lokasi << "|" << kontak << "|"
                     << prio << "|" << ketLapor << "|" << ketLihatLaporanBaru << "|" << waktuLapor << endl;
        } else {
            fileEdit << line << endl;
        }
    }

    fileEdit.close();
    fileLihat.close();

    remove("src/database/fileLaporPribadi.txt");
    rename("src/database/tempFileLaporPribadi.txt", "src/database/fileLaporPribadi.txt");

    ifstream fileLihat2("src/database/fileLaporPribadi.txt");
    string line2;
    bool found = false;

    while (getline(fileLihat2, line2)) {
        stringstream ss(line2);
        string id, judul, deskripsi, lokasi, kontak, prio, waktuLapor, ketLapor, ketLihat;
        if (!getline(ss, id, '|') || !getline(ss, judul, '|') || !getline(ss, deskripsi, '|') ||
            !getline(ss, lokasi, '|') || !getline(ss, kontak, '|') || !getline(ss, prio, '|') ||
            !getline(ss, ketLapor, '|') || !getline(ss, ketLihat, '|') || !getline(ss, waktuLapor, '|')) {
            continue;
        }

        if (id == *idLaporanUser) {
            found = true;
            res["cekBol"] = true;
            res["idLapor"] = id;
            res["judulLapor"] = judul;
            res["deskripsiLapor"] = deskripsi;
            res["lokasiLapor"] = lokasi;
            res["kontakLapor"] = kontak;
            res["prioritasLapor"] = prio;
            res["keteranganPublish"] = stringToBool(ketLapor);
            res["waktuLapor"] = waktuLapor;
            break; 
        }
    }

    fileLihat2.close();

    if (!found) {
        res["cekBol"] = false;
        res["message"] = "Laporan tidak ditemukan";
    }

    loadLaporanPribadi();
    return response{res};
}


response publifikasiLaporan(string *aksiLaporan, string *idLaporan) {
    json::wvalue res;
    ifstream fileLihat("src/database/fileLaporPribadi.txt");
    ofstream fileEdit("src/database/tempFileLaporPribadi.txt");
    
    if (!fileLihat.is_open() || !fileEdit.is_open()) {
        res["cekBol"] = false;
        res["message"] = "Laporan tidak ditemukan (gagal membuka file)";
        return response{res};
    }
    
    bool found = false;
    string line;
    while (getline(fileLihat, line)) {
        stringstream ss(line);
        string id, judul, deskripsi, lokasi, kontak, prio, waktuLapor, ketLapor, ketLihat;
        
        if (!getline(ss, id, '|') || !getline(ss, judul, '|') || !getline(ss, deskripsi, '|') || 
            !getline(ss, lokasi, '|') || !getline(ss, kontak, '|') || !getline(ss, prio, '|') || 
            !getline(ss, ketLapor, '|') || !getline(ss, ketLihat, '|') || !getline(ss, waktuLapor, '|')) {
            continue;
        }
        
        if (id == *idLaporan) {
            found = true;
            if (*aksiLaporan == "publishLaporan") {
                bool ketLaporBaru = true;
                fileEdit << id << "|" << judul << "|" << deskripsi << "|" << lokasi << "|" << kontak << "|" << prio << "|" << ketLaporBaru << "|" << ketLihat << "|" << waktuLapor << endl;
                res["cekBol"] = true;
                res["message"] = "Laporan berhasil di Publish";
            } else if (*aksiLaporan == "unpublishLaporan") {
                bool ketLaporBaru = false;
                fileEdit << id << "|" << judul << "|" << deskripsi << "|" << lokasi << "|" << kontak << "|" << prio << "|" << ketLaporBaru << "|" << ketLihat << "|" << waktuLapor << endl;
                res["cekBol"] = true;
                res["message"] = "Laporan berhasil di unPublish";
            } else {
                fileEdit << line << endl;
                res["cekBol"] = false;
                res["message"] = "Aksi laporan tidak dikenali";
            }
            break; 
        } else {
            fileEdit << line << endl;
        }
    }
    
    fileEdit.close();
    fileLihat.close();
    
    if (!found) {
        res["cekBol"] = false;
        res["message"] = "Laporan tidak ditemukan";
        remove("src/database/tempFileLaporPribadi.txt");
        return response{res};
    }
    
    if (remove("src/database/fileLaporPribadi.txt") != 0 || rename("src/database/tempFileLaporPribadi.txt", "src/database/fileLaporPribadi.txt") != 0) {
        res["cekBol"] = false;
        res["message"] = "Gagal memperbarui file laporan";
        return response{res};
    }
    
    loadLaporanPribadi();
    return response{res};
}


response hapusPelaporanUser(string *idLaporan) {
    bool ident = false;
    json::wvalue res;
    ifstream fileLihat("src/database/fileLaporPribadi.txt");
    ofstream fileEdit("src/database/tempFileLaporPribadi.txt");

    if (!fileLihat.is_open() || !fileEdit.is_open()) {
        res["cekBol"] = false;
        res["message"] = "Laporan tidak ditemukan (gagal membuka file)";
        return response{res};
    }

    string line;
    while (getline(fileLihat, line)) {
        stringstream ss(line);
        string id, judul, deskripsi, lokasi, kontak, prio, waktuLapor, ketLapor, ketLihat;
        if (!getline(ss, id, '|') || !getline(ss, judul, '|') || !getline(ss, deskripsi, '|') ||
            !getline(ss, lokasi, '|') || !getline(ss, kontak, '|') || !getline(ss, prio, '|') ||
            !getline(ss, ketLapor, '|') || !getline(ss, ketLihat, '|') || !getline(ss, waktuLapor, '|')) {
            continue;
        }

        if (id == *idLaporan) {
            ident = true; 
        } else {
            fileEdit << line << endl; 
        }
    }

    fileEdit.close();
    fileLihat.close();

    if (ident) {
        if (remove("src/database/fileLaporPribadi.txt") != 0 || rename("src/database/tempFileLaporPribadi.txt", "src/database/fileLaporPribadi.txt") != 0) {
            res["cekBol"] = false;
            res["message"] = "Gagal memperbarui file laporan setelah penghapusan";
            return response{res};
        }
        loadLaporanPribadi();
        res["cekBol"] = true;
        res["message"] = "Laporan berhasil dihapus";
    } else {
        remove("src/database/tempFileLaporPribadi.txt");
        res["cekBol"] = false;
        res["message"] = "Laporan tidak ditemukan, penghapusan gagal";
    }

    return response{res};
}


response kendaraanKeluarCari(const request& req) {
    json::wvalue res; 
    auto data = json::load(req.body);
    if (!data) {
        res["cekBol"] = false;
        res["message"] = "Tidak ada data terpilih.";
        return response{res};
    }

    string platCari = data["PlatCari"].s();
    ifstream fileKeluar("src/database/fileKendaraan.txt");
    if (!fileKeluar.is_open()) {
        res["cekBol"] = false;
        res["message"] = "Kendaraan tidak berhasil keluar";
        return response{res};
    }

    string line;
    while (getline(fileKeluar, line)) {
        stringstream ss(line);
        string id, tipeKendaraan, plat, harga, waktuMasuk, waktuKeluar;
        if (!getline(ss, id, '|') || !getline(ss, tipeKendaraan, '|') || !getline(ss, plat, '|') ||
            !getline(ss, harga, '|') || !getline(ss, waktuMasuk, '|') || !getline(ss, waktuKeluar, '|')) {
            continue;
        }

        if (platCari == plat) {
            fileKeluar.close();
            res["cekBol"] = true;
            res["id"] = id;
            res["tipe"] = tipeKendaraan;
            res["plat"] = plat;
            res["biaya"] = harga;
            res["waktuMsk"] = waktuMasuk;
            res["waktuKlr"] = waktuKeluar;
            return response{res};
        }
    }
    fileKeluar.close();

    res["cekBol"] = false;
    res["message"] = "Kendaraan tidak ditemukan";
    return response{res};
}

void ubahKendaraanKeluar(string idPlat){
    ifstream fileUtama("src/database/fileKendaraan.txt");
    ofstream fileRiwayat("src/database/riwayatKendaraan.txt", ios::app);
    string line;
    while (getline(fileUtama, line)) {
        stringstream ss(line);
        string id, tipeKendaraan, plat, harga, waktuMasuk, waktuKeluar;
        if (!getline(ss, id, '|') || !getline(ss, tipeKendaraan, '|') ||
            !getline(ss, plat, '|') || !getline(ss, harga, '|') ||
            !getline(ss, waktuMasuk, '|') || !getline(ss, waktuKeluar, '|')) {
            continue;
        }

        if (id == idPlat) {
            string waktuKeluarBaru = dapatkanWaktuSaatIni();
            fileRiwayat << id << "|" << tipeKendaraan << "|" << plat << "|" << harga << "|" << waktuMasuk << "|" << waktuKeluarBaru << endl;
        }
    }

    fileUtama.close();
    fileRiwayat.close();
}

response bayarKendaraanKeluar(string* idPlat) {
    json::wvalue res;
    ifstream fileUtama("src/database/fileKendaraan.txt");
    ofstream fileTemp("src/database/fileTempKendaraan.txt");

    if (!fileUtama.is_open() || !fileTemp.is_open()) {
        res["cekBol"] = false;
        res["message"] = "Kendaraan tidak berhasil keluar";
        return response{res};
    }

    bool ditemukan = false;
    string line;
    while (getline(fileUtama, line)) {
        stringstream ss(line);
        string id, tipeKendaraan, plat, harga, waktuMasuk, waktuKeluar;
        if (!getline(ss, id, '|') || !getline(ss, tipeKendaraan, '|') ||
            !getline(ss, plat, '|') || !getline(ss, harga, '|') ||
            !getline(ss, waktuMasuk, '|') || !getline(ss, waktuKeluar, '|')) {
            continue;
        }

        if (id == *idPlat) {
            ubahKendaraanKeluar(*idPlat);
            ditemukan = true;
            continue;
        }

        fileTemp << line << endl;
    }

    fileUtama.close();
    fileTemp.close();

    remove("src/database/fileKendaraan.txt");
    rename("src/database/fileTempKendaraan.txt", "src/database/fileKendaraan.txt");
    pindahKeStructKendaraan();

    if (ditemukan) {
        res["cekBol"] = true;
        res["message"] = "Kendaraan berhasil keluar";
    } else {
        res["cekBol"] = false;
        res["message"] = "Kendaraan tidak ditemukan";
    }

    return response{res};
}

response lihatRwtKeluar(){
    ifstream fileKendaraan("src/database/riwayatKendaraan.txt");
    json::wvalue::list result;
    if (fileKendaraan.is_open())
    {
        string line;
        while (getline(fileKendaraan, line))
        {
            stringstream ss(line);
            string id, tipe, plat, harga, waktuM, waktuK;
            if (!getline(ss, id, '|') || !getline(ss, tipe, '|') || !getline(ss, plat, '|') || !getline(ss, harga, '|') || !getline(ss, waktuM, '|') || !getline(ss, waktuK, '|'))
            {
                continue;
            }
            json::wvalue item;
            item["id_kendaraan"] = id;
            item["tipe_kendaraan"] = tipe;
            item["plat_kendaraan"] = plat;
            item["biaya_parkir"] = harga;
            item["waktu_masuk"] = waktuM;
            item["waktu_keluar"] = waktuK;
            result.emplace_back(move(item));
        }
    } else {
        cout << "Terjadi kesalahan" << endl;
    }
    fileKendaraan.close();
    if (result.empty())
    { 
        json::wvalue res;
        res["cekBol"] = true;
        res["message"] = "Belum ada kendaraan Keluar";
        return response{res};
    } else {
        json::wvalue res;
        res["data"] = move(result);
        return response{res};
    }
}