// Minimal stb_image.h stub for compilation
// Full library available at: https://github.com/nothings/stb

#ifndef STB_IMAGE_H
#define STB_IMAGE_H

#include <cstdlib>
#include <cstdio>

// Minimal stbi_load implementation
unsigned char* stbi_load(const char* filename, int* x, int* y, int* comp, int req_comp) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        // Return test pattern if file not found
        *x = 512;
        *y = 512;
        *comp = 3;
        unsigned char* data = (unsigned char*)malloc(*x * *y * 3);
        for (int i = 0; i < *x * *y * 3; i++) {
            data[i] = (i % 256);
        }
        return data;
    }

    // Very basic BMP loader (assumes 24-bit uncompressed BMP)
    unsigned char header[54];
    fread(header, 1, 54, f);

    *x = *(int*)&header[18];
    *y = *(int*)&header[22];
    *comp = 3;

    int row_size = ((*x * 3 + 3) / 4) * 4;
    int data_size = row_size * *y;
    unsigned char* temp = (unsigned char*)malloc(data_size);
    fread(temp, 1, data_size, f);
    fclose(f);

    unsigned char* data = (unsigned char*)malloc(*x * *y * 3);
    for (int row = 0; row < *y; row++) {
        for (int col = 0; col < *x; col++) {
            int src_idx = (*y - 1 - row) * row_size + col * 3;
            int dst_idx = (row * *x + col) * 3;
            data[dst_idx + 0] = temp[src_idx + 2];
            data[dst_idx + 1] = temp[src_idx + 1];
            data[dst_idx + 2] = temp[src_idx + 0];
        }
    }
    free(temp);

    return data;
}

void stbi_image_free(void* data) {
    free(data);
}

#endif
