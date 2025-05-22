#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <queue>

using namespace std;

int bud, brd, bid, velicinaMS;

struct MS {
   char *podaci;
   sem_t BSEM;
   sem_t OSEM;
   int ULAZ;   // pokazuje na mjesto gdje ce se UPISAT podatak
   int IZLAZ;  // pokazuje na mjesto od kojeg se CITA podatak
   char posljednjiPodatak = '0';
};

vector<MS> UMS;
vector<MS> IMS;
sem_t BSEM_ispis;

char dohvati_ulaz(int I) 
{
   return 'A' + (rand() % 26);  // vraća slučajna slova između A i Z
}

char obradi_ulaz(int I, char U) { return rand() % brd; }

void obradi(int J, char P, char *r, int *t) 
{
   *r = tolower(P);
   *t = rand() % bid;
   sleep(rand() % 2 + 2);
}

void *ulazna_dretva(void *arg) 
{
   int I = *((int *)arg);
   while (true) {
      char U = dohvati_ulaz(I);   // neko slovo između 'a' i 'z'
      int T = obradi_ulaz(I, U);  // daje index UMS-a u koji stavljamo slovo

      sem_wait(&UMS[T].BSEM);  // ulazak u KO
      UMS[T].podaci[UMS[T].ULAZ] = U;
      UMS[T].ULAZ = (UMS[T].ULAZ + 1) % velicinaMS;

      if (UMS[T].ULAZ != UMS[T].IZLAZ) {
         sem_post(&UMS[T].OSEM);  // postavi OSEM
      }

      if (UMS[T].ULAZ == UMS[T].IZLAZ) {
         UMS[T].podaci[UMS[T].ULAZ] = U;
         UMS[T].IZLAZ = (UMS[T].IZLAZ + 1) % velicinaMS;
      }
      sem_post(&UMS[T].BSEM);  // izlazak iz KO

      sem_wait(&BSEM_ispis);
      cout << "\nU" << I << ": dohvati_ulaz('" << I << "')=>'" << U << "'; ";
      cout << "obradi_ulaz('" << U << "')=>" << T << "; ";
      cout << "'" << U << "' => UMS[" << T << "]";
      cout << "\nUMS[]:";
      for (int i = 0; i < brd; i++) {
         for (int j = 0; j < velicinaMS; j++) {
            char c = UMS[i].podaci[j];
            cout << c;
         }
         cout << " ";
      }
      cout << "\nIMS[]:";
      for (int i = 0; i < bid; i++) {
         for (int j = 0; j < velicinaMS; j++) {
            char c = IMS[i].podaci[j];
            cout << c;
         }
         cout << " ";
      }
      sem_post(&BSEM_ispis);

      sleep(rand() % 6 + 5);  // 5-10 sek
   }
}

void *radna_dretva(void *arg) 
{
   int J = *((int *)arg);
   while (true) {
      sem_wait(&UMS[J].OSEM);  // ima li uopce podataka za citanje?
      sem_wait(&UMS[J].BSEM);  // ulazak u KO
      char P = UMS[J].podaci[UMS[J].IZLAZ];
      UMS[J].podaci[UMS[J].IZLAZ] = '-';
      UMS[J].IZLAZ = (UMS[J].IZLAZ + 1) % velicinaMS;
      char r;  // r = podatak koji smo obradili
      int t;   // t = koji IMS ćemo koristit
      obradi(J, P, &r, &t);
      sem_post(&UMS[J].BSEM);  // izlazak iz KO

      sem_wait(&IMS[t].BSEM);  // ulazak u KO
      IMS[t].podaci[IMS[t].ULAZ] = r;
      IMS[t].ULAZ = (IMS[t].ULAZ + 1) % velicinaMS;
      if (IMS[t].ULAZ == IMS[t].IZLAZ) {
         IMS[t].IZLAZ = (IMS[t].IZLAZ + 1) % velicinaMS;
      }
      sem_wait(&BSEM_ispis);
      cout << "\nR" << J << ": uzimam iz UMS[" << J << "]=>'" << P
           << "' i obradujem";
      cout << "\nUMS[]:";
      for (int i = 0; i < brd; i++) {
         for (int j = 0; j < velicinaMS; j++) {
            char c = UMS[i].podaci[j];
            cout << c;
         }
         cout << " ";
      }
      cout << "\nIMS[]:";
      for (int i = 0; i < bid; i++) {
         for (int j = 0; j < velicinaMS; j++) {
            char c = IMS[i].podaci[j];
            cout << c;
         }
         cout << " ";
      }
      sem_post(&BSEM_ispis);
      sem_post(&IMS[t].BSEM);  // izlazak iz KO
   }
}

void *izlazna_dretva(void *arg) 
{
   int K = *((int *)arg);
   while (true) {
      sem_wait(&IMS[K].BSEM);  // ulazak u KO
      char vrijednost;
      bool prazan = true;
      for (int i = 0; i < velicinaMS; i++) {
         if (IMS[K].podaci[i] >= 'a' && IMS[K].podaci[i] <= 'z') {
            prazan = false;
            break;
         }
      }
      vrijednost = IMS[K].posljednjiPodatak;
      if (!prazan && IMS[K].podaci[IMS[K].IZLAZ] != '-') {
         vrijednost = IMS[K].podaci[IMS[K].IZLAZ];
         IMS[K].posljednjiPodatak = vrijednost;
         IMS[K].podaci[IMS[K].IZLAZ] = '-';
         IMS[K].IZLAZ = (IMS[K].IZLAZ + 1) % velicinaMS;
      }
      if (IMS[K].podaci[IMS[K].IZLAZ] == '-') {
         vrijednost = IMS[K].posljednjiPodatak;
      }

      sem_wait(&BSEM_ispis);
      cout << "\nDretva " << K << " ispisuje " << vrijednost;
      sem_post(&BSEM_ispis);

      sem_post(&IMS[K].BSEM);  // izlazak iz KO

      sleep(3);
   }
}

int main(void) 
{
   srand(time(NULL));
   cout << "Unesite broj ulaznih dretvi: ";
   cin >> bud;
   cout << "Unesite broj radnih dretvi: ";
   cin >> brd;
   cout << "Unesite broj izlaznih dretvi: ";
   cin >> bid;
   cout << "Unesite velicinu meduspremnika: ";
   cin >> velicinaMS;

   pthread_t ulazne_dretve[bud], radne_dretve[brd], izlazne_dretve[bid];
   int i, idUlaznih[bud], idRadnih[brd], idIzlaznih[bid];

   UMS.reserve(brd);
   IMS.reserve(bid);

   sem_init(&BSEM_ispis, 0, 1);

   for (int j = 0; j < brd; j++) {
      sem_init(&UMS[j].BSEM, 0, 1);
      sem_init(&UMS[j].OSEM, 0, 0);
      UMS[j].ULAZ = 0;
      UMS[j].IZLAZ = 0;
      UMS[j].podaci = new char[velicinaMS];
      memset(UMS[j].podaci, '-', velicinaMS * sizeof(char));
   }
   for (int j = 0; j < bid; j++) {
      sem_init(&IMS[j].BSEM, 0, 1);
      IMS[j].ULAZ = 0;
      IMS[j].IZLAZ = 0;
      IMS[j].podaci = new char[velicinaMS];
      memset(IMS[j].podaci, '-', velicinaMS * sizeof(char));
   }
   for (i = 0; i < bud; i++) {
      idUlaznih[i] = i;
      pthread_create(&ulazne_dretve[i], NULL, ulazna_dretva, &idUlaznih[i]);
   }
   sleep(30);
   for (i = 0; i < brd; i++) {
      idRadnih[i] = i;
      pthread_create(&radne_dretve[i], NULL, radna_dretva, &idRadnih[i]);
   }
   sleep(10);
   for (i = 0; i < bid; i++) {
      idIzlaznih[i] = i;
      pthread_create(&izlazne_dretve[i], NULL, izlazna_dretva, &idIzlaznih[i]);
   }
   for (i = 0; i < bud; i++) {
      pthread_join(ulazne_dretve[i], NULL);
   }
   for (i = 0; i < brd; i++) {
      pthread_join(radne_dretve[i], NULL);
   }
   for (i = 0; i < bid; i++) {
      pthread_join(izlazne_dretve[i], NULL);
   }
   return 0;
}
