#include <stdlib.h>
#include <GL/glut.h>
#include <assert.h>
#include <fstream>


#ifndef IMAGE_LOADER_H_INCLUDED
#define IMAGE_LOADER_H_INCLUDED


//Bir g�r�nt�y� temsil eder. ( �rnek vermek gerekirse Header Files'in i�indeki fotograf.h dosyas� )
class Fotograf { 

public:

	Fotograf(char* pixelOlusturma, int genislikOlusturma, int yukseklikOlusturma);

	~Fotograf();	
	/* Fotograftaki her pikselin rengini g�steren bir diziyi ifade eder. 
	*  form (R1, G1, B1, R2, G2, B2, ...)
	*  Renk bile�enleri 0 ila 255 aras�ndad�r.
	*/
	
	int tYan;
	char* tPixel;
	int tUst;
};


Fotograf* bmpOlusturma(const char* dosyaAdi); //Dosyadan bir bitmap g�r�nt�s� okur.

#endif

GLfloat piramitAci = 0.0f; // Pramitin d�nme a��s� (GLfloat-->GL'in kendi float'� )
						   //float piramitAci=0.0;
GLfloat kupAci = 0.0f; // K�b�n d�nme a��s�


int t_yenileme = 20; // milisaniye cinsinden yenileme aral���	

using namespace std;


float xHareket = 0, yHareket = 0, zHareket = 0, xHareket2 = 0, yHareket2 = 0, zHareket2 = 0;	// Pozisyonlar� s�f�rl�yoruz.

int en = 300, boy = 300;	//Boyutland�rma


GLuint fotografId1;	// Kaplama olu�turma
GLuint fotografId2;	// Kaplama olu�turma

void Ayarlar() {
	glClearColor(0.3f, 0.4f, 0.9f, 1.0f); /* opaque bir Kaplama Alan�-mavi. Penceremizin arka alan
										  rengidir; asl�nda ekran�n hangi renkle temizlenece�ini belirtir.*/
	glClearDepth(1.0f); /*glClearDepth fonksiyonunu 1.0 de�eri ile �a��rarak derinlik
						tamponunun temizlenmesini sa�l�yoruz. 1.0 ile Arka plan derinli�ini en uza�a ayarlan�yor*/
	glEnable(GL_DEPTH_TEST); //z-ekseni i�in derinlik testi
							 /* Derinlik testinin yap�labilmesi i�in GL_DEPTH_TEST sembolik sabitini, glEnable fonksiyonuna
							 parametre olarak ge�erek �a��r�yoruz.*/
	glDepthFunc(GL_LEQUAL); // Karma��k grafiklerde Derinlik testi i�in kullan�l�r:
							/*Daha sonra glDepthFunc fonksiyonu ile derinlik testinde kullan�lacak fonksiyonu belirtiyoruz.
							Burada parametre olarak ge�ilen GL_LEQUAL sembolik sabitinin belirtti�i derinlik testi
							fonksiyonu, o an gelen z de�eri, tamponda saklanan z de�erine e�it ise veya ondan daha k���kse
							ba�ar�l� olur.*/
	glShadeModel(GL_SMOOTH); // karma��k grafiklerde D�zg�n g�lgelendirmeyi etkinle�tirir
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); /* karma��k grafiklerde perspektif
													   ayarlamalar�n�(d�zeltmelerini) yapar.*/
}
/* Pencere yeniden boyama fonksiyonu olu�turuyoruz. Pencere ilk g�r�nt�lendi�inde ve pencerenin
yeniden boyanmas� gerekti�inde �a�r�l�r. */



Fotograf::Fotograf(char* pixelOlusturma, int genislikOlusturma, int yukseklikOlusturma) : 
	tPixel(pixelOlusturma), tYan(genislikOlusturma), tUst(yukseklikOlusturma) {

}

Fotograf::~Fotograf() {
	delete[] tPixel;
}

namespace {
	//Form kullanarak d�rt karakterlik bir diziyi tamsay�ya d�n��t�r�r
	int intCevirme(const char* byteTanim) {
		return (int)(((unsigned char)byteTanim[3] << 24) |
			((unsigned char)byteTanim[2] << 16) |
			((unsigned char)byteTanim[1] << 8) |
			(unsigned char)byteTanim[0]);
	}

	//Form kullanarak iki karakterlik bir diziyi k�sa karaktere d�n��t�r�r
	short shortCevirme(const char* byteTanim) {
		return (short)(((unsigned char)byteTanim[1] << 8) |
			(unsigned char)byteTanim[0]);
	}

	//Form kullanarak sonraki d�rt bayt� tam say� olarak okur
	int intOkuma(ifstream &input) {
		char buffer[4];
		input.read(buffer, 4);
		return intCevirme(buffer);
	}

	//Sonraki iki bayt� form bi�imini kullanarak short olarak okur
	short shortOkuma(ifstream &input) {
		char buffer[2];
		input.read(buffer, 2);
		return shortCevirme(buffer);
	}

	//otomatik_pixel ile ayn�, fakat bu diziler i�in
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

// BMP �al��t�rma ve hata kontrol k�sm�
Fotograf* bmpOlusturma(const char* dosyaAdi) {
	ifstream input;
	input.open(dosyaAdi, ifstream::binary);
	assert(!input.fail() || !"B�yle bir dosya yok");
	char buffer[2];
	input.read(buffer, 2);
	assert(buffer[0] == 'B' && buffer[1] == 'M' || !"Bitmap ile ayn� de�il");
	input.ignore(8);
	int dataOffset = intOkuma(input);

	//Header k�sm�n� oku. " header k�sm� 120 sat�r "
	int basBoyut = intOkuma(input);
	int tYan;
	int tUst;
	switch (basBoyut) {
	case 40:
		//pixel olu�turma ve s�k��t�rma i�lemleri
		tYan = intOkuma(input);
		tUst = intOkuma(input);
		input.ignore(2);
		assert(shortOkuma(input) == 24 || !"Fotograf piksel ba��na 24 bit de�il");
		assert(shortOkuma(input) == 0 || !"Fotograf s�k��t�r�lm��");
		break;
	case 12:
		//
		tYan = shortOkuma(input);
		tUst = shortOkuma(input);
		input.ignore(2);
		assert(shortOkuma(input) == 24 || !"Fotograf piksel ba��na 24 bit de�il");
		break;
	case 64:
		
		assert(!"bitmap y�klenemiyor");
		break;
	case 108:
		
		assert(!"bitmap y�klenemiyor");
		break;
	case 124:
		
		assert(!"bitmap y�klenemiyor");
		break;
	default:
		assert(!"Bilinmeyen bitmap bi�imi");
	}

	//Veri okuma i�lemi
	int bitSatirlari = ((tYan * 3 + 3) / 4) * 4 - (tYan * 3 % 4);
	int boyut = bitSatirlari * tUst;
	diziOlusturma<char> tPixel(new char[boyut]);
	input.seekg(dataOffset, ios_base::beg);
	input.read(tPixel.get(), boyut);

	//Verileri do�ru bi�ime getirme i�lemi
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
{//yaln�z �zel i�lev tu�lar�n� kullanabiliriz-->F1, 	Insert, ok tu�lar� gibi
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
	glutPostRedisplay();//hareket(i�lem) sonras� g�ncelleme i�in
}

void klavye2(unsigned char dugme, int x, int y)
{//yaln�z karakter tu�lar�n� kullanabiliriz
	switch (dugme)
	{
	case 'w':hareketYukari2(); break;
	case's':hareketAsagi2(); break;
	case'a':hareketSol2(); break;
	case'd':hareketSag2(); break;
	case'+':hareketOn2(); break;
	case'-':hareketArka2(); break;
	}
	glutPostRedisplay();//hareket(i�lem) sonras�
}


//Kedi ve k�pek fotograflar�n�n ciktisini verir.
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
														tampon bile�enlerini silmek icin kullanilmaktadir.Tampon ile kastimiz bellekte ki bir is icin
														ayr�lm�� belirli bir hafiza parcasidir. Bir cizimin kirmizi,yesil,ve mavi bileseleri renk tamponu
														ve ya piksel tamponu olarak adlandirilir.*/
	glMatrixMode(GL_MODELVIEW); /* Kameran�n pozisyonu ve cisimlerin 3D geometrisi ile ilgili
								�izme, g�r�nt�leme, konumland�rma ve y�nlendirme i�lemlerinden �nce matrislerin hangi i�lem i�in
								d�zenlenece�i belirtilmelidir: bu �rnekte -->model-view matrisi kulan�l�yor*/
								// piramit i�in 4 ��gen laz�m


	glLoadIdentity(); // Orjinal Duruma d�n�l�r (Reset)
	
	glTranslatef(1.5f, 0.0f, -6.0f);
	
		glTranslatef(xHareket, yHareket, zHareket); 
	
	
		
	
	glRotatef(piramitAci, 1.0f, 1.0f, 0.0f);
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, fotografId2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_TRIANGLES); // piramit i�in 4 ��gen laz�m
						   // �n

	glTexCoord2f(0.5f, 1.0f); // Kaplama Alana
	glVertex3f(0.0f, 1.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f);// Kaplama Alana
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0f, 0.0f); // Kaplama Alana
	glVertex3f(1.0f, -1.0f, 1.0f);

	// sa�
	glTexCoord2f(0.5f, 1.0f); // Kaplama Alana
	glVertex3f(0.0f, 1.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f); // Kaplama Alana
	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0f, 0.0f); // Kaplama Alan�
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
	glEnd(); // Piramit �izimi bitti
			 // Kenarlar� Farkl� renklerde olan renkli k�p yapal�m
	
	glLoadIdentity(); // Orjinal Duruma d�n�l�r (Reset) (Reset model-view matrix)
	glTranslatef(-2.0f, 0.0f, -7.0f); // �ekli sa�a ve geriye kayd�r

	glTranslatef(xHareket2, yHareket2, zHareket2); // sola ve geriye �tele
	
	
	glRotatef(kupAci, 1.0f, 1.0f, 1.0f); // Rotate (1,0,0)-axis (x-ekseni etraf�nda d�n��)
	

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, fotografId1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glColor3f(1.0f, 1.0f, 1.0f);

	glBegin(GL_QUADS); // K�p i�in 6 kare-y�z laz�m
					   // �st y�z (y = 1.0f)
					   // k��eleri saat y�n�n�n tersine s�ralayaca��z
	 
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	// Alt y�z (y = -1.0f)


	
	glTexCoord2f(0.0f, 0.0f); // Kaplama Alana
	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	
	// �n y�z (z = 1.0f)

	glTexCoord2f(0.0f, 0.0f); // Kaplama Alana
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	
	
	// Arka y�z (z = -1.0f)

	glTexCoord2f(0.0f, 0.0f); // Kaplama Alana
	glVertex3f(1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);

	// sol y�z (x = -1.0f)
	
	glTexCoord2f(0.0f, 0.0f); // Kaplama Alana
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	
	// sa� y�z (x = 1.0f)
	glTexCoord2f(0.0f, 0.0f); // Kaplama Alana
	glVertex3f(1.0f, 1.0f, -1.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glEnd(); // Renkli k�p �iziminin sonu
			 //glflush(); -->Yetmez, glutSwapBuffers()laz�m
	glutSwapBuffers(); /*�er�eveyi �izdikten sonra �n ve arka tamponlar de�i�ir. Burada, biri
					   g�sterilen di�eri gizlenen, �ift tampon modunda pencere kullan�yoruz. Bu durumda OpenGL
					   komutlar�n�n g�r�nt�lenmesi daima gizli tampon i�ine yap�l�r. glutSwapBuffers(), �a�r�ld���nda,
					   tamponlar� de�i�toku� ederek �izileni pencere i�inde g�r�nt�ler. Tercih edilmesinin nedeni,
					   �er�eve i�eri�inin sat�r sat�r olu�umunu insan g�z�nden saklamas�d�r*/
	piramitAci += 0.1;
	kupAci -= 0.1f;
}
void zamanFonk(int value)
{
	glutPostRedisplay(); //cizimFonk()'nun yapt��� i�lem sonras� g�ncelleme i�in
	glutTimerFunc(t_yenileme, zamanFonk, 0); // g�ncelleme
}


/*pencerenin en boy oran�n� i�in: */
void gorunumFonk(GLsizei x, GLsizei y) //x:geni�lik, y:y�kseklik
{ // "GLsizei"(negatif olmayan tamsay� demektir. int tYan; diyebilirdik)
  // Yeni pencerenin en boy oran�n� hesaplama
	if (y == 0) y = 1; // 0'a b�l�mden korunmak i�in
	GLfloat oran = (GLfloat)x / (GLfloat)y; //float oran; diyebilirdik
											/* Genellikle pencere geni�li�i ile ayn�d�r. E�er viewport geni�lik / y�kseklik oran�
											projeksiyondakinden farkl� ise nesneler olduklar�ndan farkl� bi�imde g�r�nebilirler*/
	glViewport(0, 0, x, y);
	// en boy oran�n� g�r�n�m penceresiyle e�le�ecek �ekilde ayarlama:
	glMatrixMode(GL_PROJECTION); // projeksiyon (izd���m) matrisi
								 //izd���m g�r�n�m�ne ayarlamak i�in
	glLoadIdentity(); // Orjinal Duruma d�n�l�r (Reset)
					  // Perspectif projeksiyon yapar--> gluPerspective (theta, aspect, dnear, dfar)
	gluPerspective(45.0f, oran, 1.0f, 20.0f);
	//Bu fonksiyondaki theta perspektifin yapt��� d�n�� a��s�n�,
	//aspect en / boy oran�n�,
	//dnear ve dfar ise s�ras�yla yak�n ve uzak perspektif panellerine olan uzakl��� vermektedir.
	//0-180 aras� olacak.Nesneye ne kadar yak�nsak o kadar dar a�� olmal�,
	//ne kadar uzaksak o kadar geni� a�� olmal�
	//(float)en/(float)boy --> tam ekran yap�ld���nda bozulmas�n diye
	//1.0 kameran�n modele(�okgene) en yak�n olabilece�i yer
	//20.0 kameran�n modele(�okgene) en uzak olabilece�i yer
}

/* GLUT, main()den ba�layarak bir konsol uygulamas� olarak �al���r */
int main(int argc, char** argv) {

	glutInit(&argc, argv); // GLUT'u ba�lat
	glutInitDisplayMode(GLUT_DOUBLE); // �ift tampon kullanma
	glutInitWindowPosition(75, 75); //pencere ba�lang�� noktas�
	glutInitWindowSize(1024, 768); // Pencere geni�lik ve y�kseklik ayarlayal�m
	glutCreateWindow("BG Final 201713172050 Aliosman Arslan"); //pencere ba�l���
	ciktiVermeFonk(); //cizimin ciktisini verir
	glutDisplayFunc(cizimFonk); // kendisine parametre olarak g�nderilen cizimFonk fonksiyonu �a��r�r ve �izimi yapar
	glutKeyboardFunc(klavye2);
	glutSpecialFunc(klavye); // klavyeden kontrol ayarlar�
	glutReshapeFunc(gorunumFonk); // E�er pencere yeniden boyutland�r�l�rsa yeni geni�lik ve y�kseklik de�erleri atan�r.
	Ayarlar(); // Kendi OpenGL Ayarlar�m�z
	glutTimerFunc(0, zamanFonk, 0); // zamanlay�c� �a�r�m�
	glutMainLoop(); // �izimi d�ng�ye sokar.

	return 0;
}