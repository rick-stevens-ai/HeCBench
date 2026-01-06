#ifndef REFERENCE_H
#define REFERENCE_H

int reference(int N, int D, int top_k, const float* Xdata, const int* labelData) {
    int count = 0;
    for (int row = 0; row < N; row++) {
        const int label = labelData[row];
        const float label_pred = Xdata[row * D + label];
        int ngt = 0;
        for (int col = 0; col < D; col++) {
            const float pred = Xdata[row * D + col];
            if (pred > label_pred || (pred == label_pred && col <= label)) {
                ++ngt;
            }
        }
        if (ngt <= top_k) {
            ++count;
        }
    }
    return count;
}

#endif // REFERENCE_H
