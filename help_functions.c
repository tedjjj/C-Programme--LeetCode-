void display_TOF_block_with_address(const char *filename, long block_num) {
    FILE *f;
    tab_tof_hdr header;
    
    f = fopen(filename, "rb");
    if (f == NULL) {
        printf("Error: Cannot open TOF file '%s'\n", filename);
        return;
    }
    
    // Read header
    if (fread(&header, sizeof(tab_tof_hdr), 1, f) != 1) {
        printf("Error reading TOF header\n");
        fclose(f);
        return;
    }
    
    // Check if block number is valid
    if (block_num < 1 || block_num > header.nBlock) {
        printf("Error: Block number %ld is out of range (1-%ld)\n", block_num, header.nBlock);
        fclose(f);
        return;
    }
    
    // Calculate the address of the block
    long block_address = sizeof(tab_tof_hdr) + (block_num - 1) * sizeof(tab_block_tof);
    
    // Read the block
    tab_block_tof block;
    fseek(f, block_address, SEEK_SET);
    if (fread(&block, sizeof(tab_block_tof), 1, f) != 1) {
        printf("Error reading block #%ld\n", block_num);
        fclose(f);
        return;
    }
    
    // Display block information
    printf("\n===============================================\n");
    printf("TOF BLOCK #%ld from file: %s\n", block_num, filename);
    printf("===============================================\n");
    
    printf("Block Information:\n");
    printf("  Number of records: %d\n", block.nb);
    printf("  Overflow head: %ld\n", block.overflow_head);
    printf("  Overflow tail: %ld\n", block.overflow_tail);
    printf("  Block size: %ld bytes\n", sizeof(tab_block_tof));
    printf("------------------------------------------------\n");
    
    // Display records
    if (block.nb > 0) {
        printf("Records in this block:\n");
        printf("+-----+-----+--------+---------------+\n");
        printf("| From| To  | Rating | Timestamp     |\n");
        printf("+-----+-----+--------+---------------+\n");
        
        for (int i = 0; i < block.nb; i++) {
            type_enrg rec = block.tab[i];
            printf("| %-4d| %-4d|   %-4d  | %-13d |\n", 
                   rec.from, rec.to, rec.rating, rec.timestamp);
        }
        printf("+-----+-----+--------+---------------+\n");
    } else {
        printf("This block is empty.\n");
    }
    
    fclose(f);
    printf("===============================================\n\n");
}

void display_LNOF_block_with_address(const char *filename, long block_num) {
    FILE *f;
    tab_lnof_hdr header;
    
    f = fopen(filename, "rb");
    if (f == NULL) {
        printf("Error: Cannot open LNOF file '%s'\n", filename);
        return;
    }
    
    // Read header
    if (fread(&header, sizeof(tab_lnof_hdr), 1, f) != 1) {
        printf("Error reading LNOF header\n");
        fclose(f);
        return;
    }
    
    // Check if block number is valid
    if (block_num < 1 || block_num > header.nBlocks) {
        printf("Error: Block number %ld is out of range (1-%ld)\n", block_num, header.nBlocks);
        fclose(f);
        return;
    }
    
    // Calculate the address of the block
    long block_address = sizeof(tab_lnof_hdr) + (block_num - 1) * sizeof(tab_block_lnof);
    
    // Read the block
    tab_block_lnof block;
    fseek(f, block_address, SEEK_SET);
    if (fread(&block, sizeof(tab_block_lnof), 1, f) != 1) {
        printf("Error reading block #%ld\n", block_num);
        fclose(f);
        return;
    }
    
    // Display block information
    printf("\n===============================================\n");
    printf("LNOF BLOCK #%ld from file: %s\n", block_num, filename);
    printf("===============================================\n");
   
    printf("Block Information:\n");
    printf("  Number of records: %d\n", block.nb);
    printf("  Next block in chain: %ld\n", block.next);
    printf("  Parent TOF block: %ld\n", block.parent_block);
    printf("  Block size: %ld bytes\n", sizeof(tab_block_lnof));
    printf("------------------------------------------------\n");
    
    // Display records
    if (block.nb > 0) {
        printf("Records in this block:\n");
        printf("+-----+-----+--------+---------------+\n");
        printf("| From| To  | Rating | Timestamp     |\n");
        printf("+-----+-----+--------+---------------+\n");
        
        for (int i = 0; i < block.nb; i++) {
            type_enrg rec = block.tab[i];
            printf("| %-4d| %-4d|   %-4d  | %-13d |\n", 
                   rec.from, rec.to, rec.rating, rec.timestamp);
        }
        printf("+-----+-----+--------+---------------+\n");
    } else {
        printf("This block is empty.\n");
    }
    
    fclose(f);
    printf("===============================================\n\n");
}
void display_block_with_address() {
    int file_choice;
    char filename[100];
    long block_num;
    FILE *f;
    
    printf("\n===============================================\n");
    printf("DISPLAY BLOCK WITH ADDRESS\n");
    printf("===============================================\n");
    
    // Choose which file to read from
    printf("Choose file type:\n");
    printf("1. TOF file (Task1.txt)\n");
    printf("2. LNOF file (overflow.txt)\n");
    printf("3. TOF merged file (merged.txt) ");
    scanf("%d", &file_choice);
    while (getchar() != '\n'); // Clear input buffer
    
    // Get block number
    printf("Enter block number to display: ");
    scanf("%ld", &block_num);
    while (getchar() != '\n'); // Clear input buffer
    
    if (file_choice == 1) {
        strcpy(filename, "Task1.txt");
        display_TOF_block_with_address(filename, block_num);
    }
    else if (file_choice == 2) {
        strcpy(filename, "overflow.txt");
        display_LNOF_block_with_address(filename, block_num);
    }
    else  if (file_choice == 3) {
        strcpy(filename, "merged.txt");
        display_TOF_block_with_address(filename, block_num);
    }
    else {
        printf("Invalid choice! Please enter 1 or 2 or 3.\n");
    }
}



void display_entire_LNOF(const char *filename) {
    FILE *f;
    tab_lnof_hdr header;
    
    f = fopen(filename, "rb");
    if (f == NULL) {
        printf("Error: Cannot open LNOF file '%s'\n", filename);
        return;
    }
    
    fread(&header, sizeof(tab_lnof_hdr), 1, f);
    
    printf("\n===============================================\n");
    printf("LNOF FILE: %s\n", filename);
    printf("Blocks: %ld, Records: %ld\n", header.nBlocks, header.nIns);
    printf("===============================================\n");
    
    for (long i = 1; i <= header.nBlocks; i++) {
        tab_block_lnof block;
        
        fseek(f, sizeof(tab_lnof_hdr) + (i - 1) * sizeof(tab_block_lnof), SEEK_SET);
        fread(&block, sizeof(tab_block_lnof), 1, f);
        
        printf("\n--- Block %ld ---\n", i);
        printf("Records: %d, Next: %ld, Parent TOF: %ld\n", 
               block.nb, block.next, block.parent_block);
        
        for (int j = 0; j < block.nb; j++) {
            type_enrg rec = block.tab[j];
            printf("  Record %d: from=%d to=%d rating=%d timestamp=%d\n",
                   j, rec.from, rec.to, rec.rating, rec.timestamp);
        }
        
        if (block.nb == 0) {
            printf("  [Empty block]\n");
        }
    }
    
    fclose(f);
    printf("\n===============================================\n");
}
/*-------------------------------------------------------------------------*/
// Function to display specific overflow chain for a TOF block
void display_overflow_chain_for_TOF_block(char *lnof_fname, long tof_block_num)
{
    block_LNOF *F;
    LNOF_open(&F, lnof_fname, 'e');
    
    printf("=== Overflow Chain for TOF Block %ld ===\n", tof_block_num);
    
    tab_block_lnof buff;
    int chain_found = 0;
    int total_records = 0;
    int block_count = 0;
    
    // Search for blocks belonging to this TOF block
    for (long i = 1; i <= F->h.nBlocks; i++)
    {
        LNOF_readBlock(F, i, &buff);
        
        if (buff.nb > 0 && buff.parent_block == tof_block_num) {
            if (!chain_found) {
                printf("Found overflow chain:\n");
                chain_found = 1;
            }
            
            block_count++;
            total_records += buff.nb;
            
            printf("  LNOF Block %ld:\n", i);
            printf("    Next block: %ld\n", buff.next);
            printf("    Records (%d):\n", buff.nb);
            
            for (int j = 0; j < buff.nb; j++) {
                printf("      %d. %d -> %d (rating: %d) (timestamp: %d) \n",
                       j + 1,
                       buff.tab[j].from,
                       buff.tab[j].to,
                       buff.tab[j].rating ,
                       buff.tab[j].timestamp 
                    );
            }
        }
    }
    
    if (!chain_found) {
        printf("No overflow chain found for TOF block %ld\n", tof_block_num);
    } else {
        printf("\nChain Summary:\n");
        printf("  Total overflow blocks: %d\n", block_count);
        printf("  Total overflow records: %d\n", total_records);
        printf("  Average records per block: %.2f\n", 
               (float)total_records / block_count);
    }
    
    LNOF_close(F);
}

/*-------------------------------------------------------------------------*/
void TOF_display(char *fname)
{
    block_TOF *F;
    TOF_open(&F, fname, 'e'); // Open existing file

    printf("=== TOF File Contents ===\n");
    printf("Number of Blocks: %ld\n", F->h.nBlock);
    printf("Number of Records: %ld\n\n", F->h.nIns);
    printf("Number of Overflows: %ld\n\n", F->h.nOver);

    tab_block_tof buff;

    for (long i = 1; i <= F->h.nBlock; i++)
    {
        TOF_readBlock(F, i, &buff);
        printf("Block %ld (contains %d records):\n", i, buff.nb);

        for (int j = 0; j <= buff.nb - 1 ; j++)
        {
            printf("  Record %d: from=%d, to=%d, rating=%d, timestamp=%d\n",j, 
                   
                   buff.tab[j].from,
                   buff.tab[j].to,
                   buff.tab[j].rating,
                   buff.tab[j].timestamp);
          
        }
          if (buff.overflow_head != -1)
            {
                printf("   Overflow part head : %d\n", buff.overflow_head);
                printf("   Overflow part tail : %d\n", buff.overflow_tail);
            }
        printf("\n");
    }

    TOF_close(F);
}