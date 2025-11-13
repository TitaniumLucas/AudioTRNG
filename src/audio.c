#include "audio.h"
#include <stdio.h>
#include <stdlib.h>

int at_read_audio() {
    FILE *file_pointer = NULL;
    long file_size = 0;
    unsigned char *file_buffer = NULL;
    size_t bytes_read;
    char * file_path = "../../audio/boarding-accouncement-1.wav";
    file_pointer = fopen(file_path, "rb"); //open file in read binary mode
    if (file_pointer == NULL) {
        printf("Error: could not open file");
        return 1;
    }
    // get file size
    fseek(file_pointer, 0, SEEK_END);
    file_size = ftell(file_pointer);
    fseek(file_pointer, 0, SEEK_SET);

    if (file_size <= 0){
        printf("Error: file is empty");
        fclose(file_pointer);
        return 1;
    }
    // allocate memory for the entire file
    file_buffer = (unsigned char *)malloc(file_size);

    //  check if allocation succesfull
    if (file_buffer == NULL) {
        printf("Error: could not allocate memory");
        fclose(file_pointer);
        return 1;
    }
    bytes_read = fread(file_buffer, 1, file_size, file_pointer);
    //check if read succesful
    if (bytes_read != (size_t)file_size) {
        printf("Error: read unsuccesful. Read %zu of %ld bytes.\n", bytes_read, file_size);
        free(file_buffer);
        fclose(file_pointer);
        return 1;
    }
    printf("Successfully read %ld bytes from file\n", file_size);

    printf("First 16 bytes:\n");
    for (int i = 0; i < 16 && i < file_size; i++) {
        printf("%02X ", file_buffer[i]);
    }
    printf("\n");

    free(file_buffer);
    fclose(file_pointer);

    return 0;
}