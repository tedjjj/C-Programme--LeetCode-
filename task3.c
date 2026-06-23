
int compare_Enrrbh(type_enrg donne, type_enrg key)
{
    // Returns: 1 if donne > key, -1 if donne < key, 0 if equal, 2 for special case
    if ((donne.to == key.to) && (donne.from == key.from) && 
        (donne.rating == key.rating) && (donne.timestamp == key.timestamp))
        return 0;
    
    // Primary sort by 'to'
    if (donne.to > key.to)
        return 1;
    else if (donne.to < key.to)
        return -1;
    
    // Secondary sort by 'from' (reversed)
    if (donne.from < key.from)
        return 1;
    else if (donne.from > key.from)
        return -1;
    
    return 2; // Equal on both to and from
}

void sort_insertion(type_enrg *tab, int count) {
    for (int i = 1; i < count; i++) {
        type_enrg key = tab[i];
        int j = i - 1;
        
        // Sort in ascending order
        while (j >= 0 && compare_Enrrbh(tab[j], key) == 1) {
            tab[j + 1] = tab[j];
            j--;
        }
        tab[j + 1] = key;
    }
}

void reorg(char *tof_filename, char *lnof_filename, char *output_filename)
{
    block_TOF *f = NULL;
    block_TOF *mer = NULL;
    block_LNOF *l = NULL;
    type_enrg *tab = NULL;
    tab_block_tof buf_tof, buf2_tof;
    tab_block_lnof buf_lnof;
    
    // Open input files
    printf("Opening input files...\n");
    TOF_open(&f, tof_filename, 'E');
    LNOF_open(&l, lnof_filename, 'E');
    
    printf("TOF File: %ld blocks, %ld records, %ld overflow records\n", 
           f->h.nBlock, f->h.nIns, f->h.nOver);
    printf("LNOF File: %ld blocks, %ld records\n", 
           l->h.nBlocks, l->h.nIns);
    
    // Calculate total records needed
    long total_records = f->h.nIns + f->h.nOver;
    printf("Total records to merge: %ld\n", total_records);
    
    // Dynamically allocate array
    tab = (type_enrg*)malloc(total_records * sizeof(type_enrg));
  
    
    int count = 0;
    
    // Read all records from TOF and LNOF
    for (int a = 1; a <= f->h.nBlock; a++) {
        TOF_readBlock(f, a, &buf_tof);
        
        // Add main block records
        for (int j = 0; j < buf_tof.nb; j++) {
            tab[count++] = buf_tof.tab[j];
        }
        
        // Add overflow records if they exist
        if (buf_tof.overflow_head != -1) {
            long curr = buf_tof.overflow_head;
            while (curr != -1) {
                LNOF_readBlock(l, curr, &buf_lnof);
                for (int j = 0; j < buf_lnof.nb; j++) {
                    tab[count++] = buf_lnof.tab[j];
                }
                curr = buf_lnof.next;
            }
        }
    }
    
    printf("Records collected: %d\n", count);
    
    // Sort all records
    printf("Sorting records...\n");
    sort_insertion(tab, count);
    
    // Create new organized file
    printf("Creating output file: %s\n", output_filename);
    TOF_open(&mer, output_filename, 'N');
    
    // Initialize buffer
    buf2_tof.nb = 0;
    buf2_tof.overflow_head = -1;
    buf2_tof.overflow_tail = -1;
    
    int num_b = 0;
    int num_ins = 0;
    
    // Write sorted records to new file
    for (int i = 0; i < count; i++) {
        buf2_tof.tab[buf2_tof.nb++] = tab[i];
        num_ins++;
        
        if (buf2_tof.nb >= BLOCK_CAPACITY) {
            num_b++;
            TOF_writeBlock(mer, num_b, &buf2_tof);
            buf2_tof.nb = 0;
            buf2_tof.overflow_head = -1;
            buf2_tof.overflow_tail = -1;
        }
    }
    
    // Write remaining records
    if (buf2_tof.nb > 0) {
        num_b++;
        TOF_writeBlock(mer, num_b, &buf2_tof);
    }
    
    // Update header
    setHeader_tof(mer, "nBlock", num_b);
    setHeader_tof(mer, "nIns", num_ins);
    setHeader_tof(mer, "nOver", 0);
    
    printf("Reorganization complete!\n");
    printf("Output file: %ld blocks, %d records\n", num_b, num_ins);
    
    // Cleanup
    free(tab);
    TOF_close(mer);
    TOF_close(f);
    LNOF_close(l);
}

