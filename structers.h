

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define BLOCK_CAPACITY 32
#define LNOF_BLOCK_CAPACITY 5 
#define LOADING_FACTOR 0.75
#define MAX_TEMP_RECORDS 10000

/*=============================================================================
                            TYPE DEFINITIONS
=============================================================================*/

// Enregistrement de base: une évaluation d'amitié
typedef struct type_enrg
{
    int from;        // ID de l'étudiant qui évalue
    int to;          // ID de l'étudiant évalué
    int rating;      // Note d'amitié (-1 à +3)
    int timestamp;   // Moment de l'évaluation
} type_enrg;

//enregistrement utilise dans la recherche dun record avec le timestamp le plus recent 
typedef struct SearchResult {
    long tof_block_num;     // Block number in TOF (1-based)
    bool is_overflow;       // True if record is in overflow
    long block_num;         // Block number (TOF or LNOF, 1-based)
    int record_pos;         // Position within the block (0-based)
    type_enrg record;       // The actual record found
} SearchResult;

/*-----------------------------------------------------------------------------
                        OVERFLOW FILE (LNOF)
-----------------------------------------------------------------------------*/

// Bloc de débordement
typedef struct type_block_lnof
{
    type_enrg tab[LNOF_BLOCK_CAPACITY];  // Enregistrements
    int nb;                          // Nombre d'enregistrements dans ce bloc
    long next;                       // Index du bloc suivant dans la chaîne (-1 si dernier)
    long parent_block;               // Index du bloc TOF parent (pour vérification)
} tab_block_lnof;

// En-tête du fichier LNOF
typedef struct lnof_hdr
{
    long nBlocks;    // Nombre total de blocs utilisés dans le fichier
    long nIns;       // Nombre total d'enregistrements stockés
    long freeList;   // Index du premier bloc libre (-1 si aucun)
} tab_lnof_hdr;

// Structure de contrôle du fichier LNOF
typedef struct LNOF
{
    FILE *f;           // Pointeur vers le fichier
    tab_lnof_hdr h;    // En-tête en mémoire
} block_LNOF;

/*-----------------------------------------------------------------------------
                    MAIN SEQUENTIAL FILE (TOF)
-----------------------------------------------------------------------------*/

// Bloc principal
typedef struct type_block_tof
{
    type_enrg tab[BLOCK_CAPACITY];  // Enregistrements triés
    int nb;                          // Nombre d'enregistrements
    long overflow_head;              // Index du premier bloc overflow (-1 si aucun)
    long overflow_tail;              // Index du dernier bloc overflow (-1 si aucun)
} tab_block_tof;

// En-tête du fichier TOF
typedef struct tof_hdr
{
    long nBlock;     // Nombre de blocs dans le fichier principal
    long nIns;       // Nombre total d'enregistrements dans TOF
    long nOver;      // Nombre total d'enregistrements dans les overflows
} tab_tof_hdr;

// Structure de contrôle du fichier TOF
typedef struct TOF
{
    FILE *f;         // Pointeur vers le fichier
    tab_tof_hdr h;   // En-tête en mémoire
} block_TOF;

/*=============================================================================
                        ABSTRACT MACHINE - TOF
=============================================================================*/

void TOF_open(block_TOF **F, char *fname, char mode)
{
    *F = malloc(sizeof(block_TOF));
    
    if (mode == 'E' || mode == 'e')
    {
        (*F)->f = fopen(fname, "rb+");
        if ((*F)->f == NULL) {
            perror("TOF_open");
            exit(EXIT_FAILURE);
        }
        fread(&((*F)->h), sizeof(tab_tof_hdr), 1, (*F)->f);
    }
    else
    {
        (*F)->f = fopen(fname, "wb+");
        if ((*F)->f == NULL) {
            perror("TOF_open");
            exit(EXIT_FAILURE);
        }
        (*F)->h.nBlock = 0;
        (*F)->h.nIns = 0;
        (*F)->h.nOver = 0;
        fwrite(&((*F)->h), sizeof(tab_tof_hdr), 1, (*F)->f);
    }
}

void TOF_close(block_TOF *F)
{
    fseek(F->f, 0L, SEEK_SET);
    fwrite(&F->h, sizeof(tab_tof_hdr), 1, F->f);
    fclose(F->f);
    free(F);
}

void TOF_readBlock(block_TOF *F, long i, tab_block_tof *buf)
{
    fseek(F->f, sizeof(tab_tof_hdr) + (i - 1) * sizeof(tab_block_tof), SEEK_SET);
    fread(buf, sizeof(tab_block_tof), 1, F->f);
}

void TOF_writeBlock(block_TOF *F, long i, tab_block_tof *buf)
{
    fseek(F->f, sizeof(tab_tof_hdr) + (i - 1) * sizeof(tab_block_tof), SEEK_SET);
    fwrite(buf, sizeof(tab_block_tof), 1, F->f);
}

void setHeader_tof(block_TOF *F, char *hname, long val)
{
    if (strcmp(hname, "nBlock") == 0) {
        F->h.nBlock = val;
        return;
    }
    if (strcmp(hname, "nIns") == 0) {
        F->h.nIns = val;
        return;
    }
    if (strcmp(hname, "nOver") == 0) {
        F->h.nOver = val;
        return;
    }
    fprintf(stderr, "setHeader_tof: Unknown header '%s'\n", hname);
}

long getHeader_tof(block_TOF *F, char *hname)
{
    if (strcmp(hname, "nBlock") == 0) return F->h.nBlock;
    if (strcmp(hname, "nIns") == 0) return F->h.nIns;
    if (strcmp(hname, "nOver") == 0) return F->h.nOver;
    fprintf(stderr, "getHeader_tof: Unknown header '%s'\n", hname);
    return -1;
}

/*=============================================================================
                        ABSTRACT MACHINE - LNOF
=============================================================================*/

void LNOF_open(block_LNOF **F, char *fname, char mode)
{
    *F = malloc(sizeof(block_LNOF));
    
    if (mode == 'E' || mode == 'e')
    {
        (*F)->f = fopen(fname, "rb+");
        if ((*F)->f == NULL) {
            perror("LNOF_open");
            exit(EXIT_FAILURE);
        }
        fread(&((*F)->h), sizeof(tab_lnof_hdr), 1, (*F)->f);
    }
    else
    {
        (*F)->f = fopen(fname, "wb+");
        if ((*F)->f == NULL) {
            perror("LNOF_open");
            exit(EXIT_FAILURE);
        }
        (*F)->h.nBlocks = 0;
        (*F)->h.nIns = 0;
        (*F)->h.freeList = 0;
        fwrite(&((*F)->h), sizeof(tab_lnof_hdr), 1, (*F)->f);
    }
}

void LNOF_close(block_LNOF *F)
{
    fseek(F->f, 0L, SEEK_SET);
    fwrite(&F->h, sizeof(tab_lnof_hdr), 1, F->f);
    fclose(F->f);
    free(F);
}

void LNOF_readBlock(block_LNOF *F, long i, tab_block_lnof *buf)
{
    fseek(F->f, sizeof(tab_lnof_hdr) + (i - 1) * sizeof(tab_block_lnof), SEEK_SET);
    fread(buf, sizeof(tab_block_lnof), 1, F->f);
}

void LNOF_writeBlock(block_LNOF *F, long i, tab_block_lnof *buf)
{
    fseek(F->f, sizeof(tab_lnof_hdr) + (i - 1) * sizeof(tab_block_lnof), SEEK_SET);
    fwrite(buf, sizeof(tab_block_lnof), 1, F->f);
}

void setHeader_lnof(block_LNOF *F, char *hname, long val)
{
    if (strcmp(hname, "nBlocks") == 0) {
        F->h.nBlocks = val;
        return;
    }
    if (strcmp(hname, "nIns") == 0) {
        F->h.nIns = val;
        return;
    }
    if (strcmp(hname, "freeList") == -1) {
        F->h.freeList = val;
        return;
    }
    fprintf(stderr, "setHeader_lnof: Unknown header '%s'\n", hname);
}

long getHeader_lnof(block_LNOF *F, char *hname)
{
    if (strcmp(hname, "nBlocks") == 0) return F->h.nBlocks;
    if (strcmp(hname, "nIns") == 0) return F->h.nIns;
    if (strcmp(hname, "freeList") == 0) return F->h.freeList;
    fprintf(stderr, "getHeader_lnof: Unknown header '%s'\n", hname);
    return -1;
}