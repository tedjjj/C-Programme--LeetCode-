#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#define loading_fac 0.75 

int Tof_loading(char *Data)
{
    FILE *Data_file;
    block_TOF **tof_file;
    Data_file = fopen(Data, "r");
    if (Data_file == NULL)
    {
        perror("DATA_file_open");
        exit(EXIT_FAILURE);
    }

    TOF_open(tof_file, "task1.txt", 'n');

    type_enrg information;
    tab_block_tof buff;

    long i = 1, num_b = 0, num_ins = 0;
    int max_per_block = (int)(loading_fac * BLOCK_CAPACITY);

    buff.nb = 0;

    while (fscanf(Data_file, "%d %d %d %d", &information.from, &information.to,
                  &information.rating, &information.timestamp) == 4)
    {
        buff.tab[buff.nb] = information;
        buff.nb++;
        num_ins++;

        if (buff.nb >= max_per_block)
        {
            buff.overflow_head = -1;
            buff.overflow_tail = -1;
            TOF_writeBlock(*tof_file, i, &buff);
            num_b++;
            i++;
            buff.nb = 0;
        }
    }

    if (buff.nb > 0){

    
            buff.overflow_head = -1;
            buff.overflow_tail = -1;
        TOF_writeBlock(*tof_file, i, &buff);
        num_b++;
    }

    fclose(Data_file);
    setHeader_tof(*tof_file, "nBlock", num_b);
    setHeader_tof(*tof_file, "nIns", num_ins);
    long h=0 ; 
    setHeader_tof(*tof_file, "nOver", h);
    TOF_close(*tof_file);

    return 0;
}
/*-------------------------------------------------------------------------------------------------------------------------*/
