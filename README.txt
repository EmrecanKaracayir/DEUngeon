Normal sartlar altinda buradaki .sln dosyasini Visual Studio ile
acarsaniz direk calistir diyerek calistirabilmeniz lazim.

Eger uyumsuzluk olursa diye ben onceden build aldim, asagida tam
nerede oldugu yaziyor.

Oyun exe           : "./out/DEUngeon/bin/Debug - x64/"
Kutuphane dll      : "./out/DEUngeon/bin/Debug - x64/"

(DLL dosyasi exe ile ayni yolda olmasi lazim)

Kod dosyalari     : "./projects/DEUngeon/src/"
Kutuphane headeri : "./projects/DEUngeon/include/"
Kutuphane lib     : "./projects/DEUngeon/lib/"

----------------------------------------------------------------

Main.cpp        : Pencerenin olusturulmasi ve motora devir.
Engine.hpp      : Ana oyun loop'u ve oyun degiskenleri
Map.hpp         : Harita ve tunelleri olusturma
Actor.hpp       : Oyun aktorleri, hareketleri ve degiskenleri
PathFinding.hpp : AStar algoritmasi ile yol bulma

----------------------------------------------------------------

SPACE ya da ENTER         : Oyunu duraklatir ya da baslatir.
ESC                       : Oyunu kapatir.
WASD ya da YON TUSLARI    : Karakteri hareket ettirir.
SOL SHIFT ya da SAG SHIFT : Atilma hareketini baslatir.