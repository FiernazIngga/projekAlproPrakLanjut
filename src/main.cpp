#include "function.h"

string cariBasePath() {
    string exePath = getExecutablePath();
    string base = exePath;
    for (int i = 0; i < 10; ++i) {
        if (fileExists(base + "/public/templates/home.html")) {
            return base;
        }
        base += "/..";
    }
    return "File tidak ditemukan.";
};

int main() {
    loadPengumuman();
    pindahKeStructKendaraan();
    loadLaporanPribadi();
    SimpleApp app;
    string basePath = cariBasePath();

    CROW_ROUTE(app, "/")([basePath](){ return response(bacaFile(basePath + "/public/templates/home.html"));});
    CROW_ROUTE(app, "/kendaraanMasuk")([basePath](){return response( bacaFile(basePath + "/public/templates/kendaraanMasuk.html")); });
    CROW_ROUTE(app, "/cekKendaraan")([basePath](){ return response(bacaFile(basePath + "/public/templates/cekKendaraan.html")); });
    CROW_ROUTE(app, "/cariKendaraan")([basePath](){ return response(bacaFile(basePath + "/public/templates/cariKendaraan.html")); });
    CROW_ROUTE(app, "/cariAsc")([basePath](){ return response(bacaFile(basePath + "/public/templates/cariAsc.html")); });
    CROW_ROUTE(app, "/cariDesc")([basePath](){ return response(bacaFile(basePath + "/public/templates/cariDesc.html")); });
    CROW_ROUTE(app, "/pengumuman")([basePath](){ return response(bacaFile(basePath + "/public/templates/pengumuman.html")); });
    CROW_ROUTE(app, "/isiPengumuman")([basePath](){ return response(bacaFile(basePath + "/public/templates/isiPengumuman.html")); });
    CROW_ROUTE(app, "/editPengumuman")([basePath](){ return response(bacaFile(basePath + "/public/templates/editPengumuman.html")); });
    CROW_ROUTE(app, "/pengumumanLaporPengguna")([basePath](){ return response(bacaFile(basePath + "/public/templates/pengumumanUser.html")); });
    CROW_ROUTE(app, "/yourTickets")([basePath](){ return response(bacaFile(basePath + "/public/client/yourTickets.html")); });
    CROW_ROUTE(app, "/pengumumanUser")([basePath](){ return response(bacaFile(basePath + "/public/client/pengumumanUser.html")); });
    CROW_ROUTE(app, "/kendaraanTidakDitemukan")([basePath](){ return response(bacaFile(basePath + "/public/client/kendaraanTidakDitemukan.html")); });
    CROW_ROUTE(app, "/lapor")([basePath](){ return response(bacaFile(basePath + "/public/client/lapor.html")); });
    CROW_ROUTE(app, "/kendaraanKeluar")([basePath](){ return response(bacaFile(basePath + "/public/templates/kendaraanKeluar.html")); });
    CROW_ROUTE(app, "/riwayatKendaraanKeluar")([basePath](){ return response(bacaFile(basePath + "/public/templates/riwayatKendaraanKeluar.html")); });
    CROW_ROUTE(app, "/dataParkirUser").methods(HTTPMethod::POST)([](const request& req) { return kendaraanMasuk(req); });
    CROW_ROUTE(app, "/lihatSemuaKendaraan").methods(HTTPMethod::GET)([]() { return kumpulanData(); });
    CROW_ROUTE(app, "/lihatSemuaKendaraanKeluar").methods(HTTPMethod::GET)([]() { return lihatRwtKeluar(); });
    CROW_ROUTE(app, "/lihatAsc").methods(HTTPMethod::GET)([]() { return kumpulkanDataAsc(); });
    CROW_ROUTE(app, "/lihatDesc").methods(HTTPMethod::GET)([]() { return kumpulkanDataDesc(); });
    CROW_ROUTE(app, "/cariKendaraanAksi").methods(HTTPMethod::POST)([](const request& req) { return returnCekKendaraan(req); });
    CROW_ROUTE(app, "/dataPengumuman").methods(HTTPMethod::POST)([](const request& req) { return kirimDataPengumuman(req); });
    CROW_ROUTE(app, "/lihatPengumuman").methods(HTTPMethod::GET)([]() { return lihatPengumuman(); });
    CROW_ROUTE(app, "/publifikasi/<int>").methods(HTTPMethod::GET)([](int idPengumuman) { return publishPengumuman(&idPengumuman); });
    CROW_ROUTE(app, "/unpublifikasi/<int>").methods(HTTPMethod::GET)([](int idPengumuman) { return unpublishPengumuman(&idPengumuman); });
    CROW_ROUTE(app, "/lihatDataPengumuman/<int>").methods(HTTPMethod::GET)([](int idPengumuman) { return lihatDataPengumuman(&idPengumuman); });
    CROW_ROUTE(app, "/hapusData/<int>").methods(HTTPMethod::GET)([](int idPengumuman) { return hapusPengumuman(&idPengumuman); });
    CROW_ROUTE(app, "/editDataPengumuman/<int>").methods(HTTPMethod::POST)([](const request &req, int idPengumuman) { return editPengumuman(req, &idPengumuman); });
    CROW_ROUTE(app, "/isiTiket/<string>").methods(HTTPMethod::GET)([](string idTiket) { return isiTiket(&idTiket); });
    CROW_ROUTE(app, "/lihatKendaraanPribadi/<string>").methods(HTTPMethod::GET)([](string idKendaraanPribadi) { return lihatKendaraanPribadi(&idKendaraanPribadi); });
    CROW_ROUTE(app, "/laporPribadi").methods(HTTPMethod::POST)([](const request &req) { return masukinLaporanPribadi(req); });
    CROW_ROUTE(app, "/lihatLaporanPribadi").methods(HTTPMethod::GET)([]() { return lihatLaporanPribadi(); });
    CROW_ROUTE(app, "/lihatDataLaporan/<string>").methods(HTTPMethod::GET)([](string idLaporanUser) { return lihatLaporanUser(&idLaporanUser); });
    CROW_ROUTE(app, "/aksiPenangananLaporan/<string>/<string>").methods(HTTPMethod::GET)([](string aksiLaporan, string idLaporan) { return publifikasiLaporan(&aksiLaporan, &idLaporan); });
    CROW_ROUTE(app, "/hapusPelaporanUser/<string>").methods(HTTPMethod::GET)([](string idLaporanUser) { return hapusPelaporanUser(&idLaporanUser); });
    CROW_ROUTE(app, "/kendaraanAkanKeluar").methods(HTTPMethod::POST)([](const request& req) { return kendaraanKeluarCari(req); });
    CROW_ROUTE(app, "/hapusKendaraanKeluar/<string>").methods(HTTPMethod::GET)([](string idPlat) { return bayarKendaraanKeluar(&idPlat); });
    
    app.port(8080).multithreaded().run();
}

