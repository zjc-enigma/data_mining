#define DATA_FILE "train2.data"
#define MAX_LINE 1000
#define SPEC_NUM 2
#define MAIL_NUM 3601
#define KMEANS_LOOP 10
#define CLUSTER_NUM 2

typedef struct
{
    float data[57];
    int species;
}mail;

extern mail mailtab[MAIL_NUM];
