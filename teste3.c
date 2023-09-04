#include <mpi.h>
#include <stdio.h>

#define square_size 8

MPI_Status status;

int main(int argc, char *argv[])
{
    int numtasks, taskid;
    int i, j, k = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

    int rows, offset, remainpart, orginalrows, height, width, tamanho_matriz;
    int **a, **b, **c;

    for (int i = 0; i < argc; i++)
    {
        if (!strcmp(argv[i], "-d"))
        {
            if (strcmp(argv[i + 1], "small") == 0)
                // tamanho_matriz = 1600;
                tamanho_matriz = 8;
            else if (strcmp(argv[i + 1], "medium") == 0)
                // tamanho_matriz = 2400;
                tamanho_matriz = 4000;
            else if (strcmp(argv[i + 1], "large") == 0)
                tamanho_matriz = 4800;
            // tamanho_matriz = 3200;
            i++;
        }
    }

    height = width = tamanho_matriz;

    // on rank 0, let's build a big mat of int
    if (taskid == 0)
    {
        // a = new int *[height];
        // int *t = new int[height * width];
        // for (i = 0; i < height; ++i)
        //     a[i] = &t[i * width];

        a = (int **)malloc(height * sizeof(int *));
        int *t = (int *)malloc(height * width * sizeof(int));
        for (i = 0; i < width; ++i)
            a[i] = &t[i * width];

        for (i = 0; i < height; i++)
            for (j = 0; j < width; j++)
                a[i][j] = ++k;
        for (i = 0; i < height; i++)
        {
            for (j = 0; j < width; j++)
                printf("%d ", a[i][j]);
            printf("\n");
        }
    }

    // for everyone, lets compute numbers of rows, numbers of int and displacements for everyone. only 0 will use these arrays, but it's a practical way to get `rows`
    int nbrows[numtasks];
    int sendcounts[numtasks];
    int displs[numtasks];
    displs[0] = 0;
    for (i = 0; i < numtasks; i++)
    {
        nbrows[i] = height / numtasks;
        if (i < height % numtasks)
        {
            nbrows[i] = nbrows[i] + 1;
        }
        sendcounts[i] = nbrows[i] * width;
        if (i > 0)
        {
            displs[i] = displs[i - 1] + sendcounts[i - 1];
        }
    }
    rows = nbrows[taskid];

    // scattering operation.
    // the case of the root is particular, since the communication is not to be done...hence, the flag mpi_in_place is used.
    if (taskid == 0)
    {
        MPI_Scatterv(&a[0][0], sendcounts, displs, MPI_INT, MPI_IN_PLACE, 0, MPI_INT, 0, MPI_COMM_WORLD);
    }
    else
    {
        // allocation of memory for the piece of mat on the other nodes.
        // a = new int *[rows];
        // int *t = new int[rows * width];
        // for (i = 0; i < rows; ++i)
        //     a[i] = &t[i * width];

        a = (int **)malloc(rows * sizeof(int *));
        int *t = (int *)malloc(rows * width * sizeof(int));
        for (i = 0; i < rows; ++i)
            a[i] = &t[i * width];

        MPI_Scatterv(NULL, sendcounts, displs, MPI_INT, &a[0][0], rows * width, MPI_INT, 0, MPI_COMM_WORLD);
    }
    // printing, one proc at a time
    if (taskid > 0)
    {
        MPI_Status status;
        MPI_Recv(NULL, 0, MPI_INT, taskid - 1, 0, MPI_COMM_WORLD, &status);
    }
    printf("rank %d rows: %d width: %d\n", taskid, rows, width);

    for (i = 0; i < rows; i++)
    {
        for (j = 0; j < width; j++)
            printf("%d\t", a[i][j]);
        printf("\n");
    }
    if (taskid < numtasks - 1)
    {
        MPI_Send(NULL, 0, MPI_INT, taskid + 1, 0, MPI_COMM_WORLD);
    }

    // freeing the memory !

    free(a);

    MPI_Finalize();

    return 0;
}