#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void obradi_sigusr1();        // prekid prioriteta 1
void obradi_sigusr2();        // prekid prioriteta 2
void obradi_sigterm();        // prekid prioriteta 3
void obradi_sigint();         // prekid prioriteta 4
void obradi_prekid(int sig);  // glavni izbornik
void blokiraj_odblokiraj_signale(int blokiraj);
void obradi_kraj();  // kraj programa

int kraj = 0;
int OZNAKA_CEKANJA[4] = {0};
int KON[4] = {0};
int tekuci_p = 0;

int main() 
{
   struct sigaction act;

   // 1. maskiranje signala SIGUSR1
   act.sa_handler = obradi_prekid;
   sigemptyset(&act.sa_mask);
   act.sa_flags = 0;
   sigaction(SIGUSR1, &act, NULL);

   // 2. maskiranje signala SIGUSR2
   act.sa_handler = obradi_prekid;
   sigemptyset(&act.sa_mask);
   sigaction(SIGUSR2, &act, NULL);

   // 3. maskiranje signala SIGTERM
   act.sa_handler = obradi_prekid;
   sigemptyset(&act.sa_mask);
   sigaction(SIGTERM, &act, NULL);

   // 4. maskiranje signala SIGINT
   act.sa_handler = obradi_prekid;
   sigemptyset(&act.sa_mask);
   sigaction(SIGINT, &act, NULL);

   // 5. maskiranje signala SIGQUIT
   act.sa_handler = obradi_kraj;
   sigemptyset(&act.sa_mask);
   sigaction(SIGQUIT, &act, NULL);

   printf("Program (PID = %ld) krenuo s radom\n", (long)getpid());

   int k = 1;
   while (!kraj) {
      printf("Glavni program (PID = %ld): iteracija %d\n", (long)getpid(), k++);
      sleep(1);
   }

   printf("Program (PID = %ld) zavrsio s radom\n", (long)getpid());

   return 0;
}

void obradi_prekid(int sig) 
{
   int i, index;

   if (sig == SIGUSR1) {
      blokiraj_odblokiraj_signale(1);
      printf("\nPrihvat prekida (prioritet = 1)...\n");
      for (int j = 0; j < 2; j++) {
         sleep(1);  // čekanje dvije sekunde
      }
      blokiraj_odblokiraj_signale(0);
      index = 0;
   }
   if (sig == SIGUSR2) {
      blokiraj_odblokiraj_signale(1);
      printf("\nPrihvat prekida (prioritet = 2)...\n");
      for (int j = 0; j < 2; j++) {
         sleep(1);  // čekanje dvije sekunde
      }
      blokiraj_odblokiraj_signale(0);
      index = 1;
   }
   if (sig == SIGTERM) {
      blokiraj_odblokiraj_signale(1);
      printf("\nPrihvat prekida (prioritet = 3)...\n");
      for (int j = 0; j < 2; j++) {
         sleep(1);  // čekanje dvije sekunde
      }
      blokiraj_odblokiraj_signale(0);
      index = 2;
   }
   if (sig == SIGINT) {
      blokiraj_odblokiraj_signale(1);
      printf("\nPrihvat prekida (prioritet = 4)...\n");
      for (int j = 0; j < 2; j++) {
         sleep(1);  // čekanje dvije sekunde
      }
      blokiraj_odblokiraj_signale(0);
      index = 3;
   }

   OZNAKA_CEKANJA[index] = 1;
   printf("Dignuta zastavica K_Z[%d] = %d\n", index + 1, OZNAKA_CEKANJA[index]);

   for (int j = 0; j < 4; j++) {
      if (OZNAKA_CEKANJA[j] != 0) {
         i = j + 1;
      }
   }

   // tekuci_p = 0;
   while (i > tekuci_p) {
      printf("Oznake čekanja:\n");
      for (int j = 0; j < 4; j++) {
         printf(" K_Z[%d] = %d\n", j + 1, OZNAKA_CEKANJA[j]);
      }
      OZNAKA_CEKANJA[i - 1] = 0;  // spuštam oznaku čekanja max_prioriteta
      KON[i - 1] = tekuci_p;      // spremanje tekuceg prioriteta u kontekst
      tekuci_p = i;               // tekuci prioritet postaje najveci prioritet

      blokiraj_odblokiraj_signale(0);  // 0 - omogući

      if (tekuci_p == 1) {
         printf("-----------\n");
         printf("Tekuci prioritet: %d\n", tekuci_p);
         printf("Kontekst:\n");
         for (int j = 0; j < 4; j++) {
            printf(" KON[%d] = %d\n", j + 1, KON[j]);
         }
         printf("-----------\n");
         obradi_sigusr1();
         printf("\nPovratak iz prekida (prioritet = 1)...\n");
         for (int j = 0; j < 2; j++) {
            sleep(1);
         }

      } else if (tekuci_p == 2) {
         printf("-----------\n");
         printf("Tekuci prioritet: %d\n", tekuci_p);
         printf("Kontekst:\n");
         for (int j = 0; j < 4; j++) {
            printf(" KON[%d] = %d\n", j + 1, KON[j]);
         }
         printf("-----------\n");
         obradi_sigusr2();
         printf("\nPovratak iz prekida (prioritet = 2)...\n");
         for (int j = 0; j < 2; j++) {
            sleep(1);
         }

      } else if (tekuci_p == 3) {
         printf("-----------\n");
         printf("Tekuci prioritet: %d\n", tekuci_p);
         printf("Kontekst:\n");
         for (int j = 0; j < 4; j++) {
            printf(" KON[%d] = %d\n", j + 1, KON[j]);
         }
         printf("-----------\n");
         obradi_sigterm();
         printf("\nPovratak iz prekida (prioritet = 3)...\n");
         for (int j = 0; j < 2; j++) {
            sleep(1);
         }
      } else if (tekuci_p == 4) {
         printf("-----------\n");
         printf("Tekuci prioritet: %d\n", tekuci_p);
         printf("Kontekst:\n");
         for (int j = 0; j < 4; j++) {
            printf(" KON[%d] = %d\n", j + 1, KON[j]);
         }
         printf("-----------\n");
         obradi_sigint();
         printf("\nPovratak iz prekida (prioritet = 4)...\n");
         for (int j = 0; j < 2; j++) {
            sleep(1);
         }
      }

      blokiraj_odblokiraj_signale(1);  // 1 - onemogući

      tekuci_p = KON[i - 1];
      KON[i - 1] = 0;
      i = 0;

      for (int j = 0; j < 4; j++) {
         if (OZNAKA_CEKANJA[j] != 0) {
            i = j + 1;
         }
      }
      printf("-----------\n");
      printf("Tekuci prioritet: %d\n", tekuci_p);
      printf("Kontekst:\n");
      for (int j = 0; j < 4; j++) {
         printf(" KON[%d] = %d\n", j + 1, KON[j]);
      }
      printf("-----------\n");
   }
}

void obradi_sigusr1() 
{
   for (int j = 1; j <= 10; j++) {
      printf("Obrada prekida (prioritet = 1): %d/10\n", j);
      sleep(1);  // čeka se sekundu između obrade
   }
}

void obradi_sigusr2() 
{
   for (int j = 1; j <= 10; j++) {
      printf("Obrada prekida (prioritet = 2): %d/10\n", j);
      sleep(1);  // čeka se sekundu između obrade
   }
}

void obradi_sigterm() 
{
   for (int j = 1; j <= 10; j++) {
      printf("Obrada prekida (prioritet = 3): %d/10\n", j);
      sleep(1);  // čeka se sekundu između obrade
   }
}

void obradi_sigint() 
{
   for (int j = 1; j <= 10; j++) {
      printf("Obrada prekida (prioritet = 4): %d/10\n", j);
      sleep(1);  // čeka se sekundu između obrade
   }
}

void obradi_kraj() 
{
   printf("Primljen signal SIGQUIT, pospremam prije izlaska...\n");
   kraj = 1;
}

void blokiraj_odblokiraj_signale(int blokiraj) 
{
   sigset_t signali;
   sigemptyset(&signali);
   sigaddset(&signali, SIGUSR1);
   sigaddset(&signali, SIGUSR2);
   sigaddset(&signali, SIGTERM);
   sigaddset(&signali, SIGINT);
   if (blokiraj == 1)
      pthread_sigmask(SIG_BLOCK, &signali, NULL);
   else
      pthread_sigmask(SIG_UNBLOCK, &signali, NULL);
}
