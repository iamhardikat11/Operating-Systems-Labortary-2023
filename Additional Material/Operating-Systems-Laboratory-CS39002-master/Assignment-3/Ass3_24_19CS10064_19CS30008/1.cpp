#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iomanip>
#include <iostream>

using namespace std;

typedef struct _process_data {
    double** A;
    double** B;
    double** C;
    int veclen, i, j;

    _process_data(double** a, double** b, double** c, int v, int i, int j) {
        A = a;
        B = b;
        C = c;
        veclen = v;
        this->i = i;
        this->j = j;
    }
} ProcessData;

// Function to print a matrix of size r x c
void print_matrix(double** mat, int r, int c) {
    cout << fixed << setprecision(4);
    cout << "[";
    for (int i = 0; i < r; i++) {
        cout << "[";
        for (int j = 0; j < c; j++) {
            cout << mat[i][j];
            if (j != c - 1) {
                cout << ", ";
            }
        }
        cout << (i != r - 1 ? "],\n" : "]");
    }
    cout << "]" << endl;
    return;
}

/*
    Creates and returns a double matrix of size r x c
    filled with random double values in the shared memory
*/
double** get_matrix(int r, int c, int& shmid, int shmids[]) {
    // Create shared memory segment of size r * sizeof(double*)
    shmid = shmget(IPC_PRIVATE, r * sizeof(double*), IPC_CREAT | 0666);

    // Attach the memory segment mat to the address space of this process
    double** mat = (double**)shmat(shmid, NULL, 0);
    for (int i = 0; i < r; i++) {
        // Create shared memory segment of size c * sizeof(double)
        shmids[i] = shmget(IPC_PRIVATE, c * sizeof(double), IPC_CREAT | 0666);
        // Attach the memory segment mat[i] to the address space of this process
        mat[i] = (double*)shmat(shmids[i], NULL, 0);
        for (int j = 0; j < c; j++) {
            // Fill mat[i][j] with a random double value between 0 and 10
            mat[i][j] = ((double)rand() / RAND_MAX) * (double)10.0;
        }
    }
    return mat;
}

/* 
    Multiplies ith row of matrix A with jth column of matrix B,
    and stores the result in the cell C[i][j]
*/
void* mult(void* arg) {
    ProcessData* data = (ProcessData*)arg;
    data->C[data->i][data->j] = 0;
    for (int i = 0; i < data->veclen; i++) {
        data->C[data->i][data->j] += data->A[data->i][i] * data->B[i][data->j];
    }
    return NULL;
}

/*
    Destroy a matrix in the shared memory by taking SHMIDs' input
*/
void destroy(int shmid, int shmids[], int n, double**& mat) {
    for (int i = 0; i < n; i++) {
        // Detach the memory segment mat[i] from the address space of this process
        shmdt(mat[i]);
        // Mark the segment entified by shmids[i] to be destroyed
        shmctl(shmids[i], IPC_RMID, NULL);
    }
    // Detach the memory segment mat from the address space of this process
    shmdt(mat);
    // Mark the segment identified by shmid to be destroyed
    shmctl(shmid, IPC_RMID, NULL);
}

int main() {
    int r1, r2, c1, c2;
    cout << "Enter the number of rows of matrix A (r1): ";
    cin >> r1;
    cout << "Enter the number of columns of matrix A (c1): ";
    cin >> c1;
    cout << "Enter the number of rows of matrix B (r2): ";
    cin >> r2;
    cout << "Enter the number of columns of matrix B (c2): ";
    cin >> c2;

    // Exit if r2 is not equal to c1
    if (c1 != r2) {
        cout << "Cannot multiply matrices, c1 should be equal to r2" << endl;
        exit(1);
    }

    // To store all the shared memory ID's
    int shmid1, shmid2, shmid3;
    int shmids_A[r1], shmids_B[r2], shmids_C[r1];

    // Create matrices A, B and C in the shared memory and fill them with random double values
    double** A = get_matrix(r1, c1, shmid1, shmids_A);
    double** B = get_matrix(r2, c2, shmid2, shmids_B);
    double** C = get_matrix(r1, c2, shmid3, shmids_C);

    // Print Matrix A
    cout << endl
         << "Matrix A:" << endl;
    print_matrix(A, r1, c1);

    // Print Matrix B
    cout << endl
         << "Matrix B:" << endl;
    print_matrix(B, r2, c2);

    /* 
        Create r1 * c2 processes, each process multiplies a row
        of matrix A with a column of matrix B, and stores the result in
        the appropriate cell of preallocated matrix C
    */
    for (int i = 0; i < r1; i++) {
        for (int j = 0; j < c2; j++) {
            pid_t pid = fork();
            if (pid == 0) {
                ProcessData* data = new ProcessData(A, B, C, c1, i, j);
                mult(data);
                delete data;
                exit(0);
            } else if (pid < 0) {
                perror("fork");
                exit(1);
            }
        }
    }

    // Wait until all the child processes had finished execution.
    while (wait(NULL) > 0)
        ;

    // Print Matrix C
    cout << endl
         << "Matrix C:" << endl;
    print_matrix(C, r1, c2);
    cout << endl;

    // Destroy the shared memory segments
    destroy(shmid1, shmids_A, r1, A);
    destroy(shmid2, shmids_B, r2, B);
    destroy(shmid3, shmids_C, r1, C);

    return 0;
}