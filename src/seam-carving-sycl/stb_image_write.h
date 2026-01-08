// Minimal stb_image_write.h stub for compilation
// Full library available at: https://github.com/nothings/stb

#ifndef STB_IMAGE_WRITE_H
#define STB_IMAGE_WRITE_H

#include <cstdio>
#include <cstring>

// Minimal BMP writer
int stbi_write_bmp(const char* filename, int w, int h, int comp, const void* data) {
    FILE* f = fopen(filename, "wb");
    if (!f) return 0;

    // BMP file header
    unsigned char header[54] = {0};
    header[0] = 'B'; header[1] = 'M';  // Signature

    int row_size = ((w * 3 + 3) / 4) * 4;
    int data_size = row_size * h;
    int file_size = 54 + data_size;

    *(int*)&header[2] = file_size;
    *(int*)&header[10] = 54;  // Data offset
    *(int*)&header[14] = 40;  // Header size
    *(int*)&header[18] = w;
    *(int*)&header[22] = h;
    *(short*)&header[26] = 1;   // Planes
    *(short*)&header[28] = 24;  // Bits per pixel
    *(int*)&header[34] = data_size;

    fwrite(header, 1, 54, f);

    // Write pixel data (convert RGB to BGR and flip)
    const unsigned char* src = (const unsigned char*)data;
    unsigned char* row_data = new unsigned char[row_size];

    for (int y = h - 1; y >= 0; y--) {
        memset(row_data, 0, row_size);
        for (int x = 0; x < w; x++) {
            int src_idx = (y * w + x) * 3;
            int dst_idx = x * 3;
            row_data[dst_idx + 0] = src[src_idx + 2];  // B
            row_data[dst_idx + 1] = src[src_idx + 1];  // G
            row_data[dst_idx + 2] = src[src_idx + 0];  // R
        }
        fwrite(row_data, 1, row_size, f);
    }

    delete[] row_data;
    fclose(f);
    return 1;
}

#endif
