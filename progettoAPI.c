#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define LENGTH 256
#define DIMENSIONE 10
#define BLANK '_'

typedef struct TipoTransizione{
  int stato;
  char simbolo_letto;
  char simbolo_scritto;
  int nuovo_stato;
  char direzione;
}TipoTransizione;

typedef struct ListaTransizioni{
  TipoTransizione transizione;
  struct ListaTransizioni *next;
}ListaTransizioni;

typedef struct listaStatiAccettazione{
  int stato;
  struct listaStatiAccettazione *next;
}ListaStatiAccettazione;

typedef struct tipomatrice{
  int nuovo_stato;
  char simbolo_scritto;
  char direzione;
  struct tipomatrice *next;
}TipoMatrice;

typedef struct processo{
  int testina;
  char *nastroDX;
  char *nastroSX;
  int stato;
  int passi;
}TipoProcesso;

typedef struct listaprocessi{
  TipoProcesso processo;
  struct listaprocessi *next;
}ListaProcessi;

//prototipi
ListaTransizioni *acquisisciTransizioni(ListaTransizioni *, int *, int *);//acquisisco le transizioni in una lista,trovo max stato e i caratteri presenti
ListaTransizioni *aggiungiAllaListaTransizioni( ListaTransizioni *, TipoTransizione *);//inserisce la transizione nella lista
ListaTransizioni *deallocaListaTransizioni( ListaTransizioni *);//dealloca la mia lista di transizioni
ListaStatiAccettazione *creaListaAccettazione(ListaStatiAccettazione *, int *);//acquisisice gli stati accettazione e anche maxPassi
ListaStatiAccettazione *aggiungiAllaListaAccettazione(ListaStatiAccettazione *,int);//aggiumge alla lista lo stato
TipoMatrice **CostruisciMatrice (TipoMatrice **, int, ListaTransizioni *, int *); //costruisci la matrice
int processaStringa(char *,TipoMatrice **, ListaStatiAccettazione *,int *, int, int);//funzione per il run,elabora una stringa alla volta
ListaProcessi *inserisciInListaProcessi(ListaProcessi *,ListaProcessi *);//inserisce le configurazioni nella lista
int ControllaAccettazione (ListaStatiAccettazione *, int); //vede se sono in uno stato di accettazione
ListaProcessi *deallocaProcesso(ListaProcessi *,TipoProcesso *, ListaProcessi **); //dealloca il processo dalla lista e restituisce il puntatore all'elemento successivo rispetto a quello deallocato
//int confrontaProcessi(TipoProcesso *, TipoProcesso *); //confronta i processi campo per campo
char acquisisciCarattereDalNastro(TipoProcesso *); //restituisce il carattere per la transizione in base a valore testina
ListaProcessi *mossaNonDeterministica(TipoProcesso *,TipoMatrice *, ListaProcessi *, char, char *); //crea una copia del padre senza modificarlo
void mossaDeterministica(TipoProcesso *,TipoMatrice *, char, char *);//modifica il padre
int muoviTestina(TipoProcesso *, char, char *);//sposta la testina e chiama controllaLimiteNastro sse va a dx o sx
//controlla se la mia testina è sul limite del nastro e rialloca in caso affermativo
void controllaLimiteNastroSX(char ** , int);
void controllaLimiteNastroDX(char **, int, char *);
ListaProcessi *liberaListaProcessi(ListaProcessi *);
void myfree(ListaProcessi *);


int main(){
  ListaTransizioni *headTransizioni=NULL;
  ListaStatiAccettazione *headAccettazione=NULL;

  int hashDiretto[LENGTH];
  int maxStato = 0;
  int maxPassi = 0;
  int i, j ,posizione;
  int rigaMatrice = 0;
  TipoMatrice **matrice = NULL;
  int uscita = 0;

  //variabili per run
  char *linea = NULL;
  size_t lunghezza = 0;
  int nread;

  for(i=0; i<LENGTH; i++)
    hashDiretto[i] = 0;
  headTransizioni = acquisisciTransizioni(headTransizioni, hashDiretto, &maxStato);
  for (i=0; i<LENGTH ; i++){
    if( hashDiretto[i] == 0)
      hashDiretto[i] = -1;
    else{
      hashDiretto[i] = rigaMatrice;
      rigaMatrice++;
    }
  }//hashtable completa

  headAccettazione = creaListaAccettazione(headAccettazione, &maxPassi);
  //creazione matrice
  matrice = (TipoMatrice**)malloc(rigaMatrice * (maxStato+1) * sizeof(TipoMatrice *));
  for(i = 0; i < rigaMatrice; i++)
		for(j=0; j < maxStato+1 ; j++) //maxstato+1 corrisponde al limite delle colonne
		{
			posizione = i * (maxStato+1) +j;
			matrice[posizione] = NULL;
		}
  matrice = CostruisciMatrice(matrice , maxStato+1 , headTransizioni ,hashDiretto); //costruisce matrice di transizioni carattere x stato --> new carattere x new state x direction
  //dealloco le transizioni
  headTransizioni = deallocaListaTransizioni(headTransizioni);
  free(headTransizioni);

  //run
  nread=getline(&linea, &lunghezza, stdin);
	while((nread=getline(&linea, &lunghezza, stdin)) != -1 ){
    linea[nread-1] = '\0';
    uscita = processaStringa(linea, matrice, headAccettazione, hashDiretto, maxPassi, maxStato);
    if (uscita == -1)
      printf("U\n");
    else if(uscita==0)
      printf("0\n");
    else
      printf("1\n");
  }

  for (int i=0 ; i < rigaMatrice ; i++) //pulisci matrice
 	    free(matrice[i]);
  free(matrice);


  return 0;
}
int processaStringa(char *linea, TipoMatrice **mat, ListaStatiAccettazione *hAccettazione, int *hash, int maxPassi, int maxStato){
  // TipoProcesso *processoIniziale = NULL;
  ListaProcessi *processoCorrente = NULL;
  int i;
  int uscita = 0;
  char carattere; //carattere sul nastro da processare
  TipoMatrice *puntatoreAlleTransizioni = NULL;
  int posizione = 0;
  ListaProcessi *hProcessi = malloc(sizeof(ListaProcessi));


//inizializzo processo iniziale
  // processoIniziale = malloc(sizeof(TipoProcesso));
  hProcessi->processo.passi = 0;
  hProcessi->processo.stato = 0;
  hProcessi->processo.testina = 0;
  hProcessi->processo.nastroDX = (char*)malloc((DIMENSIONE+1)*sizeof(char));
  hProcessi->processo.nastroSX = (char*)malloc((DIMENSIONE+1)*sizeof(char));

  //riempo i nastri del processo iniziale
  for (i = 0; linea[i] != '\0' && i < DIMENSIONE ; i++)
    hProcessi->processo.nastroDX[i] = linea[i]; // caso 1: length(linea) <= DIM : i caratteri di linea li piazzo sul mio nastro iniziale, gli altri li riempio con blank.
  for(; i < DIMENSIONE; i++)//se devo ancora riempire riempo
  	  	hProcessi->processo.nastroDX[i] = BLANK;
  hProcessi->processo.nastroDX[i] = '\0';
  memset(hProcessi->processo.nastroSX, BLANK, DIMENSIONE);
  hProcessi->processo.nastroSX[DIMENSIONE] = '\0';

  hProcessi->next = NULL;

  // hProcessi = inserisciInListaProcessi(hProcessi, processoIniziale);

  //iteration time
  while(hProcessi != NULL && uscita != 1){ //esco quando mi trovo in uno stato di accettazione(uscita = 1) oppure quando non ho più processi disponibili (deallocazione)

      processoCorrente = hProcessi;
      while(processoCorrente != NULL){ //BST

            if ((ControllaAccettazione(hAccettazione, processoCorrente->processo.stato)) == 1){
                uscita = 1;
                hProcessi = liberaListaProcessi(hProcessi);
                break; //esco dal for e dal while se mi trovo in uno stato di accettazione
            }

            if ( processoCorrente->processo.passi > maxPassi){
              uscita = -1;
              hProcessi = deallocaProcesso(hProcessi, &(processoCorrente->processo), &processoCorrente); //dealloco il processo che ha raggiunto il limite dei passi poichè ho finito la ricerca su questo ramo
              continue ; //processo successivo
            }

           // se non devo accettare, non ho finito i passi posso leggere il carattere per eseguire la transizione
           carattere = acquisisciCarattereDalNastro(&(processoCorrente->processo)); //carattere da processare

           if (hash[(int) carattere] < 0){ //se il carattere non è  disponibile nella matrice significa che non posso eseguire la transizione, devo deallocare il processo
             hProcessi = deallocaProcesso(hProcessi, &(processoCorrente->processo),&processoCorrente);
             continue; //ricomincio la bst
           }

           //controllo se fa sempre le stesse mosse ( sempre a destra/sinistra, stesso stato e non scrivo niente)

           //posso eseguire la mia mossa e creare le configurazioni
           posizione = ( hash[(int) carattere] * (maxStato+1) + processoCorrente->processo.stato);
           puntatoreAlleTransizioni = mat[posizione]; //ora sono sicuro di puntare a delle possibili transizioni

           if(puntatoreAlleTransizioni == NULL) //caso in cui non ho transizioni possibili per il carattere esistente nella matrice
           {
             hProcessi = deallocaProcesso(hProcessi, &(processoCorrente->processo), &processoCorrente);
             continue;
           }

           //ora so di avere delle transizioni disponibili

           while(puntatoreAlleTransizioni != NULL){ //per ogni figlio devo creare un processo/transizione/configurazione

             if (puntatoreAlleTransizioni->next == NULL){  //l'ultimo figlio modifica il padre, così i primi devono copiarlo e modificarlo nella loro procedura
                mossaDeterministica(&(processoCorrente->processo), puntatoreAlleTransizioni, carattere, linea);
             }
             else { //forka i figli e li inserisce nella lista processi
               hProcessi = mossaNonDeterministica(&(processoCorrente->processo),puntatoreAlleTransizioni, hProcessi ,carattere, linea);
             }

             puntatoreAlleTransizioni = puntatoreAlleTransizioni->next;
           }

             processoCorrente = processoCorrente->next;
      }

  }

  return uscita;
}

ListaProcessi * liberaListaProcessi(ListaProcessi * hProcessi){
  ListaProcessi *current = NULL;
  while (hProcessi != NULL){
    current = hProcessi;
    hProcessi = hProcessi->next;
    myfree(current);
  }
  return hProcessi;
}

char acquisisciCarattereDalNastro(TipoProcesso *processo){

  if (processo->testina < 0)
    return processo->nastroSX[-(processo->testina)-1];
  else
    return processo->nastroDX[processo->testina];

}

ListaProcessi *mossaNonDeterministica(TipoProcesso *padre, TipoMatrice * transizione, ListaProcessi * hProcessi, char carattereNastro, char * linea){

  int lunghezzaSX = 0, lunghezzaDX = 0;
  ListaProcessi *nuovo = malloc(sizeof(ListaProcessi));

  lunghezzaSX = strlen(padre->nastroSX);
  lunghezzaDX = strlen(padre->nastroDX);
  nuovo->processo.nastroDX =(char *)malloc((lunghezzaDX + 1)*sizeof(char));
  nuovo->processo.nastroSX =(char *)malloc((lunghezzaSX + 1)*sizeof(char)); //strlen non conta il terminatore di una stringa

  strncpy(nuovo->processo.nastroDX, padre->nastroDX, lunghezzaDX);
  strncpy(nuovo->processo.nastroSX, padre->nastroSX, lunghezzaSX);
  nuovo->processo.nastroDX[lunghezzaDX] = '\0';
  nuovo->processo.nastroSX[lunghezzaSX] = '\0'; //ho copiato il nastro del padre nel figlio, metendo il terminatore a fine nastro

  nuovo->processo.passi = padre->passi + 1;
  nuovo->processo.testina = padre->testina;

  if(carattereNastro != transizione->simbolo_scritto){

    if(padre->testina >= 0)
      nuovo->processo.nastroDX[padre->testina] = transizione->simbolo_scritto;
    else
     nuovo->processo.nastroSX[-(padre->testina) - 1] = transizione->simbolo_scritto;
  }

  nuovo->processo.testina = muoviTestina(&(nuovo->processo), transizione->direzione, linea);

  nuovo->processo.stato = transizione->nuovo_stato;

  hProcessi = inserisciInListaProcessi(hProcessi, nuovo); //devo deallocare nuovo in questa funzione?? poichè in inserisciInListaProcessi alloco spazio per la lista

  return hProcessi;
}

void mossaDeterministica(TipoProcesso *padre, TipoMatrice *transizione, char carattereNastro, char *linea){

    (padre->passi)++; //aumento i passi

    if(carattereNastro != transizione->simbolo_scritto){ //scrivo sul nastro

        if (padre->testina >= 0)
            padre->nastroDX[padre->testina] = transizione->simbolo_scritto;
        else
            padre->nastroSX[-(padre->testina) - 1] = transizione->simbolo_scritto;
    }

    padre->testina = muoviTestina(padre, transizione->direzione, linea);

    padre->stato = transizione->nuovo_stato;
    return;
}

int muoviTestina(TipoProcesso * processo, char direzione, char *linea){

  if (direzione == 'L'){
      (processo->testina)-- ;
      if(processo->testina < 0)
        controllaLimiteNastroSX(&(processo->nastroSX), -(processo->testina) - 1);
  }
  else if(direzione == 'R'){
      (processo->testina)++;
      if(processo->testina >= 0)
        controllaLimiteNastroDX(&(processo->nastroDX), (processo->testina) , linea);
  }

  return processo->testina;
}

void controllaLimiteNastroSX(char ** nastro, int testina){

  int dimNastro = 0, i;

  if((*nastro)[testina] == '\0'){
    dimNastro = strlen((*nastro));

    (*nastro) = realloc((*nastro),(dimNastro * 2 + 1)* sizeof(char));

    for (i = dimNastro; i < dimNastro * 2; i++)
      (*nastro)[i] = BLANK;
    (*nastro)[i] = '\0';
  }

  return;
}

void controllaLimiteNastroDX(char ** nastro, int testina, char *linea){

  int dimLinea = 0, dimNastro = 0, i;

  if((*nastro)[testina] == '\0'){


    dimLinea = strlen(linea);
    dimNastro = strlen((*nastro));
    (*nastro) = realloc((*nastro), (dimNastro * 2 + 1)*sizeof(char));

    if(dimLinea < dimNastro){ //caso 1: tutti blank
        for (i = dimNastro; i < dimNastro * 2 ; i++ )
          (*nastro)[i] = BLANK;
        (*nastro)[i] = '\0';
    }
    else{

        if(dimLinea < 2 * dimNastro){ //caso un po' linea e un po' blank
           for(i = dimNastro; linea[i] != '\0'; i++)
              (*nastro)[i] = linea[i];
            for(; i < 2 * dimNastro; i++)
              (*nastro)[i] = BLANK;
            (*nastro)[i] = '\0';
        }
        else { //caso solo linea
            for (i = dimNastro; i < 2 * dimNastro ; i++)
                (*nastro)[i] = linea[i];
            (*nastro)[i] = '\0';
        }

    }
  }

  return;
}

ListaProcessi *deallocaProcesso(ListaProcessi *hProcessi, TipoProcesso *key, ListaProcessi **indirizzoPC){ //dealloca il processo dalla lista dei processi e ritorna il puntatore all'elemento successivo
  // Store head node
  ListaProcessi * temp = hProcessi, *prev = NULL;

    // If head node itself holds the key to be deleted
    if (temp != NULL && &(hProcessi->processo) == key)
    {
        hProcessi = hProcessi->next;   // Changed head
        *indirizzoPC = (*indirizzoPC)->next;
        myfree(temp);               // free old head

        return hProcessi;
    }// in questo caso quindi il successore è l'elemento successivo alla testa

    // Search for the key to be deleted, keep track of the
    // previous node as we need to change 'prev->next'
    while (temp != NULL && &(temp->processo) != key)
    {
        prev = temp;
        temp = temp->next;
    }

    // If key was not present in linked list
    if (temp == NULL)
			return hProcessi; //questo caso non è contemplato

    // Unlink the node from linked list
    prev->next = temp->next;
    *indirizzoPC = (*indirizzoPC)->next;
    myfree(temp);  // Free memory

		return hProcessi;
}
//
// int confrontaProcessi(TipoProcesso *current, TipoProcesso *key){
//     if((current->stato == key->stato) && (current->testina == key->testina) && (current->passi == key->passi) && (strncmp(current->nastroDX, key->nastroDX, DIMENSIONE) == 0)  && (strncmp(current->nastroSX, key->nastroSX, DIMENSIONE) == 0) )
//         return 1;
//     return 0;
// }

int ControllaAccettazione(ListaStatiAccettazione *hAccettazione, int stato){ //ritorna 1 se devo accettare, alla fine della lista di accettazione esco e ritorno 0
    ListaStatiAccettazione *current = hAccettazione;

    while(current != NULL){

      if (current->stato == stato)
          return 1;
      current = current->next;
    }

    return 0;
}

ListaProcessi *inserisciInListaProcessi(ListaProcessi *hProcessi, ListaProcessi *nuovo){
  nuovo->next = hProcessi;
  hProcessi = nuovo;
  return hProcessi;
}
TipoMatrice **CostruisciMatrice(TipoMatrice **mat, int colonne, ListaTransizioni * h,int * hashDiretto){
  ListaTransizioni *temp = h;
  TipoMatrice *nuovo;
  int posizione, rigaAttuale, colonnaAttuale;

  while(temp!=NULL){
    rigaAttuale = hashDiretto[(int)temp->transizione.simbolo_letto];
    colonnaAttuale = temp->transizione.stato;
    posizione = ( rigaAttuale * colonne + colonnaAttuale);
    nuovo=malloc(sizeof(TipoMatrice));
    (*nuovo).nuovo_stato = temp->transizione.nuovo_stato;
		(*nuovo).simbolo_scritto = temp->transizione.simbolo_scritto;
		(*nuovo).direzione = temp->transizione.direzione;
    nuovo->next = mat[posizione];
    mat[posizione] = nuovo;
    temp = temp->next;
  }

  free(temp);
  return mat;
}

ListaStatiAccettazione *creaListaAccettazione(ListaStatiAccettazione *h, int * maxPassi){
  char *linea = NULL;
  size_t lunghezza = 0;
  int *currentState = NULL;

  getline(&linea, &lunghezza, stdin);//leggo primo stato

  currentState = (int *)malloc(sizeof(int));
  while(strcmp(linea,"max\n")!=0){
    sscanf(linea, "%d", currentState);
    h = aggiungiAllaListaAccettazione(h, *currentState);
    getline(&linea, &lunghezza, stdin);
  }

  getline(&linea, &lunghezza, stdin);
  sscanf(linea, "%d", maxPassi);

  free(linea);
  free(currentState);
  return h;
}

ListaStatiAccettazione *aggiungiAllaListaAccettazione(ListaStatiAccettazione *h,int current){
  ListaStatiAccettazione *nuovo = malloc(sizeof(ListaStatiAccettazione));
  nuovo->stato = current;
  nuovo->next = h;
  h = nuovo;
  return h;
}

ListaTransizioni *acquisisciTransizioni(ListaTransizioni *h, int * hashDiretto, int * maxStato){
  char *linea = NULL;
  size_t lunghezza = 0;
  TipoTransizione *transizione = malloc(sizeof(TipoTransizione));

  getline(&linea, &lunghezza, stdin);//leggo tr
  getline(&linea, &lunghezza, stdin);//leggo prima transizione

  while(strcmp(linea,"acc\n")!=0){
      sscanf(linea,"%d%s%s%s%d", &(transizione->stato), &(transizione->simbolo_letto), &(transizione->simbolo_scritto), &(transizione->direzione), &(transizione->nuovo_stato));
      if( *maxStato < transizione->nuovo_stato)
        *maxStato = transizione->nuovo_stato;
      hashDiretto[(int)transizione->simbolo_letto] = 1;
      h = aggiungiAllaListaTransizioni(h,transizione);
      getline(&linea, &lunghezza, stdin);
  }
  free(transizione);
  free(linea);
  return h;
}

ListaTransizioni *aggiungiAllaListaTransizioni(ListaTransizioni *h, TipoTransizione *trans){
    ListaTransizioni * nuovo = malloc(sizeof(ListaTransizioni));
		(*nuovo).transizione.simbolo_letto = trans->simbolo_letto;
		(*nuovo).transizione.simbolo_scritto = trans->simbolo_scritto;
		(*nuovo).transizione.stato = trans->stato;
		(*nuovo).transizione.nuovo_stato = trans->nuovo_stato;
		(*nuovo).transizione.direzione = trans->direzione;
		nuovo->next = h;
		h = nuovo;
	  return h;
}

ListaTransizioni *deallocaListaTransizioni(ListaTransizioni *h){
  ListaTransizioni *current = h;
  while(h != NULL){
    current = h;
    h = h->next;
    free(current);
  }
  return h;
}

void myfree(ListaProcessi *temp){
  if(temp->processo.nastroDX != NULL)
    free(temp->processo.nastroDX);
  if(temp->processo.nastroSX != NULL)
    free(temp->processo.nastroSX);
  if(temp != NULL)
    free(temp);
  return;
}
