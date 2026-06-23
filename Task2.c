

/*-----------------------------------------------------------------*/
int compare_Enr(type_enrg donne, type_enrg key)
/*We return 0 if we found equality (from-to ) 1 if our data(ffrom-to)> key else -1 */ 

{
    if (donne.from < key.from)
        return -1;
    else if (donne.from > key.from)
        return 1;
    
    if (donne.to < key.to)
        return -1;
    else if (donne.to > key.to)
        return 1;
    
    return 0;
}

/*---------------------------------------------------------------------------------------------*/
void recherche_block(tab_block_tof *block, type_enrg key, long *j, int *greater)
{
    /*Rechaire binaire de le block avec la function compare_enr */
    /*j est le pos ou en doit inseree*/
    /* greater est utiliser pour faire la recherche binaire entre les blocs apres */
    *j = 0;
    *greater = 1;
    
    if (block->nb == 0) {
        *greater = 0;  // frghan block
        return;
    }
    
    long min = 0;
    long max = block->nb - 1;
    long mid;
    int res;
    
    while (min <= max)
    {
        mid = (min + max) / 2;
        res = compare_Enr(block->tab[mid], key);
        
        if (res == 0) {
            *j = mid;
            *greater = 2;
            return;
        } else if (res < 0) {
            min = mid + 1;
        } else {
            max = mid - 1;
        }
    }
    
    *j = min;
    
    if (min == 0) {
        res = compare_Enr(block->tab[0], key);
        if (res > 0) {
            *greater = -1;
            return;
        }
    }
    
    if (min >= block->nb) {
        *greater = 0;
    } else {
        *greater = 1;
    }
}

/*----------------------------------------------------------------------------------*/
void recherche_tof(type_enrg key, block_TOF *tof_file, long *i, long *j)
{

    /*rechaire binaire qui retourne le num de block a insere et le position j */
    long nBlocks = tof_file->h.nBlock;
    long min = 1;
    long max = nBlocks;
    int greater = 0;
    tab_block_tof buff;
    
    while (min <= max)
    {
        long mid = (min + max) / 2;
        TOF_readBlock(tof_file, mid, &buff);
        
        recherche_block(&buff, key, j, &greater);
        
        switch (greater)
        {
            case 0:  // Key > tous ce qui est  fl block
                min = mid + 1;
                break;
            case -1: // Key < Key > tous ce qui est  fl block
                max = mid - 1;
                break;
            case 2:  // Found 
                *i = mid;
                return;
            case 1:  // Key f hed l block 
                *i = mid;
                return;
        }
    }
    
    // Key not found,  position rechaire 
    if (min > nBlocks) {
        *i = nBlocks;
        TOF_readBlock(tof_file, *i, &buff);
        *j = buff.nb;
    } else if (max < 1) {
        *i = 1;
        *j = 0;
    } else {
        *i = min;
        TOF_readBlock(tof_file, *i, &buff);
        recherche_block(&buff, key, j, &greater);
    }
}

/*-------------------------------------------------*/
void internal_shift(long j, type_enrg key, tab_block_tof *buf)
{
    /*function to insert in the tof block in case where its not full */
    if (buf->nb >= BLOCK_CAPACITY) {
        printf("Error: Block is full\n");
        return;
    }
    
    for(int i = buf->nb; i > j; i--)
    {
        buf->tab[i] = buf->tab[i-1];
    }
    
    buf->tab[j] = key;
    buf->nb++;
}


/*-------------------------------------------------*/
void insert_lnof(long tof_block, type_enrg key, block_TOF *tof_file, block_LNOF *overflow_file)
{
    /*insertion dans l overflow elle prend charge de tous les cas possible directement */


    tab_block_tof buf_tof;
    TOF_readBlock(tof_file, tof_block, &buf_tof);
    tab_block_lnof buf_overflow;
    
    if (buf_tof.overflow_head == -1)  //le block tof n a pas d overflow 
    {
        // on prend le block lnof le vide 
        long free_block = overflow_file->h.freeList;
        
        if (free_block == 0) {  // on a bessoin de cree un nouveu free block ////////////////////////////////////////////////////
            free_block = overflow_file->h.nBlocks + 1;
            
            // Initialisation 
            buf_overflow.nb = 1;
            buf_overflow.parent_block = tof_block;
            buf_overflow.tab[0] = key;
            buf_overflow.next = -1;
            
         
            LNOF_writeBlock(overflow_file, free_block, &buf_overflow);
            
            //  headers upd
            overflow_file->h.nBlocks = free_block;
            overflow_file->h.freeList = 0;  
        } else {
            // lecture de block vide 
            LNOF_readBlock(overflow_file, free_block, &buf_overflow);
            
            //  free list incremantation 
            overflow_file->h.freeList = buf_overflow.next;
            
            // Reinitialissatio  
            buf_overflow.nb = 1;
            buf_overflow.parent_block = tof_block;
            buf_overflow.tab[0] = key;
            buf_overflow.next = -1;
            
            LNOF_writeBlock(overflow_file, free_block, &buf_overflow);
        }
        
        buf_tof.overflow_head = free_block;
        buf_tof.overflow_tail = free_block;
        
        //  headers
        tof_file->h.nOver++;
        overflow_file->h.nIns++;
        
    } else {  //deja il y une overflow chaine 
        // lecture du tail 
        LNOF_readBlock(overflow_file, buf_tof.overflow_tail, &buf_overflow);
        
        if (buf_overflow.nb < LNOF_BLOCK_CAPACITY) 
        {
            buf_overflow.tab[buf_overflow.nb] = key;
            buf_overflow.nb++;
            LNOF_writeBlock(overflow_file, buf_tof.overflow_tail, &buf_overflow);
            overflow_file->h.nIns++;
        } 
        else  // besonin dun nouveau  overflow block
        {
            long free_block = overflow_file->h.freeList;
            tab_block_lnof new_overflow;
            
            if (free_block == 0) {
                /* meme logic precedante */
                free_block = overflow_file->h.nBlocks + 1;
                
                new_overflow.nb = 1;
                new_overflow.parent_block = tof_block;
                new_overflow.tab[0] = key;
                new_overflow.next = -1;
                
                LNOF_writeBlock(overflow_file, free_block, &new_overflow);
                
                overflow_file->h.nBlocks = free_block;
            } else {

                LNOF_readBlock(overflow_file, free_block, &new_overflow);
                

                overflow_file->h.freeList = new_overflow.next;

                new_overflow.nb = 1;
                new_overflow.parent_block = tof_block;
                new_overflow.tab[0] = key;
                new_overflow.next = -1;
                
                LNOF_writeBlock(overflow_file, free_block, &new_overflow);
            }
            
            // tail updating 
            buf_overflow.next = free_block;
            LNOF_writeBlock(overflow_file, buf_tof.overflow_tail, &buf_overflow);
            

            buf_tof.overflow_tail = free_block;

            tof_file->h.nOver++;
            overflow_file->h.nIns++;
        }
    }
    
    TOF_writeBlock(tof_file, tof_block, &buf_tof);
}
/*---------------------------------------------------------------------------------------------------------*/
void insert_record(type_enrg key, block_TOF *tof_file, block_LNOF *overflow_file)
{   /*function commence par chercher ou insere l info apres insere dans le tof ( espace suffisant ) ou a l overflow avec insert_lnof*/
    long block_idx, pos_idx;
    tab_block_tof tof_buf;
    

    recherche_tof(key, tof_file, &block_idx, &pos_idx);
    TOF_readBlock(tof_file, block_idx, &tof_buf);
    
    if (tof_buf.nb < BLOCK_CAPACITY)  
    {
        internal_shift(pos_idx, key, &tof_buf);
        TOF_writeBlock(tof_file, block_idx, &tof_buf);
        tof_file->h.nIns++;
    } 
    else  
    {
        insert_lnof(block_idx, key, tof_file, overflow_file);
    }
}

/*-------------------------------------------------------------------------*/
void insert_N(long n, long *start_pos, long max_records)
{
    /*function generale dde op insert_N*/
    /*insertion commence de la posiotion donne start_pos du fichier de donne general */
    /*inseree en incremantant start_pos et evitant d arrive a le max */
    block_TOF *tof_file;
    block_LNOF *overflow_file;
    FILE *data_file;
    

    TOF_open(&tof_file, "Task1.txt", 'e');
    
    // ouverture ou creation de fichier overflow selon nOver 
    if (tof_file->h.nOver == 0) {
        LNOF_open(&overflow_file, "overflow.txt", 'n');
    } else {
        LNOF_open(&overflow_file, "overflow.txt", 'e');
    }
    
 
    data_file = fopen("full_data.txt", "r");
    if (data_file == NULL) {
        perror("Failed to open data file");
        exit(EXIT_FAILURE);
    }
    
    // Skipping to the start_pos
    type_enrg temp;
    for (long i = 0; i < *start_pos; i++) {
        if (fscanf(data_file, "%d %d %d %d", 
                   &temp.from, &temp.to, &temp.rating, &temp.timestamp) != 4) {
            printf("Error reading data file at position %ld\n", i);
            break;
        }
    }
    
    // Insertion N enrgstrment 
    long inserted = 0;
    type_enrg record;
    
    while (inserted < n && (*start_pos) + inserted < max_records) {
        if (fscanf(data_file, "%d %d %d %d", 
                   &record.from, &record.to, &record.rating, &record.timestamp) == 4) {
            insert_record(record, tof_file, overflow_file);
            inserted++;
        } else {
            printf("Error reading record or end of file reached\n");
            break;
        }
    }
    
    // start_pos + + 
    *start_pos += inserted;
    
    printf("Inserted %ld records ! \n", inserted);
    
    fclose(data_file);
    LNOF_close(overflow_file);
    TOF_close(tof_file);
}
/*-------------------------------------------------------------------------*/

long get_timestamp_above_900M() {
   /*function that retunr from the pc the time-stamp to update it when updating */
    
    time_t current_time = time(NULL);  
    
    long timestamp = (long)current_time;
    
    if (timestamp <= 900000000L) {

        timestamp = 900000001L;
    }
    
    return timestamp;

}

/*---------------------------------------------*/
SearchResult compare_results(SearchResult r1, SearchResult r2) {
    // function pour la comparaison entre les enregistrement et determiner celle avec le plus grand time stamp 
    if (r1.record.timestamp > r2.record.timestamp) {
        return r1;
    } else if (r2.record.timestamp > r1.record.timestamp) {
        return r2;
    } else {
        // timestamp == 
        return r1;
    }
}


/*-------------------------------------------*/
// rechaire sequentiellle dans le bloc donnes et dans son overflow et gardee le records avec timestamp >>
void search_in_block(block_TOF *tof_file, block_LNOF *lnof_file, 
                     long block_num, int from, int to, SearchResult *best_result) {
    
    tab_block_tof tof_block;
    TOF_readBlock(tof_file, block_num, &tof_block);
    
    SearchResult result;
    
    // search in TOF blocks 
    for (int i = 0; i < tof_block.nb; i++) {
        if (tof_block.tab[i].from == from && tof_block.tab[i].to == to) {
            result.tof_block_num = block_num;
            result.is_overflow = false;
            result.block_num = block_num;
            result.record_pos = i;
            result.record = tof_block.tab[i];
            
            *best_result = compare_results(*best_result, result);
        }
    }
    
    // search in overflow 
    if (tof_block.overflow_head != -1) {
        long current_overflow = tof_block.overflow_head;
        
        while (current_overflow != -1) {
            tab_block_lnof lnof_block;
            LNOF_readBlock(lnof_file, current_overflow, &lnof_block);
            
            for (int i = 0; i < lnof_block.nb; i++) {
                if (lnof_block.tab[i].from == from && lnof_block.tab[i].to == to) {
                    result.tof_block_num = block_num;
                    result.is_overflow = true;
                    result.block_num = current_overflow;
                    result.record_pos = i;
                    result.record = lnof_block.tab[i];
                    
                    *best_result = compare_results(*best_result, result);
                }
            }
            
            current_overflow = lnof_block.next;
        }
    }
}
/*----------------------------------------------------------------------------------------------*/
// search generale 
SearchResult search_records(int from, int to) {
    // cette function applique la rechaire binare puis verifie le block sans suivant et sans precedant avec la fuction precedente 
    // retourne le records avec time stamp >>>>

    block_TOF *tof_file = NULL;
    block_LNOF *lnof_file = NULL;
    SearchResult best_result;
    
    // Initialisationn 
    best_result.record.timestamp = 0;
    best_result.tof_block_num = -1;
    best_result.is_overflow = false;
    best_result.block_num = -1;
    best_result.record_pos = -1;
    

    TOF_open(&tof_file, "Task1.txt", 'E');
    LNOF_open(&lnof_file, "overflow.txt", 'E');
    
    long nBlocks = getHeader_tof(tof_file, "nBlock");
    
    if (nBlocks == 0) {
        TOF_close(tof_file);
        LNOF_close(lnof_file);
        return best_result;
    }
   
    //rechaire binare dans le fichier 
    type_enrg search_key;
    search_key.from = from;
    search_key.to = to;
    
    long i, j;
    recherche_tof(search_key, tof_file, &i, &j);
    
    // rechaire dans le block i = target_blcok  j = target pos
    // search fl target block
    if ( i  >= 1 && i <= nBlocks) {
        search_in_block(tof_file, lnof_file, i, from, to, &best_result);
    }
    
    // search fl precedandt de target block 
    if ( i > 1 && i - 1 <= nBlocks) {
        search_in_block(tof_file, lnof_file, i - 1, from, to, &best_result);
    }
    
    // seacrh fl avant de target block 
    if (i < nBlocks && i + 1 <= nBlocks) {
        search_in_block(tof_file, lnof_file, i + 1, from, to, &best_result);
    }
    
    TOF_close(tof_file);
    LNOF_close(lnof_file);
    
    return best_result;
}

/*------------------------------------------------------------------*/
void update_rec(int from, int to, int newrate) {
    /* function qui update le rating entre deux amies (rating le plus recent ) */

    SearchResult result = search_records(from, to); // rechercehe de records avec le timestamp>>
    
    if (result.record_pos == -1) { // le cas ou le records avec (from - to) n exsite pas 
        printf("No record found with the given keys.\n");
        return;
    }
    
    
    block_TOF *tof_file ;
    block_LNOF *lnof_file ;
    type_enrg key ; 
    
    TOF_open(&tof_file, "Task1.txt", 'E');
    LNOF_open(&lnof_file, "overflow.txt", 'E');
    
    if (result.is_overflow) {
        // le cas ou le record est dans l overflow fichier 
        tab_block_lnof buf_lnof;
        LNOF_readBlock(lnof_file, result.block_num, &buf_lnof);
        
        // rating mise a jour 
        key.from = buf_lnof.tab[result.record_pos].from ;
        key.rating = newrate ; 
        key.to = buf_lnof.tab[result.record_pos].to ; 
        key.timestamp = get_timestamp_above_900M() ; 

        insert_lnof(buf_lnof.parent_block,key,tof_file,lnof_file);
        
    } else {
        // record fl tof insertion dasn  l nof 
        tab_block_tof buf_tof;
        TOF_readBlock(tof_file, result.block_num, &buf_tof);


        key.from = buf_tof.tab[result.record_pos].from ;
        key.rating = newrate ; 
        key.to = buf_tof.tab[result.record_pos].to ; 
        key.timestamp = get_timestamp_above_900M() ; 
    


        
        buf_tof.tab[result.record_pos].rating = newrate;
        buf_tof.tab[result.record_pos].timestamp = get_timestamp_above_900M() ;

        insert_lnof(result.block_num,key,tof_file,lnof_file ) ; 
        
        
    
        

    }

printf("---------updated------");
    TOF_close(tof_file);
    LNOF_close(lnof_file);
}
/*-----------------------------------------------------------------------*/

void search_rat(int from, int to) {
    /* meme logic de update sauf qon vas affichier */
    SearchResult result = search_records(from, to);
    
    if (result.record_pos == -1) {
        printf("No record found with the given keys.\n");
        return;
    }
    
    

    block_TOF *tof_file ;
    block_LNOF *lnof_file ;
    
    TOF_open(&tof_file, "Task1.txt", 'E');
    LNOF_open(&lnof_file, "overflow.txt", 'E');
    
    if (result.is_overflow) {
        // record fl lnof
        tab_block_lnof buf_lnof;
        LNOF_readBlock(lnof_file, result.block_num, &buf_lnof);
        // displayy 
        printf("Rating Betwen student1 (from %d) and student2 (to %d ) is %d" , from, to , buf_lnof.tab[result.record_pos].rating) ; 
        
    } else {
        // record dans tof 
        tab_block_tof buf_tof;
        TOF_readBlock(tof_file, result.block_num, &buf_tof);
        //dispaly 
        printf("Rating Betwen student1 (from %d) and student2 (to %d ) is %d" , from, to , buf_tof.tab[result.record_pos].rating) ; 
    }

    TOF_close(tof_file);
    LNOF_close(lnof_file);
}

/*------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------*/
int compare_Enr_from(type_enrg donne, int from_key) {
    /*comparaison entre les records selon une seul key form */
    if (donne.from < from_key)
        return -1;
    else if (donne.from > from_key)
        return 1;
    else
        return 0;
}

/*-------------------------------------------------------------------------*/
void recherche_block_from(tab_block_tof *block, int from_key, long *first_pos, long *last_pos) {
    /*rechaire binaire dans le bloc avec form berk */
    /*cette rechaire retourne  first_pos = position de la premier occurance  et last_pos = de la dernier pour apres faire une requttle intervalle */
    *first_pos = -1;
    *last_pos = -1;
    
    if (block->nb == 0) {
        return;
    }
    
    long left = 0;
    long right = block->nb - 1;
    long mid;
    int cmp;
    
    while (left <= right) {
        mid = (left + right) / 2;
        cmp = compare_Enr_from(block->tab[mid], from_key);
        
        if (cmp == 0) {
            *first_pos = mid;
            right = mid - 1;
        } else if (cmp < 0) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    if (*first_pos == -1) {
        return;
    }
    
    left = *first_pos;
    right = block->nb - 1;
    
    while (left <= right) {
        mid = (left + right) / 2;
        cmp = compare_Enr_from(block->tab[mid], from_key);
        
        if (cmp == 0) {
            *last_pos = mid;
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
}

/*-------------------------------------------------------------------------*/
void recherche_tof_from(int from_key, block_TOF *tof_file, long *first_block, long *last_block) {
    /*RECHAIRE BINAIRE ENTRE LES BLOCS TOF AVEC L KEY FROM */
    /*RETOURNE UN INTERVALLE ENTRE FIRST_B ET LAST_B */
    *first_block = -1;
    *last_block = -1;
      /* DETERMINATION DE FIRST_bLOCK -----*/
    long nBlocks = tof_file->h.nBlock;
    if (nBlocks == 0) {
        return;
    }
    
    long left = 1;
    long right = nBlocks;
    long mid;
    tab_block_tof block;
    
    while (left <= right) {
        mid = (left + right) / 2;
        TOF_readBlock(tof_file, mid, &block);
        
        if (block.nb == 0) {
            if (mid < (nBlocks + 1) / 2) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
            continue;
        }
        
        int cmp_first = compare_Enr_from(block.tab[0], from_key);
        int cmp_last = compare_Enr_from(block.tab[block.nb - 1], from_key);
        
        if (cmp_first <= 0 && cmp_last >= 0) {
            *first_block = mid;
            right = mid - 1;
        } else if (cmp_last < 0) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    if (*first_block == -1) {
        return;
    }
    /* DETERMINATION DE LAST_bLOCK -----*/
    
    left = *first_block;
    right = nBlocks;
    
    while (left <= right) {
        mid = (left + right) / 2;
        TOF_readBlock(tof_file, mid, &block);
        
        if (block.nb == 0) {
            if (mid < (*first_block + nBlocks) / 2) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
            continue;
        }
        
        int cmp_first = compare_Enr_from(block.tab[0], from_key);
        int cmp_last = compare_Enr_from(block.tab[block.nb - 1], from_key);
        
        if (cmp_first <= 0 && cmp_last >= 0) {
            *last_block = mid;
            left = mid + 1;
        } else if (cmp_first > 0) {
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }
}

/*-----------------------------------------------------------------------*/
void display_all_friends(int from_student) {
    /*
    CETTE FUNCTION PROCEDE COMME SUIT : 
    1/- RECHAIRE BINAIRE AVEC recherche_tof_from 
    2/- PRENDRE CHARGES DES EDGES (VERIFIER LES BLOCS AVANT ET APREST )
    3/- PARCOURS SEQUNETIELLE DE L INTERVALLE DONNES  
    3/- SAUVGARGEMENT DES AMIES DANS UN TABLEU D AMIES 
    4/- PARCOURS DE LA TABLE EFFECTUE LA FUCNTION search_records POUR GARDER LE RATING LE PLUS RECENT ET LE DISPLAYER 
    */
    
    block_TOF *tof_file = NULL;
    block_LNOF *lnof_file = NULL;
    
    TOF_open(&tof_file, "Task1.txt", 'E');
    LNOF_open(&lnof_file, "overflow.txt", 'E');
    
    long nBlocks = getHeader_tof(tof_file, "nBlock");

    // VERIFIER LE FICHIER TOF 
    if (nBlocks == 0) {
        printf("No records found in the TOF fie .\n");
        TOF_close(tof_file);
        LNOF_close(lnof_file);
        return;
    }
    
    printf("\n=== Friends list for student %d (with most recent ratings) ===\n", from_student);
    
    // etape une 1 : binary search 
    long first_block, last_block;
    recherche_tof_from(from_student, tof_file, &first_block, &last_block);
    
    // tables des amies 
    #define MAX_FRIENDS 1000
    int friends_table[MAX_FRIENDS];
    int friends_count = 0;
    
    // etape 02 preparation 
    long start_block, end_block;
    
    if (first_block == -1 || last_block == -1) {
      // no intervall found 
      // to avoid missing values we will parcours all file in that case 
        start_block = 1;
        end_block = nBlocks;
        printf("Binary search found no blocks, searching ALL %ld blocks...\n", nBlocks);
    } else {
        // we found an intervall 
        //we expand the intervall to avoid missing some values in the bonderies 
        start_block = (first_block > 1) ? first_block - 1 : 1;
        end_block = (last_block < nBlocks) ? last_block + 1 : nBlocks;
        printf("Searching blocks %ld to %ld (expanded range)...\n", start_block, end_block);
    }
    
    // etape 02 
    for (long block_num = start_block; block_num <= end_block; block_num++) {
        if (block_num < 1 || block_num > nBlocks) {
            continue;
        }
        
        tab_block_tof tof_block;
        TOF_readBlock(tof_file, block_num, &tof_block);
        
        // parcours sequentielle 
        for (int i = 0; i < tof_block.nb; i++) {
            if (tof_block.tab[i].from == from_student) {
                int to_student = tof_block.tab[i].to;
                
                // verifie si l amies existe dans notre tableu ou pas 
                int found = 0;
                for (int j = 0; j < friends_count; j++) {
                    if (friends_table[j] == to_student) {
                        found = 1;
                        break;
                    }
                }
                
                // ajoute l amie a la table 
                if (!found) {
                    if (friends_count < MAX_FRIENDS) {
                        friends_table[friends_count] = to_student;
                        friends_count++;
                    } 
                }
            }
        }
        
        //overflow handling 
        if (tof_block.overflow_head != -1) {
            long current_overflow = tof_block.overflow_head;
            
            while (current_overflow != -1) {
                tab_block_lnof lnof_block;
                LNOF_readBlock(lnof_file, current_overflow, &lnof_block);
                
                //parcours sequentielle 
                for (int i = 0; i < lnof_block.nb; i++) {
                    if (lnof_block.tab[i].from == from_student) {
                        int to_student = lnof_block.tab[i].to;
                        
                        // meme verification precedante 
                        int found = 0;
                        for (int j = 0; j < friends_count; j++) {
                            if (friends_table[j] == to_student) {
                                found = 1;
                                break;
                            }
                        }
                        
                        if (!found) {
                            if (friends_count < MAX_FRIENDS) {
                                friends_table[friends_count] = to_student;
                                friends_count++;
                            } 
                        }
                    }
                }
                
                current_overflow = lnof_block.next;
            }
        }
    }
    
    // etape 4 affichage 
    if (friends_count == 0) {
        printf("No friends found for student %d.\n", from_student);
    } else {
        printf("\nFriend ID\tMost Recent Rating\n");
        printf("---------\t------------------\n");
        
        int displayed_count = 0;
        for (int i = 0; i < friends_count; i++) {
            int to_student = friends_table[i];
            
            SearchResult result = search_records(from_student, to_student);
            
          
                printf("%9d\t%17d\n", to_student, result.record.rating);
                displayed_count++;
            
        }
        
        printf("\nTotal unique friends: %d\n", displayed_count);
       
    }
    
    // Close files
    TOF_close(tof_file);
    LNOF_close(lnof_file);
}

