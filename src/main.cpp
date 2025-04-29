#include "function.h"

int main() {
    SimpleApp app;

    CROW_ROUTE(app, "/")([](){ return response(bacaFile("D:/Codingku/CobaCrow/public/templates/home.html"));});
    CROW_ROUTE(app, "/kendaraanMasuk")([](){return response( bacaFile("D:/Codingku/CobaCrow/public/templates/kendaraanMasuk.html")); });
    CROW_ROUTE(app, "/cekKendaraan")([](){ return response(bacaFile("D:/Codingku/CobaCrow/public/templates/cekKendaraan.html")); });
    CROW_ROUTE(app, "/cariKendaraan")([](){ return response(bacaFile("D:/Codingku/CobaCrow/public/templates/cariKendaraan.html")); });
    CROW_ROUTE(app, "/cariAsc")([](){ return response(bacaFile("D:/Codingku/CobaCrow/public/templates/cariAsc.html")); });
    CROW_ROUTE(app, "/cariDesc")([](){ return response(bacaFile("D:/Codingku/CobaCrow/public/templates/cariDesc.html")); });
    CROW_ROUTE(app, "/pengumuman")([](){ return response(bacaFile("D:/Codingku/CobaCrow/public/templates/pengumuman.html")); });
    CROW_ROUTE(app, "/isiPengumuman")([](){ return response(bacaFile("D:/Codingku/CobaCrow/public/templates/isiPengumuman.html")); });
    CROW_ROUTE(app, "/editPengumuman")([](){ return response(bacaFile("D:/Codingku/CobaCrow/public/templates/editPengumuman.html")); });
    CROW_ROUTE(app, "/yourTickets")([](){ return response(bacaFile("D:/Codingku/CobaCrow/public/client/yourTickets.html")); });
    CROW_ROUTE(app, "/pengumumanUser")([](){ return response(bacaFile("D:/Codingku/CobaCrow/public/client/pengumumanUser.html")); });
    CROW_ROUTE(app, "/kendaraanTidakDitemukan")([](){ return response(bacaFile("D:/Codingku/CobaCrow/public/client/kendaraanTidakDitemukan.html")); });
    CROW_ROUTE(app, "/dataParkirUser").methods(HTTPMethod::POST)([](const request& req) { return kendaraanMasuk(req); });
    CROW_ROUTE(app, "/lihatSemuaKendaraan").methods(HTTPMethod::GET)([]() { return kumpulanData(); });
    CROW_ROUTE(app, "/lihatAsc").methods(HTTPMethod::GET)([]() { return kumpulkanDataAsc(); });
    CROW_ROUTE(app, "/lihatDesc").methods(HTTPMethod::GET)([]() { return kumpulkanDataDesc(); });
    CROW_ROUTE(app, "/cariKendaraanAksi").methods(HTTPMethod::POST)([](const request& req) { return returnCekKendaraan(req); });
    CROW_ROUTE(app, "/dataPengumuman").methods(HTTPMethod::POST)([](const request& req) { return kirimDataPengumuman(req); });
    CROW_ROUTE(app, "/lihatPengumuman").methods(HTTPMethod::GET)([]() { return lihatPengumuman(); });
    CROW_ROUTE(app, "/publifikasi/<int>").methods(HTTPMethod::GET)([](int idPengumuman) { return publishPengumuman(idPengumuman); });
    CROW_ROUTE(app, "/unpublifikasi/<int>").methods(HTTPMethod::GET)([](int idPengumuman) { return unpublishPengumuman(idPengumuman); });
    CROW_ROUTE(app, "/lihatDataPengumuman/<int>").methods(HTTPMethod::GET)([](int idPengumuman) { return lihatDataPengumuman(idPengumuman); });
    CROW_ROUTE(app, "/hapusData/<int>").methods(HTTPMethod::GET)([](int idPengumuman) { return hapusPengumuman(idPengumuman); });
    CROW_ROUTE(app, "/editDataPengumuman/<int>").methods(HTTPMethod::POST)([](const request &req, int idPengumuman) { return editPengumuman(req, idPengumuman); });
    CROW_ROUTE(app, "/isiTiket/<string>").methods(HTTPMethod::GET)([](string idTiket) { return isiTiket(idTiket); });
    CROW_ROUTE(app, "/lihatKendaraanPribadi/<string>").methods(HTTPMethod::GET)([](string idKendaraanPribadi) { return lihatKendaraanPribadi(idKendaraanPribadi); });
    
    app.port(8080).multithreaded().run();
}

