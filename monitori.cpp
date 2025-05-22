#include <pthread.h>
#include <unistd.h>

#include <iostream>
#include <list>
#include <mutex>

using namespace std;

// čitači čitaju sadržaj iste liste - mogu se izvesti paralelno
// pisač dodaje element na kraju liste
// umetanja moraju biti međusobno isključiva - pisači ne smiju paralelno
// umetanja se mogu obaviti paralelno s čitanjem liste (pisač i čitač paralelno)
// pretpostavka: dodani element se najprije inicijalizira, a onda doda u listu

pthread_mutex_t m;
pthread_cond_t redCitaca;
pthread_cond_t redPisaca;
pthread_cond_t redBrisaca;

list<int> lista;
list<int>::iterator it;

int brCitacaCeka = 0;
int brCitacaCita = 0;
int brBrisacaCeka = 0;
int brBrisacaBrise = 0;
int brPisacaCeka = 0;
int brPisacaPise = 0;

int procitajElementListe(int pozicija) {
   list<int>::iterator it2 = lista.begin();
   advance(it2, pozicija);
   return *it2;
}

void dodajListi(int arg) { lista.push_back(arg); }

void obrisiElementListe(int pozicija) 
{
   list<int>::iterator it2 = lista.begin();
   advance(it2, pozicija);
   lista.erase(it2);
}

void *citac(void *arg) 
{
   int id = *((int *)arg);
   while (true) {
      int x = rand() % lista.size();  // slučajni indeks nekog elementa liste
      pthread_mutex_lock(&m);
      it = next(lista.begin(), x);
      cout << "Citac " << id << " zeli citati element " << x << " liste\n";
      cout << "Aktivnih: citaca = " << brCitacaCita
           << ", pisaca = " << brPisacaPise << ", brisaca = " << brBrisacaBrise
           << endl;
      cout << "Lista: ";
      for (it = lista.begin(); it != lista.end(); it++) {
         cout << *it << " ";
      }
      cout << endl;
      brCitacaCeka++;
      while (brBrisacaBrise + brBrisacaCeka > 0) {
         pthread_cond_wait(&redCitaca, &m);
      }
      brCitacaCita++;
      brCitacaCeka--;
      int y = procitajElementListe(x);
      cout << "Citac " << id << " cita element " << x
           << " liste (vrijednost=" << y << ")\n";
      cout << "Aktivnih: citaca = " << brCitacaCita
           << ", pisaca = " << brPisacaPise << ", brisaca = " << brBrisacaBrise
           << endl;
      cout << "Lista: ";
      for (it = lista.begin(); it != lista.end(); it++) {
         cout << *it << " ";
      }
      cout << endl;
      pthread_mutex_unlock(&m);

      sleep(rand() % 6 + 5);

      pthread_mutex_lock(&m);
      brCitacaCita--;
      if (brCitacaCita == 0 && brBrisacaCeka > 0) {
         pthread_cond_signal(&redBrisaca);
      }
      cout << "Citac " << id << " vise ne koristi listu\n";
      cout << "Aktivnih: citaca = " << brCitacaCita
           << ", pisaca = " << brPisacaPise << ", brisaca = " << brBrisacaBrise
           << endl;
      cout << "Lista: ";
      for (it = lista.begin(); it != lista.end(); it++) {
         cout << *it << " ";
      }
      cout << endl;
      pthread_mutex_unlock(&m);

      sleep(rand() % 6 + 5);
   }
}

void *pisac(void *arg) 
{
   int id = *((int *)arg);
   while (true) {
      int x = rand() % 100 + 1;
      pthread_mutex_lock(&m);
      cout << "Pisac " << id << " zeli dodati vrijednost " << x << " u listu\n";
      cout << "Aktivnih: citaca = " << brCitacaCita
           << ", pisaca = " << brPisacaPise << ", brisaca = " << brBrisacaBrise
           << endl;
      cout << "Lista: ";
      for (it = lista.begin(); it != lista.end(); it++) {
         cout << *it << " ";
      }
      cout << endl;
      brPisacaCeka++;
      // kada neki pisac pise, niti jedan drugi pisac ne moze pisati
      // niti brisaci mogu brisati, ali citaci mogu citati
      while ((brBrisacaBrise + brBrisacaCeka > 0) || (brPisacaPise > 0)) {
         pthread_cond_wait(&redPisaca, &m);
      }
      brPisacaPise++;
      brPisacaCeka--;
      cout << "Pisac " << id << " zapocinje dodavanje vrijednosti " << x
           << " na kraj liste\n";
      cout << "Aktivnih: citaca = " << brCitacaCita
           << ", pisaca = " << brPisacaPise << ", brisaca = " << brBrisacaBrise
           << endl;
      cout << "Lista: ";
      for (it = lista.begin(); it != lista.end(); it++) {
         cout << *it << " ";
      }
      cout << endl;
      pthread_mutex_unlock(&m);

      sleep(rand() % 6 + 5);

      pthread_mutex_lock(&m);
      brPisacaPise--;
      if (brPisacaPise == 0 && brBrisacaCeka > 0) {
         pthread_cond_signal(&redBrisaca);
      } else if (brPisacaCeka > 0) {
         pthread_cond_signal(&redPisaca);
      }
      dodajListi(x);
      cout << "Pisac " << id << " dodao vrijednost " << x << " na kraj liste\n";
      cout << "Aktivnih: citaca = " << brCitacaCita
           << ", pisaca = " << brPisacaPise << ", brisaca = " << brBrisacaBrise
           << endl;
      cout << "Lista: ";
      for (it = lista.begin(); it != lista.end(); it++) {
         cout << *it << " ";
      }
      cout << endl;
      pthread_mutex_unlock(&m);

      sleep(rand() % 6 + 5);
   }
}

void *brisac(void *arg) 
{
   int id = *((int *)arg);
   while (true) {
      int x = rand() % lista.size();
      pthread_mutex_lock(&m);
      cout << "Brisac " << id << " zeli obrisati element " << x << " liste\n";
      cout << "Aktivnih: citaca = " << brCitacaCita
           << ", pisaca = " << brPisacaPise << ", brisaca = " << brBrisacaBrise
           << endl;
      cout << "Lista: ";
      for (it = lista.begin(); it != lista.end(); it++) {
         cout << *it << " ";
      }
      cout << endl;
      brBrisacaCeka++;
      while (brPisacaPise + brCitacaCita + brBrisacaBrise > 0) {
         pthread_cond_wait(&redBrisaca, &m);
      }
      brBrisacaBrise++;
      brBrisacaCeka--;
      int y = procitajElementListe(x);
      cout << "Brisac " << id << " zapocinje s brisanjem elementa " << x
           << " liste (vrijednost=" << y << ")\n";
      cout << "Aktivnih: citaca = " << brCitacaCita
           << ", pisaca = " << brPisacaPise << ", brisaca = " << brBrisacaBrise
           << endl;
      cout << "Lista: ";
      for (it = lista.begin(); it != lista.end(); it++) {
         cout << *it << " ";
      }
      cout << endl;
      pthread_mutex_unlock(&m);

      sleep(rand() % 6 + 5);

      pthread_mutex_lock(&m);
      obrisiElementListe(x);
      brBrisacaBrise--;
      if (brBrisacaBrise == 0 && brBrisacaCeka > 0) {
         pthread_cond_signal(&redBrisaca);
      } else if (brPisacaPise == 0 && brPisacaCeka > 0) {
         pthread_cond_signal(&redPisaca);
      } else if (brCitacaCeka > 0) {
         pthread_cond_signal(&redCitaca);
      }
      cout << "Brisac " << id << " obrisao element liste " << x
           << " (vrijednost=" << y << ")\n";
      cout << "Aktivnih: citaca = " << brCitacaCita
           << ", pisaca = " << brPisacaPise << ", brisaca = " << brBrisacaBrise
           << endl;
      cout << "Lista: ";
      for (it = lista.begin(); it != lista.end(); it++) {
         cout << *it << " ";
      }
      cout << endl;
      pthread_mutex_unlock(&m);

      sleep(rand() % 6 + 5);
   }
}

int main(void) 
{
   srand(time(NULL));
   pthread_mutex_init(&m, NULL);
   pthread_cond_init(&redCitaca, NULL);
   pthread_cond_init(&redPisaca, NULL);
   pthread_cond_init(&redBrisaca, NULL);

   pthread_t citaci[10];
   pthread_t pisaci[2];
   pthread_t brisaci;

   int id_citaca[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
   int id_pisaca[2] = {0, 1};
   int id_brisaca = 1;

   for (int i = 0; i < 2; i++) {
      pthread_create(&pisaci[i], NULL, pisac, (void *)&id_pisaca[i]);
   }
   sleep(25);
   for (int i = 0; i < 10; i++) {
      pthread_create(&citaci[i], NULL, citac, (void *)&id_citaca[i]);
   }
   sleep(10);
   pthread_create(&brisaci, NULL, brisac, (void *)&id_brisaca);

   for (int i = 0; i < 10; i++) {
      pthread_join(citaci[i], NULL);
   }
   for (int i = 0; i < 2; i++) {
      pthread_join(pisaci[i], NULL);
   }
   pthread_join(brisaci, NULL);

   pthread_mutex_destroy(&m);
   pthread_cond_destroy(&redCitaca);
   pthread_cond_destroy(&redPisaca);
   pthread_cond_destroy(&redBrisaca);

   return 0;
}
