#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINIE 1024
#define MAX_TRANZACTII 100005

typedef struct {
    char data_vanzare[12];
    char id_produs[25];
    char nume_produs[100];
    char categorie[100];
    char subcategorie[100];
    float pret_unitar;
    int cantitate_vanduta;
    char tara[100];
    char oras[100];
    float venit;
    int luna;
} Tranzactie;

typedef struct {
    char nume[150];
    float total_venit;
} RaportProdus;



int cmp_dupa_produs(const void *a, const void *b) {
    return strcmp(((Tranzactie*)a)->nume_produs, ((Tranzactie*)b)->nume_produs);
}

int cmp_dupa_categorie(const void *a, const void *b) {
    return strcmp(((Tranzactie*)a)->categorie, ((Tranzactie*)b)->categorie);
}

int cmp_dupa_tara_oras(const void *a, const void *b) {
    Tranzactie *t1 = (Tranzactie*)a;
    Tranzactie *t2 = (Tranzactie*)b;
    int rez = strcmp(t1->tara, t2->tara);
    return (rez == 0) ? strcmp(t1->oras, t2->oras) : rez;
}

int cmp_dupa_subcat_luna(const void *a, const void *b) {
    Tranzactie *t1 = (Tranzactie*)a;
    Tranzactie *t2 = (Tranzactie*)b;
    int rez = strcmp(t1->subcategorie, t2->subcategorie);
    return (rez == 0) ? (t1->luna - t2->luna) : rez;
}

int cmp_raport_desc(const void *a, const void *b) {
    float diff = ((RaportProdus*)b)->total_venit - ((RaportProdus*)a)->total_venit;
    return (diff > 0) - (diff < 0);
}


void venit_lunar(Tranzactie *t, int n) {
    float venit_pe_luni[13] = {0};
    for (int i = 0; i < n; i++) {
        if (t[i].luna >= 1 && t[i].luna <= 12)
            venit_pe_luni[t[i].luna] += t[i].venit;
    }
    printf("\n>>> VENIT TOTAL PE LUNI:\n");
    for (int i = 1; i <= 12; i++)
        printf("Luna %02d: %10.2f\n", i, venit_pe_luni[i]);
}

void top_5_produse(Tranzactie *t, int n) {
    qsort(t, n, sizeof(Tranzactie), cmp_dupa_produs);
    RaportProdus *lista_raport = calloc(n, sizeof(RaportProdus));
    if (!lista_raport) return;

    int m = 0;
    for (int i = 0; i < n; i++) {
        if (i > 0 && strcmp(t[i].nume_produs, t[i-1].nume_produs) == 0) {
            lista_raport[m-1].total_venit += t[i].venit;
        } else {
            strncpy(lista_raport[m].nume, t[i].nume_produs, 149);
            lista_raport[m].total_venit = t[i].venit;
            m++;
        }
    }
    qsort(lista_raport, m, sizeof(RaportProdus), cmp_raport_desc);

    printf("\n>>> TOP 5 PRODUSE DUPA VENIT:\n");
    for (int i = 0; i < 5 && i < m; i++)
        printf("%d. %-30s | %.2f\n", i+1, lista_raport[i].nume, lista_raport[i].total_venit);

    free(lista_raport);
}

void vanzari_categorii(Tranzactie *t, int n) {
    qsort(t, n, sizeof(Tranzactie), cmp_dupa_categorie);
    printf("\n>>> VANZARI PE CATEGORII:\n");
    float suma = 0;
    for (int i = 0; i < n; i++) {
        suma += t[i].venit;
        if (i == n-1 || strcmp(t[i].categorie, t[i+1].categorie) != 0) {
            printf("%-20s : %.2f\n", t[i].categorie, suma);
            suma = 0;
        }
    }
}

void top_oras_tara(Tranzactie *t, int n) {
    qsort(t, n, sizeof(Tranzactie), cmp_dupa_tara_oras);
    printf("\n>>> CEL MAI BUN ORAS PER TARA:\n");
    char oras_max[100] = "";
    float v_max = 0, v_oras = 0;

    for (int i = 0; i < n; i++) {
        v_oras += t[i].venit;
        if (i == n-1 || strcmp(t[i].oras, t[i+1].oras) != 0 || strcmp(t[i].tara, t[i+1].tara) != 0) {
            if (v_oras > v_max) {
                v_max = v_oras;
                strcpy(oras_max, t[i].oras);
            }
            v_oras = 0;
            if (i == n-1 || strcmp(t[i].tara, t[i+1].tara) != 0) {
                printf("%-15s : %-15s (%.2f)\n", t[i].tara, oras_max, v_max);
                v_max = 0;
            }
        }
    }
}

void tendinte_subcat(Tranzactie *t, int n) {
    qsort(t, n, sizeof(Tranzactie), cmp_dupa_subcat_luna);
    printf("\n>>> TENDINTE LUNARE PE SUBCATEGORII:\n");
    float suma = 0;
    for (int i = 0; i < n; i++) {
        suma += t[i].venit;
        if (i == n-1 || t[i].luna != t[i+1].luna || strcmp(t[i].subcategorie, t[i+1].subcategorie) != 0) {
            printf("Subcat: %-15s | Luna: %02d | Venit: %.2f\n", t[i].subcategorie, t[i].luna, suma);
            suma = 0;
        }
    }
}

int main() {
    FILE *f = fopen("sales.csv", "r");
    if (!f) { perror("Eroare"); return 1; }

    Tranzactie *tranzactii = malloc(MAX_TRANZACTII * sizeof(Tranzactie));
    if (!tranzactii) { fclose(f); return 1; }

    char linie[MAX_LINIE];
    int nr = 0;

    fgets(linie, MAX_LINIE, f); // Header
    while (fgets(linie, MAX_LINIE, f) && nr < MAX_TRANZACTII) {
        char *ptr = linie;
        ptr[strcspn(ptr, "\r\n")] = 0;

        char *token = strtok(ptr, ",");
        if (!token) continue;

        int y, m, d;
        if (sscanf(token, "%d-%d-%d", &y, &m, &d) == 3) tranzactii[nr].luna = m;
        strncpy(tranzactii[nr].data_vanzare, token, 11);

        if ((token = strtok(NULL, ","))) strcpy(tranzactii[nr].id_produs, token);
        if ((token = strtok(NULL, ","))) strcpy(tranzactii[nr].nume_produs, token);
        if ((token = strtok(NULL, ","))) strcpy(tranzactii[nr].categorie, token);
        if ((token = strtok(NULL, ","))) strcpy(tranzactii[nr].subcategorie, token);
        if ((token = strtok(NULL, ","))) tranzactii[nr].pret_unitar = atof(token);
        if ((token = strtok(NULL, ","))) tranzactii[nr].cantitate_vanduta = atoi(token);
        if ((token = strtok(NULL, ","))) strcpy(tranzactii[nr].tara, token);
        if ((token = strtok(NULL, ","))) strcpy(tranzactii[nr].oras, token);

        tranzactii[nr].venit = tranzactii[nr].pret_unitar * tranzactii[nr].cantitate_vanduta;
        nr++;
    }
    fclose(f);

    int optiune;
    do {
        printf("\n--------- MENIU ANALIZA VANZARI -----------\n");
        printf("1. Venit total pe luni\n2. Top 5 produse\n3. Vanzari pe categorii\n");
        printf("4. Cel mai bun oras per tara\n5. Tendinte subcategorii\n0. Iesire\n");
        printf("Alege: ");
        if (scanf("%d", &optiune) != 1) { while(getchar() != '\n'); continue; }

        switch(optiune) {
            case 1: venit_lunar(tranzactii, nr); break;
            case 2: top_5_produse(tranzactii, nr); break;
            case 3: vanzari_categorii(tranzactii, nr); break;
            case 4: top_oras_tara(tranzactii, nr); break;
            case 5: tendinte_subcat(tranzactii, nr); break;
        }
    } while(optiune != 0);

    free(tranzactii);
    return 0;
}
