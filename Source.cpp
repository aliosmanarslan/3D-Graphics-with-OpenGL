#include <stdlib.h>
#include <GL/glut.h>
#include <assert.h>
#include <fstream>


#ifndef IMAGE_LOADER_H_INCLUDED
#define IMAGE_LOADER_H_INCLUDED


//Bir görüntüyü temsil eder. ( Örnek vermek gerekirse Header Files'in içindeki fotograf.h dosyasý )
class Fotograf { 

public:

	Fotograf(char* pixelOlusturma, int genislikOlusturma, int yukseklikOlusturma);

	~Fotograf();	
	/* Fotograftaki her pikselin rengini gösteren bir diziyi ifade eder. 
	*  form (R1, G1, B1, R2, G2, B2, ...)
	*  Renk bileþenleri 0 ila 255 arasýndadýr.
	*/
	
	int tYan;
	char* tPixel;
	int tUst;
};


Fotograf* bmpOlusturma(const char* dosyaAdi); //Dosyadan bir bitmap görüntüsü okur.

#endif

GLfloat piramitAci = 0.0f; // Pramitin dönme açýsý (GLfloat-->GL'in kendi float'ý )
						   //float piramitAci=0.0;
GLfloat kupAci = 0.0f; // Kübün dönme açýsý


int t_yenileme = 20; // milisaniye cinsinden yenileme aralýðý	

using namespace std;


float xHareket = 0, yHareket = 0, zHareket = 0, xHareket2 = 0, yHareket2 = 0, zHareket2 = 0;	// Pozisyonlarý sýfýrlýyoruz.

int en = 300, boy = 300;	//Boyutlandýrma


GLuint fotografId1;	// Kaplama oluþturma
GLuint fotografId2;	// Kaplama oluþturma

void Ayarlar() {
	glClearColor(0.3f, 0.4f, 0.9f, 1.0f); /* opaque bir Kaplama Alaný-mavi. Penceremizin arka alan
										  rengidir; aslýnda ekranýn hangi renkle temizleneceðini belirtir.*/
	glClearDepth(1.0f); /*glClearDepth fonksiyonunu 1.0 deðeri ile çaðýrarak derinlik
						tamponunun temizlenmesini saðlýyoruz. 1.0 ile Arka plan derinliðini en uzaða ayarlanýyor*/
	glEnable(GL_DEPTH_TEST); //z-ekseni için derinlik testi
							 /* Derinlik testinin yapýlabilmesi için GL_DEPTH_TEST sembolik sabitini, glEnable fonksiyonuna
							 parametre olarak geçerek çaðýrýyoruz.*/
	glDepthFunc(GL_LEQUAL); // Karmaþýk grafiklerde Derinlik testi için kullanýlýr:
							/*Daha sonra glDepthFunc fonksiyonu ile derinlik testinde kullanýlacak fonksiyonu belirtiyoruz.
							Burada parametre olarak geçilen GL_LEQUAL sembolik sabitinin belirttiði derinlik testi
							fonksiyonu, o an gelen z deðeri, tamponda saklanan z deðerine eþit ise veya ondan daha küçükse
							baþarýlý olur.*/
	glShadeModel(GL_SMOOTH); // karmaþýk grafiklerde Düzgün gölgelendirmeyi etkinleþtirir
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); /* karmaþýk grafiklerde perspektif
													   ayarlamalarýný(düzeltmelerini) yapar.*/
}
/* Pencere yeniden boyama fonksiyonu oluþturuyoruz. Pencere ilk görüntülendiðinde ve pencerenin
yeniden boyanmasý gerektiðinde çaðrýlýr. */



Fotograf::Fotograf(char* pixelOlusturma, int genislikOlusturma, int yukseklikOlusturma) : 
	tPixel(pixelOlusturma), tYan(genislikOlusturma), tUst(yukseklikOlusturma) {

}

Fotograf::~Fotograf() {
	delete[] tPixel;
}

namespace {
	//Form kullanarak dört karakterlik bir diziyi tamsayýya dönüþtürür
	int intCevirme(const char* byteTanim) {
		return (int)(((unsigned char)byteTanim[3] << 24) |
			((unsigned char)byteTanim[2] << 16) |
			((unsigned char)byteTanim[1] << 8) |
			(unsigned char)byteTanim[0]);
	}

	//Form kullanarak iki karakterlik bir diziyi kýsa karaktere dönüþtürür
	short shortCevirme(const char* byteTanim) {
		return (short)(((unsigned char)byteTanim[1] << 8) |
			(unsigned char)byteTanim[0]);
	}

	//Form kullanarak sonraki dört baytý tam sayý olarak okur
	int intOkuma(ifstream &input) {
		char buffer[4];
		input.read(buffer, 4);
		return intCevirme(buffer);
	}

	//Sonraki iki baytý form biçimini kullanarak short olarak okur
	short shortOkuma(ifstream &input) {
		char buffer[2];
		input.read(buffer, 2);
		return shortCevirme(buffer);
	}

	//otomatik_pixel ile ayný, fakat bu diziler için
	template<class diziPixel>
	class diziOlusturma {
	private:
		diziPixel* array;
		mutable bool ciktiVerme;
	public:
		explicit diziOlusturma(diziPixel* array_ = NULL) :
			array(array_), ciktiVerme(false) {
		}

		diziOlusturma(const diziOlusturma<diziPixel> &aarray) {
			array = aarray.array;
			ciktiVerme = aarray.ciktiVerme;
			aarray.ciktiVerme = true;
		}

		~diziOlusturma() {
			if (!ciktiVerme && array != NULL) {
				delete[] array;
			}
		}

		diziPixel* get() const {
			return array;
		}

		diziPixel &operator*() const {
			return *array;
		}

		void operator=(const diziOlusturma<diziPixel> &aarray) {
			if (!ciktiVerme && array != NULL) {
				delete[] array;
			}
			array = aarray.array;
			ciktiVerme = aarray.ciktiVerme;
			aarray.ciktiVerme = true;
		}

		diziPixel* operator->() const {
			return array;
		}

		diziPixel* release() {
			ciktiVerme = true;
			return array;
		}

		void reset(diziPixel* array_ = NULL) {
			if (!ciktiVerme && array != NULL) {
				delete[] array;
			}
			array = array_;
		}

		diziPixel* operator+(int i) {
			return array + i;
		}

		diziPixel &operator[](int i) {
			return array[i];
		}
	};
}

// BMP çalýþtýrma ve hata kontrol kýsmý
Fotograf* bmpOlusturma(const char* dosyaAdi) {
	ifstream input;
	input.open(dosyaAdi, ifstream::binary);
	assert(!input.fail() || !"Böyle bir dosya yok");
	char buffer[2];
	input.read(buffer, 2);
	assert(buffer[0] == 'B' && buffer[1] == 'M' || !"Bitmap ile ayný deðil");
	input.ignore(8);
	int dataOffset = intOkuma(input);

	//Header kýsmýný oku. " header kýsmý 120 satýr "
	int basBoyut = intOkuma(input);
	int tYan;
	int tUst;
	switch (basBoyut) {
	case 40:
		//pixel oluþturma ve sýkýþtýrma iþlemleri
		tYan = intOkuma(input);
		tUst = intOkuma(input);
		input.ignore(2);
		assert(shortOkuma(input) == 24 || !"Fotograf piksel baþýna 24 bit deðil");
		assert(shortOkuma(input) == 0 || !"Fotograf sýkýþtýrýlmýþ");
		break;
	case 12:
		//
		tYan = shortOkuma(input);
		tUst = shortOkuma(input);
		input.ignore(2);
		assert(shortOkuma(input) == 24 || !"Fotograf piksel baþýna 24 bit deðil");
		break;
	case 64:
		
		assert(!"bitmap yüklenemiyor");
		break;
	case 108:
		
		assert(!"bitmap yüklenemiyor");
		break;
	case 124:
		
		assert(!"bitmap yüklenemiyor");
		break;
	default:
		assert(!"Bilinmeyen bitmap biçimi");
	}

	//Veri okuma iþlemi
	int bitSatirlari = ((tYan * 3 + 3) / 4) * 4 - (tYan * 3 % 4);
	int boyut = bitSatirlari * tUst;
	diziOlusturma<char> tPixel(new char[boyut]);
	input.seekg(dataOffset, ios_base::beg);
	input.read(tPixel.get(), boyut);

	//Verileri doðru biçime getirme iþlemi
	diziOlusturma<char> pixelSize2(new char[tYan * tUst * 3]);
	for (int y = 0; y < tUst; y++) {
		for (int x = 0; x < tYan; x++) {
			for (int c = 0; c < 3; c++) {
				pixelSize2[3 * (tYan * y + x) + c] =
					tPixel[bitSatirlari * y + 3 * x + (2 - c)];
			}
		}
	}

	input.close();
	return new Fotograf(pixelSize2.release(), tYan, tUst);
}


void hareketSag(void)
{
	glLoadIdentity();		//koordinat sistemimiz etkilenmesin
		
		xHareket += 0.1;
	
}
void hareketSol(void)
{
	glLoadIdentity();
		
		xHareket -= 0.1;
	

}
void hareketYukari(void)
{
	glLoadIdentity();
		
		yHareket += 0.1;
	

}
void hareketAsagi(void)
{
	glLoadIdentity();
		
		yHareket -= 0.1;
	
}
void hareketArka(void)
{
	glLoadIdentity();
		
		zHareket -= 0.1;
	
}
void hareketOn(void)
{
	glLoadIdentity();

		zHareket += 0.1;
	

}

void hareketSag2(void)
{
	glLoadIdentity();		//koordinat sistemimiz etkilenmesin
							
	xHareket2 += 0.1;

}
void hareketSol2(void)
{
	glLoadIdentity();
	
	xHareket2 -= 0.1;


}
void hareketYukari2(void)
{
	glLoadIdentity();
	yHareket2 += 0.1;


}
void hareketAsagi2(void)
{
	glLoadIdentity();
	
	yHareket2 -= 0.1;

}
void hareketArka2(void)
{
	glLoadIdentity();
	
	zHareket2 -= 0.1;

}
void hareketOn2(void)
{
	glLoadIdentity();

	
	zHareket2 += 0.1;



}


//
GLuint kaplamaYukleme(Fotograf* image) {
	GLuint fotografId;
	glGenTextures(1, &fotografId);
	glBindTexture(GL_TEXTURE_2D, fotografId);
	glTexImage2D(GL_TEXTURE_2D,
		0,
		GL_RGB,
		image->tYan, image->tUst,
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		image->tPixel);
	return fotografId;
}

void klavye(int dugme, int x, int y)
{//yalnýz özel iþlev tuþlarýný kullanabiliriz-->F1, 	Insert, ok tuþlarý gibi
	switch (dugme)
	{
	case GLUT_KEY_LEFT:hareketSol(); break;
	case GLUT_KEY_RIGHT:hareketSag(); break;
	case GLUT_KEY_UP:hareketYukari(); break;
	case GLUT_KEY_DOWN:hareketAsagi(); break;
	case GLUT_KEY_PAGE_UP:hareketArka(); break;
	case GLUT_KEY_PAGE_DOWN:hareketOn(); break;

		break;
	}
	glutPostRedisplay();//hareket(iþlem) sonrasý güncelleme için
}

void klavye2(unsigned char dugme, int x, int y)
{//yalnýz karakter tuþlarýný kullanabiliriz
	switch (dugme)
	{
	case 'w':hareketYukari2(); break;
	case's':hareketAsagi2(); break;
	case'a':hareketSol2(); break;
	case'd':hareketSag2(); break;
	case'+':hareketOn2(); break;
	case'-':hareketArka2(); break;
	}
	glutPostRedisplay();//hareket(iþlem) sonrasý
}


//Kedi ve köpek fotograflarýnýn ciktisini verir.
void ciktiVermeFonk() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	Fotograf* image1 = bmpOlusturma("kedi.bmp");
	fotografId1 = kaplamaYukleme(image1);
	delete image1;
	Fotograf* image2 = bmpOlusturma("kopek.bmp");
	fotografId2 = kaplamaYukleme(image2);
	delete image2;
}

void cizimFonk()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); /* belirli bir bellek tamponunu veya
														tampon bileþenlerini silmek icin kullanilmaktadir.Tampon ile kastimiz bellekte ki bir is icin
														ayrýlmýþ belirli bir hafiza parcasidir. Bir cizimin kirmizi,yesil,ve mavi bileseleri renk tamponu
														ve ya piksel tamponu olarak adlandirilir.*/
	glMatrixMode(GL_MODELVIEW); /* Kameranýn pozisyonu ve cisimlerin 3D geometrisi ile ilgili
								çizme, görüntüleme, konumlandýrma ve yönlendirme iþlemlerinden önce matrislerin hangi iþlem için
								düzenleneceði belirtilmelidir: bu örnekte -->model-view matrisi kulanýlýyor*/
								// piramit için 4 üçgen lazým


	glLoadIdentity(); // Orjinal Duruma dönülür (Reset)
	
	glTranslatef(1.5f, 0.0f, -6.0f);
	
		glTranslatef(xHareket, yHareket, zHareket); 
	
	
		
	
	glRotatef(piramitAci, 1.0f, 1.0f, 0.0f);
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, fotografId2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_TRIANGLES); // piramit için 4 üçgen lazým
						   // Ön

	glTexCoord2f(0.5f, 1.0f); // Kaplama Alana
	glVertex3f(0.0f, 1.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f);// Kaplama Alana
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0f, 0.0f); // Kaplama Alana
	glVertex3f(1.0f, -1.0f, 1.0f);

	// sað
	glTexCoord2f(0.5f, 1.0f); // Kaplama Alana
	glVertex3f(0.0f, 1.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f); // Kaplama Alana
	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0f, 0.0f); // Kaplama Alaný
	glVertex3f(1.0f, -1.0f, -1.0f);
	// arka
	glTexCoord2f(0.5f, 1.0f); // Kaplama Alana
	glVertex3f(0.0f, 1.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f);// Kaplama Alana
	glVertex3f(1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f); // Kaplama Alana
	glVertex3f(-1.0f, -1.0f, -1.0f);
	// sol
	glTexCoord2f(0.5f, 1.0f); // Kaplama Alana
	glVertex3f(0.0f, 1.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f);// Kaplama Alana
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f); // Kaplama Alana
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glEnd(); // Piramit çizimi bitti
			 // Kenarlarý Farklý renklerde olan renkli küp yapalým
	
	glLoadIdentity(); // Orjinal Duruma dönülür (Reset) (Reset model-view matrix)
	glTranslatef(-2.0f, 0.0f, -7.0f); // þekli saða ve geriye kaydýr

	glTranslatef(xHareket2, yHareket2, zHareket2); // sola ve geriye ötele
	
	
	glRotatef(kupAci, 1.0f, 1.0f, 1.0f); // Rotate (1,0,0)-axis (x-ekseni etrafýnda dönüþ)
	

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, fotografId1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glColor3f(1.0f, 1.0f, 1.0f);

	glBegin(GL_QUADS); // Küp için 6 kare-yüz lazým
					   // üst yüz (y = 1.0f)
					   // köþeleri saat yönünün tersine sýralayacaðýz
	 
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	// Alt yüz (y = -1.0f)


	
	glTexCoord2f(0.0f, 0.0f); // Kaplama Alana
	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	
	// Ön yüz (z = 1.0f)

	glTexCoord2f(0.0f, 0.0f); // Kaplama Alana
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	
	
	// Arka yüz (z = -1.0f)

	glTexCoord2f(0.0f, 0.0f); // Kaplama Alana
	glVertex3f(1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);

	// sol yüz (x = -1.0f)
	
	glTexCoord2f(0.0f, 0.0f); // Kaplama Alana
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	
	// sað yüz (x = 1.0f)
	glTexCoord2f(0.0f, 0.0f); // Kaplama Alana
	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd(); // Renkli küp çiziminin sonu
			 //glflush(); -->Yetmez, glutSwapBuffers()lazým
	glutSwapBuffers(); /*Çerçeveyi çizdikten sonra Ön ve arka tamponlar deðiþir. Burada, biri
					   gösterilen diðeri gizlenen, çift tampon modunda pencere kullanýyoruz. Bu durumda OpenGL
					   komutlarýnýn görüntülenmesi daima gizli tampon içine yapýlýr. glutSwapBuffers(), çaðrýldýðýnda,
					   tamponlarý deðiþtokuþ ederek çizileni pencere içinde görüntüler. Tercih edilmesinin nedeni,
					   çerçeve içeriðinin satýr satýr oluþumunu insan gözünden saklamasýdýr*/
	piramitAci += 0.1;
	kupAci -= 0.1f;
}
void zamanFonk(int value)
{
	glutPostRedisplay(); //cizimFonk()'nun yaptýðý iþlem sonrasý güncelleme için
	glutTimerFunc(t_yenileme, zamanFonk, 0); // güncelleme
}


/*pencerenin en boy oranýný için: */
void gorunumFonk(GLsizei x, GLsizei y) //x:geniþlik, y:yükseklik
{ // "GLsizei"(negatif olmayan tamsayý demektir. int tYan; diyebilirdik)
  // Yeni pencerenin en boy oranýný hesaplama
	if (y == 0) y = 1; // 0'a bölümden korunmak için
	GLfloat oran = (GLfloat)x / (GLfloat)y; //float oran; diyebilirdik
											/* Genellikle pencere geniþliði ile aynýdýr. Eðer viewport geniþlik / yükseklik oraný
											projeksiyondakinden farklý ise nesneler olduklarýndan farklý biçimde görünebilirler*/
	glViewport(0, 0, x, y);
	// en boy oranýný görünüm penceresiyle eþleþecek þekilde ayarlama:
	glMatrixMode(GL_PROJECTION); // projeksiyon (izdüþüm) matrisi
								 //izdüþüm görünümüne ayarlamak için
	glLoadIdentity(); // Orjinal Duruma dönülür (Reset)
					  // Perspectif projeksiyon yapar--> gluPerspective (theta, aspect, dnear, dfar)
	gluPerspective(45.0f, oran, 1.0f, 20.0f);
	//Bu fonksiyondaki theta perspektifin yaptýðý dönüþ açýsýný,
	//aspect en / boy oranýný,
	//dnear ve dfar ise sýrasýyla yakýn ve uzak perspektif panellerine olan uzaklýðý vermektedir.
	//0-180 arasý olacak.Nesneye ne kadar yakýnsak o kadar dar açý olmalý,
	//ne kadar uzaksak o kadar geniþ açý olmalý
	//(float)en/(float)boy --> tam ekran yapýldýðýnda bozulmasýn diye
	//1.0 kameranýn modele(çokgene) en yakýn olabileceði yer
	//20.0 kameranýn modele(çokgene) en uzak olabileceði yer
}

/* GLUT, main()den baþlayarak bir konsol uygulamasý olarak çalýþýr */
int main(int argc, char** argv) {

	glutInit(&argc, argv); // GLUT'u baþlat
	glutInitDisplayMode(GLUT_DOUBLE); // çift tampon kullanma
	glutInitWindowPosition(75, 75); //pencere baþlangýç noktasý
	glutInitWindowSize(1024, 768); // Pencere geniþlik ve yükseklik ayarlayalým
	glutCreateWindow("BG Final 201713172050 Aliosman Arslan"); //pencere baþlýðý
	ciktiVermeFonk(); //cizimin ciktisini verir
	glutDisplayFunc(cizimFonk); // kendisine parametre olarak gönderilen cizimFonk fonksiyonu çaðýrýr ve çizimi yapar
	glutKeyboardFunc(klavye2);
	glutSpecialFunc(klavye); // klavyeden kontrol ayarlarý
	glutReshapeFunc(gorunumFonk); // Eðer pencere yeniden boyutlandýrýlýrsa yeni geniþlik ve yükseklik deðerleri atanýr.
	Ayarlar(); // Kendi OpenGL Ayarlarýmýz
	glutTimerFunc(0, zamanFonk, 0); // zamanlayýcý çaðrýmý
	glutMainLoop(); // çizimi döngüye sokar.

	return 0;
}