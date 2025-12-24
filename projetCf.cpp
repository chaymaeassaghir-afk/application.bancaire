
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define RESET   "\033[0m"
#define ROUGE   "\033[31m"
#define VERT    "\033[32m"
#define JAUNE   "\033[33m"
#define BLEU    "\033[34m"
#define CYAN    "\033[36m"
#define BLANC   "\033[37m"
#define ROSE  "\033[35m"
#include <windows.h>
#ifdef _WIN32
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
#endif
#ifdef _WIN32
    #include <conio.h>
#else
    #include <termios.h>
    #include <unistd.h>
#endif

//__________________structures_______________
typedef struct {
    int ent;
    char car[20];
} mot_de_passe;

typedef struct {
    char id[18]; // 17 chiffres + '\0'
    char nom[50];
    char prenom[50];
    float solde;
    int RIB;
    int etat; // 0 = bloquï¿½, 1 = dï¿½bloquï¿½
    mot_de_passe mdp;
    int carte_internationale; // 0 = non, 1 = oui
    int valide ; // 0=en attente 1=valide
} compte;

typedef struct {
    int jour;
    int mois;
    int annee;
    int heure;
    int minute;
} date;

typedef struct {
    char nom[20];
    char prenom[20];
    char cin[7];
    date date_naissance;
    char adresse[30];
    char type_compte[10];
    int tel;
    char gmail[30];
    mot_de_passe cle_sec;
    mot_de_passe cle_confirm;
} donne;
 
 //________________________________________________________
 

/*_______________ Prototypes ____________*/
int champs_remplis(donne *D);
int date_valide(date d);
compte ajouter(donne *D);
date DateActuelle(void);
void supprimerCompte(const char *id);
void supprimerCompte(const char *id_admin,const char *id_compte);
void Versement(const char *id, float montant);
void afficherCompte(const char *id);
void payerFacture(const char *id, float montant, const char *typeFacture);
void changerEtatCarte(const char *id, int nouvelEtat);
int connexion_client(char *idConnecte );
int connexion(const char *type,char *idConnecte);
void virement(const char *id_src, const char *id_dst, float montant);
void virement_permanent(const char *id_src, const char *id_dst, float montant, const char *periode, date start_date);
void afficherHistorique(const char *id);
int modifierMotDePasse(const char *id);
void rendreCarteInternationale(const char *id);
void demandeChequier(const char *id);
void prendreRendezVous(const char *id);
donne* remplissage_donne();
void validerCompte(const char *id);
void afficherRendezVous();
void afficherArchive();
void afficherComptesEnAttente();
int dateDejaReservee(date d);
void initialiserSuperAdmin();
void demanderCompteAdmin();
void afficherDemandesAdmin();
void validerAdmin(const char *id_super_admin);
int superAdminExiste();   
void clearScreen();
void afficherNotifications(const char *idCompte);
void saisir_mdp_invisible(char *mdp, int max_len);
int afficher_menu_fleches(const char *titre,const char **options, int nb_options);
//_______________________________________________________________


int afficher_menu_fleches(const char *titre,const char **options, int nb_options) {
    int choix = 0;
    
    while (1) {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
        
        printf(CYAN"=== %s ===\n\n"RESET, titre);
        
        // Afficher les options
        for (int i = 0; i < nb_options; i++) {
            if (i == choix) {
                printf(VERT" > %s <\n"RESET, options[i]);
            } else {
                printf("   %s\n", options[i]);
            }
        }
        
        printf(JAUNE"\n[Fleches Haut/Bas] Naviguer  [Entree] Valider\n"RESET);
        
#ifdef _WIN32
        int touche = _getch();
        
        if (touche == 224) {  // Flèches
            touche = _getch();
            if (touche == 72) {  // Haut
                choix--;
                if (choix < 0) choix = nb_options - 1;
            }
            else if (touche == 80) {  // Bas
                choix++;
                if (choix >= nb_options) choix = 0;
            }
        }
        else if (touche == 13) {  // Entrée
            return choix + 1;
        }
        else if (touche == 27) {  // Échap
            return 0;
        }
#else
        struct termios old, new;
        tcgetattr(STDIN_FILENO, &old);
        new = old;
        new.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &new);
        
        char c = getchar();
        
        if (c == 27) {
            getchar();  // '['
            c = getchar();
            
            if (c == 'A') {  // Haut
                choix--;
                if (choix < 0) choix = nb_options - 1;
            }
            else if (c == 'B') {  // Bas
                choix++;
                if (choix >= nb_options) choix = 0;
            }
        }
        else if (c == '\n' || c == '\r') {
            tcsetattr(STDIN_FILENO, TCSANOW, &old);
            return choix + 1;
        }
        
        tcsetattr(STDIN_FILENO, TCSANOW, &old);
#endif
    }
}
void viderBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}


void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void saisir_mdp_invisible(char *mdp, int max_len) {
    int i = 0;
    char ch;
    
#ifdef _WIN32
    // Version Windows
    while (1) {
        ch = _getch();
        
        if (ch == 13) {  // Enter
            mdp[i] = '\0';
            printf("\n");
            break;
        }
        else if (ch == 8 && i > 0) {  // Backspace
            i--;
            printf("\b \b");
        }
        else if (i < max_len - 1 && ch >= 32 && ch <= 126) {
            mdp[i++] = ch;
            printf("*");
        }
    }
#else
    // Version Linux/Mac
    struct termios old, new;
    tcgetattr(STDIN_FILENO, &old);
    new = old;
    new.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new);
    
    while (1) {
        ch = getchar();
        
        if (ch == '\n' || ch == '\r') {
            mdp[i] = '\0';
            printf("\n");
            break;
        }
        else if (ch == 127 && i > 0) {  // Backspace
            i--;
            printf("\b \b");
        }
        else if (i < max_len - 1 && ch >= 32 && ch <= 126) {
            mdp[i++] = ch;
            printf("*");
        }
    }
    
    tcsetattr(STDIN_FILENO, TCSANOW, &old);
#endif
}
/* ___________Implï¿½mentations _____________*/

int champs_remplis(donne *D) {
    return strlen(D->nom) > 0 &&
           strlen(D->prenom) > 0 &&
           strlen(D->adresse) > 0 &&
           strlen(D->type_compte) > 0 &&
           strlen(D->gmail) > 0 &&
           strlen(D->cle_sec.car) > 0 &&
           strlen(D->cle_confirm.car) > 0;
}

int date_valide(date d) {
    if(d.jour>=1 && d.jour <=31){
    	if(d.mois>=1 && d.mois <=12){
    		if(d.annee>=1800 && d.annee <= 2025){
    			return 1;
			}
		}
	}
	return 0;
}
/*foction pour remplire les donnees du client */
donne* remplissage_donne(){
	
	donne *D = (donne*)malloc(sizeof(donne)); // AJOUTER CETTE LIGNE
    if (D == NULL) return NULL;
    
    printf("nom:\n");
    scanf("%49s", D->nom);
    
	
    printf("prenom:\n");
    scanf("%49s", D->prenom);
    printf("cin:\n");
    scanf("%6s", D->cin);
    viderBuffer();
    printf("date de naissance (jour mois annee):\n");
    scanf("%d %d %d", &D->date_naissance.jour, &D->date_naissance.mois, &D->date_naissance.annee);
    return D;
}


//inscription
compte ajouter(donne *D) {
    compte C;
    int type;
    
    // Validation date
    if (!date_valide(D->date_naissance)) {
        printf(" Date invalide. Reessayez (JJ/MM/AAAA):\n");
        scanf("%d/%d/%d", &D->date_naissance.jour, 
              &D->date_naissance.mois, &D->date_naissance.annee);
        while (getchar() != '\n');
    }
    
    // Adresse
    printf("Adresse:\n");
    scanf("%29s", D->adresse);
    while (getchar() != '\n');
    
    // Type de compte
    do {
        printf("\nType de compte:\n");
        printf("1. Courant\n2. epargne\n3. etudiant\n4. Premium\n");
        printf("Choix: ");
        if (scanf("%d", &type) != 1) {
            while (getchar() != '\n');
            type = 0;
            continue;
        }
        while (getchar() != '\n');
        
        switch (type) {
            case 1: strcpy(D->type_compte, "courant"); break;
            case 2: strcpy(D->type_compte, "epargne"); break;
            case 3: strcpy(D->type_compte, "etudiant"); break;
            case 4: strcpy(D->type_compte, "premium"); break;
            default: printf(" Choix invalide\n"); break;
        }
    } while (type < 1 || type > 4);
    
    // Tï¿½lï¿½phone
    do {
        printf("Numero de telephone (ex: 0612345678):\n");
        if (scanf("%d", &D->tel) != 1 || D->tel <= 0) {
            printf("? Numero invalide\n");
            while (getchar() != '\n');
            D->tel = 0;
        }
        while (getchar() != '\n');
    } while (D->tel <= 0);
    
    // Email avec validation basique
    int email_valide = 0;
    do {
        printf("Entrer votre email:\n");
        scanf("%29s", D->gmail);
        while (getchar() != '\n');
        
        if (strchr(D->gmail, '@') != NULL && strchr(D->gmail, '.') != NULL) {
            email_valide = 1;
        } else {
            printf(" Email invalide (doit contenir @ et .)\n");
        }
    } while (!email_valide);
    
    // Mot de passe
    do {
        printf("Mot de passe (8 caracteres minimum):\n");
        saisir_mdp_invisible(D->cle_sec.car, 20);
        D->cle_sec.ent = (int)strlen(D->cle_sec.car);
        
        if (D->cle_sec.ent < 8) {
            printf(" Le mot de passe doit contenir au moins 8 caracteres\n");
        }
    } while (D->cle_sec.ent < 8);
    
    // Confirmation mot de passe
    do {
        printf("Confirmer le mot de passe:\n");
        saisir_mdp_invisible(D->cle_confirm.car, 20);
        
        if (strcmp(D->cle_sec.car, D->cle_confirm.car) != 0) {
            printf(" Les mots de passe ne correspondent pas\n");
        }
    } while (strcmp(D->cle_sec.car, D->cle_confirm.car) != 0);
    
    // Vï¿½rification champs
    if (!champs_remplis(D)) {
        printf(" Veuillez remplir tous les champs.\n");
        return C;  // Retour avec structure vide
    }
    
    // Crï¿½ation du compte
    strcpy(C.nom, D->nom);
    strcpy(C.prenom, D->prenom);
    C.solde = 0.0f;
    C.etat = 1;
    C.RIB = rand() % 1000000;
    C.carte_internationale = 0;
    C.valide = 0;  // ?? IMPORTANT: initialiser ï¿½ 0 (en attente)
    
    // Gï¿½nï¿½ration ID unique
    int id_unique = 0;
    while (!id_unique) {
        for (int i = 0; i < 17; i++) {
            C.id[i] = '0' + (rand() % 10);
        }
        C.id[17] = '\0';
        
        // Vï¿½rifier unicitï¿½
        FILE *test = fopen("comptes_en_attente.dat", "r");
        if (test == NULL) {
            id_unique = 1;  // Fichier vide
        } else {
            char id_temp[18];
            id_unique = 1;
            while (fscanf(test, "%17s%*[^\n]", id_temp) == 1) {
                if (strcmp(id_temp, C.id) == 0) {
                    id_unique = 0;
                    break;
                }
            }
            fclose(test);
        }
    }
    
    // Stocker mot de passe (?? ï¿½ HASHER en production)
    strncpy(C.mdp.car, D->cle_sec.car, sizeof(C.mdp.car)-1);
    C.mdp.car[sizeof(C.mdp.car)-1] = '\0';
    C.mdp.ent = (int)strlen(C.mdp.car);
    
    // Enregistrement - MODE TEXTE AVEC EMAIL, ADRESSE ET Tï¿½Lï¿½PHONE
    FILE *f = fopen("comptes_en_attente.dat", "a");
    if (f != NULL) {
        fprintf(f, "%17s %s %s %.2f %d %d %s %d %d %s %s %d\n",
                C.id, C.nom, C.prenom, C.solde, C.RIB, 
                C.etat, C.mdp.car, C.carte_internationale, C.valide,
                D->adresse, D->gmail, D->tel);
        fclose(f);
        
        printf("\n+-----------------------------------------------+\n");
        printf("|   Compte cree avec succes    !                  |\n");
        printf("|  ID: %-37s  |\n", C.id);
        printf("|  Statut:   EN ATTENTE DE VALIDATION             |\n");
        printf("|                                                 |\n");
        printf("|  Votre compte sera active apres validation      |\n");
        printf("|  par un administrateur.                         |\n");
        printf("+-----------------------------------------------+\n");
    } else {
        perror("? Erreur ouverture fichier");
    }
    
    return C;
}
//__________________________________________________
date DateActuelle(void) {
    date d;
    printf("Jour: ");
    scanf("%d", &d.jour);
    printf("Mois: ");
    scanf("%d", &d.mois);
    printf("Annee: ");
    scanf("%d", &d.annee);
    printf("Heure: ");
    scanf("%d", &d.heure);
    printf("Minute: ");
    scanf("%d", &d.minute);
    return d;
}


//___________gestion du compte____________________________


/* Supprimer un compte par ID (ID est une chaîne) */
void supprimerCompte(const char *id) {
    FILE *f = fopen("comptes.dat", "r");
    FILE *temp = fopen("temp.dat", "w");
    FILE *archive = fopen("archive.dat","a"); // Changement en mode 'a' pour ajouter à la fin
    
    if (!f || !temp || !archive) {
        perror("Erreur ouverture fichier");
        if (f) fclose(f);
        if (temp) fclose(temp);
        if (archive) fclose(archive);
        return;
    }

    // Déclaration des variables pour la lecture des 12 champs
    char idF[18], nom[50], prenom[50], mdp_buf[20], adresse[100], email[50];
    float solde;
    int RIB, etat, carte_flag, valide_flag, tel;
    int found = 0;
    
    while (fscanf(f, "%17s %49s %49s %f %d %d %19s %d %d %99s %49s %d",
                  idF, nom, prenom, &solde, &RIB, &etat, mdp_buf, 
                  &carte_flag, &valide_flag, adresse, email, &tel) == 12) {
        
        if (strcmp(idF, id) == 0) {
            found = 1;
            // Archivage de l'enregistrement complet
            fprintf(archive, "%s %s %s %.2f %d %d %s %d %d %s %s %d\n",
                    idF, nom, prenom, solde, RIB, etat, mdp_buf, 
                    carte_flag, valide_flag, adresse, email, tel);
            continue; /* Skip pour supprimer */
        } 
        
        // Réécriture de l'enregistrement complet (12 champs) dans temp.dat
        fprintf(temp, "%s %s %s %.2f %d %d %s %d %d %s %s %d\n",
                idF, nom, prenom, solde, RIB, etat, mdp_buf, 
                carte_flag, valide_flag, adresse, email, tel);
    }

    fclose(f);
    fclose(temp);
    fclose(archive);

    if (remove("comptes.dat") != 0) perror("remove");
    if (rename("temp.dat", "comptes.dat") != 0) perror("rename");

    if (found) printf("Compte ID:%s supprime et archive avec succes !\n", id);
    else printf("Compte introuvable (ID: %s)\n", id);
}

int modifierMotDePasse(const char *id) {
    FILE *f = fopen("comptes.dat", "r");
    FILE *temp = fopen("temp.dat", "w");

    if (!f || !temp) {
        perror(ROUGE "Erreur ouverture fichier" RESET);
        if (f) fclose(f);
        if (temp) fclose(temp);
        return 0;
    }

    char ancien[20], nouveau[20], confirm[20];
    printf("Ancien mot de passe: ");
    scanf("%19s", ancien);
    viderBuffer();

    char ligne[512];
    int trouve = 0, mdp_change = 0;

    while (fgets(ligne, sizeof(ligne), f)) {

        char idF[18], nom[50], prenom[50], mdp[20];
        char adresse[100], email[50];
        float solde;
        int RIB, etat, carte, valide, tel;

        int lus = sscanf(ligne,
            "%17s %49s %49s %f %d %d %19s %d %d %99s %49s %d",
            idF, nom, prenom, &solde, &RIB, &etat,
            mdp, &carte, &valide, adresse, email, &tel);

        /* Ligne invalide ? on recopie sans toucher */
        if (lus != 12) {
            fputs(ligne, temp);
            continue;
        }

        if (strcmp(idF, id) == 0) {
            trouve = 1;

            if (strcmp(mdp, ancien) != 0) {
                printf(ROUGE "Ancien mot de passe incorrect.\n" RESET);
            } else {
                printf(VERT "Nouveau mot de passe (>=8): " RESET);
                scanf("%19s", nouveau);
                viderBuffer();

                if (strlen(nouveau) < 8) {
                    printf(ROUGE "Mot de passe trop court.\n" RESET);
                } else {
                    printf("Confirmer nouveau mot de passe: ");
                    scanf("%19s", confirm);
                    viderBuffer();

                    if (strcmp(nouveau, confirm) != 0) {
                        printf(ROUGE "Confirmation incorrecte.\n" RESET);
                    } else {
                        strcpy(mdp, nouveau);
                        mdp_change = 1;
                    }
                }
            }
        }

        fprintf(temp,
            "%s %s %s %.2f %d %d %s %d %d %s %s %d\n",
            idF, nom, prenom, solde, RIB, etat,
            mdp, carte, valide, adresse, email, tel);
    }

    fclose(f);
    fclose(temp);

    /* ?? On remplace le fichier SEULEMENT si changement réel */
    if (mdp_change) {
        remove("comptes.dat");
        rename("temp.dat", "comptes.dat");
        printf(VERT"Mot de passe modifie avec succes.\n"RESET);
        return 1;
    } else {
        remove("temp.dat");
        if (!trouve)
            printf("Compte introuvable.\n");
        return 0;
    }
}

//modifier les information

void modifier_info(char *idConnecte) {
    FILE *f = fopen("comptes.dat", "r");
    FILE *temp = fopen("temp.dat", "w");

    if (!f || !temp) {
        printf(ROUGE"Erreur ouverture fichier\n" RESET);
        if (f) fclose(f);
        if (temp) fclose(temp);
        return;
    }

    int choix;
    printf(CYAN"\n+-----------------------------------------------+\n"RESET);
    printf(CYAN"|      MODIFICATION DES INFORMATIONS              |\n"RESET);
    printf(CYAN"+-------------------------------------------------+\n"RESET);
    printf(CYAN"|"RESET ROSE"  1."RESET"  Modifier l'adresse\n");
    printf(CYAN"|"RESET ROSE"  2."RESET"  Modifier l'email\n");
    printf(CYAN"|"RESET ROSE"  3."RESET"  Modifier le numero de telephone\n");
    printf(CYAN"+-------------------------------------------------+\n"RESET);
    printf("Votre choix : ");
    scanf("%d", &choix);
    viderBuffer();

    char nouvelleAdresse[100] = "";
    char nouvelEmail[50] = "";
    int nouveauTel = 0;

    switch (choix) {
        case 1:
            printf("Nouvelle adresse : ");
            fgets(nouvelleAdresse, sizeof(nouvelleAdresse), stdin);
            nouvelleAdresse[strcspn(nouvelleAdresse, "\n")] = 0;
            break;

        case 2:
            printf("Nouvel email : ");
            scanf("%49s", nouvelEmail);
            viderBuffer();

            if (!strchr(nouvelEmail, '@') || !strchr(nouvelEmail, '.')) {
                printf(ROUGE" Email invalide.\n"RESET);
                fclose(f);
                fclose(temp);
                remove("temp.dat");
                return;
            }
            break;

        case 3:
            printf("Nouveau numero telephone : ");
            scanf("%d", &nouveauTel);
            viderBuffer();
            if (nouveauTel <= 0) {
                printf(ROUGE" Numero invalide.\n" RESET);
                fclose(f);
                fclose(temp);
                remove("temp.dat");
                return;
            }
            break;

        default:
            printf(" Choix invalide.\n");
            fclose(f);
            fclose(temp);
            remove("temp.dat");
            return;
    }

    // Variables pour lecture fichier
    char id[18], nom[30], prenom[30];
    float solde;
    int rib, etat, carte_int, valide;
    char mdp[20], adresse[100], email[50];
    int tel;
    int found = 0;

    // Lire le fichier compte par compte
    while (fscanf(f,
        "%17s %29s %29s %f %d %d %19s %d %d %99s %49s %d",
        id, nom, prenom, &solde, &rib, &etat,
        mdp, &carte_int, &valide,
        adresse, email, &tel) == 12) {

        if (strcmp(id, idConnecte) == 0) {
            found = 1;

            // Appliquer la modification choisie
            if (choix == 1) strcpy(adresse, nouvelleAdresse);
            if (choix == 2) strcpy(email, nouvelEmail);
            if (choix == 3) tel = nouveauTel;
        }

        // Rï¿½ï¿½crire dans temp
        fprintf(temp, "%17s %s %s %.2f %d %d %s %d %d %s %s %d\n",
                id, nom, prenom, solde, rib, etat,
                mdp, carte_int, valide,
                adresse, email, tel);
    }

    fclose(f);
    fclose(temp);

    if (!found) {
        printf("? Compte introuvable\n");
        remove("temp.dat");
        return;
    }

    // Mettre ï¿½ jour le fichier original
    remove("comptes.dat");
    rename("temp.dat", "comptes.dat");

    printf(VERT"\n Modification effectue avec succees !\n"RESET);
}

//_________________________________________________________________


//________gestion des produits bancaires__________
void changerEtatCarte(const char *id, int nouvelEtat) {
    FILE *f = fopen("comptes.dat", "r");
    FILE *temp = fopen("temp.dat", "w");

    if (!f || !temp) {
        perror(ROUGE"Erreur ouverture fichier"RESET);
        if (f) fclose(f);
        if (temp) fclose(temp);
        return;
    }

    char ligne[512];
    int trouve = 0;

    while (fgets(ligne, sizeof(ligne), f)) {
        char idF[18], nom[50], prenom[50], mdp[20];
        char adresse[100], email[50];
        float solde;
        int RIB, etat, carte, valide, tel;

        int lus = sscanf(ligne,
            "%17s %49s %49s %f %d %d %19s %d %d %99s %49s %d",
            idF, nom, prenom, &solde, &RIB, &etat,
            mdp, &carte, &valide, adresse, email, &tel);

        /* ligne invalide ? on recopie */
        if (lus != 12) {
            fputs(ligne, temp);
            continue;
        }

        if (strcmp(idF, id) == 0) {
            etat = nouvelEtat;   // ? MODIFICATION CORRECTE
            trouve = 1;
            printf(VERT"Etat de la carte modifie.\n"RESET);
        }

        fprintf(temp,
            "%s %s %s %.2f %d %d %s %d %d %s %s %d\n",
            idF, nom, prenom, solde, RIB, etat,
            mdp, carte, valide, adresse, email, tel);
    }

    fclose(f);
    fclose(temp);

    if (trouve) {
        remove("comptes.dat");
        rename("temp.dat", "comptes.dat");
    } else {
        remove("temp.dat");
        printf(ROUGE"Compte non trouve.\n"RESET);
    }
}


void rendreCarteInternationale(const char *id) {
    FILE *f = fopen("comptes.dat", "r");
    FILE *temp = fopen("temp.dat", "w");

    if (!f || !temp) {
        perror(ROUGE"Erreur ouverture fichier"RESET);
        if (f) fclose(f);
        if (temp) fclose(temp);
        return;
    }

    char ligne[512];
    int trouve = 0;
    int modifie = 0;

    while (fgets(ligne, sizeof(ligne), f)) {

        char idF[18], nom[50], prenom[50], mdp[20];
        char adresse[100], email[50];
        float solde;
        int RIB, etat, carte, valide, tel;

        int lus = sscanf(ligne,
            "%17s %49s %49s %f %d %d %19s %d %d %99s %49s %d",
            idF, nom, prenom, &solde, &RIB, &etat,
            mdp, &carte, &valide, adresse, email, &tel);

        /* Ligne invalide ? on la recopie sans toucher */
        if (lus != 12) {
            fputs(ligne, temp);
            continue;
        }

        if (strcmp(idF, id) == 0) {
            trouve = 1;

            printf(BLEU"Voulez-vous activer la carte internationale ? (1.Oui / 2.Non): "RESET);
            int o;
            if (scanf("%d", &o) != 1) {
                viderBuffer();
                o = 2;
            }
            viderBuffer();

            if (o == 1) {
                carte = 1;
                modifie = 1;
                printf(VERT"Carte internationale activee pour %s\n"RESET, id);
            } else {
                printf(JAUNE"Activation annulee.\n"RESET);
            }
        }

        fprintf(temp,
            "%s %s %s %.2f %d %d %s %d %d %s %s %d\n",
            idF, nom, prenom, solde, RIB, etat,
            mdp, carte, valide, adresse, email, tel);
    }

    fclose(f);
    fclose(temp);

    if (modifie) {
        remove("comptes.dat");
        rename("temp.dat", "comptes.dat");
    } else {
        remove("temp.dat");
        if (!trouve)
            printf(ROUGE"Compte introuvable (ID: %s)\n"RESET, id);
    }
}

/* Demande de chequier : enregistre la demande */
void demandeChequier(const char *id) {
    FILE *f = fopen("demandes_chequier.dat", "ab");
    if (!f) {
        perror(ROUGE"Erreur ouverture demandes_chequier.dat"RESET);
        return;
    }
    date d = DateActuelle();
    fprintf(f, "ID:%s | Demande Chequier | Date:%02d/%02d/%04d %02d:%02d\n",
            id, d.jour, d.mois, d.annee, d.heure, d.minute);
    fclose(f);
    printf(BLEU"Demande de chequier enregistree pour %s\n"RESET, id);
}

//_____________________________________________________________________


//___________connexion___________________
int connexion_client(char *idConnecte) {
    char id[18];
    char mdp[20];
    int trouve = 0;
    
    printf(ROSE"=== Connexion client ===\n"RESET);
    printf("ID : ");
    scanf("%17s", id);
    while (getchar() != '\n');
    
    printf("Mot de passe : ");
    saisir_mdp_invisible(mdp, 20);
    
    
    FILE *f = fopen("comptes.dat", "r");
    if (!f) {
        printf(ROUGE"Erreur ouverture fichier clients\n"RESET);
        return 0;
    }
    
    char idF[18], nom[50], prenom[50], mdpF[20];
    char adresse[30] = "", email[30] = "";
    float solde;
    int rib, etat, carte_flag, valide_flag = 1, tel = 0;
    
    char ligne[512];
    while (fgets(ligne, sizeof(ligne), f)) {
        // Essayer de lire 12 champs
        int nb = sscanf(ligne, "%17s %49s %49s %f %d %d %19s %d %d %29s %29s %d",
                        idF, nom, prenom, &solde, &rib, &etat, 
                        mdpF, &carte_flag, &valide_flag,
                        adresse, email, &tel);
        
        // Si échec, essayer 8 champs (ancien format)
        if (nb != 12) {
            nb = sscanf(ligne, "%17s %49s %49s %f %d %d %19s %d",
                        idF, nom, prenom, &solde, &rib, &etat, 
                        mdpF, &carte_flag);
        }
        
        if (nb >= 8) {  // Au moins 8 champs lus
            if (strcmp(idF, id) == 0 && strcmp(mdpF, mdp) == 0) {
                trouve = 1;
                strcpy(idConnecte, id);
                break;
            }
        }
    }
    
    fclose(f);
    
    if (trouve) {
        printf(CYAN"Connexion reussie !\n"RESET);
        return 1;
    } else {
        printf(JAUNE"ID ou mot de passe incorrect !\n"RESET);
        return 0;
    }
}
int connexion(const char *type, char *idConnecte) {
    char id[18];
    char mdp[20];
    int trouve = 0;
    
    printf(ROSE"=== Connexion %s ===\n"RESET, type);
    printf("ID : ");
    scanf("%17s", id);
    while (getchar() != '\n');
    
    printf("Mot de passe : ");
    saisir_mdp_invisible(mdp, 20);
    
    FILE *f = NULL;
    FILE *s = NULL;
    
    if (strcmp(type, "client") == 0) {
        f = fopen("comptes.dat", "rb");
        if (!f) {
            printf(ROUGE" Erreur ouverture fichier clients\n"RESET);
            return 0;
        }
        
        // Pour les clients : lire tout le format
        compte c;
        char idF[18], nom[30], prenom[30], mdpF[20],adresse[30],email[40];
        float solde;
        int rib, etat, carte_inter, valide,carte_flag,valide_flag,tel;
        
        while (fread(&c,sizeof(compte),1,f)==1) {
            
            if (strcmp(idF, id) == 0 && strcmp(mdpF, mdp) == 0) {
                
                trouve = 1;
                strcpy(idConnecte, id);
                break;
            }
        }
        fclose(f);
    }
    else if (strcmp(type, "admin") == 0) {
        f = fopen("admins.dat", "rb");
        s = fopen("suppadmins.dat", "rb");
        
        if (!f && !s) {
            printf(ROUGE" Erreur ouverture fichiers admins\n"RESET);
            return 0;
        }
        
        char idF[18], mdpF[20];
        
        // Vï¿½rifier dans admins.dat
        if (f) {
            while (fscanf(f, "%17s %19s", idF, mdpF) == 2) {
                if (strcmp(idF, id) == 0 && strcmp(mdpF, mdp) == 0) {
                    trouve = 1;
                    strcpy(idConnecte, id);
                    break;
                }
            }
            fclose(f);
        }
        
        // Vï¿½rifier dans suppadmins.dat si pas trouvï¿½
        if (!trouve && s) {
            while (fscanf(s, "%17s %19s", idF, mdpF) == 2) {
                if (strcmp(idF, id) == 0 && strcmp(mdpF, mdp) == 0) {
                    trouve = 1;
                    strcpy(idConnecte, id);
                    break;
                }
            }
            fclose(s);
        }
        else if (s) {
            fclose(s);
        }
    }
    else {
        printf(JAUNE" Type d'utilisateur invalide !\n"RESET);
        return 0;
    }
    
    if (trouve) {
        printf(CYAN" Connexion reussie en tant que %s !\n"RESET, type);
        return 1;
    } else {
        printf(JAUNE" Identifiant ou mot de passe incorrect.\n"RESET);
        return 0;
    }
}
//_____________________________________________________________

//____________operations_______________________________________
void Versement(const char *id, float montant) {
    FILE *f = fopen("comptes.dat", "r");
    FILE *temp = fopen("temp.dat", "w");
    FILE *op = fopen("operations.dat", "a");

    if (!f || !temp || !op) {
        perror(ROUGE"Erreur ouverture fichier"RESET);
        if (f) fclose(f);
        if (temp) fclose(temp);
        if (op) fclose(op);
        return;
    }

    char ligne[512];
    int trouve = 0;

    while (fgets(ligne, sizeof(ligne), f)) {
        char idF[18], nom[50], prenom[50], mdp[20];
        char adresse[100], email[50];
        float solde;
        int RIB, etat, carte, valide, tel;

        int lus = sscanf(ligne,
            "%17s %49s %49s %f %d %d %19s %d %d %99s %49s %d",
            idF, nom, prenom, &solde, &RIB, &etat,
            mdp, &carte, &valide, adresse, email, &tel);

        if (lus != 12) {
            // ligne invalide ? on recopie telle quelle
            fputs(ligne, temp);
            continue;
        }

        if (strcmp(idF, id) == 0) {
            solde += montant;
            trouve = 1;
        }

        fprintf(temp,
            "%s %s %s %.2f %d %d %s %d %d %s %s %d\n",
            idF, nom, prenom, solde, RIB, etat,
            mdp, carte, valide, adresse, email, tel);
    }

    fclose(f);
    fclose(temp);

    if (!trouve) {
        remove("temp.dat");
        fclose(op);
        printf("Compte non trouvé.\n");
        return;
    }

    fprintf(op, "Versement %.2f vers %s\n", montant, id);
    fclose(op);

    remove("comptes.dat");
    rename("temp.dat", "comptes.dat");

    printf(VERT"Versement effectué avec succes.\n"RESET);
}


void payerFacture(const char *id, float montant, const char *typeFacture) {
    FILE *f = fopen("comptes.dat", "r");
    FILE *temp = fopen("temp.dat", "w");
    FILE *op = fopen("operations.dat", "a");

    if (!f || !temp || !op) {
        perror(ROUGE"Erreur ouverture fichier"RESET);
        if (f) fclose(f);
        if (temp) fclose(temp);
        if (op) fclose(op);
        return;
    }

    char ligne[512];
    int trouve = 0;
    date d = DateActuelle();

    while (fgets(ligne, sizeof(ligne), f)) {
        char idF[18], nom[50], prenom[50], mdp[20];
        char adresse[100], email[50];
        float solde;
        int RIB, etat, carte, valide, tel;

        int lus = sscanf(ligne,
            "%17s %49s %49s %f %d %d %19s %d %d %99s %49s %d",
            idF, nom, prenom, &solde, &RIB, &etat,
            mdp, &carte, &valide, adresse, email, &tel);

        if (lus != 12) {
            fputs(ligne, temp);   // ligne invalide ? on recopie
            continue;
        }

        if (strcmp(idF, id) == 0) {
            trouve = 1;
            if (solde >= montant) {
                solde -= montant;
                printf(BLEU"Paiement %.2f DH [%s] effectué. Nouveau solde : %.2f DH\n"RESET,
                       montant, typeFacture, solde);

                fprintf(op,
                    "Paiement | ID:%s | Montant:%.2f | Type:%s | Date:%02d/%02d/%04d %02d:%02d\n",
                    id, montant, typeFacture,
                    d.jour, d.mois, d.annee, d.heure, d.minute);
            } else {
                printf(JAUNE"Solde insuffisant pour payer [%s]\n"RESET, typeFacture);
            }
        }

        fprintf(temp,
            "%s %s %s %.2f %d %d %s %d %d %s %s %d\n",
            idF, nom, prenom, solde, RIB, etat,
            mdp, carte, valide, adresse, email, tel);
    }

    fclose(f);
    fclose(temp);
    fclose(op);

    if (!trouve) {
        remove("temp.dat");
        printf(JAUNE"Compte non trouve.\n"RESET);
        return;
    }

    remove("comptes.dat");
    rename("temp.dat", "comptes.dat");
}

void virement(const char *id_src, const char *id_dst, float montant) {
    FILE *f = fopen("comptes.dat", "r");
    FILE *temp = fopen("temp.dat", "w");
    FILE *op = fopen("operations.dat", "a");

    if (!f || !temp || !op) {
        perror(ROUGE"Erreur ouverture fichier"RESET);
        if (f) fclose(f);
        if (temp) fclose(temp);
        if (op) fclose(op);
        return;
    }

    char ligne[512];
    int trouve_src = 0, trouve_dst = 0;
    float solde_src = 0;

    /* ---------- 1ère passe : validation ---------- */
    while (fgets(ligne, sizeof(ligne), f)) {
        char id[18];
        float solde;

        if (sscanf(ligne, "%17s %*s %*s %f", id, &solde) != 2)
            continue;

        if (strcmp(id, id_src) == 0) {
            trouve_src = 1;
            solde_src = solde;
        }
        if (strcmp(id, id_dst) == 0) {
            trouve_dst = 1;
        }
    }

    if (!trouve_src || !trouve_dst || solde_src < montant) {
        printf(ROUGE"Virement impossible.\n"RESET);
        fclose(f); fclose(temp); fclose(op);
        remove("temp.dat");
        return;
    }

    rewind(f);

    /* ---------- 2ème passe : mise à jour ---------- */
    while (fgets(ligne, sizeof(ligne), f)) {
        char id[18], nom[50], prenom[50], mdp[20];
        char adresse[100], email[50];
        float solde;
        int RIB, etat, carte, valide, tel;

        int lus = sscanf(ligne,
            "%17s %49s %49s %f %d %d %19s %d %d %99s %49s %d",
            id, nom, prenom, &solde, &RIB, &etat,
            mdp, &carte, &valide, adresse, email, &tel);

        if (lus != 12) {
            fputs(ligne, temp);   // ligne invalide ? on recopie telle quelle
            continue;
        }

        if (strcmp(id, id_src) == 0)
            solde -= montant;
        else if (strcmp(id, id_dst) == 0)
            solde += montant;

        fprintf(temp,
            "%s %s %s %.2f %d %d %s %d %d %s %s %d\n",
            id, nom, prenom, solde, RIB, etat,
            mdp, carte, valide, adresse, email, tel);
    }

    date d = DateActuelle();
    fprintf(op,
        "Virement %s -> %s | Montant %.2f | %02d/%02d/%04d %02d:%02d\n",
        id_src, id_dst, montant,
        d.jour, d.mois, d.annee, d.heure, d.minute);

    fclose(f);
    fclose(temp);
    fclose(op);

    remove("comptes.dat");
    rename("temp.dat", "comptes.dat");

    printf(VERT"Virement effectue avec succes.\n"RESET);
}

/* Enregistre un virement permanent (standing order) dans un fichier */
void virement_permanent(const char *id_src, const char *id_dst, float montant, const char *periode, date start_date) {
    FILE *f = fopen("virements_permanents.dat", "ab");
    if (!f) {
        perror(ROUGE"Erreur ouverture virements_permanents.txt"RESET);
        return;
    }
    /* format: id_src id_dst montant periode dd/mm/yyyy hh:mm */
    fprintf(f, "%s %s %.2f %s %02d/%02d/%04d %02d:%02d\n",
            id_src, id_dst, montant, periode, start_date.jour, start_date.mois, start_date.annee, start_date.heure, start_date.minute);
    fclose(f);
    printf(VERT"Virement permanent enregistre : %s -> %s, %.2f, %s\n"RESET, id_src, id_dst, montant, periode);
}

//______________________________________________

//__________affichage_______

// Afficher les notifications d'un compte spï¿½cifique
void afficherNotifications(const char *idCompte) {
    FILE *f = fopen("notifications.dat", "r");
    
    if (!f) {
        printf(ROSE"\n=== NOTIFICATIONS ===\n"RESET);
        printf("Aucune notification pour le moment.\n");
        printf(ROSE"=====================\n"RESET);
        return;
    }
    
    char ligne[256];
    int compteur = 0;
    
    printf(ROSE"\n========== NOTIFICATIONS POUR %s ==========\n\n"RESET, idCompte);
    
    while (fgets(ligne, sizeof(ligne), f)) {
        // Vï¿½rifier si la ligne contient l'ID recherchï¿½
        if (strstr(ligne, idCompte) != NULL) {
            compteur++;
            printf("%d. %s", compteur, ligne);
        }
    }
    
    fclose(f);
    
    if (compteur == 0) {
        printf("Aucune notification pour ce compte.\n");
    } else {
        printf(ROSE"\n================================================\n"RESET);
        printf("Total: %d notification(s)\n", compteur);
    }
}


/* Afficher compte aprï¿½s vï¿½rif du mot de passe */
void afficherCompte(const char *id) {
    FILE *f = fopen("comptes.dat", "r");   // ? TEXTE
    if (!f) {
        perror(ROUGE"Erreur ouverture comptes.dat"RESET);
        return;
    }

    char ligne[512];
    int trouve = 0;

    while (fgets(ligne, sizeof(ligne), f)) {

        char idF[18], nom[50], prenom[50], mdp_buf[20];
        float solde;
        int RIB, etat, carte;

        int lus = sscanf(ligne,
            "%17s %49s %49s %f %d %d %19s %d",
            idF, nom, prenom, &solde, &RIB, &etat, mdp_buf, &carte);

        if (lus < 8) continue;  // ligne invalide ? ignorer

        if (strcmp(idF, id) == 0 ) {
            printf(CYAN"\n====== Informations du compte ============\n"RESET);
            printf(CYAN "|" RESET " %-20s : %s\n", "ID du compte", idF);
            printf(CYAN "|" RESET " %-20s : %s\n", "Nom", nom);
            printf(CYAN "|" RESET " %-20s  : %s\n", "Prénom", prenom);
            printf(CYAN "|" RESET " %-20s : %.2f DH\n", "Solde actuel", solde);
            printf(CYAN "|" RESET " %-20s : %d\n", "RIB", RIB);
            printf(CYAN "|" RESET " %-20s  : %s\n","État carte", etat ? "Débloquée" : "Bloquée");
            printf(CYAN "|" RESET " %-20s : %s\n","Carte inter.", carte ? "Oui" : "Non");

            printf(CYAN"=============================================\n"RESET);
            trouve = 1;
            break;
        }
    }

    fclose(f);

    if (!trouve)
        printf(JAUNE"Aucun compte trouvé ou mot de passe incorrect.\n"RESET);
}

/* Affiche l'historique d'un compte (operations.txt) */
void afficherHistorique(const char *id) {
    FILE *f = fopen("operations.dat", "rb");
    if (!f) {
        perror(ROUGE"Erreur ouverture operations.txt"RESET);
        return;
    }
    char ligne[256];
    int trouve = 0;
    printf(ROSE"=== Historique pour ID %s ===\n"RESET, id);
    while (fgets(ligne, sizeof(ligne), f)) {
        if (strstr(ligne, id) != NULL) {
            printf("%s", ligne);
            trouve = 1;
        }
    }
    if (!trouve) printf(JAUNE"Aucune operation trouvee pour cet ID.\n"RESET);
    fclose(f);
}

//________________________________________________________________________________________

//verifier si la date deja reserver 
int dateDejaReservee(date d) {
    FILE *f = fopen("rendezvous.dat", "rb");
    if (!f) return 0; // Si le fichier n'existe pas, pas de conflit

    int jour, mois, annee, heure, minute;
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        if (sscanf(line, "ID:%*s | RDV | Date:%d/%d/%d %d:%d", 
                   &jour, &mois, &annee, &heure, &minute) == 5) {
            if (jour == d.jour && mois == d.mois && annee == d.annee &&
                heure == d.heure && minute == d.minute) {
                fclose(f);
                return 1; // Date dï¿½jï¿½ rï¿½servï¿½e
            }
        }
    }
    fclose(f);
    return 0; // Date libre
}

// Prendre rendez-vous : enregistre rendez-vous
void prendreRendezVous(const char *id) {
    date d;
    char sujet[80];

    // Boucle jusqu'ï¿½ ce qu'une date libre soit choisie
    do {
        printf("Saisir date/heure du rendez-vous :\n");
        d = DateActuelle(); // Ici on suppose que DateActuelle() retourne la date saisie
        if (dateDejaReservee(d)) {
            printf(JAUNE"Cette date est deja reservee, veuillez choisir une autre.\n"RESET);
        } else {
            break;
        }
    } while (1);

    printf("Saisir sujet du rendez-vous (sans espaces ou utilisez _ ) : ");
    scanf("%79s", sujet);

    FILE *f = fopen("rendezvous.dat", "ab");
    if (!f) {
        perror("Erreur ouverture rendezvous.dat");
        return;
    }

    fprintf(f, "ID:%s | RDV | Date:%02d/%02d/%04d %02d:%02d | Sujet:%s\n",
            id, d.jour, d.mois, d.annee, d.heure, d.minute, sujet);
    fclose(f);
    printf(BLEU"Rendez-vous enregistre pour %s\n"RESET, id);
}




//__________________________________________________________






//____________partie admin___________________________
//----entrer----
//inscription _

//ajouter le premier admin 
int superAdminExiste() {
 FILE *f = fopen("suppadmins.dat","rb");
    if (!f) return 0;
    
    char id[18];
    int existe = (fscanf(f, "%17s", id) == 1);
    fclose(f);
    return existe;
}

void initialiserSuperAdmin() {
    
    
    char id[18];
    char mdp[20];
    char confirm[20];
    
    printf(CYAN"\n+================================================+\n"RESET);
    printf(CYAN"   INITIALISATION DU SUPER ADMINISTRATEUR     \n"RESET);
    printf(CYAN"   (Cette operation ne peut etre faite qu'une \n"RESET);
    printf(CYAN"    seule fois)                                \n"RESET);
    printf(CYAN"+================================================+\n\n"RESET);
    
    // Gï¿½nï¿½rer l'ID comme pour les comptes clients
    srand((unsigned int)time(NULL) ^ (unsigned int)rand());
    for (int i = 0; i < 17; i++) {
        id[i] = '0' + (rand() % 10);
    }
    id[17] = '\0';
    
    printf("Creer le premier Super Admin :\n");
    printf("ID genere automatiquement : %s\n", id);
    
    do {
        printf("Mot de passe (min 8 caracteres) : ");
        scanf("%19s", mdp);
        if (strlen(mdp) < 8) {
            printf(ROUGE"Mot de passe trop court !\n"RESET);
        }
    } while (strlen(mdp) < 8);
    
    do {
        printf("Confirmer le mot de passe : ");
        scanf("%19s", confirm);
        if (strcmp(mdp, confirm) != 0) {
            printf(ROUGE"Les mots de passe ne correspondent pas !\n"RESET);
        }
    } while (strcmp(mdp, confirm) != 0);
    
    // Crï¿½er le fichier admins.txt avec le super admin
    FILE *f = fopen("suppadmins.dat", "wb");
    if (!f) {
        printf(ROUGE"Erreur creation suppadmins.dat"RESET);
        return;
    }
    
    fprintf(f, "%s %s\n", id, mdp);
    fclose(f);
    
    printf(BLEU"\n+================================================+\n"RESET);
    printf(BLEU"     SUPER ADMIN CREE AVEC SUCCES !           \n"RESET);
    printf(BLEU"     ID : %-39s \n", id);
    printf(BLEU"                                               \n"RESET);
    printf(BLEU"     Vous pouvez maintenant vous connecter       \n"RESET);
    printf(BLEU"+================================================+\n"RESET);
}

// Ajouter compte


void demanderCompteAdmin() {
    char id[18];
    char nom[50];
    char prenom[50];
    char mdp[20];
    char confirm[20];
    char email[50];
    
    printf(CYAN"\n+------------------------------------------------+\n"RESET);
    printf(CYAN"        DEMANDE DE COMPTE ADMINISTRATEUR         \n"RESET);
    printf(CYAN"+------------------------------------------------+\n"RESET);
    
    printf("Nom : ");
    scanf("%49s", nom);
    
    printf("Prenom : ");
    scanf("%49s", prenom);
    
    printf("Email : ");
    scanf("%49s", email);
    
    // Gï¿½nï¿½rer l'ID comme pour les comptes clients
    srand((unsigned int)time(NULL) ^ (unsigned int)rand());
    for (int i = 0; i < 17; i++) {
        id[i] = '0' + (rand() % 10);
    }
    id[17] = '\0';
    
    printf("\nID genere automatiquement : %s\n", id);
    
    do {
        printf("Mot de passe (min 8 caracteres) : ");
        saisir_mdp_invisible(mdp, 20);
        if (strlen(mdp) < 8) {
            printf(JAUNE"Mot de passe trop court !\n"RESET);
        }
    } while (strlen(mdp) < 8);
    
    do {
        printf("Confirmer le mot de passe : ");
        saisir_mdp_invisible(confirm, 20);
        if (strcmp(mdp, confirm) != 0) {
            printf("Les mots de passe ne correspondent pas !\n");
        }
    } while (strcmp(mdp, confirm) != 0);
    
    // Enregistrer dans admins_en_attente.txt
    FILE *attente = fopen("admin_en_attente.dat", "ab");
    if (!attente) {
        printf("Erreur ouverture admin_en_attente.dat");
        return;
    }
    
    fprintf(attente, "%s %s %s %s %s\n", id, mdp, nom, prenom, email);
    fclose(attente);
    
    printf(BLEU"\n+------------------------------------------------+\n"RESET);
    printf(VERT"   Demande enregistree avec succes !          \n"RESET);
    printf(BLEU"  Votre ID : %-33s \n"RESET, id);
    printf("                                               \n");
    printf(BLEU"  Notez bien cet ID pour la connexion !       \n"RESET);
    printf(BLEU"  Votre demande sera examinee par un          \n"RESET);
    printf(BLEU"  super administrateur.                        \n"RESET);
    printf(BLEU"+------------------------------------------------+\n"RESET);
}

// ==========  AFFICHER LES DEMANDES (SUPER ADMIN) ==========
//les compte creer par les admins peut etre valider seulement par le supper admin 
void afficherDemandesAdmin() {
    FILE *f = fopen("admin_en_attente.dat", "rb");
    if (!f) {
        printf("Aucune demande de compte admin en attente.\n");
        return;
    }
    
    char id[18], mdp[20], nom[50], prenom[50], email[50];
    int count = 0;
    
    printf(CYAN"\n+------------------------------------------------------------+\n"RESET);
    printf(CYAN"         DEMANDES DE COMPTES ADMIN EN ATTENTE             \n"RESET);
    printf(CYAN"+------------------------------------------------------------+\n"RESET);
    
    while (fscanf(f, "%17s %19s %49s %49s %49s", id, mdp, nom, prenom, email) == 5) {
        count++;
        printf(" %d. ID: %-17s                                  \n", count, id);
        printf("    Nom: %-20s Prenom: %-20s \n", nom, prenom);
        printf("    Email: %-45s \n", email);
        printf(CYAN"+------------------------------------------------------------+\n"RESET);
    }
    
    if (count == 0) {
        printf("              Aucune demande en attente                       \n");
        printf(CYAN"+------------------------------------------------------------+\n"RESET);
    }
    
    fclose(f);
}

//  VALIDER UN ADMIN 

void validerAdmin(const char *id_super_admin) {
    // Vï¿½rifier si c'est le super admin
    FILE *f = fopen("suppadmins.dat", "rb");
    if (!f) {
        printf("ERREUR : Fichier admins introuvable.\n");
        return;
    }
    
    char idF[18], mdpF[20];
    int est_super = 0;
    
    // Le premier admin du fichier est le super admin
    if (fscanf(f, "%17s %19s", idF, mdpF) == 2) {
        if (strcmp(idF, id_super_admin) == 0) {
            est_super = 1;
        }
    }
    fclose(f);
    
    if (!est_super) {
        printf("\n+------------------------------------------------+\n");
        printf("ï¿½  ERREUR : Acces refuse !                      ï¿½\n");
        printf("ï¿½  Seul le super admin peut valider            ï¿½\n");
        printf("ï¿½  les demandes d'admin.                        ï¿½\n");
        printf("+------------------------------------------------+\n");
        return;
    }
    
    // Afficher les demandes
    afficherDemandesAdmin();
    
    char id_a_valider[18];
    printf("\nID de l'admine valider : ");
    scanf("%17s", id_a_valider);
    
    // Rechercher et valider
    FILE *attente = fopen("admin_en_attente.dat", "rb");
    FILE *temp = fopen("temp.dat", "wb");
    FILE *valides = fopen("admins.dat", "ab");
    
    if (!attente || !temp || !valides) {
        printf("Erreur ouverture fichier");
        if (attente) fclose(attente);
        if (temp) fclose(temp);
        if (valides) fclose(valides);
        return;
    }
    
    char id[18], mdp[20], nom[50], prenom[50], email[50];
    int trouve = 0;
    
    while (fscanf(attente, "%17s %19s %49s %49s %49s", id, mdp, nom, prenom, email) == 5) {
        if (strcmp(id, id_a_valider) == 0) {
            trouve = 1;
            // Transfï¿½rer vers admins.txt
            fprintf(valides, "%s %s\n", id, mdp);
            
            printf("\n+------------------------------------------------+\n");
            printf(VERT"  Admin valide avec succes !                  \n"RESET);
            printf("  ID: %-37s \n", id);
            printf("  Nom: %-36s \n", nom);
            printf("+------------------------------------------------+\n");
            
            continue; // Ne pas recopier dans temp
        }
        // Recopier les autres demandes
        fprintf(temp, "%s %s %s %s %s\n", id, mdp, nom, prenom, email);
    }
    
    fclose(attente);
    fclose(temp);
    fclose(valides);
    
    remove("admin_en_attente.dat");
    rename("temp.dat", "admin_en_attente.dat");
    
    if (!trouve) {
        printf("Admin introuvable dans les demandes.\n");
    }
}

//______operation_______
//Afficher les compte en ettentes de validation "pour client"
void afficherComptesEnAttente() {
    FILE *f = fopen("comptes_en_attente.dat", "rb");
    if (!f) {
        printf("Aucun compte en attente de validation.\n");
        return;
    }

    compte c;
    char mdp_buf[20];
    char adresse[30], email[30];
    int carte_flag, valide_flag, tel;
    int count = 0;

    printf(ROSE"\n+------------------------------------------------------------------------+\n"RESET);
    printf(ROSE"                    COMPTES EN ATTENTE DE VALIDATION                    \n"RESET);
    printf(ROSE"------------------------------------------------------------------------\n"RESET);
    int result;
    while ((result = fscanf(f, "%17s %49s %49s %f %d %d %19s %d %d %29s %29s %d",
                  c.id, c.nom, c.prenom, &c.solde, &c.RIB, &c.etat, 
                  mdp_buf, &carte_flag, &valide_flag,
                  adresse, email, &tel)) == 12)  {
        count++;
        printf("    %d. ID: %-17s                                             \n", count, c.id);
        printf("    Nom: %-25s Prenom: %-25s \n", c.nom, c.prenom);
        printf("    RIB: %-10d                                                   \n", c.RIB);
        printf(ROSE"------------------------------------------------------------------------\n"RESET);
    }

    if (count == 0) {
        printf("              Aucun compte en attente de validation                     \n");
    }

    printf(ROSE"+------------------------------------------------------------------------+\n"RESET);
    fclose(f);
}

//fonction pour valider les comptes des clients
void validerCompte(const char *id) {
    FILE *f = fopen("comptes_en_attente.dat", "r");
    FILE *temp = fopen("temp.dat", "w");
    FILE *valides = fopen("comptes.dat", "a");
    
    if (!f || !temp || !valides) {
        perror(" Erreur ouverture fichier");
        if (f) fclose(f);
        if (temp) fclose(temp);
        if (valides) fclose(valides);
        return;
    }
    
    compte c;
    char mdp_buf[20];
    char adresse[30], email[30];
    int carte_flag, valide_flag, tel;
    int trouve = 0;
    
    // Lire avec les 12 champs (incluant adresse, email, tel)
    while (fscanf(f, "%17s %49s %49s %f %d %d %19s %d %d %29s %29s %d",
                  c.id, c.nom, c.prenom, &c.solde, &c.RIB, &c.etat, 
                  mdp_buf, &carte_flag, &valide_flag,
                  adresse, email, &tel) == 12) {
        
        if (strcmp(c.id, id) == 0) {
            trouve = 1;
            
            // Transfï¿½rer vers comptes validï¿½s avec valide = 1
            // ï¿½crire les 12 champs dans comptes.dat
            fprintf(valides, "%s %s %s %.2f %d %d %s %d 1 %s %s %d\n",
                    c.id, c.nom, c.prenom, c.solde, c.RIB, c.etat, 
                    mdp_buf, carte_flag,
                    adresse, email, tel);
            
            printf("\n+------------------------------------------------+\n");
            printf("|  Compte valide avec succes !                     |\n");
            printf("|  ID: %-37s                                       |\n", c.id);
            printf("|  Client: %-33s                                   |\n", c.nom);
            printf("|  Email: %-34s                                    |\n", email);
            printf("+------------------------------------------------+\n");
            
            // Ne pas recopier dans temp (suppression de la file d'attente)
            continue;
        }
        
        // Recopier les autres comptes en attente (avec les 12 champs)
        fprintf(temp, "%s %s %s %.2f %d %d %s %d %d %s %s %d\n",
                c.id, c.nom, c.prenom, c.solde, c.RIB, c.etat, 
                mdp_buf, carte_flag, valide_flag,
                adresse, email, tel);
    }
    
    fclose(f);
    fclose(temp);
    fclose(valides);
    
    if (remove("comptes_en_attente.dat") != 0) {
        perror(" Erreur remove");
    }
    if (rename("temp.dat", "comptes_en_attente.dat") != 0) {
        perror(" Erreur rename");
    }
    
    if (!trouve) {
        printf(" Aucun compte en attente trouve avec l'ID: %s\n", id);
    }
}
//____________________gestion des comptes________________________
//verifier que c'est un admin
int estAdmin(const char *id_admin) {
    FILE *f = fopen("admins.dat", "rb");
    if (!f) {
        perror("Erreur ouverture fichier admins");
        return 0;
    }
    
    char idF[18], mdpF[20];
    while (fscanf(f, "%17s %19s", idF, mdpF) == 2) {
        if (strcmp(idF, id_admin) == 0) {
            fclose(f);
            return 1; // C'est un admin
        }
    }
    
    fclose(f);
    return 0; // Pas admin
}
//supprimer compte d'un client
void supprimerCompte(const char *id_admin, const char *id_compte) {

   

    FILE *admins = fopen("admins.dat", "r");
    FILE *supp = fopen("suppadmins.dat", "r");

    if (!admins && !supp) {
        printf("ERREUR : Aucun fichier admin disponible.\n");
        return;
    }

    char idF[18], mdpF[20];
    int est_admin = 0;

    if (admins) {
        while (fscanf(admins, "%17s %19s", idF, mdpF) == 2) {
            if (strcmp(idF, id_admin) == 0) {
                est_admin = 1;
                break;
            }
        }
        fclose(admins);
    }

    if (!est_admin && supp) {
        while (fscanf(supp, "%17s %19s", idF, mdpF) == 2) {
            if (strcmp(idF, id_admin) == 0) {
                est_admin = 1;
                break;
            }
        }
        fclose(supp);
    }

    if (!est_admin) {
        printf("ERREUR : Seuls les administrateurs peuvent supprimer un compte.\n");
        return;
    }

   

    FILE *f = fopen("comptes.dat", "r");
    FILE *temp = fopen("temp.dat", "w");
    FILE *archive = fopen("archive.dat", "a");

    if (!f || !temp || !archive) {
        perror("Erreur ouverture fichier");
        if (f) fclose(f);
        if (temp) fclose(temp);
        if (archive) fclose(archive);
        return;
    }

    compte c;
    char mdp_buf[20], adresse[30], email[30];
    int carte_flag, valide_flag, tel;
    int trouve = 0;

    while (1) {

        int nb = fscanf(f, "%17s %49s %49s %f %d %d %19s %d",
                        c.id, c.nom, c.prenom,
                        &c.solde, &c.RIB, &c.etat,
                        mdp_buf, &carte_flag);

        if (nb == EOF) break;
        if (nb != 8) break;

        long pos = ftell(f);

        if (fscanf(f, "%d", &valide_flag) != 1) {
            valide_flag = -1;
            fseek(f, pos, SEEK_SET);
        }

        fscanf(f, "%29s %29s %d", adresse, email, &tel);

        if (strcmp(c.id, id_compte) == 0) {
            trouve = 1;

            fprintf(archive,
                "%s %s %s %.2f %d %d %s %d %s %s %d [SUPPRIME_PAR:%s]\n",
                c.id, c.nom, c.prenom,
                c.solde, c.RIB, c.etat,
                mdp_buf, carte_flag,
                adresse, email, tel, id_admin);

            continue; // ne pas réécrire dans temp.dat
        }

        if (valide_flag == -1) {
            fprintf(temp,
                "%s %s %s %.2f %d %d %s %d %s %s %d\n",
                c.id, c.nom, c.prenom,
                c.solde, c.RIB, c.etat,
                mdp_buf, carte_flag,
                adresse, email, tel);
        } else {
            fprintf(temp,
                "%s %s %s %.2f %d %d %s %d %d %s %s %d\n",
                c.id, c.nom, c.prenom,
                c.solde, c.RIB, c.etat,
                mdp_buf, carte_flag, valide_flag,
                adresse, email, tel);
        }
    }

    fclose(f);
    fclose(temp);
    fclose(archive);

    remove("comptes.dat");
    rename("temp.dat", "comptes.dat");

   

    if (trouve)
        printf(VERT" Compte %s supprime avec succes par l’admin %s\n"RESET, id_compte, id_admin);
    else
        printf(JAUNE" Compte %s introuvable\n"RESET, id_compte);
}


//____________audit_______________________________________________
//afficher les rendez vous 
void afficherRendezVous() {
    FILE *f = fopen("rendezvous.dat", "rb");
    if (!f) {
        printf(JAUNE"Aucun rendez-vous enregistre\n"RESET);
        return;
    }

    char ligne[256];
    printf(ROSE"----- Liste des rendez-vous -----\n"RESET);
    while (fgets(ligne, sizeof(ligne), f)) {
        printf("%s", ligne); // Affiche chaque ligne du fichier
    }
    printf(ROSE"----- Fin de la liste -----\n"RESET);
    fclose(f);
}
//affichage d'archive des comptes supprimer 
void afficherArchive() {
    FILE *f = fopen("archive.dat", "rb");

    if (!f) {
        printf("\nAucune archive disponible.\n");
        return;
    }

    compte c;
    char mdp_buf[20];
    int carte_flag;
    int count = 0;

    printf(BLEU"\n+------------------------------------------------------------------------+\n"RESET);
    printf(BLEU"                        ARCHIVE DES COMPTES SUPPRIMES                   \n"RESET);
    printf(BLEU"------------------------------------------------------------------------\n"RESET);

    while (fscanf(f, "%17s %49s %49s %f %d %d %19s %d",
                  c.id, c.nom, c.prenom, &c.solde, &c.RIB, &c.etat,
                  mdp_buf, &carte_flag) == 8) {

        count++;

        printf(" %d. ID: %-17s                                             \n", count, c.id);
        printf("    Nom: %-25s Prenom: %-25s \n", c.nom, c.prenom);
        printf("    Solde: %-10.2f   RIB: %-10d                         \n", c.solde, c.RIB);
        printf("    etat: %-10s  Carte Inter.: %-3s                     \n",
               (c.etat == 1) ? "Debloque" : "Bloque", 
               (carte_flag == 1) ? "Oui" : "Non");
        printf(BLEU"------------------------------------------------------------------------\n"RESET);
    }

    if (count == 0) {
        printf("                         Aucune donnee dans l'archive                    \n");
    }

    printf("+------------------------------------------------------------------------+\n");

    fclose(f);
}


void traiterDemandesChequier() {
    FILE *f = fopen("demandes_chequier.dat", "r");
    if (!f) {
        printf("Aucune demande de chequier en attente.\n");
        return;
    }
    
    // 1. AFFICHER LES DEMANDES
    char ligne[256];
    int count = 0;
    
    printf(ROSE"\n+------------------------------------------------------------+\n"RESET);
    printf(ROSE"|         DEMANDES DE CHÉQUIER EN ATTENTE                   |\n"RESET);
    printf(ROSE"+------------------------------------------------------------+\n"RESET);
    
    while (fgets(ligne, sizeof(ligne), f)) {
        count++;
        printf("| %d. %s", count, ligne);
    }
    
    if (count == 0) {
        printf(ROSE"|"RESET"           Aucune demande en attente                       "ROSE"|\n"RESET);
        printf(ROSE"+------------------------------------------------------------+\n"RESET);
        fclose(f);
        return;
    }
    
    printf(ROSE"+------------------------------------------------------------+\n"RESET);
    printf("Total: %d demande(s)\n\n", count);
    
    fclose(f);
    
    // 2. DEMANDER L'ID À TRAITER
    char id_a_traiter[20];
    printf("Entrez l'ID du compte a traiter (ou 0 pour annuler): ");
    scanf("%19s", id_a_traiter);
    viderBuffer();
    
    if (strcmp(id_a_traiter, "0") == 0) {
        printf("Opération annulee.\n");
        return;
    }
    
    // 3. TRAITER LA DEMANDE
    f = fopen("demandes_chequier.dat", "r");
    FILE *temp = fopen("temp_chequier.dat", "w");
    FILE *notif = fopen("notifications.dat", "a");
    
    if (!f || !temp || !notif) {
        perror("Erreur ouverture fichier");
        if (f) fclose(f);
        if (temp) fclose(temp);
        if (notif) fclose(notif);
        return;
    }
    
    int trouve = 0;
    
    while (fgets(ligne, sizeof(ligne), f)) {
        char id[20];
        sscanf(ligne, "ID:%19s", id);
        
        if (strcmp(id, id_a_traiter) == 0) {
            trouve = 1;
            
            // Déterminer une date de retrait (exemple : +3 jours)
            date d = DateActuelle();
            d.jour += 3;
            
            // Écrire notification
            fprintf(notif, "ID:%s | Notification: Votre chequier sera disponible le %02d/%02d/%04d a partir de %02d:%02d\n",
                    id, d.jour, d.mois, d.annee, d.heure, d.minute);
            
            printf(VERT"\n Demande traitee pour %s\n"RESET, id);
            printf("  Retrait prevu le %02d/%02d/%04d\n", d.jour, d.mois, d.annee);
            
            // Ne pas recopier cette demande dans temp (= suppression)
            continue;
        }
        
        // Recopier les autres demandes
        fprintf(temp, "%s", ligne);
    }
    
    fclose(f);
    fclose(temp);
    fclose(notif);
    
    if (!trouve) {
        printf("\n Aucune demande trouvee pour l'ID: %s\n", id_a_traiter);
        remove("temp_chequier.dat");
        return;
    }
    
    // Remplacer l'ancien fichier
    remove("demandes_chequier.dat");
    rename("temp_chequier.dat", "demandes_chequier.dat");
    
    printf("\nFichier mis à jour avec succes.\n");
}


void pauseConsole() {
    int c;
    // vider le buffer si du texte reste après scanf
    while ((c = getchar()) != '\n' && c != EOF);
    printf("\nAppuyez sur Entrée pour continuer...");
    getchar();  
}

//------------------------------------------------


void printMiddleScreen(const char *text) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int columns, rows;

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    int x = (columns - strlen(text)) / 2;
    int y = rows / 2;

    COORD pos = {x, y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);

    printf("%s", text);
}

int main() {

    int choix_role, choix_menu;
    char idConnecte[18] = "";
    int est_connecte = 0;
    char type_utilisateur[10] = "";
    int est_super_admin = 0;

#ifdef _WIN32
    system("chcp 65001 > nul");
#endif

    srand(time(NULL));
    clearScreen();

    // ================= LOGO =================
    printf(CYAN"+------------------------------------------------+\n"RESET);
    printf(CYAN"|     SYSTEME DE GESTION BANCAIRE AVANCE        |\n"RESET);
    printf(CYAN"+------------------------------------------------+\n"RESET);

    // ========== SUPER ADMIN ==========
    if (!superAdminExiste()) {
        printf(BLEU"\nPremiere utilisation detectee.\n"RESET);
        printf(BLEU"Creation du Super Administrateur...\n"RESET);
        initialiserSuperAdmin();
        pauseConsole();
    }

    // ================= BOUCLE PRINCIPALE =================
    while (1) {

        // ================= SELECTION ROLE =================
        while (!est_connecte) {

            const char *menu_principal[] = {
                "Je suis un Client",
                "Je suis un Administrateur",
                "Quitter"
            };

            choix_role = afficher_menu_fleches("MENU PRINCIPAL", menu_principal, 3);

            if (choix_role == 3 || choix_role == 0) {
                printf(CYAN"\nMerci d'avoir utilise notre systeme.\n"RESET);
                return 0;
            }

            // ================= CLIENT =================
            if (choix_role == 1) {

                while (!est_connecte) {

                    const char *menu_client[] = {
                        "Creer un compte client",
                        "Connexion",
                        "Retour"
                    };

                    choix_menu = afficher_menu_fleches("ESPACE CLIENT", menu_client, 3);

                    if (choix_menu == 3 || choix_menu == 0) break;

                    if (choix_menu == 1) {
                        donne *D = remplissage_donne();
                        if (D) {
                            ajouter(D);
                            free(D);
                        }
                        pauseConsole();
                    }
                    else if (choix_menu == 2) {
                        if (connexion_client(idConnecte)) {
                            est_connecte = 1;
                            strcpy(type_utilisateur, "client");
                        }
                        pauseConsole();
                    }
                }
            }

            // ================= ADMIN =================
            else if (choix_role == 2) {

                while (!est_connecte) {

                    const char *menu_admin_login[] = {
                        "Connexion Administrateur",
                        "Demande de compte Admin",
                        "Retour"
                    };

                    choix_menu = afficher_menu_fleches("ESPACE ADMINISTRATEUR", menu_admin_login, 3);

                    if (choix_menu == 3 || choix_menu == 0) break;

                    if (choix_menu == 1) {
                        if (connexion("admin", idConnecte)) {
                            est_connecte = 1;
                            strcpy(type_utilisateur, "admin");

                            FILE *f = fopen("suppadmins.dat", "rb");
                            if (f) {
                                char first[18];
                                if (fscanf(f, "%17s", first) == 1 &&
                                    strcmp(first, idConnecte) == 0) {
                                    est_super_admin = 1;
                                }
                                fclose(f);
                            }
                        }
                        pauseConsole();
                    }
                    else if (choix_menu == 2) {
                        demanderCompteAdmin();
                        pauseConsole();
                    }
                }
            }
        }

        // ================= MENU CLIENT CONNECTE =================
        if (strcmp(type_utilisateur, "client") == 0) {

            const char *menu_client_connecte[] = {
                "Consulter mon compte",
                "Historique",
                "Modifier mot de passe",
                "Versement",
                "Virement",
                "Payer facture",
                "Virement permanent",
                "Bloquer/Debloquer carte",
                "Activer carte internationale",
                "Demander un chequier",
                "Prendre rendez-vous",
                "Notifications",
                "Modifier infos personnelles",
                "Supprimer mon compte",
                "Deconnexion"
            };

            choix_menu = afficher_menu_fleches("MENU CLIENT", menu_client_connecte, 15);

            if (choix_menu == 15 || choix_menu == 0) {
                est_connecte = 0;
                strcpy(type_utilisateur, "");
                strcpy(idConnecte, "");
                continue;
            }

            switch (choix_menu) {
                case 1: afficherCompte(idConnecte); break;
                case 2: afficherHistorique(idConnecte); break;
                case 3: modifierMotDePasse(idConnecte); break;
                case 4: {
                    float montant;
                    printf("Montant : ");
                    if (scanf("%f", &montant) == 1)
                        Versement(idConnecte, montant);
                    break;
                }
                case 5: {
                    char idDest[18];
                    float montant;
                    printf("ID destinataire : ");
                    scanf("%17s", idDest);
                    printf("Montant : ");
                    scanf("%f", &montant);
                    virement(idConnecte, idDest, montant);
                    break;
                }
                case 6: {
                    char typeFact[20];
                    float montant;
                    printf("Type facture : ");
                    scanf("%19s", typeFact);
                    printf("Montant : ");
                    scanf("%f", &montant);
                    payerFacture(idConnecte, montant, typeFact);
                    break;
                }
                case 7: {
                    char idDest[18], periodicite[20];
                    float montant;
                    printf("ID destinataire : ");
                    scanf("%17s", idDest);
                    printf("Montant : ");
                    scanf("%f", &montant);
                    printf("Periodicite : ");
                    scanf("%19s", periodicite);
                    date d = DateActuelle();
                    virement_permanent(idConnecte, idDest, montant, periodicite, d);
                    break;
                }
                case 8: {
                    int ch;
                    printf("1.Bloquer  2.Debloquer : ");
                    scanf("%d", &ch);
                    changerEtatCarte(idConnecte, ch == 2);
                    break;
                }
                case 9: rendreCarteInternationale(idConnecte); break;
                case 10: demandeChequier(idConnecte); break;
                case 11: prendreRendezVous(idConnecte); break;
                case 12: afficherNotifications(idConnecte); break;
                case 13: modifier_info(idConnecte); break;
                case 14: supprimerCompte(idConnecte); break;
            }

            pauseConsole();
        }

        // ================= MENU ADMIN CONNECTE =================
        else if (strcmp(type_utilisateur, "admin") == 0) {

            const char *menu_admin[] = {
                "Consulter compte",
                "Valider comptes",
                "Supprimer compte",
                "Bloquer/Debloquer compte",
                "Versement",
                "Historique compte",
                "Traiter demandes chequier",
                est_super_admin ? "Afficher demandes admins" : "Afficher rendez-vous",
                est_super_admin ? "Valider admin" : "Afficher archive",
                "Deconnexion"
            };

            choix_menu = afficher_menu_fleches("MENU ADMINISTRATEUR", menu_admin, 10);

            if (choix_menu == 10 || choix_menu == 0) {
                est_connecte = 0;
                est_super_admin = 0;
                strcpy(type_utilisateur, "");
                strcpy(idConnecte, "");
                continue;
            }

            switch (choix_menu) {
                case 1: {
                    char idC[18];
                    printf("ID compte : ");
                    scanf("%17s", idC);
                    afficherCompte(idC);
                    break;
                }
                case 2: {
                	afficherComptesEnAttente();
                	char idC[18];
                	printf("ID a valider:");
                	scanf("%17s",idC);
                	validerCompte(idC);
                	
					break;
				}
                case 3: {
                    char idC[18];
                    printf("ID a supprimer : ");
                    scanf("%17s", idC);
                    supprimerCompte(idConnecte, idC);
                    break;
                }
                case 4: {
                    char idC[18];
                    int a;
                    printf("ID compte : ");
                    scanf("%17s", idC);
                    printf("1.Bloquer 2.Debloquer : ");
                    scanf("%d", &a);
                    changerEtatCarte(idC, a == 2);
                    break;
                }
                case 5: {
                    char idC[18];
                    float montant;
                    printf("ID compte : ");
                    scanf("%17s", idC);
                    printf("Montant : ");
                    scanf("%f", &montant);
                    Versement(idC, montant);
                    break;
                }
                case 6: {
                    char idC[18];
                    printf("ID compte : ");
                    scanf("%17s", idC);
                    afficherHistorique(idC);
                    break;
                }
                case 7: traiterDemandesChequier(); break;
                case 8: est_super_admin ? afficherDemandesAdmin() : afficherRendezVous(); break;
                case 9: est_super_admin ? validerAdmin(idConnecte) : afficherArchive(); break;
            }

            pauseConsole();
        }
    }
}

