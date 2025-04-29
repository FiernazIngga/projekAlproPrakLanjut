const burger = document.querySelector('#hamburgerMenu');
const burgerJuga = document.querySelector('.burgerNav');
const containerMenu = document.querySelector('.containerMenu2');
const jPeng = document.querySelectorAll('.judul');
const laporKirim = document.querySelector('#kirimLaporan');

let active = false;
burger.addEventListener('click', () => {
    active = !active;
    active? burgerJuga.classList.add('active') : burgerJuga.classList.remove('active');
    active? containerMenu.style.height = '15em' : containerMenu.style.height = '0em';
});

function tiketKu() {
    const id = new URLSearchParams(window.location.search).get("id");
    window.location.href = `/yourTickets?id=${id}`;
}

function pengumumanKu() {
    const id = new URLSearchParams(window.location.search).get("id");
    window.location.href = `/pengumumanUser?id=${id}`;
}

function lapor() {
    const id = new URLSearchParams(window.location.search).get("id");
    window.location.href = `/lapor?id=${id}`;
}

document.addEventListener("DOMContentLoaded", function () {
    const checkbox = document.getElementById("anonymousCheckbox");
    const kontakGroup = document.getElementById("kontakGroup");

    checkbox.addEventListener("change", function () {
        if (checkbox.checked) {
            kontakGroup.style.display = "none";
        } else {
            kontakGroup.style.display = "block";
        }
    })
})

laporKirim.addEventListener('click', () => {
    const judulLaporan = document.querySelector('#judulLapor').value;
    const deskripsiLapor = document.querySelector('#deskripsiLapor').value;
    const kontak1 = document.querySelector('#kontak1').value;
    const kontak2 = document.querySelector('#kontak2').value;
    if (!judulLaporan && !deskripsiLapor && !kontak1) {
        alert("Patikan Judul Laporan, Deskripsi, dan Kontak 1 sudah terisi")
    }
})